;---------- Write ACPI Numeric Info (Name=Value) --------------;
; INPUT:   RSI = Source pointer to System Info buffer          ;
;          RBX = Source pointer to Name string                 ;
;          RDI = Destination pointer                           ;
;                                                              ;
; OUTPUT:  RSI = Updated RSI after Read data, used for next    ;
;          RDI = Updated RDI after Write data, used for next   ;
;--------------------------------------------------------------;
AcpiNumberPrint:
push rsi
mov rsi,rbx
call StringWrite
pop rsi
lodsd               ; EAX = Number of Local APICs / IOAPICs / NUMA Domains /...
mov bl,0            ; BL = 0, mode for decimal print
mov byte [rdi],'?'
inc rdi
cmp eax,10000h	 
jae @f
dec rdi
call DecimalPrint32
@@:
ret

