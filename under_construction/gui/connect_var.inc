

; Field for custom block size edit
BlockSizeEdit     DQ  ?    ; CUSTOM for selection: L1/L2/L3/L4/Custom


;------------------------------------------------------------------------------;
;               MSG message descriptor structure (for Win64).                  ;
;------------------------------------------------------------------------------;

; Name	      Size      Comments
;--------------------------------------------------------------
; hwnd        dq        Window handle , 0 = thread message
; message     dd        Message identifier - for applications
;             dd        Extension, reserved by system
; wParam      dq        Additional information #1
; lParam      dq        Additional information #2
; time	      dd        Time at which the message posted
; pt          POINT     Cursor coordinatesd
; -           dd        Reserved, not specified in the MSDN
;--------------------------------------------------------------
; POINT message descriptor structure (for Win64), yet not used.
; Name	  Size	 Comments
;--------------------------------------------------------------
; x	 dd	  Cursor X-position
; y	 dd	  Cursor Y-position
;--------------------------------------------------------------

; This variables, defined as (?) located after constants with predefined
; valuse. This method allows reduce executable file size.
; Constants is a part of executable file size, because pre-defined values.
; Variables (for example this) not a part of executable file size, because
; no pre-defined values. 

DialogueMsg_Win0    MSG     ; Message structure for wait event, parent window
DialogueMsg_Win1    MSG     ; Message structure for wait event, child window 


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

