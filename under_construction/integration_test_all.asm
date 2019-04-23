
; Template for integration tests.
; Integration test for GUI application main window. 

include 'win64a.inc'

include 'global\connect_equ.inc'
include 'gui\connect_equ.inc'
include 'scenario_brief\connect_equ.inc'
include 'scenario_draw\connect_equ.inc'
include 'scenario_main\connect_equ.inc'
include 'scenario_simple\connect_equ.inc'
include 'system_info\connect_equ.inc'
include 'targets_bandwidth\connect_equ.inc'
include 'targets_latency\connect_equ.inc'
include 'targets_math\connect_equ.inc'
include 'threads_manager\connect_equ.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

; initialize stack
sub rsp,8*5

; message box and wait user input
; xor ecx,ecx	              ; RCX = Parm#1 = Parent window handle or 0
; lea rdx,[ABOUT_ID]        ; RDX = Parm#2 = Pointer to "About" string
; lea r8,[ABOUT_CAP]        ; R8  = Parm#3 = Pointer to caption
; mov r9d,0040h             ; R9  = Parm#4 = Message box icon type = Info
; call [MessageBoxA]


; Get system information into fixed buffer
; TODO. Interpreting and visual error status code
call GetSystemParameters
jc ErrorProgram
call SysParmsToBuffer
call SysParmsToGui

; Create (Open) parent window = Window 0
; Terminology notes: WS = Window Style
lea rsi,[PRODUCT_ID]       ; RSI = Window name string pointer
lea rdi,[Dialogue_Win0]    ; RDI = Dialogue descriptor pointer
mov r10d,WS_VISIBLE+WS_DLGFRAME+WS_SYSMENU ; R10 = Window Style
mov r11d,WIN0_XBASE        ; R11 = Window X-base
mov r12d,WIN0_YBASE        ; R12 = Window Y-base
mov r13d,WIN0_XSIZE        ; R13 = Window X-size
mov r14d,WIN0_YSIZE        ; R14 = Window Y-size
xor ebx,ebx                ; RBX = 0, means no parent handle
xor eax,eax                ; AH  = 0 , means parent = Application
call CreateApplicationWindow     ; AL  = 0 , means first call
lea r15,[MessageBadWindow]       ; R14 = Error string, used if abort
jz ErrorProgram 			           ; Go if error returned

; Get messages with wait user events for window from OS
; GetMessage subroutine parameters
; Parm#1 = Pointer to structure receive data = DialMsg
; Parm#2 = Window handle whose message retrieved, 0=any
; Parm#3 = Filter Min , both Min=Max=0 means no filtering
; Parm#4 = Filter Max , both Min=Max=0 means no filtering
; Output RAX = Message status, used values (but this is not a message code):
; 0, means returned WM_QUIT, returned when window subroutine
;    call PostQuitMessage subroutine, means quit
; 1, means process event
mov byte [Win0_Init],1           ; Enable commands handling by callback routine
lea rdi,[DialogueMsg_Win0]       ; RDI = MSG structure
call WaitEvent                   ; Window 1 work as callbacks inside this

; Exit point
ExitProgram:

; Unit test purpose dummy point
ErrorProgram:

; exit
xor ecx,ecx
call [ExitProcess]

include 'global\connect_code.inc'
include 'gui\connect_code.inc'
include 'scenario_brief\connect_code.inc'
include 'scenario_draw\connect_code.inc'
include 'scenario_main\connect_code.inc'
include 'scenario_simple\connect_code.inc'
include 'system_info\connect_code.inc'
include 'targets_bandwidth\connect_code.inc'
include 'targets_latency\connect_code.inc'
include 'targets_math\connect_code.inc'
include 'threads_manager\connect_code.inc'

section '.data' data readable writeable

; This for unit test build.
; TODO. Remove this requirements cause.
; CacheL1Data:
; CpuVendorString:
; CpuNameString:
; DD 3 DUP (0)

include 'global\connect_const.inc'
include 'gui\connect_const.inc'
include 'scenario_brief\connect_const.inc'
include 'scenario_draw\connect_const.inc'
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
include 'scenario_draw\connect_var.inc'
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
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

