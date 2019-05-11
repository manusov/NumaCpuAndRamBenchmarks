.window_debug_01:

mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
; sub rsp,32
push rbp

; Set benchmarks options = F (User dialogue results)
call SimpleStart
; Run benchmarks
call SimpleProgress
jc SimpleError
; Show benchmarks results
call SimpleStop


; Message box and wait user input
; xor ecx,ecx	                  ; RCX = Parm#1 = Parent window handle or 0
; lea rdx,[.debug_01_content]   ; RDX = Parm#2 = Pointer to content string
; lea r8,[.debug_01_caption]    ; R8  = Parm#3 = Pointer to caption
; mov r9d,0040h                 ; R9  = Parm#4 = Message box icon type = Info
; call [MessageBoxA]

pop rbp
mov rsp,rbp
ret

; .debug_01_caption  DB  'Debug window',0
; .debug_01_content  DB  'CTRL-ALT-F1',0
