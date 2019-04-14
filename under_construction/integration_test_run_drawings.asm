
; Template for integration tests.
; Integration test for GUI application drawings window. 

include 'win64a.inc'
include 'global\connect_equ.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

sub rsp,8*5


xor ecx,ecx
call [ExitProcess]

include 'global\connect_code.inc'

section '.data' data readable writeable
include 'global\connect_const.inc'
include 'global\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

