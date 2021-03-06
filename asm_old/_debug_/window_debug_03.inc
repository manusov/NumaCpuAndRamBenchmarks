.window_debug_03:

mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32

; Message box and wait user input
xor ecx,ecx	                  ; RCX = Parm#1 = Parent window handle or 0
lea rdx,[.debug_03_content]   ; RDX = Parm#2 = Pointer to content string
lea r8,[.debug_03_caption]    ; R8  = Parm#3 = Pointer to caption
mov r9d,0040h                 ; R9  = Parm#4 = Message box icon type = Info
call [MessageBoxA]

mov rsp,rbp
ret

.debug_03_caption  DB  'Debug window',0
.debug_03_content  DB  'CTRL-ALT-F3',0


