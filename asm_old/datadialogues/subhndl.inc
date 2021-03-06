;------------------------------------------------------------------------------;
;                        Object with space sharing.                            ;
;------------------------------------------------------------------------------;

; This variables, defined as (?) located after constants with predefined
; valuse. This method allows reduce executable file size.
; Constants is a part of executable file size, because pre-defined values.
; Variables (for example this) not a part of executable file size, because
; no pre-defined values. 

; SubHandles is buffer for true handles (RAX after create)
; dialogue window objects (elements).
; At this table global Handle 00 associated with local handle 10h
; 128 qwords = 128*8 = 1024 bytes = 1KB per window
SubHandles         DQ  128 DUP (?)   ; 1024 bytes, Window 0 = Main
                   DQ  128 DUP (?)   ; 1024 bytes, Window 1 = Drawings

; BigTexts is buffer for dynamically build text strings.
BigTexts           DB  1024 DUP (?)  ; 1024 bytes, Big texts

