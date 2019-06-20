;------------------------------------------------------------------------------;
;              Get system parameters for main window scenario:                 ;
;              constants and variables with pre-defined values.                ;
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
;     String for non-fatal warning, visualized if application can start.       ;
;                  See also fatal errors: file errormsg.inc.                   ;
;------------------------------------------------------------------------------;

; Application can start with this warnings
; Note WoW means Windows-on-Windows, 32-bit application under Win64.

MsgWarnAPI  DB  'WARNING: system is not fully NCRB-compatible,',0Dh,0Ah
            DB  'missing OS API functions list:',0Dh,0Ah,0Dh,0Ah,0

MsgWarnWoW  DB  'WARNING: NCRB32 runs under Win64,',0Dh,0Ah
            DB  'NCRB64 is optimal for this platform',0
