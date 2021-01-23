;------------------------------------------------------------------------------;
;                          Template for unit tests.                            ;
;         Unit test for MATHEMATICS PERFORMANCE measurement routines:          ;
;         measure and show single result (delta TSC per instruction),          ; 
;                        for fixed-selected pattern.                           ; 
;------------------------------------------------------------------------------;

include 'win64a.inc'
include 'global\connect_equ.inc'
include 'targets_math\connect_equ.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

; initialization
sub rsp,8*5
cld

; run target performance fragment
xor eax,eax
push rax rax rax
mov al,4      ; ID = 4 = FSINCOS
call GetMathPattern
mov ecx,16384
mov ebp,5000  ; 10000 ; 000
lea r8,[MathData]
mov [rsp+0],ecx                  ; ECX = Instructions count per block
mov [rsp+8],ebp                  ; EBP = Measurement repeats count
rdtsc
shl rdx,32
lea r15,[rax+rdx]
call rbx                         ; call target fragment, old qword [rbx]
rdtsc
shl rdx,32
add rax,rdx
sub rax,r15
mov [rsp+16],rax                 ; RAX = Delta TSC

; calculations and build result strings
lea rsi,[MessageText]
lea rdi,[TEMP_BUFFER]
call StringWrite
finit
fild qword [rsp+16]   ; Load delta TSC
fild dword [rsp]      ; Load copy of work counter: executed instructions count
fimul dword [rsp+8]   ; Multiply by copy of measurement counter 
fdivp st1,st0         ; Result ST0 = TSC clocks per Instruction
fstp qword [rsp+16]
pop rax rax rax
mov bx,0300h          ; format = 3 numbers after decimal point
call DoublePrint
mov al,0
stosb

; show result strings
lea rdx,[TEMP_BUFFER]  ; RDX = Parm #2 = Message
lea r8,[WinCaption]    ; R8  = Parm #3 = Caption (upper message)
xor r9d,r9d            ; R9  = Parm #4 = Message flags
xor ecx,ecx            ; RCX = Parm #1 = Parent window
call [MessageBoxA]

; exit
xor ecx,ecx
call [ExitProcess]

include 'global\connect_code.inc'
include 'targets_math\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'targets_math\connect_const.inc'

WinCaption   DB ' Unit test for math performance measurement routines' , 0
MessageText  DB 'dTSC/Instruction = ',0

include 'global\connect_var.inc'
include 'targets_math\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphics 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

