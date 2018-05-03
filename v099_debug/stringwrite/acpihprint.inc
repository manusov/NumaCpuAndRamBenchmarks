;---------- Write ACPI Header ---------------------------------;
; INPUT:   RSI = Source pointer to Header Name string          ;
;          RBX = Source pointer to ACPI tags and rev. dword    ;
;          RDI = Destination pointer                           ;
;          RCX = Must be 0 at this context                     ;
;                                                              ;
; OUTPUT:  RSI = Updated RBX after Read data, used for next    ;
;          RDI = Updated RDI after Write data, used for next   ;
;--------------------------------------------------------------;
AcpiHeaderPrint:
call StringWrite   ; Print ACPI table name, examples = "MADT tags", "SRAT tags"
mov ax,'  '
stosw
stosw
stosw
stosb
mov rsi,rbx          ; Load second source pointer
mov cl,6             ; Load entire RCX not required, it 0  
rep movsb            ; Part 1 of tags
stosw                ; Interval = 2 spaces
mov cl,8
rep movsb            ; Part 2 of tags
stosw
stosw
push rsi
lea rsi,[TextOemRev]
call StringWrite     ; Print "OEM Rev="
pop rsi
lodsd
call HexPrint32      ; OEM Rev value, hex
mov al,'h'
stosb
mov ax,'  '
stosw
stosw
stosb
ret

