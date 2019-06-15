;------------------------------------------------------------------------------;
;                         Template for unit tests.                             ;
;                   Unit test for system information routines:                 ;
;                    compiling only, without show information.                 ;
;------------------------------------------------------------------------------;

include 'win32a.inc'
include 'global\connect_equ.inc'
include 'system_info\connect_equ.inc'

format PE GUI 4.0
entry start
section '.text' code readable executable
start:

cld
lea esi,[MessageText]
lea edi,[TEMP_BUFFER]

; ... for run under debugger ...

;- call SystemFunctionsLoad

;- call CheckCpuId

;- call SystemFunctionsLoad
;- lea esi,[edi+1024]
;- call GetAcpiMadt

;- call GetCpuName

;- call SystemFunctionsLoad
;- call GetLargePagesInfo

;- call SystemFunctionsLoad
;- call GetOsMemoryInfo

;- call SystemFunctionsLoad
;- call GetOsNumaInfo

;- call SystemFunctionsLoad
;- call GetProcessorGroups

;- call SystemFunctionsLoad
;- lea esi,[edi+1024]
;- call GetOsTopologyInfo

;- call MeasureTsc

; ...

call StringWrite
mov al,0
stosb

push 0             ; Parm #4 = Message flags
push WinCaption    ; Parm #3 = Caption (upper message)
push TEMP_BUFFER   ; Parm #2 = Message
push 0             ; Parm #1 = Parent window
call [MessageBoxA]

push 0
call [ExitProcess]

include 'global\connect_code.inc'
include 'system_info\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'system_info\connect_const.inc'

WinCaption   DB ' Unit test for system information routines' , 0
MessageText  DB 'Build-only test...' , 0

include 'global\connect_var.inc'
include 'system_info\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

