;------------------------------------------------------------------------------;
; Engineering sample: memory latency measurement.                              ;
; Note. LCM = Linear Congruential Method for pseudo-random numbers generation  ;
; Before integration:                                                          ;
; 1) Make regular and optimal string build: " = ", units, CR/LF.               ;
; 2) Check x87 stack correct usage.                                            ;
; 3) Block length yet must be integral power of 2. Required more flexibility.  ;
; 4) Calculate this better without integer division T=1/F, use x87.            ;   
; 5) Add "Latency brief" to "Vector brief" ?                                   ;
;------------------------------------------------------------------------------;

 BLOCK_SIZE     EQU  32*1024*1024
 REPEATS_NUM    EQU  30

; BLOCK_SIZE     EQU  16*1024*1024
; REPEATS_NUM    EQU  30

; BLOCK_SIZE     EQU  16*1024
; REPEATS_NUM    EQU  500000

; BLOCK_SIZE     EQU  128*1024
; REPEATS_NUM    EQU  100000

; BLOCK_SIZE     EQU  4*1024*1024
; REPEATS_NUM    EQU  1500

MEMORY_BASE     EQU  qword [r15+00]
MEMORY_SIZE     EQU  qword [r15+08]
MEASURE_COUNT   EQU  dword [r15+16]
DELTA_TSC       EQU  qword [r15+20]
DELTA_TSC_L     EQU  dword [r15+20]
DELTA_TSC_H     EQU  dword [r15+24]
BUILD_TEXT      EQU  [r15+1024]

lea r15,[TEMP_BUFFER]

xor ecx,ecx                        ; Parm#1 = RCX = Virtual address, 0=Auto
mov rdx,BLOCK_SIZE                 ; Parm#2 = RDX = Memory block size
mov MEMORY_SIZE,rdx                ; Assign work block size
shl rdx,1                          ; memory size * 2 = main buf. + offsets buf. 
mov r8d,MEM_RESERVE + MEM_COMMIT   ; Parm#3 = R8  = Allocation type
mov r9d,PAGE_READWRITE             ; Parm#4 = R9  = Page Attribute
call [VirtualAlloc]                ; Call memory allocation WinAPI
test rax,rax                       ; Return = RAX = Memory pointer
jz ErrorMemory1                    ; Go error if memory allocation error

mov MEMORY_BASE,rax               ; Assign work block base
mov MEASURE_COUNT,REPEATS_NUM     ; Assign measurements repeats count

mov rbx,MEMORY_BASE
mov rdi,rbx
mov rcx,MEMORY_SIZE
add rbx,rcx
mov rsi,rbx
shr rcx,3
mov rbp,rcx
mov edx,8
xor eax,eax
BuildLinear:                 ; This cycle for build sequental offsets list
mov [rbx],rax
add rbx,rdx
add rax,rdx
dec rcx
jnz BuildLinear

lea rbx,[rbp-1]
mov r10,rsi
mov r11,rbp

;---

;BuildRandom:                 ; This cycle for build random offsets list
;rdrand rax
;jnc BuildRandom
;and rax,rbx
;shl rax,3
;add rax,r10
;mov r8,[rsi]
;mov r9,[rax]
;mov [rax],r8
;mov [rsi],r9
;add rsi,rdx
;dec rbp
;jnz BuildRandom

RANDOM_SEED        EQU  3
RANDOM_MULTIPLIER  EQU  05DEECE66Dh  
RANDOM_ADDEND      EQU  0Bh   

push r15

mov r15,RANDOM_SEED
mov r12,RANDOM_MULTIPLIER
mov r13,RANDOM_ADDEND
mov r14,rdx

BuildRandom:                 ; This cycle for build random offsets list

mov rax,r15
mul r12
add rax,r13
mov r15,rax

and rax,rbx
shl rax,3
add rax,r10
mov r8,[rsi]
mov r9,[rax]
mov [rax],r8
mov [rsi],r9
add rsi,r14
dec rbp
jnz BuildRandom

mov rdx,r14

pop r15

;---

mov rsi,r10
mov rcx,r11
xor ebx,ebx
BuildLinked:                 ; This cycle for build linked random offsets list
mov rax,[rsi]
add rax,rdi
mov [rax],rbx
mov rbx,rax
add rsi,rdx
dec rcx
jnz BuildLinked 

rdtsc                       ; Get time at START of measured fragment
mov esi,eax
mov edi,edx

mov ebp,MEASURE_COUNT
.WalkRepeat:
mov rax,rbx
.WalkLinked:               ; Walk linked list, use fetched data as next address
mov rax,[rax]
test rax,rax
jnz .WalkLinked
dec ebp
jnz .WalkRepeat

rdtsc                       ; Get time at STOP of measured fragment
sub eax,esi
sbb edx,edi
mov DELTA_TSC_L,eax
mov DELTA_TSC_H,edx

mov rcx,MEMORY_BASE               ; Parm#1 = RCX = Base address
jrcxz .NoMemory                    ; Skip if RCX=0 means block not allocated
xor edx,edx                        ; Parm#2 = RDX = Size, 0=Auto by allocated
mov r8d,MEM_RELEASE                ; Parm#3 = R8  = Type of memory free op.
call [VirtualFree]                 ; Call memory release WinAPI
test rax,rax                       ; Return = RAX = Operation status
jz ErrorMemory2                    ; Go error if memory release error
.NoMemory:

lea rdi,BUILD_TEXT
lea rsi,[StringBase]
call StringWrite
mov rax,MEMORY_BASE
call HexPrint64              ; Print base address of allocated block
mov al,'h'
stosb
call StringWrite
mov rax,MEMORY_SIZE
call HexPrint64              ; Print size of allocated block, only walked part
mov al,'h'
stosb
call StringWrite
mov bl,0
mov eax,MEASURE_COUNT
call DecimalPrint32          ; Print number of measurement repeats
call StringWrite
finit
fild [TscClockHz]
fidiv [Constant10E6]
push rax
fstp qword [rsp]
pop rax
mov bx,0200h
call DoublePrint             ; Print CPU TSC clock frequency (MHz)
mov eax,' Mhz'
stosd
call StringWrite
fild [TscPeriodFs]
fidiv [Constant10E3]
push rax
fstp qword [rsp]
pop rax
call DoublePrint             ; Print CPU TSC clock period (picoseconds)
mov al,' '
stosb
mov ax,'ps'
stosw
call StringWrite

mov eax,MEASURE_COUNT
mov rdx,MEMORY_SIZE
shr rdx,3
mul rdx
push rax rax
fild DELTA_TSC
fild qword [rsp]
fdivp st1,st0
fst qword [rsp]
fild qword [TscPeriodFs]
fmulp
fidiv [Constant10E6]
fstp qword [rsp+8]
pop rax
mov bx,0200h
call DoublePrint             ; Print latency at units = TSC clocks
mov al,' '
stosb
mov eax,'clks'
stosd
call StringWrite
pop rax
call DoublePrint             ; Print latency at units = nanoseconds
mov al,' '
stosb
mov ax,'ns'
stosw
mov al,0
stosb

lea rdx,BUILD_TEXT      ; RDX = Parm #2 = Message
lea r8,[DebugCaption]   ; R8  = Parm #3 = Caption (upper message)
xor r9,r9               ; R9  = Parm #4 = Message flags
MessageAndExit:
xor rcx,rcx 		        ; RCX = Parm #1 = Parent window
call [MessageBoxA]      ; Show message box

xor ecx,ecx
call [ExitProcess]      ; Exit application

ErrorMemory1:
lea rdx,[MsgMemory1]
jmp ErrorEntry
ErrorMemory2:
lea rdx,[MsgMemory2]
ErrorEntry:
xor r8d,r8d
mov r9d,MB_ICONERROR
jmp MessageAndExit

DebugCaption   DB  '  Memory latency test (LCM based) ',0
MsgMemory1     DB  'Memory allocation error',0
MsgMemory2     DB  'Memory release error',0

StringBase     DB 'Base = ',0
               DB  0Ah, 0Dh, 'Size = ',0
               DB  0Ah, 0Dh, 'Repeats = ',0
               DB  0Ah, 0Dh, 'TSC frequency = ',0
               DB  0Ah, 0Dh, 'TSC period = ',0
               DB  0Ah, 0Dh, 'Latency clocks = ',0
               DB  0Ah, 0Dh, 'Latency time = ',0
              
Constant10E3   DD  1000
Constant10E6   DD  1000000

