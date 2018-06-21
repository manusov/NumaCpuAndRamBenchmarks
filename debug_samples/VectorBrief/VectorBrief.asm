; TODO:
; 1) + Patterns for all listed instructions.
; 2) + Check CPU feature before run pattern, extend VECLK entry by VECPU entry.
; 3) + Add check OS features.
; 4) + Change cycle exit criteria by VECPU end-bit.
; 5) + Support = f(context), yet CPU only.
; 6) - Prepare to integration to NCRB: use caller, use CPUID/TSC subroutines.
; 7) - Integrate to NCRB: "Vector brief" button.

; 1) Don't store spaces and duplicated CR, LF. Use same method as NCRB.
; 2) Redesign CPUID checks, same as NCRB.
; 3) Redesign pattern caller same as NCRB.



;------------------------------------------------------------------------------;
; Processor and OS AVX features detector with instruction timings measurement  ;
;------------------------------------------------------------------------------;

format PE64 GUI
entry start
include 'win64a.inc'

;---------- Code section ------------------------------------------------------;

section '.text' code readable executable
start:
;--- Prepare context ---
sub rsp,8*5  ; Reserve stack for API use and make stack dqword aligned
cld

;--- Detect minimum ---
; Check EFLAGS.21, if control at this point, means CPUID supported,
; but can be locked by virtualization or other non-standard contexts
;--- Check for ID bit writeable for "1" ---
lea rdi,[TEMP_BUFFER]
call CheckCpuId
jc LerrorCPU              ; Go if cannot set EFLAGS.21=0

;--- CPUID support valdated, use CPUID ---
xor eax,eax         ; Function = 00000000h, check standard CPUID functions
cpuid
cmp eax,1
jb LerrorCPU        ; Go error if CPUID function 00000001h not supported 
mov eax,80000000h   ; Function = 80000000h, check for extended CPUID functions
cpuid
cmp eax,80000004h
jb LerrorCPU        ; Go error if CPUID function 80000004h not supported
;--- Check x87 FPU support, it used for timings measurement ---
mov eax,1           ; Function = 00000001h, get CPU standard features list
cpuid
test dl,0001b
jz LerrorFPU        ; Go error if FPU feature bit = 0 (means x87 FPU absent)
;--- Read CPU name string ---
lea rdi,[NameBuffer]
call GetCpuName
;--- Measure CPU clock ---
lea rdi,[TEMP_BUFFER]
call MeasureCpuClk
;--- Prepare frequency string for visual ---
finit
fild qword [rdi-16]
fld [DividendNs]
fdiv st0,st1
fstp [TscNs]       ; TSC period at nanoseconds 
fdiv [DivisorMHz]
fstp qword [rdi]
mov rax,[rdi]
mov bx,0200h
lea rdi,[stsc]
call DoublePrint 

;--- Pre-clear R13, R14, R15 used as features bitmaps ---
xor r13d,r13d
xor r14d,r14d
xor r15d,r15d
;--- Check support AVX (Sandy Bridge) ---
; AVX feature = CPUID#1 bit ECX.28
;---
mov eax,1
cpuid           ; Get ECX:EDX = CPU Standard Features List
mov r14d,ecx    ; R14[31-0] = Storage for CPUID#1 ECX
mov r13d,edx    ; R13[31-0] = Storage for CPUID#1 EDX 
;--- Check support OS context management features ---
; XMM128     context = XCR0.1
; YMM256     context = XCR0.2
; ZMM[0-15]  context = XCR0.6
; ZMM[16-31] context = XCR0.7
; K[0-7]     context = XCR0.5
;---
mov eax,008000000h
and ecx,eax
cmp ecx,eax
jne L1          ; Go skip if OSXSAVE(ECX.27) not supported
xor ecx,ecx
xgetbv
shl rax,32
or r14,rax      ; R14[63-32] = Storage for XCR0[31-00]
L1:
;--- Check support AVX2 (Haswell) ---
; AVX2 feature = CPUID#7 Subfunction#0 EBX.5 
;--- Check support AVX3 / AVX512F=Foundation (Skylake Xeon) ---
; AVX512F    feature = CPUID#7 Subfunction#0 EBX.16
; AVX512CD   feature = CPUID#7 Subfunction#0 EBX.28
; AVX512PF   feature = CPUID#7 Subfunction#0 EBX.26
; AVX512ER   feature = CPUID#7 Subfunction#0 EBX.27
; AVX512VL   feature = CPUID#7 Subfunction#0 EBX.31
; AVX512BW   feature = CPUID#7 Subfunction#0 EBX.30
; AVX512DQ   feature = CPUID#7 Subfunction#0 EBX.17
; AVX512IFMA feature = CPUID#7 Subfunction#0 EBX.21
; AVX512VBM  feature = CPUID#7 Subfunction#0 ECX.1
;---
xor eax,eax
cpuid
cmp eax,7
jb L2
mov eax,7
xor ecx,ecx
cpuid
mov r15d,ebx    ; R15[31-00] = Storage for CPUID#7 EBX
shl rcx,32
or r15,rcx      ; R15[63-32] = Storage for CPUID#7 ECX
L2:
;--- Copy CPU name string, skip left spaces ---
lea rsi,[NameBuffer]
lea rdi,[WinMessage]
mov ecx,48
L10:            ; Skip left spaces in the CPU name string
lodsb
cmp al,' '
loope L10
jrcxz L11
dec rsi
;-BUG- rep movsb  ; Copy CPU name string without left spaces
L17:              ; Copy CPU name string without left spaces and zeroes
lodsb
cmp al,0
jne L16
mov al,' '
L16:
stosb
loop L17
L11:
;--- Built text block ---
lea rbx,[VisualEntriesCPU]
L12:
mov rdi,[rbx]         ; Get destination string from Visual Entry (VE)
test rdi,rdi
jz L40
mov cl,[rbx+8]        ; Get control byte from Visual Entry (VE) 
mov rax,r13
test cl,cl
js L14
mov rax,r14
test cl,40h           ; Control byte, bit[6] = Select 0=R14, 1=R15
jz L14
mov rax,r15
L14:
mov edx,ecx
and edx,3Fh           ; Control byte, bits[5-0] = tested bit number
bt rax,rdx            ; Test selected feature bit, set CF=1 if bit=1
lea rsi,[Sup]         ; RSI = Pointer to string "supported"
jc L13                ; If "1" means supported
lea rsi,[Nsup]        ; RSI = Pointer to string "not supported"
L13:                  ; Cycle for copy string
lodsb
cmp al,0
je L15
stosb
jmp L13
L15:
add rbx,9
jmp L12               ; Cycle if this entry not last
L40:

;--- Clear FPU state, clear temporary buffer for timings measurement ---
finit
lea rdi,[TEMP_BUFFER]
mov ecx,TEMP_BUFFER_SIZE / 8
xor eax,eax
rep stosq

;--- Start timings measurement with build text block continue ---
lea r12,[VisualEntriesCLK]
L20:                  ; Point for cycle
mov rdi,[r12+00]      ; RDI = Pointer for string build
test rdi,rdi
jz L21                ; Go if end of sequence

mov cx,[r12+32]       ; Get control byte from Visual Entry (VE) 
lea rsi,[NsupCPU]     ; RSI = Pointer to string "not supported by CPU"
mov rax,r13
test cl,cl
js L54
mov rax,r14
test cl,40h           ; Control byte, bit[6] = Select 0=R14, 1=R15
jz L54
mov rax,r15
L54:
mov edx,ecx
and edx,3Fh           ; Control byte, bits[5-0] = tested bit number
bt rax,rdx            ; Test selected feature bit, set CF=1 if bit=1
jnc L53               ; If "1" means supported

lea rsi,[NsupOS]      ; RSI = Pointer to string "not supported by OS"
mov rax,r13
test ch,ch
js L56
mov rax,r14
test ch,40h            ; Control byte, bit[6] = Select 0=R14, 1=R15
jz L56
mov rax,r15
L56:
movzx edx,ch
and edx,3Fh            ; Control byte, bits[5-0] = tested bit number
bt rax,rdx             ; Test selected feature bit, set CF=1 if bit=1
jc L55                 ; If "1" means supported

L53:                   ; Cycle for copy string
lodsb
cmp al,0
je L22
stosb
jmp L53
L55:

push r14

push rdi r12 r13
xor eax,eax
cpuid
mov rbx,[r12+08]          ; RBX = Address for subroutine call
mov rcx,[r12+16]          ; RCX = Array length, units = instructions
mov rbp,[r12+24]          ; RBP = Measurement iterations
lea r8,[VectorLoad]       ; R8  = Pointer to vector load data
lea rsi,[TEMP_BUFFER]
lea rdi,[rsi+TEMP_BUFFER_SIZE/2]
rdtsc
shl rdx,32
lea r14,[rax+rdx]
call rbx
rdtsc
shl rdx,32
add rax,rdx
sub rax,r14
xchg r14,rax
xor eax,eax
cpuid
pop r13 r12 rdi

mov rax,[r12+16]       ; Instructions count
mul qword [r12+24]     ; Measurement count
push rax r14
fild qword [rsp+08]
fild qword [rsp]       ; st0 = clocks count , st1 = instructions count
fdiv st0,st1
fst qword [rsp]
fmul [TscNs]
fstp qword [rsp+08]
ffree st0
fincstp

pop rax
mov bx,0300h
call DoublePrint
mov ax,' ('
stosw
pop rax
call DoublePrint
mov eax,' ns)'
stosd

pop r14

L22:                   ; Point for skip if instruction not supported
add r12,34
jmp L20
L21:                   ; Point for exit cycle

;--- Visualize text strings in the window ---
lea rdx,[WinMessage]  ; RDX = Parm #2 = Message
lea r8,[WinCaption]   ; R8  = Parm #3 = Caption (upper message)
xor r9d,r9d           ; R9  = Parm #4 = Message flags
Lmsg:
xor ecx,ecx           ; RCX = Parm #1 = Parent window
call [MessageBoxA]
;--- Exit program ---
xor ecx,ecx           ; ECX = Parm #1
call [ExitProcess]
;--- Errors handling: if too old CPU or absent FPU ---

LerrorCPU:            ; This entry point for too old CPU
lea rdx,[OldCpuError]
LerrorEntry:
xor r8d,r8d           ; 0 means error message window
mov r9d,MB_ICONERROR  ; Message Box = Icon Error
jmp Lmsg 

LerrorFPU:            ; This entry point for FPU absence
lea rdx,[NoFpuError]
jmp LerrorEntry

;--- Continue Code section: subroutines ---------------------------------------;

; Platform detect support
include 'include\checkcpuid.inc'
include 'include\getcpuname.inc'
include 'include\measurecpuclk.inc'
; Strings builds support
include 'include\doubleprint.inc'
; Target timings routines
include 'include\read_sse128.inc'
include 'include\write_sse128.inc'
include 'include\copy_sse128.inc'
include 'include\read_avx256.inc'
include 'include\write_avx256.inc'
include 'include\copy_avx256.inc'
include 'include\read_avx512.inc'
include 'include\write_avx512.inc'
include 'include\copy_avx512.inc'
include 'include\sqrt_sse128.inc'
include 'include\sqrt_avx256.inc'
include 'include\sqrt_avx512.inc'
include 'include\cos_x87.inc'
include 'include\sincos_x87.inc'

;---------- Data section ------------------------------------------------------;

section '.data' data readable writeable

; Messages for errors reporting
OldCpuError  DB 'Too old CPU or CPUID feature locked',0
NoFpuError   DB 'FPU x87 absent or CPUID feature locked',0

; Main message title string
WinCaption   DB ' VectorBrief v0.01',0

; Transit buffer for CPU name conversion (remove extra spaces)
NameBuffer   DB 48 DUP (' ')

; Main message body
WinMessage   DB 48 DUP (' '),0Ah,0Dh
             DB 'TSC clock (MHz) = '
stsc         DB 10 DUP (' '),0Ah,0Dh,0Ah,0Dh
             DB 'Processor features, detect by CPUID'
             DB 0Ah,0Dh
             DB 'AVX 256-bit : '
s1           DB '             ',0Ah,0Dh
             DB 'AVX2 256-bit : '
s2           DB '             ',0Ah,0Dh
             DB 'AVX3 512-bit = AVX512F (Foundation) : '
s3           DB '             ',0Ah,0Dh
             DB 'AVX512CD (Conflict Detection) : '
scd          DB '             ',0Ah,0Dh          
             DB 'AVX512PF (Prefetch) : '
spf          DB '             ',0Ah,0Dh          
             DB 'AVX512ER (Exponential and Reciprocal) : '
ser          DB '             ',0Ah,0Dh          
             DB 'AVX512VL (Vector Length) : '
svl          DB '             ',0Ah,0Dh          
             DB 'AVX512BW (Byte and Word) : '
sbw          DB '             ',0Ah,0Dh          
             DB 'AVX512DQ (Doubleword and Quadword) : '
sdq          DB '             ',0Ah,0Dh          
             DB 'AVX512IFMA (Integer Fused Multiply and Add) : '
sif          DB '             ',0Ah,0Dh          
             DB 'AVX512VBM (Vector Byte Manipulation) : '
svb          DB '             ',0Ah,0Dh,0Ah,0Dh
             DB 'OS context management features, detect by XCR0'
             DB  0Ah,0Dh
             DB 'SSE128 registers XMM[0-15] bits [0-127] : '
sc1          DB '             ',0Ah,0Dh
             DB 'AVX256 registers YMM[0-15] bits [128-255] : '
sc2          DB '             ',0Ah,0Dh
             DB 'AVX512 registers ZMM[0-15] bits[511-256] : '
sc3          DB '             ',0Ah,0Dh
             DB 'AVX512 registers ZMM[16-31] bits[0-511] : '
sc4          DB '             ',0Ah,0Dh
             DB 'AVX512 predicate registers K[0-7] : '
sc5          DB '             ',0Ah,0Dh,0Ah,0Dh
             DB 'Instruction timings per 1 core (TSC clocks and nanoseconds)'
             DB 0Ah,0Dh
; ST = String Timing
             DB 'SSE128 read : '             
st128r       DB '                        ',0Ah,0Dh 
             DB 'SSE128 write : '             
st128w       DB '                        ',0Ah,0Dh 
             DB 'SSE128 copy : '             
st128c       DB '                        ',0Ah,0Dh 
             DB 'AVX256 read : '             
st256r       DB '                        ',0Ah,0Dh 
             DB 'AVX256 write : '             
st256w       DB '                        ',0Ah,0Dh 
             DB 'AVX256 copy : '             
st256c       DB '                        ',0Ah,0Dh 
             DB 'AVX512 read : '             
st512r       DB '                        ',0Ah,0Dh 
             DB 'AVX512 write : '             
st512w       DB '                        ',0Ah,0Dh 
             DB 'AVX512 copy : '             
st512c       DB '                        ',0Ah,0Dh 
             DB 'SQRTPD xmm : '             
stsqxmm      DB '                        ',0Ah,0Dh 
             DB 'VSQRTPD ymm : '             
stsqymm      DB '                        ',0Ah,0Dh 
             DB 'VSQRTPD zmm : '             
stsqzmm      DB '                        ',0Ah,0Dh 
             DB 'FCOS : '             
stfcs        DB '                        ',0Ah,0Dh 
             DB 'FSINCOS : '             
stfscs       DB '                        ',0Ah,0Dh 
             DB 0

; Strings "supported" , "not supported"
Sup          DB 'supported',0
Nsup         DB 'not supported',0
NsupCPU      DB 'not supported by CPU',0
NsupOS       DB 'not supported by OS',0

; Terminator for lists
MACRO VEEND
{
DQ  0
}
; This macro for pointer to updated string = F (tested bit)
; VECPU means Visual Entry for CPUID results visual
MACRO VECPU x1,x2,x3,x4
{       
; Pointer to string for write " supported / not supported "
DQ  x1 
; x2 = bits[5-0] = bit number in the 64-bit register
; x3 = bit[6] = register number: 0=R14, 1=R15
; x4 = bit[7] = register number: override to R13 
DB  x4 shl 7 + x3 shl 6 + x2    
}        
; List of strings to build, CPUID features
VisualEntriesCPU:
VECPU  s1  , 28+00 , 0 , 0
VECPU  s2  , 05+00 , 1 , 0
VECPU  s3  , 16+00 , 1 , 0
VECPU  scd , 28+00 , 1 , 0
VECPU  spf , 26+00 , 1 , 0
VECPU  ser , 27+00 , 1 , 0
VECPU  svl , 31+00 , 1 , 0
VECPU  sbw , 30+00 , 1 , 0
VECPU  sdq , 17+00 , 1 , 0
VECPU  sif , 21+00 , 1 , 0
VECPU  svb , 01+32 , 1 , 0
VECPU  sc1 , 01+32 , 0 , 0
VECPU  sc2 , 02+32 , 0 , 0
VECPU  sc3 , 06+32 , 0 , 0
VECPU  sc4 , 07+32 , 0 , 0
VECPU  sc5 , 05+32 , 0 , 0
VEEND 

; This macro for pointer to updated string = F (measured clock)
; VECLK means Visual Entry for CPU Clocks per Instruction results visual
MACRO VECLK x1,x2,x3,x4, x5,x6,x7, x8,x9,x10
{       
; Pointer to string for write " xxxx.xxx (xxxx.xx ns) ", 0 means terminator
DQ  x1
; Pointer to target measured code fragment
DQ  x2
; Block length, units = instructions (example 1024 * 16 bytes = 16384 bytes)
DQ  x3
; Number of measurement iterations
DQ  x4
; x5 = bits[5-0] = bit number in the 64-bit register
; x6 = bit[6] = register number: 0=R14, 1=R15
; x7 = bit[7] = register number: override to R13 
DB   x7 shl 7 + x6 shl 6 + x5    ; For CPU
DB  x10 shl 7 + x9 shl 6 + x8    ; For OS
}
; List of strings to build, CPU Clocks per Instruction timings
; Note. OS SSE pre-check reduced to x87 check, 
; for support SSE when not supported XCR0
VisualEntriesCLK:
VECLK  st128r  , Pattern_Read_SSE128  , 1024 , 200000 , 25+00 , 0 , 1 , 00+00 , 0 , 1  ; 01+32 , 0 , 0 
VECLK  st128w  , Pattern_Write_SSE128 , 1024 , 200000 , 25+00 , 0 , 1 , 00+00 , 0 , 1  ; 01+32 , 0 , 0 
VECLK  st128c  , Pattern_Copy_SSE128  , 512  , 200000 , 25+00 , 0 , 1 , 00+00 , 0 , 1  ; 01+32 , 0 , 0
VECLK  st256r  , Pattern_Read_AVX256  , 512  , 200000 , 28+00 , 0 , 0 , 02+32 , 0 , 0 
VECLK  st256w  , Pattern_Write_AVX256 , 512  , 200000 , 28+00 , 0 , 0 , 02+32 , 0 , 0 
VECLK  st256c  , Pattern_Copy_AVX256  , 256  , 200000 , 28+00 , 0 , 0 , 02+32 , 0 , 0 
VECLK  st512r  , Pattern_Read_AVX512  , 256  , 200000 , 16+00 , 1 , 0 , 06+32 , 0 , 0
VECLK  st512w  , Pattern_Write_AVX512 , 256  , 200000 , 16+00 , 1 , 0 , 06+32 , 0 , 0 
VECLK  st512c  , Pattern_Copy_AVX512  , 128  , 200000 , 16+00 , 1 , 0 , 06+32 , 0 , 0  
VECLK  stsqxmm , Pattern_Sqrt_SSE128  , 1024 , 50000  , 26+00 , 0 , 1 , 01+32 , 0 , 0
VECLK  stsqymm , Pattern_Sqrt_AVX256  , 1024 , 50000  , 28+00 , 0 , 0 , 02+32 , 0 , 0  
VECLK  stsqzmm , Pattern_Sqrt_AVX512  , 1024 , 50000  , 16+00 , 1 , 0 , 06+32 , 0 , 0
VECLK  stfcs   , Pattern_Cos_X87      , 1024 , 5000   , 00+00 , 0 , 1 , 00+00 , 0 , 1    
VECLK  stfscs  , Pattern_SinCos_X87   , 1024 , 5000   , 00+00 , 0 , 1 , 00+00 , 0 , 1
VEEND

; Data for floating point operations
align 64
VectorLoad  DQ  11.9 , 22.8 , 33.7 , 44.6
            DQ  55.5 , 66.4 , 77.3 , 88.2
DivisorMHz  DQ  1000000.0
DividendNs  DQ  1000000000.0
TscNs       DQ  ?

; Continue data section, multifunctional buffer
TEMP_BUFFER_SIZE  = 16384      ; Required 16 Kilobytes
align 64                       ; Aligned by typical cache line size is 64 bytes
TEMP_BUFFER  DB  TEMP_BUFFER_SIZE DUP (?)

;---------- Imported data section ---------------------------------------------;

section '.idata' import data readable writeable
library kernel32 , 'KERNEL32.DLL' , user32 , 'USER32.DLL'
include 'api\kernel32.inc'
include 'api\user32.inc'

;---------- Fixups section ----------------------------------------------------;

data fixups
end data
