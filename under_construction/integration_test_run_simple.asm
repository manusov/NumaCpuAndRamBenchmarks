
; Template for integration tests.
; Integration test for GUI application Simple benchmark window. 

include 'win64a.inc'
include 'global\connect_equ.inc'
include 'scenario_main\connect_equ.inc'
include 'scenario_simple\connect_equ.inc'
include 'system_info\connect_equ.inc'
include 'targets_bandwidth\connect_equ.inc'
include 'targets_math\connect_equ.inc'
include 'threads_manager\connect_equ.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

; initializing stack frame
sub rsp,8*5


lea rbx,[SystemParameters]
call GetSystemParameters


;-----------------------------------------;
OPTION_ASM_METHOD          EQU  12  ; { asm methods (routines) list }
OPTION_TARGET_OBJECT       EQU  0   ; { L1, L2, L3, L4, DRAM, STORAGE, CUSTOM }
OPTION_THREADS_COUNT       EQU  0   ; { 0=default SMP AUTO or value 1-256 }  
OPTION_HYPER_THREADING     EQU  0   ; { GRAY_NOT_SUP, DISABLED, ENABLED }
OPTION_PROCESSOR_GROUPS    EQU  2   ; { GRAY_NOT_SUP, DISABLED, ENABLED }   
OPTION_NUMA                EQU  0   ; { GRAY_NOT_SUP, UNAWARE, FORCE_ONE, FORCE LOCAL, FORCE REMOTE }
OPTION_LARGE_PAGES         EQU  0   ; { GRAY_NOT_SUP, DISABLED, ENABLED }
OPTION_MEASUREMENT         EQU  1   ; { FAST, SLOW, FAST_ADAPTIVE, SLOW_ADAPTIVE }
OPTION_CUSTOM_BLOCK_START  EQU  0   ; Override start block size or 0=default
OPTION_CUSTOM_BLOCK_END    EQU  0   ; Override end block size or 0=default
OPTION_CUSTOM_BLOCK_DELTA  EQU  0   ; Override delta block size or 0=default

lea rsi,[UserParms]
mov [rsi + UPB.OptionAsm]        , OPTION_ASM_METHOD
mov [rsi + UPB.OptionThreads]    , OPTION_THREADS_COUNT
mov [rsi + UPB.OptionHT]         , OPTION_HYPER_THREADING
mov [rsi + UPB.OptionPG]         , OPTION_PROCESSOR_GROUPS 
mov [rsi + UPB.OptionNUMA]       , OPTION_NUMA
mov [rsi + UPB.OptionLP]         , OPTION_LARGE_PAGES  
mov [rsi + UPB.OptionTarget]     , OPTION_TARGET_OBJECT
mov [rsi + UPB.OptionMeasure]    , OPTION_MEASUREMENT 
mov [rsi + UPB.CustomBlockStart] , OPTION_CUSTOM_BLOCK_START
mov [rsi + UPB.CustomBlockEnd]   , OPTION_CUSTOM_BLOCK_END
mov [rsi + UPB.CustomBlockDelta] , OPTION_CUSTOM_BLOCK_DELTA
;-----------------------------------------;

call SessionStart

call SessionProgress

call SessionStop

;-----------------------------------------;
; emulate options settings for unit test
lea rbx,[InputParms]
mov [rbx+IPB.UpdatedAsm],34  ; 35  ; 12
mov [rbx+IPB.OperandWidth],256
mov [rbx+IPB.UpdatedThreads],1
mov [rbx+IPB.UpdatedHT],1
mov [rbx+IPB.UpdatedLP],2
mov [rbx+IPB.UpdatedNUMA],3
mov [rbx+IPB.UpdatedPG],2
mov [rbx+IPB.UpdatedTarget],4
mov [rbx+IPB.UpdatedMeasure],3
mov [rbx+IPB.StartBlockSize],24576
mov [rbx+IPB.MeasureRepeats],100000
mov [rbx+IPB.AllocatedBlock1],10000h
mov [rbx+IPB.AllocatedBlock2],20000h
mov [rbx+IPB.MemoryTotal],( 1024*15 + 512 ) * 2
mov [rbx+IPB.MemoryPerThread],1024*15 + 512
lea rbx,[OutputParms]
mov [rbx+OPB.OStimerDelta],1573000
mov [rbx+OPB.TSCtimerDelta],370000000
xor eax,eax
mov [rbx+OPB.TSCperiodNs],rax
mov [rbx+OPB.TSCfrequencyHz],rax
;-----------------------------------------;

; call routines from "Run simple" scenario
call ResultSimple 

; input parameters for build message box
xor ecx,ecx             ; RCX = Parm#1 = Parent window handle or 0
lea rdx,[TEMP_BUFFER]   ; RDX = Dialogue window Parm#2 = Pointer to string
lea r8,[PRODUCT_ID]     ; R8  = Parm#3 = Pointer to caption
xor r9d,r9d             ; R9  = Parm#4 = Message box icon type
call [MessageBoxA]

; exit
xor ecx,ecx
call [ExitProcess]

include 'global\connect_code.inc'
include 'scenario_main\connect_code.inc'
include 'scenario_simple\connect_code.inc'
include 'system_info\connect_code.inc'
include 'targets_bandwidth\connect_code.inc'
include 'targets_math\connect_code.inc'
include 'threads_manager\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'scenario_main\connect_const.inc'
include 'scenario_simple\connect_const.inc'
include 'system_info\connect_const.inc'
include 'targets_bandwidth\connect_const.inc'
include 'targets_math\connect_const.inc'
include 'threads_manager\connect_const.inc'

include 'global\connect_var.inc'
include 'scenario_main\connect_var.inc'
include 'scenario_simple\connect_var.inc'
include 'system_info\connect_var.inc'
include 'targets_bandwidth\connect_var.inc'
include 'targets_math\connect_var.inc'
include 'threads_manager\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

