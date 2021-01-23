;------------------------------------------------------------------------------;
;                        Template for unit tests.                              ;
;      Unit test for global library: show some strings and numeric values.     ;
;------------------------------------------------------------------------------;

include 'win32a.inc'
include 'global\connect_equ.inc'

format PE GUI 4.0
entry start
section '.text' code readable executable
start:

cld
lea esi,[MessageText]
lea edi,[TEMP_BUFFER]

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

mov eax,089ABCDEFh
mov edx,001234567h
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

lea esi,[NumData1]
mov ecx,4
mov bx,0300h
@@:
lodsd
xchg edx,eax
lodsd
xchg edx,eax
call DoublePrint
mov ax,' |'
stosw
stosb
loop @b 
mov ax,0A0Dh
stosw

lea esi,[NumData2]
mov ecx,4
mov bl,0FFh
@@:
lodsd
xchg edx,eax
lodsd
xchg edx,eax
call SizePrint64
mov ax,' |'
stosw
stosb
loop @b 
mov ax,0A0Dh
stosw

lea esi,[NumData3]
mov ecx,4
mov bl,1  ; Units = KB
@@:
lodsd
xchg edx,eax
lodsd
xchg edx,eax
call SizePrint64
mov ax,' |'
stosw
stosb
loop @b 
mov ax,0A0Dh
stosw

mov al,0
stosb

push 0                 ; Parm #4 = Message flags
push WinCaption        ; Parm #3 = Caption (upper message)
push TEMP_BUFFER       ; Parm #2 = Message
push 0                 ; Parm #1 = Parent window
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
NumData3     DQ  0 , 1024*15 + 512 , 1024*15 + 256 , 1024*15 + 128 

include 'global\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphics 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

