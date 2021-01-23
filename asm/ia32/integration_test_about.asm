;------------------------------------------------------------------------------;
;                       Template for integration tests.                        ;
;           Integration test for "About" window: show "About" window.          ; 
;------------------------------------------------------------------------------;

include 'win32a.inc'
include 'global\connect_equ.inc'

format PE GUI 4.0
entry start
section '.text' code readable executable
start:

; message box and wait user input
push MB_ICONINFORMATION   ; Parm#4 = Message box icon type = Info
push ABOUT_CAP            ; Parm#3 = Pointer to caption
push ABOUT_ID             ; Parm#2 = Pointer to "About" string
push 0	                  ; Parm#1 = Parent window handle or 0
call [MessageBoxA]

; exit
push 0
call [ExitProcess]

include 'global\connect_code.inc'

section '.data' data readable writeable
include 'global\connect_const.inc'
include 'global\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphics 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

