;------------------------------------------------------------------------------;
;              Get system parameters for main window scenario:                 ;
;              constants and variables with pre-defined values.                ;
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
;     String for non-fatal warning, visualized if application can start.       ;
;                  See also fatal errors: file errormsg.inc.                   ;
;------------------------------------------------------------------------------;

; Application can start with this warnings

MsgWarnAPI  DB  'WARNING: system is not fully NCRB-compatible,',0Dh,0Ah
            DB  'missing OS API functions list:',0Dh,0Ah,0Dh,0Ah,0
