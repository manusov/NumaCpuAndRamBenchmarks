;---------- Delay used for return from thread routine ---------;
; INPUT:   Nobe                                                ;
; OUTPUT:  Nobe                                                ;  
;--------------------------------------------------------------;
DelayThreadReturn:
push rcx
mov ecx,100000h
loop $
pop rcx
ret
