;-------------------------------------------------------------------;
; Send message BM_GETCHECK, "BM" = Button Message.                  ;
; Use fact: BST_UNCHECKED=0, BST_CHECKED=1, BST_INDETERMINATE=2     ;
; INPUT:   RSI = Index of element handle                            ;
; OUTPUT:  RAX = Message status, f(message type)                    ;
;             for BM_GETCHECK:                                      ;
;             BST_UNCHECKED=0, BST_CHECKED=1, BST_INDETERMINATE=2   ;
;          ZF Flag: 0(NZ) = Status other than BST_CHECKED           ;
;                   1(Z)  = Status is BST_CHECKED                   ; 
;-------------------------------------------------------------------;

SendMessageGetCheck:
mov rbp,rsp                  ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                      ; Make parameters shadow
; Operation                 
mov rcx,[SubHandles-128+rsi*8]  ; RCX = Parm#1 = Handle, Input
mov edx,BM_GETCHECK             ; RDX = Parm#2 = Message
xor r8d,r8d                     ; R8  = Parm#3 = Not used = 0
xor r9d,r9d                     ; R9  = Parm#4 = Not used = 0
call [SendMessage]
cmp rax,1                       ; BST_CHECKED=1, set ZF=1 if this
; Done
mov rsp,rbp
ret

