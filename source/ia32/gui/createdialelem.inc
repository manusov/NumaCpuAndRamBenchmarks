;--- Cycle for create all dialogue elements ------------------------;
;                                                                   ;
; INPUT:   EBX = Parent window handle                               ;
;          ESI = Pointer to elements descriptors list               ;
;          EDI = Start (numeric) handle for sequence                ;
;          EBP = Parent (true) module handle = [wc.hInstance]       ;
;                                                                   ;
; OUTPUT:  None                                                     ;
;-------------------------------------------------------------------;

CreateDialogueElements:
cld
; Start cycle for create elements
.BuildCycle:
xor ecx,ecx            ; ECX = 0 for some parameters push
xor eax,eax            ; EAX = 0 for compact blank at 16-bit loads
lodsw	                 ; Get X7 from macro
test eax,eax
jz .EndBuildWin        ; Go exit sequence if end detected, Ysize=0
push ecx               ; Parm#12 = Pointer to passed parameters = 0, means none
push ebp               ; Parm#11 = Parent module handle = [wc.hInstance]
push edi               ; Parm#10 = Handle to menu, if used for this window type = 0, means none
push ebx               ; Parm#9  = Parent handle, if = 0 , means default
push eax               ; Parm#8  = Y size, at this point bits EAX.[31-16]=0
lodsw
push eax               ; Parm#7  = X size
lodsw
push eax               ; Parm#6  = Y base
lodsw
push eax               ; Parm#5  = X base
lodsd
push eax               ; Parm#4  = Window style = WS_VISIBLE+WS_DLGFRAME+WS_SYSMENU
xor eax,eax
lodsw
test eax,eax
jz @f
add eax,BasePointGui
@@:
push eax               ; Parm#3  = Pointer to Window name = WindowTitle
xor eax,eax
lodsw
add eax,BasePointGui
push eax               ; Parm#2  = Pointer to Window class name = ClassName
push ecx               ; Parm#1  = Extended window style = 0 , means none
call [CreateWindowEx]
mov [SubHandles - 16*4 + edi*4 ],eax
inc edi                ; Element handle +1
test eax,eax
jnz .BuildCycle        ; Cycle for all descriptors, exit if error when EAX=0
.EndBuildWin:
; End cycle for create elements
ret

