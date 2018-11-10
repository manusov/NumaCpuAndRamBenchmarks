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
