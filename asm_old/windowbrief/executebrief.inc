;-------------------------------------------------------------------;
; Brief vector features analysing and benchmarks execution.         ;
;                                                                   ;
; INPUT:   Control context initialized by options settings,         ;
;          reserved, this context not used for this function        ;
; OUTPUT:  None                                                     ;
;                                                                   ;
;-------------------------------------------------------------------;

ExecuteBrief:

; Reserved for set benchmarks options = F (User dialogue results),
; but user dialogue results not used for vector brief test
call VectorBriefStart
; Run features detection and benchmarks
call VectorBriefProgress
jc SimpleError
; Show features detection and benchmarks results
call VectorBriefStop
ret

