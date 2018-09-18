;-------------------------------------------------------------------;
; Wait user event after open window.                                ;
; OS make callbacks WindowProc during this wait.                    ;
; INPUT:  RDI = Pointer to DialMsg structure                        ;
; OUTPUT: None                                                      ;
;-------------------------------------------------------------------;
WaitEvent:
mov rbp,rsp                 ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h  ; Align stack
sub rsp,32			            ; Make parameters shadow
; Start event wait and handling cycle
WaitEventCycle:
mov rcx,rdi                 ; Parm#1 = DialMsg
xor edx,edx                 ; Parm#2 = 0
xor r8d,r8d                 ; Parm#3 = 0
xor r9d,r9d	                ; Parm#4 = 0
call [GetMessage]
cmp rax,1		     
jb @f                       ; Go if RAX=0, exit program
jne WaitEventCycle          ; Go if RAX>1, wait for event
mov rcx,rdi                 ; At this point if RAX=1,
call [TranslateMessage]     ; process event
mov rcx,rdi
call [DispatchMessage]
jmp WaitEventCycle
; End event wait and handling cycle
@@:
mov rsp,rbp
ret


