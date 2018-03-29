;-------------------------------------------------------------------;
; Drawings Y=F(X) Benchmarks Execution.                             ;
;                                                                   ;
; INPUT:   Control context initialized by options settings.         ;   
; OUTPUT:  None                                                     ;
;                                                                   ;
;-------------------------------------------------------------------;

ExecuteDraw:

; Open context, set benchmarks options = F (User dialogue results)
call DrawStart
; Execution phase, draw Y=F(X), SPEED=F(SIZE or ADDRESS)
call DrawProgress
jc SimpleError
; Close context
call DrawStop
ret

