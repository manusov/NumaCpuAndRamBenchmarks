;---------- Delay used for return from thread routine ---------;
; INPUT:   Nobe                                                ;
; OUTPUT:  Nobe                                                ;  
;--------------------------------------------------------------;
DelayThreadReturn:
push rcx
mov ecx,1400000h  ; old = 100000h, debug version v1.02.38
loop $
pop rcx
ret
