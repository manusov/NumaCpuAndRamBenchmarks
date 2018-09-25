;--- Initializing statistics subroutine ----;
; INPUT:   None                             ;
; OUTPUT:  None, variables updated          ;
;-------------------------------------------;

InitStatistics:
xor eax,eax
lea rcx,[StatClksMinimum]
mov [Win1_Running],1       ; This variable: 0=Halted, 1=Draw coordinate grid, 2=Draw Y=F(X)
; Clear statistics
mov [rcx+08],rax           ; Maximum=0
mov [rcx+16],rax           ; Average=0
mov [rcx+24],rax           ; Sum=0
mov [rcx+32],rax           ; Both variables: DrawCounter, DrawPreviousY
dec rax
mov [rcx+00],rax           ;  Minimum = (2^64)-1
ret
