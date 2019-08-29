;---------- Delay used for return from thread routine ---------;
; INPUT:   Nobe                                                ;
; OUTPUT:  Nobe                                                ;  
;--------------------------------------------------------------;

; RESERVED, BECAUSE CONSTANT DELAY REPLACED TO WAIT THREAD TERMINATION.

;- DelayThreadReturn:
;- push ecx
;- mov ecx,100000h  ; not changed even for debug version v1.02.38 / 39
;- loop $
;- pop ecx
;- ret
