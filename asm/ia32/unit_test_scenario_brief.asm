;------------------------------------------------------------------------------;
;                        Template for unit tests.                              ;
; Unit test for Vector Brief benchmark scenario: build blank text report only. ; 
;------------------------------------------------------------------------------;

include 'win32a.inc'
include 'global\connect_equ.inc'
include 'scenario_brief\connect_equ.inc'

format PE GUI 4.0
entry start
section '.text' code readable executable
start:

cld
call RunVectorBrief
jc RunError

call ResultVectorBrief

; Other parameters
push 0                ; Parm#4 = Message box icon type
push PRODUCT_ID       ; Parm#3 = Pointer to caption
push TEMP_BUFFER + VECTOR_BRIEF_TEMP_TRANSIT  ; Parm#2 = Pointer to strings
push 0                ; Parm#1 = Parent window handle or 0
call [MessageBoxA]

RunError:

push 0
call [ExitProcess]

; This for build test

GetCpuName:
; add rdi,48+4  ; required change protocol (ADDEND) 
push eax ecx
mov ecx,48+4
mov al,0
rep stosb
pop ecx eax
ret

MeasureTsc:
; add edi,40    ; required change protocol (ADDEND) to MeasureTsc
push eax ecx
mov ecx,40
mov al,0
rep stosb
pop ecx eax
ret

GetBandwidthPattern:
GetMathPattern:
lea ebx,[DummyPattern]
ret

RunTarget:
DummyPattern:
xor eax,eax
ret

include 'global\connect_code.inc'
include 'scenario_brief\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'scenario_brief\connect_const.inc'

align 64
MathData:
DQ  11.9 , 22.8 , 33.7 , 44.6
DQ  55.5 , 66.4 , 77.3 , 88.2

WinCaption   DB ' Unit test for Vector Brief benchmark scenario' , 0
MessageText  DB 'Template text...' , 0

include 'global\connect_var.inc'
include 'scenario_brief\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphics 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

