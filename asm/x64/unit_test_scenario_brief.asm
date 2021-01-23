;------------------------------------------------------------------------------;
;                        Template for unit tests.                              ;
; Unit test for Vector Brief benchmark scenario: build blank text report only. ; 
;------------------------------------------------------------------------------;

include 'win64a.inc'
include 'global\connect_equ.inc'
include 'scenario_brief\connect_equ.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

sub rsp,8*5
cld

call RunVectorBrief
jc RunError

call ResultVectorBrief

; Other parameters
xor ecx,ecx           ; RCX = Parm#1 = Parent window handle or 0
lea rdx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_TRANSIT]  ; RDX = Parm#2 = Pointer st.
lea r8,[PRODUCT_ID]   ; R8  = Parm#3 = Pointer to caption
xor r9d,r9d           ; R9  = Parm#4 = Message box icon type
call [MessageBoxA]

RunError:

xor ecx,ecx
call [ExitProcess]

; This for build test

GetCpuName:
; add rdi,48+4  ; required change protocol (ADDEND) 
push rax rcx
mov ecx,48+4
mov al,0
rep stosb
pop rcx rax
ret

MeasureTsc:
; add rdi,40    ; required change protocol (ADDEND) to MeasureTsc
push rax rcx
mov ecx,40
mov al,0
rep stosb
pop rcx rax
ret

GetBandwidthPattern:
GetMathPattern:
lea rbx,[DummyPattern]
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

