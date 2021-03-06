.window_debug_10:

mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32

; Message box and wait user input
xor ecx,ecx	                  ; RCX = Parm#1 = Parent window handle or 0
lea rdx,[.debug_10_content]   ; RDX = Parm#2 = Pointer to content string
lea r8,[.debug_10_caption]    ; R8  = Parm#3 = Pointer to caption
mov r9d,0040h                 ; R9  = Parm#4 = Message box icon type = Info
call [MessageBoxA]

mov rsp,rbp
ret

.debug_10_caption  DB  'Debug window',0
.debug_10_content  DB  'CTRL-ALT-F10',0


