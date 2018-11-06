; Engineering sample: memory latency measurement.
; Before integration:
; 1) Make regular and optimal string build: " = ", units, CR/LF.
; 2) Check x87 stack correct usage.
; 3) Block length yet must be integral power of 2. Required more flexibility.
; 4) Calculate this better without integer division T=1/F, use x87.   
; 5) Add "Latency brief" to "Vector brief" ?


BLOCK_SIZE     EQU  32*1024*1024
REPEATS_COUNT  EQU  30

; BLOCK_SIZE     EQU  16*1024*1024
; REPEATS_COUNT  EQU  30

; BLOCK_SIZE     EQU  16*1024
; REPEATS_COUNT  EQU  500000

; BLOCK_SIZE     EQU  128*1024
; REPEATS_COUNT  EQU  100000

; BLOCK_SIZE     EQU  4*1024*1024
; REPEATS_COUNT  EQU  1500


format PE64 GUI 5.0
entry start
include 'win64a.inc'

section '.text' code readable executable
start:

sub rsp,8
cld
lea rdi,[CPUvendor]
call CheckCpuId
jc ErrorX87                        ; Go error if CPUID not supported (locked)
cmp eax,1
jb ErrorX87                        ; Go error if CPUID function 1 not supported 
mov eax,1
cpuid
test dl,0001b
jz ErrorX87                        ; Go error if x87 FPU not supported
bt ecx,30
jnc ErrorRdrand                    ; Go error if RDRAND instr. not supported    
call MeasureCpuClk
jc ErrorTsc                        ; Go error if TSC clock measurement error

xor ecx,ecx                        ; Parm#1 = RCX = Virtual address, 0=Auto
mov rdx,BLOCK_SIZE                 ; Parm#2 = RDX = Memory block size
mov [MemorySize],rdx               ; Assign work block size
shl rdx,1                          ; memory size * 2 = main buf. + offsets buf. 
mov r8d,MEM_RESERVE + MEM_COMMIT   ; Parm#3 = R8  = Allocation type
mov r9d,PAGE_READWRITE             ; Parm#4 = R9  = Page Attribute
call [VirtualAlloc]                ; Call memory allocation WinAPI
test rax,rax                       ; Return = RAX = Memory pointer
jz ErrorMemory1                    ; Go error if memory allocation error

mov [MemoryBase],rax               ; Assign work block base
mov [RepeatsCount],REPEATS_COUNT   ; Assign measurements repeats count

mov rbx,[MemoryBase]
mov rdi,rbx
mov rcx,[MemorySize]
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
BuildRandom:                 ; This cycle for build random offsets list
rdrand rax
jnc BuildRandom
and rax,rbx
shl rax,3
add rax,r10
mov r8,[rsi]
mov r9,[rax]
mov [rax],r8
mov [rsi],r9
add rsi,rdx
dec rbp
jnz BuildRandom

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

mov ebp,[RepeatsCount]
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
mov dword [DeltaTsc+0],eax
mov dword [DeltaTsc+4],edx

mov rcx,[MemoryBase]               ; Parm#1 = RCX = Base address
jrcxz .NoMemory                    ; Skip if RCX=0 means block not allocated
xor edx,edx                        ; Parm#2 = RDX = Size, 0=Auto by allocated
mov r8d,MEM_RELEASE                ; Parm#3 = R8  = Type of memory free op.
call [VirtualFree]                 ; Call memory release WinAPI
test rax,rax                       ; Return = RAX = Operation status
jz ErrorMemory2                    ; Go error if memory release error
.NoMemory:

lea rdi,[Buffer]
lea rsi,[StringBase]
call StringWrite
mov rax,[MemoryBase]
call HexPrint64              ; Print base address of allocated block
mov al,'h'
stosb
call StringWrite
mov rax,[MemorySize]
call HexPrint64              ; Print size of allocated block, only walked part
mov al,'h'
stosb
call StringWrite
mov bl,0
mov eax,[RepeatsCount]
call DecimalPrint32          ; Print number of measurement repeats
call StringWrite
finit
fild [TSCfrequency]
fidiv [Constant10E6]
push rax
fstp qword [rsp]
pop rax
mov bx,0200h
call DoublePrint             ; Print CPU TSC clock frequency (MHz)
mov eax,' Mhz'
stosd
call StringWrite
fild [TSCperiod]
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

mov eax,[RepeatsCount]
mov rdx,[MemorySize]
shr rdx,3
mul rdx
push rax rax
fild qword [DeltaTsc]
fild qword [rsp]
fdivp st1,st0
fst qword [rsp]
fild qword [TSCperiod]
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

lea rdx,[Buffer]      ; RDX = Parm #2 = Message
lea r8,[WinCaption]   ; R8  = Parm #3 = Caption (upper message)
xor r9,r9             ; R9  = Parm #4 = Message flags
MessageAndExit:
xor rcx,rcx 		      ; RCX = Parm #1 = Parent window
call [MessageBoxA]    ; Show message box

xor ecx,ecx
call [ExitProcess]    ; Exit application

ErrorX87:             ; Errors handlers
lea rdx,[MsgX87]
jmp ErrorEntry
ErrorRdrand:
lea rdx,[MsgRdrand]
jmp ErrorEntry
ErrorTsc:
lea rdx,[MsgTsc]
jmp ErrorEntry
ErrorMemory1:
lea rdx,[MsgMemory1]
jmp ErrorEntry
ErrorMemory2:
lea rdx,[MsgMemory2]
ErrorEntry:
xor r8d,r8d
mov r9d,MB_ICONERROR
jmp MessageAndExit

include 'sysinfo\checkcpuid.inc'
include 'sysinfo\measurecpuclk.inc'
include 'sysinfo\getcpuname.inc'
include 'stringwrite\stringwrite.inc'
include 'stringwrite\decprint.inc'
include 'stringwrite\hexprint.inc'
include 'stringwrite\doubleprint.inc'

section '.data' data readable writeable

WinCaption      DB  '  Memory latency test  ',0
MsgX87          DB  'x87 FPU required',0
MsgRdrand       DB  'RDRAND instruction required',0
MsgTsc          DB  'Time Stamp Counter error',0
MsgMemory1      DB  'Memory allocation error',0
MsgMemory2      DB  'Memory release error',0

StringBase      DB 'Base = ',0
                DB  0Ah, 0Dh, 'Size = ',0
                DB  0Ah, 0Dh, 'Repeats = ',0
                DB  0Ah, 0Dh, 'TSC frequency = ',0
                DB  0Ah, 0Dh, 'TSC period = ',0
                DB  0Ah, 0Dh, 'Latency clocks = ',0
                DB  0Ah, 0Dh, 'Latency time = ',0

Constant10E3    DD  1000
Constant10E6    DD  1000000

CPUvendor       DB  12 DUP (?)
TSCfrequency    DQ  ?
TSCperiod       DQ  ?
MemoryBase      DQ  ?
MemorySize      DQ  ?
RepeatsCount    DD  ?
DeltaTsc        DQ  ?

Buffer          DB  4096 DUP (?)

section '.idata' import data readable writeable

library kernel32 , 'KERNEL32.DLL' , user32 , 'USER32.DLL'
include 'api\kernel32.inc'
include 'api\user32.inc'

