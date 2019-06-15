;--- Wait user event after open window -----------------------------;
;    OS make callbacks WindowProc during this wait.                 ;
; INPUT:  EDI = Pointer to DialMsg structure                        ;
; OUTPUT: None                                                      ;
;-------------------------------------------------------------------;

WaitEvent:
; Start event wait and handling cycle
.WaitEventCycle:
xor eax,eax               ; This for compact push 0
push eax                  ; Parm#4 = 0
push eax                  ; Parm#3 = 0
push eax                  ; Parm#2 = 0
push edi                  ; Parm#1 = DialMsg
call [GetMessage]
cmp eax,1		     
jb .WaitEventDone         ; Go if EAX=0, exit program
jne .WaitEventCycle       ; Go if EAX>1, wait for event
push edi                  ; Parm#1 = DialMsg at this point if EAX=1,
call [TranslateMessage]   ; process event
push edi                  ; Parm#1 = DialMsg
call [DispatchMessage]
jmp .WaitEventCycle
; End event wait and handling cycle
.WaitEventDone:
ret
