
; Template for integration tests.
; Integration test for GUI application Vector Brief benchmark window. 

include 'win64a.inc'
include 'global\connect_equ.inc'
include 'scenario_brief\connect_equ.inc'
include 'system_info\connect_equ.inc'
include 'targets_bandwidth\connect_equ.inc'
include 'targets_math\connect_equ.inc'
include 'threads_manager\connect_equ.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

sub rsp,8*5

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

include 'global\connect_code.inc'
include 'scenario_brief\connect_code.inc'
include 'system_info\connect_code.inc'
include 'targets_bandwidth\connect_code.inc'
include 'targets_math\connect_code.inc'
include 'threads_manager\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'scenario_brief\connect_const.inc'
include 'system_info\connect_const.inc'
include 'targets_bandwidth\connect_const.inc'
include 'targets_math\connect_const.inc'
include 'threads_manager\connect_const.inc'

include 'global\connect_var.inc'
include 'scenario_brief\connect_var.inc'
include 'system_info\connect_var.inc'
include 'targets_bandwidth\connect_var.inc'
include 'targets_math\connect_var.inc'
include 'threads_manager\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

