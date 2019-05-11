;------------------------------------------------------------------------------;
;                         Template for unit tests.                             ;
;                   Unit test for system information routines:                 ;
;                    compiling only, without show information.                 ;
;------------------------------------------------------------------------------;

include 'win64a.inc'
include 'global\connect_equ.inc'
include 'system_info\connect_equ.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

sub rsp,8*5

cld
lea rsi,[MessageText]
lea rdi,[TEMP_BUFFER]

; ...

call StringWrite
mov al,0
stosb

lea rdx,[TEMP_BUFFER]  ; RDX = Parm #2 = Message
lea r8,[WinCaption]    ; R8  = Parm #3 = Caption (upper message)
xor r9d,r9d            ; R9  = Parm #4 = Message flags
xor ecx,ecx            ; RCX = Parm #1 = Parent window
call [MessageBoxA]

xor ecx,ecx
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

