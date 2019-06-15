;------------------------------------------------------------------------------;
; Benchmarks drawings speed=f(size or address), process close                  ;
; Procedure 3 of 3 = DrawStart, DrawProgress, DrawStop                         ;
;                                                                              ;
; INPUT:  None at registers                                                    ;
;         Statical text strings used                                           ;
;         MPE control and status block used                                    ;
;                                                                              ;
; OUTPUT: None                                                                 ;
;         Registers save/destroy by Microsoft x64 calling convention           ;
;------------------------------------------------------------------------------;

DrawStop:

; Reserved, no summary status yet for drawings window

ret

