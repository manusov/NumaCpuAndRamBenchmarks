;------------------------------------------------------------------------------;
;                     Template for integration tests.                          ;
;      Integration test for GUI application Vector Brief benchmark window.     ;
;------------------------------------------------------------------------------;

include 'win32a.inc'
include 'global\connect_equ.inc'
include 'scenario_brief\connect_equ.inc'
include 'system_info\connect_equ.inc'
include 'targets_bandwidth\connect_equ.inc'
include 'targets_latency\connect_equ.inc'
include 'targets_math\connect_equ.inc'
include 'threads_manager\connect_equ.inc'

format PE GUI 4.0
entry start
section '.text' code readable executable
start:

; Dynamical import for test purpose
push NameAdvapi32
call [LoadLibrary]
mov [HandleAdvapi32],eax
call SystemFunctionsLoad    ; This subroutine output CF=1 if error

; Execute Vector Brief test
call RunVectorBrief
jc RunError

; Build strings for Vector Brief test results
call ResultVectorBrief

; Show window with Vector Brief test results
push 0                ; Parm#4 = Message box icon type
push PRODUCT_ID       ; Parm#3 = Pointer to caption
push TEMP_BUFFER + VECTOR_BRIEF_TEMP_TRANSIT  ; Parm#2 = Pointer to string
push 0                ; Parm#1 = Parent window handle or 0
call [MessageBoxA]

RunError:

; Note ADVAPI32 library unload absent in this test, it exist at BUILD_NCRB.ASM

push 0
call [ExitProcess]

include 'global\connect_code.inc'
include 'scenario_brief\connect_code.inc'
include 'system_info\connect_code.inc'
include 'targets_bandwidth\connect_code.inc'
include 'targets_latency\connect_code.inc'
include 'targets_math\connect_code.inc'
include 'threads_manager\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'scenario_brief\connect_const.inc'
include 'system_info\connect_const.inc'
include 'targets_bandwidth\connect_const.inc'
include 'targets_latency\connect_const.inc'
include 'targets_math\connect_const.inc'
include 'threads_manager\connect_const.inc'

include 'global\connect_var.inc'
include 'scenario_brief\connect_var.inc'
include 'system_info\connect_var.inc'
include 'targets_bandwidth\connect_var.inc'
include 'targets_latency\connect_var.inc'
include 'targets_math\connect_var.inc'
include 'threads_manager\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

