;------------------------------------------------------------------------------;
;                     Template for integration tests.                          ;
;   Integration test for GUI application main window with child windows open.  ; 
;------------------------------------------------------------------------------;


; UNDER CONSTRUCTION.

; + AT_BUILD_PRODUCTION.  TODO. Unload dynamical import, advapi 32.
; + AT_BUILD_PRODUCTION.  TODO. Message if build main window error.
; + AT_BUILD_PRODUCTION.  TODO. Warning message if some functions not loaded.
; + AT_BUILD_PRODUCTION.  TODO. Error decode and GUI box with message by benchmarks results,
;       check CF flag after subroutines.

; Template for debug method 1 of 3 = Template debug.
; ( Use also 2 = Application debug, 3 = Window debug )


include 'win32a.inc'

include 'global\connect_equ.inc'
include 'gui\connect_equ.inc'
include 'scenario_brief\connect_equ.inc'
include 'scenario_main\connect_equ.inc'
include 'scenario_simple\connect_equ.inc'
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


; Get system information into fixed buffer
; + AT_BUILD_PRODUCTION. TODO. Interpreting and visual error status code
call GetSystemParameters
jc ErrorProgram
call SysParmsToBuffer
call SysParmsToGui

; Create (Open) parent window = Window 0
; Terminology notes: WS = Window Style
lea ebx,[Create_Win0] 
call CreateApplicationWindow    ; AL  = 0 , means first call
lea eax,[MessageBadWindow]      ; EAX = Pointer to error string, used if abort
jz ErrorProgram                 ; Go if error returned

; Get messages with wait user events for window from OS
; GetMessage (internal) subroutine parameters
; Parm#1 = Pointer to structure receive data = DialMsg
; Parm#2 = Window handle whose message retrieved, 0=any
; Parm#3 = Filter Min , both Min=Max=0 means no filtering
; Parm#4 = Filter Max , both Min=Max=0 means no filtering
; Output RAX = Message status, used values (but this is not a message code):
; 0, means returned WM_QUIT, returned when window subroutine
;    call PostQuitMessage subroutine, means quit
; 1, means process event
mov byte [Win0_Init],1         ; Enable commands handling by callback routine
lea edi,[DialogueMsg_Win0]     ; EDI = MSG structure
call WaitEvent                 ; Window 1 work as callbacks inside this

; Exit point
ExitProgram:

; Unit test purpose dummy point
ErrorProgram:

; Note ADVAPI32 library unload absent in this test, it exist at BUILD_NCRB.ASM

; exit
push 0
call [ExitProcess]

include 'global\connect_code.inc'
include 'gui\connect_code.inc'
include 'scenario_brief\connect_code.inc'
include 'scenario_main\connect_code.inc'
include 'scenario_simple\connect_code.inc'
include 'system_info\connect_code.inc'
include 'targets_bandwidth\connect_code.inc'
include 'targets_latency\connect_code.inc'
include 'targets_math\connect_code.inc'
include 'threads_manager\connect_code.inc'

section '.data' data readable writeable

; This for unit test build.
; + TODO. Remove this requirements cause.
; CacheL1Data:
; CpuVendorString:
; CpuNameString:
; DD 3 DUP (0)

include 'global\connect_const.inc'
include 'gui\connect_const.inc'
include 'scenario_brief\connect_const.inc'
include 'scenario_main\connect_const.inc'
include 'scenario_simple\connect_const.inc'
include 'system_info\connect_const.inc'
include 'targets_bandwidth\connect_const.inc'
include 'targets_latency\connect_const.inc'
include 'targets_math\connect_const.inc'
include 'threads_manager\connect_const.inc'

include 'global\connect_var.inc'
include 'gui\connect_var.inc'
include 'scenario_brief\connect_var.inc'
include 'scenario_main\connect_var.inc'
include 'scenario_simple\connect_var.inc'
include 'system_info\connect_var.inc'
include 'targets_bandwidth\connect_var.inc'
include 'targets_latency\connect_var.inc'
include 'targets_math\connect_var.inc'
include 'threads_manager\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphics 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

