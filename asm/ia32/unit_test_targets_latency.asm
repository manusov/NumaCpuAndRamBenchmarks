;------------------------------------------------------------------------------;
;                          Template for unit tests.                            ;
;             Unit test for memory LATENCY measurement routines:               ;
;         measure and show single result (delta TSC per instruction),          ; 
;                        for fixed-selected pattern.                           ; 
;------------------------------------------------------------------------------;

include 'win32a.inc'
include 'global\connect_equ.inc'
include 'targets_latency\connect_equ.inc'

format PE GUI 4.0
entry start
section '.text' code readable executable
start:

cld

; run target performance fragment: build walk list
mov al,1                            ; ID = 1 = build walk list by RDRAND
call GetLatencyPattern 
lea esi,[ TEMP_BUFFER + 0 ]
lea edi,[ TEMP_BUFFER + 24*1024 ]
mov ecx,1024*3                  ; OLD: 4096/8
call ebx                        ; Run walk list builder
                               
; run target performance fragment: walk in the walk list
xor eax,eax
push eax eax eax eax eax eax
mov al,1                        ; 2  ; OLD: ID = 2 = walk in the walk list
call GetLatencyPattern 
lea esi,[ TEMP_BUFFER + 0 ]
lea edi,[ TEMP_BUFFER + 24*1024 ]
mov ecx,1024*3                  ; OLD: 4096/8
mov ebp,100000
mov [esp+0],ecx       ; ECX = Instructions count per block
mov [esp+8],ebp       ; EBP = Measurement repeats count

push eax              ; EAX = Walker entry point ; OLD: push ebx
xor ebx,ebx           ; EBX = Unused high 32 bits of measurement repeats count

rdtsc
mov [esp+16+4],eax
mov [esp+20+4],edx
 
call dword [esp]      ; call target fragment: Run walker 

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
fimul dword [esp+8]   ; Multiply by copy of measurement counter
fdivp st1,st0         ; Result ST0 = TSC clocks per Instruction (CPI)
fstp qword [esp+16]
pop eax eax eax eax eax edx
mov bx,0300h
call DoublePrint      ; format = 3 numbers after decimal point
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
include 'targets_latency\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'targets_latency\connect_const.inc'

WinCaption   DB ' Unit test for memory latency measurement routines.'
             DB ' IA32 port.' , 0
MessageText  DB 'dTSC/Instruction = ',0

include 'global\connect_var.inc'
include 'targets_latency\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphics 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

