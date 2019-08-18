;---------- Delay used for return from thread routine ---------;
; INPUT:   Nobe                                                ;
; OUTPUT:  Nobe                                                ;  
;--------------------------------------------------------------;
DelayThreadReturn:
push ecx
mov ecx,100000h  ; not changed even for debug version v1.02.38
loop $
pop ecx
ret
