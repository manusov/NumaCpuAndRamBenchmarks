;------------------------------------------------------------------------------;
; Output warning messages box if some optional OS API functions not found      ;
;                                                                              ;
; INPUT:  CF flag = status: 0(NC) = all functions found, skip subroutine       ;
;                           1(C)  = one or some functions not found,           ;
;                                   need generating warning box                ;
;         Statical table used: FunctionsNames as text strings                  ;
;         Builted table used: FunctionsPointers, x[i]=0 = means not found      ;
;                                                                              ;
; OUTPUT: None                                                                 ;
;         Registers save/destroy by Microsoft x64 calling convention           ;
;------------------------------------------------------------------------------;
ShowWarningAPI:
jnc .L9
push rbx rsi rdi
cld
;--- Parameters for text built ---
lea rsi,[MsgWarnAPI]
lea rdi,[TempBuffer]
mov rdx,rdi                          ; RDX = Parm#2 = Pointer to string
call StringWrite
lea rsi,[FunctionsNames]
lea rbx,[FunctionsPointers]
.L0:
mov rax,[rbx]                ; Get current function pointer
add rbx,8
test rax,rax                 ; 0 means invalid pointer
jnz .L1                      ; Go if current pointer valid
call StringWrite             ; This branch - write name of not found function
mov ax,0D0Ah
stosw
jmp .L2
.L1:                         ; This branch - skip name of found function
lodsb
cmp al,0
jne .L1
.L2:
cmp byte [rsi],0
jne .L0                      ; Cycle for functions list
;--- Text termination ---
mov al,0
stosb 
;--- Other parameters ---
xor ecx,ecx	                         ; RCX = Parm#1 = Parent window handle or 0
lea r8,[ProductID]                   ; R8  = Parm#3 = Pointer to caption
mov r9d,MB_ICONWARNING               ; R9  = Parm#4 = Message box icon type
;--- Warning box and wait user input ---
mov rbx,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
call [MessageBoxA]
mov rsp,rbx
;--- Exit ---
pop rdi rsi rbx
.L9:
ret
