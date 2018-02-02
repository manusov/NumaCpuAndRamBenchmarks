;---------- Print Number Integer.Float ------------------------;
; Float part is 1 char for P1-version                          ;
; INPUT:   EAX = Number value * 10 (for visual float part)     ;
;          RDI = Destination Pointer (flat)                    ;
; OUTPUT:  RDI = New Destination Pointer (flat)                ;
;                modified because string write                 ;
;--------------------------------------------------------------;
FloatPrintP1:
push rax rbx rcx rdx
mov ecx,10
;- cdq
xor edx,edx
div ecx
mov bl,0
call DecimalPrint32   ; Print Integer part = INPUT DIV 10
FloatEntry:
mov al,'.'
stosb
xchg eax,edx 
call DecimalPrint32   ; Print Float part = INPUT MOD 10
pop rdx rcx rbx rax
ret

;---------- Print Number Integer.Float ------------------------;
; Float part is 1 char for P1-version                          ;
; INPUT:   EAX = Number value * 1000 (for visual float part)   ;
;          RDI = Destination Pointer (flat)                    ;
; OUTPUT:  RDI = New Destination Pointer (flat)                ;
;                modified because string write                 ;
;--------------------------------------------------------------;
FloatPrintP3:
push rax rbx rcx rdx
mov ecx,1000
;- cdq
xor edx,edx
div ecx
mov bl,0
call DecimalPrint32   ; Print Integer part = INPUT DIV 10
mov bl,3
jmp FloatEntry
