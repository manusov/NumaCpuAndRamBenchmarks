;--- Send message BM_GETCHECK, "BM" = Button Message ---------------;
; Use fact: BST_UNCHECKED=0, BST_CHECKED=1, BST_INDETERMINATE=2     ;
;                                                                   ;
; INPUT:   ESI = Index of element handle                            ;
; OUTPUT:  EAX = Message status, f(message type)                    ;
;             for BM_GETCHECK:                                      ;
;             BST_UNCHECKED=0, BST_CHECKED=1, BST_INDETERMINATE=2   ;
;          ZF Flag: 0(NZ) = Status other than BST_CHECKED           ;
;                   1(Z)  = Status is BST_CHECKED                   ; 
;-------------------------------------------------------------------;

SendMessageGetCheck:
push 0                                ; Parm#4 = Not used = 0
push 0                                ; Parm#3 = Not used = 0
push BM_GETCHECK                      ; Parm#2 = Message
push dword [SubHandles - 64 + esi*4]  ; Parm#1 = Handle, Input
call [SendMessage]
cmp eax,1                             ; BST_CHECKED=1, set ZF=1 if this
; Done
ret

