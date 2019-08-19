;---------- Delay used for return from thread routine ---------;
; INPUT:   Nobe                                                ;
; OUTPUT:  Nobe                                                ;  
;--------------------------------------------------------------;

; DelayThreadReturn:
; push rcx
; mov ecx,1400000h  ; old = 100000h, debug version v1.02.38
; loop $
; pop rcx
; ret

DelayThreadSmall:
push rcx
mov ecx,100000h  ; This for termination GUI scenario child thread
loop $
pop rcx
ret

DelayThreadBig:
push rcx
mov ecx,1400000h  ; This for termination target measurement child threads
loop $
pop rcx
ret
