;------------------------------------------------------------------------------;
;                         Template for unit tests.                             ;
;    Unit test for Simple benchmark scenario: build blank text report only,    ;
;                results is NOT VALID, blank report build only.                ;
;------------------------------------------------------------------------------;

; TODO. Bytes/KB/MB/GB units select criteria for all sizes output, 
; synchronize units mode for all output strings,
; local decision with different units can be non-ergonomic 

include 'win32a.inc'
include 'global\connect_equ.inc'
include 'scenario_simple\connect_equ.inc'

format PE GUI 4.0
entry start
section '.text' code readable executable
start:

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

lea ebx,[InputParms]

mov [ebx+IPB.UpdatedAsm],34  ; 35  ; 12
mov [ebx+IPB.OperandWidth],256
mov [ebx+IPB.UpdatedThreads],1
mov [ebx+IPB.UpdatedHT],1
mov [ebx+IPB.UpdatedLP],2
mov [ebx+IPB.UpdatedNUMA],3
mov [ebx+IPB.UpdatedPG],2
mov [ebx+IPB.UpdatedTarget],4
mov [ebx+IPB.UpdatedMeasure],3
mov [ebx+IPB.StartBlockSize],24576

mov dword [ebx+IPB.MeasureRepeats + 0],100000
mov dword [ebx+IPB.MeasureRepeats + 4],0

mov [ebx+IPB.AllocatedBlock1],10000h
mov [ebx+IPB.AllocatedBlock2],20000h
mov [ebx+IPB.MemoryTotal],( 1024*15 + 512 ) * 2
mov [ebx+IPB.MemoryPerThread],1024*15 + 512

lea ebx,[OutputParms]

mov dword [ebx+OPB.OStimerDelta + 0],1573000
mov dword [ebx+OPB.OStimerDelta + 4],0

mov dword [ebx+OPB.TSCtimerDelta + 0],370000000
mov dword [ebx+OPB.TSCtimerDelta + 4],0

mov eax,dword [Data_1 + 0]
mov edx,dword [Data_1 + 4]
mov dword [ebx+OPB.TSCperiodNs + 0],eax
mov dword [ebx+OPB.TSCperiodNs + 4],edx

mov eax,dword [Data_2 + 0]
mov edx,dword [Data_2 + 4]
mov dword [ebx+OPB.TSCfrequencyHz + 0],eax
mov dword [ebx+OPB.TSCfrequencyHz + 4],edx

; call routines from "Run simple" scenario
call ResultSimple 

; input parameters for build message box
push 0                  ; Parm#4 = Message box icon type
push PRODUCT_ID         ; Parm#3 = Pointer to caption
push TEMP_BUFFER        ; Parm#2 = Pointer to string
push 0                  ; Parm#1 = Parent window handle or 0
call [MessageBoxA]

; exit
push 0
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

