;-------------------------------------------------------------------;
; Cycle for create all dialogue elements                            ;
; INPUT:   RSI = Pointer to elements descriptors list               ;
;          RDI = Start (numeric) handle for sequence                ;
;          R14 = Parent (true) module handle = [wc.hInstance]       ;
;                                                                   ;
; OUTPUT:  None                                                     ;
;-------------------------------------------------------------------;

CreateDialogueElements:
lea r15,[BasePointGui]
mov rbp,rsp                  ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack because Win API used 
cld
; Start cycle for create elements
.BuildCycle:
xor ecx,ecx                  ; RCX = 0 for some parameters push
xor eax,eax                  ; This clear entire RAX, but compact blank
lodsw	                       ; Get X7 from macro
test eax,eax
jz .EndBuildWin              ; Go exit sequence if end detected, Ysize=0
push rcx                     ; Parm#12 = Pointer to passed parameters = 0, means none
push r14                     ; Parm#11 = Parent module handle = [wc.hInstance]
push rdi                     ; Parm#10 = Handle to menu, if used for this window type = 0, means none
push rbx                     ; Parm#9  = Parent handle = 0 , means default
push rax                     ; Parm#8  = Y size, at this point bits RAX.[63-16]=0
lodsw
push rax                     ; Parm#7  = X size
lodsw
push rax                     ; Parm#6  = Y base
lodsw
push rax                     ; Parm#5  = X base
lodsd
xchg r9,rax                  ; Parm#4  = Window style = WS_VISIBLE+WS_DLGFRAME+WS_SYSMENU
xor eax,eax
lodsw
mov r8,rax
test eax,eax
jz @f
add r8,r15                   ; Parm#3  = Pointer to Window name = WindowTitle
@@:
lodsw
lea rdx,[r15+rax]            ; Parm#2  = Pointer to Window class name = ClassName
sub rsp,32                   ; Build frame, note RCX = Parm#1  = Extended window style = 0 , means none
call [CreateWindowEx]
add rsp,32+8*8
mov [r15-BasePointGui+SubHandles-128+rdi*8],rax
inc rdi                      ; Element handle +1
test rax,rax
jnz .BuildCycle              ; Cycle for all descriptors, exit if error when RAX=0
.EndBuildWin:
; End cycle for create elements
ExitRbp:
mov rsp,rbp
ret

