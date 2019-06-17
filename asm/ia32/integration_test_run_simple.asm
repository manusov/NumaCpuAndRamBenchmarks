;------------------------------------------------------------------------------;
;                     Template for integration tests.                          ;
;          Integration test for GUI application Simple benchmark window,       ;
;                       runs with constant settings.                           ; 
;------------------------------------------------------------------------------;

include 'win32a.inc'
include 'global\connect_equ.inc'
include 'scenario_main\connect_equ.inc'
include 'scenario_simple\connect_equ.inc'
include 'system_info\connect_equ.inc'
include 'targets_bandwidth\connect_equ.inc'
include 'targets_latency\connect_equ.inc'
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

;- lea ebx,[SystemParameters]
call GetSystemParameters


;-----------------------------------------;
OPTION_ASM_METHOD          EQU  12                   ; { asm methods (routines) list }
OPTION_TARGET_OBJECT       EQU  TARGET_L1            ; { L1, L2, L3, L4, DRAM, CUSTOM }
OPTION_PARALLEL            EQU  PARALLEL_USED        ; { NOT_SUP, NOT_USED, USED }  
OPTION_HYPER_THREADING     EQU  HT_NOT_SUPPORTED     ; { GRAY_NOT_SUP, DISABLED, ENABLED }
OPTION_PROCESSOR_GROUPS    EQU  PG_NO_CONTROL        ; { GRAY_NOT_SUP, DISABLED, ENABLED }
OPTION_NUMA                EQU  NUMA_NOT_SUPPORTED   ; { GRAY_NOT_SUP, UNAWARE, FORCE_ONE, FORCE LOCAL, FORCE REMOTE }
OPTION_LARGE_PAGES         EQU  LP_NOT_SUPPORTED     ; { GRAY_NOT_SUP, DISABLED, ENABLED }
OPTION_MEASUREMENT         EQU  MEASURE_CAREFUL      ; { FAST, SLOW, FAST_ADAPTIVE, SLOW_ADAPTIVE }
OPTION_CUSTOM_BLOCK_START  EQU  0                    ; Override start block size or 0=default
OPTION_CUSTOM_BLOCK_END    EQU  0                    ; Override end block size or 0=default
OPTION_CUSTOM_BLOCK_DELTA  EQU  0                    ; Override delta block size or 0=default

lea esi,[UserParms]
mov [esi + UPB.OptionAsm]        , OPTION_ASM_METHOD
mov [esi + UPB.OptionParallel]   , OPTION_PARALLEL
mov [esi + UPB.OptionHT]         , OPTION_HYPER_THREADING
mov [esi + UPB.OptionPG]         , OPTION_PROCESSOR_GROUPS 
mov [esi + UPB.OptionNUMA]       , OPTION_NUMA
mov [esi + UPB.OptionLP]         , OPTION_LARGE_PAGES  
mov [esi + UPB.OptionTarget]     , OPTION_TARGET_OBJECT
mov [esi + UPB.OptionMeasure]    , OPTION_MEASUREMENT 
mov [esi + UPB.CustomBlockStart] , OPTION_CUSTOM_BLOCK_START
; mov [esi + UPB.CustomBlockEnd]   , OPTION_CUSTOM_BLOCK_END
; mov [esi + UPB.CustomBlockDelta] , OPTION_CUSTOM_BLOCK_DELTA
;-----------------------------------------;

call SessionStart

call SessionProgress

call SessionStop

;-----------------------------------------;
; emulate options settings for unit test
;lea rbx,[InputParms]
;mov [rbx+IPB.UpdatedAsm],34  ; 35  ; 12
;mov [rbx+IPB.OperandWidth],256
;mov [rbx+IPB.UpdatedThreads],1
;mov [rbx+IPB.UpdatedHT],1
;mov [rbx+IPB.UpdatedLP],2
;mov [rbx+IPB.UpdatedNUMA],3
;mov [rbx+IPB.UpdatedPG],2
;mov [rbx+IPB.UpdatedTarget],4
;mov [rbx+IPB.UpdatedMeasure],3
;mov [rbx+IPB.StartBlockSize],24576
;mov [rbx+IPB.MeasureRepeats],100000
;mov [rbx+IPB.AllocatedBlock1],10000h
;mov [rbx+IPB.AllocatedBlock2],20000h
;mov [rbx+IPB.MemoryTotal],( 1024*15 + 512 ) * 2
;mov [rbx+IPB.MemoryPerThread],1024*15 + 512
;lea rbx,[OutputParms]
;mov [rbx+OPB.OStimerDelta],1573000
;mov [rbx+OPB.TSCtimerDelta],370000000
;xor eax,eax
;mov [rbx+OPB.TSCperiodNs],rax
;mov [rbx+OPB.TSCfrequencyHz],rax
;-----------------------------------------;

; call routines from "Run simple" scenario
call ResultSimple 

; input parameters for build message box
push 0                  ; Parm#4 = Message box icon type
push PRODUCT_ID         ; Parm#3 = Pointer to caption
push TEMP_BUFFER        ; Parm#2 = Pointer to string
push 0                  ; Parm#1 = Parent window handle or 0
call [MessageBoxA]

; Note ADVAPI32 library unload absent in this test, it exist at BUILD_NCRB.ASM

; exit
push 0
call [ExitProcess]

include 'global\connect_code.inc'
include 'scenario_main\connect_code.inc'
include 'scenario_simple\connect_code.inc'
include 'system_info\connect_code.inc'
include 'targets_bandwidth\connect_code.inc'
include 'targets_latency\connect_code.inc'
include 'threads_manager\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'scenario_main\connect_const.inc'
include 'scenario_simple\connect_const.inc'
include 'system_info\connect_const.inc'
include 'targets_bandwidth\connect_const.inc'
include 'targets_latency\connect_const.inc'
include 'threads_manager\connect_const.inc'

include 'global\connect_var.inc'
include 'scenario_main\connect_var.inc'
include 'scenario_simple\connect_var.inc'
include 'system_info\connect_var.inc'
include 'targets_bandwidth\connect_var.inc'
include 'targets_latency\connect_var.inc'
include 'threads_manager\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

