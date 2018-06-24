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
call StringWrite    ; Print parameter name
pop rsi
lodsd               ; EAX = Number of Local APICs / IOAPICs / NUMA Domains /...
mov bl,0            ; BL = 0, mode for decimal print
mov byte [rdi],'?'  ; Parameter value show "?" if invalid
inc rdi
cmp eax,10000h       ; This limit used for this parameters set
jae @f               ; Skip if value >= 10000h, means invalid and show "?"
dec rdi
call DecimalPrint32  ; Print value, overwrite "?" if valid
@@:
ret

