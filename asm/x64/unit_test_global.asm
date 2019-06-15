;------------------------------------------------------------------------------;
;                        Template for unit tests.                              ;
;      Unit test for global library: show some strings and numeric values.     ;
;------------------------------------------------------------------------------;

include 'win64a.inc'
include 'global\connect_equ.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

sub rsp,8*5

cld
lea rsi,[MessageText]
lea rdi,[TEMP_BUFFER]

call StringWrite
mov ax,0A0Dh
stosw

mov eax,123456789
mov bl,0
call DecimalPrint32
mov ax,' |'
stosw
stosb
mov bl,4
mov eax,5
call DecimalPrint32
mov ax,' |'
stosw
stosb
mov bl,0
xor eax,eax
call DecimalPrint32
mov ax,0A0Dh
stosw

mov rax,0123456789ABCDEFh
call HexPrint64
mov ax,' |'
stosw
stosb
mov eax,98765432h
call HexPrint32
mov ax,' |'
stosw
stosb
mov ax,1986h
call HexPrint16
mov ax,' |'
stosw
stosb
mov al,64h
call HexPrint8
mov ax,' |'
stosw
stosb
mov al,3
call HexPrint4
mov ax,0A0Dh
stosw

lea rsi,[NumData1]
mov ecx,4
mov bx,0300h
@@:
lodsq
call DoublePrint
mov ax,' |'
stosw
stosb
loop @b 
mov ax,0A0Dh
stosw

lea rsi,[NumData2]
mov ecx,4
mov bl,0FFh
@@:
lodsq
call SizePrint64
mov ax,' |'
stosw
stosb
loop @b 
mov ax,0A0Dh
stosw

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

section '.data' data readable writeable

include 'global\connect_const.inc'

WinCaption   DB ' Unit test for global library' , 0
MessageText  DB 'Text for StringWrite routine verify...' , 0
NumData1     DQ  1.0 , -1.0 , 1985.764 , 0.0
NumData2     DQ  0 , 1024*1024 + 1024 , 1024*1024*1024 , 1536

include 'global\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

