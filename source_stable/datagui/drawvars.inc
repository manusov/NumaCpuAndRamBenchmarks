;------------------------------------------------------------------------------;
;               Variables for Window 1 - benchmarks drawings.                  ;
;------------------------------------------------------------------------------;

; Variables for graphics context
; This variables, defined as (?) located after constants with predefined
; valuse. This method allows reduce executable file size.
; Constants is a part of executable file size, because pre-defined values.
; Variables (for example this) not a part of executable file size, because
; no pre-defined values. 

; Video output control            ; 64 bytes per group
HandleMemDC        DQ  ?          ; Handle for Device Context, video controller
BitmapPointer      DQ  ?          ; Bitmap pointer
HandleBitmap       DQ  ?          ; Handle of bitmap for graphics draw
HandlesBrushes     DQ  3 DUP (?)  ; Handle for color brushes
HandleFont         DQ  ?          ; Handle for font in the drawings window
HandleDC           DQ  ?          ; Handle Graphical Device Context

;--- PAINTSTRUCT - Paint Control structure (for Win64) ---
; Name          Size            Comments
;--------------------------------------------------------------
; hdc           dq ?            Handle to display DEVICE CONTEXT, 
;                               to be used for painting
; fErase        dd ?            If nonzero - application responsible
;                               for erasing the background
; rcPaint       RECT            Rectangle for drawings region
; fRestore      dd ?            Reserved, internally used by system
; fIncUpdate    dd ?            Reserved, internally used by system
; rgbReserved   db 36 dup (?)   Reserved, internally used by system
;--------------------------------------------------------------
; RECT - Rectangle structure (for Win64)
; Name          Size       Comments
;--------------------------------------------------------------
; left          dd ?       X-coordinate of upper-left point
; top           dd ?       Y-coordinate of upper-left point
; right         dd ?       X-coordinate of lower-right point
; bottom        dd ?       Y-coordinate of lower-right point
;--------------------------------------------------------------

PaintStruc       PAINTSTRUCT   ; Paint control
GraphRect        RECT          ; Rectangle definition for visualized area
