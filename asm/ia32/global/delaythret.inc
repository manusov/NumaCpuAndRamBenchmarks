;---------- Delay used for return from thread routine ---------;
; INPUT:   Nobe                                                ;
; OUTPUT:  Nobe                                                ;  
;--------------------------------------------------------------;
DelayThreadReturn:
push ecx
mov ecx,100000h
loop $
pop ecx
ret
