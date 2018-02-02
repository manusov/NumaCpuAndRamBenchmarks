;--------------------------------------------------------------
; MSG message descriptor structure (for Win64)
; Name	      Size      Comments
;--------------------------------------------------------------
; hwnd        dq        Window handle , 0 = thread message
; message     dd        Message identifier - for applications
;             dd        Extension, reserved by system
; wParam      dq        Additional information #1
; lParam      dq        Additional information #2
; time	      dd        Time at which the message posted
; pt          POINT     Cursor coordinatesd
; -           dd        Resreved, not specified in the MSDN
;--------------------------------------------------------------
; POINT message descriptor structure (for Win64)
; Name	  Size	 Comments
;--------------------------------------------------------------
; x	 dd	  Cursor X-position
; y	 dd	  Cursor Y-position
;--------------------------------------------------------------

DialogueMsg_Win0     MSG     ; Message structure for wait event, parent win.
DialogueMsg_Win1     MSG     ; Message structure for wait event, child win. 
