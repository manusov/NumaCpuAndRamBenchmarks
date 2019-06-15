;------------------------------------------------------------------------------;
;                          Template for unit tests.                            ;
;         Unit test for MATHEMATICS PERFORMANCE measurement routines:          ;
;         measure and show single result (delta TSC per instruction),          ; 
;                        for fixed-selected pattern.                           ; 
;------------------------------------------------------------------------------;

include 'win32a.inc'
include 'global\connect_equ.inc'
include 'targets_math\connect_equ.inc'

format PE GUI 4.0
entry start
section '.text' code readable executable
start:

cld

; run target performance fragment
xor eax,eax
push eax eax eax eax eax eax
mov al,4              ; AL = Pattern ID = 4 = FSINCOS
call GetMathPattern
mov ecx,16384         ; ECX = Instructions count per block
mov ebp,5000          ; EBP = Measurement repeats count, old = 10000 ; 000
lea esi,[MathData]    ; ESI = Pointer to math. data pattern               
mov [esp+0],ecx       ; Instructions count per block, for calculations
mov [esp+8],ebp       ; Measurement repeats count, for calculations

push ebx
xor ebx,ebx           ; EBX = Unused high 32 bits of measurement repeats count

rdtsc
mov [esp+16+4],eax
mov [esp+20+4],edx
 
call dword [esp]      ; call target fragment

rdtsc
sub eax,[esp+16+4]
sbb edx,[esp+20+4]
mov [esp+16+4],eax    ; EDX:EAX = Delta TSC, qword [esp+16] for load by x87 FPU
mov [esp+20+4],edx

pop ebx

; calculations and build result strings
lea esi,[MessageText]
lea edi,[TEMP_BUFFER]
call StringWrite
finit
fild qword [esp+16]   ; Load delta TSC
fild dword [esp]      ; Load copy of work counter: executed instructions count
fimul dword [esp+8]   ; Multiply by copy of measurement counter, LOW DWORD 
fdivp st1,st0         ; Result ST0 = TSC clocks per Instruction
fstp qword [esp+16]
pop eax eax eax eax eax edx
mov bx,0300h          ; format = 3 numbers after decimal point
call DoublePrint
mov al,0
stosb

; show result strings
push 0                 ; Parm #4 = Message flags
push WinCaption        ; Parm #3 = Caption (upper message)
push TEMP_BUFFER       ; Parm #2 = Message
push 0                 ; Parm #1 = Parent window
call [MessageBoxA]

; exit
push 0
call [ExitProcess]

include 'global\connect_code.inc'
include 'targets_math\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'targets_math\connect_const.inc'

WinCaption   DB ' Unit test for math performance measurement routines.'
             DB ' IA32 port.' , 0
MessageText  DB 'dTSC/Instruction = ',0

include 'global\connect_var.inc'
include 'targets_math\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions


