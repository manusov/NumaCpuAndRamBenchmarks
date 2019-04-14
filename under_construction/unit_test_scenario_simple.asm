; TODO. Bytes/KB/MB/GB units select criteria for all sizes output, 
; synchronize units mode for all output strings,
; local decision with different units can be non-ergonomic 

; Template for unit tests.
; Unit test for Simple benchmark scenario. 

include 'win64a.inc'
include 'global\connect_equ.inc'
include 'scenario_simple\connect_equ.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

; initializing stack frame
sub rsp,8*5

; cld
; lea rsi,[MessageText]
; lea rdi,[TEMP_BUFFER]
; ; ...
; call StringWrite
; mov al,0
; stosb

; lea rdx,[TEMP_BUFFER]  ; RDX = Parm #2 = Message
; lea r8,[WinCaption]    ; R8  = Parm #3 = Caption (upper message)
; xor r9d,r9d            ; R9  = Parm #4 = Message flags
; xor ecx,ecx            ; RCX = Parm #1 = Parent window
; call [MessageBoxA]

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

mov rax,[Data_1]
mov [rbx+OPB.TSCperiodNs],rax
mov rax,[Data_2]
mov [rbx+OPB.TSCfrequencyHz],rax

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
include 'scenario_simple\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'scenario_simple\connect_const.inc'

Data_1  DQ  0.5
Data_2  DQ  1000000000.0  

; WinCaption   DB ' Unit test for Simple benchmark scenario' , 0
; MessageText  DB 'Template text...' , 0

include 'global\connect_var.inc'
include 'scenario_simple\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

