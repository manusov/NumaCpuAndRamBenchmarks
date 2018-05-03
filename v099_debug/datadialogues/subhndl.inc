;--- Object with space sharing ------------------------------------------------;

; SubHandles is buffer for true handles (RAX after create)
; dialogue window objects.
; At this table global Handle 00 associated with local handle 10h
; 128 qwords = 128*8 = 1024 bytes = 1KB per window
SubHandles         DQ  128 DUP (?)   ; 1024 bytes, Window 0
                   DQ  128 DUP (?)   ; 1024 bytes, Window 1

; BigTexts is buffer for dynamically build text strings.
BigTexts           DB  1024 DUP (?)  ; 1024 bytes, Big texts

