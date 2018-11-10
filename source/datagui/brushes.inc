;------------------------------------------------------------------------------;
;                             GUI color brushes.                               ;
;------------------------------------------------------------------------------;

; Used for graphics window, drawings Y=F(X)
; Brush color values = 00bbggrrh, bb=blue, gg=green, rr=red, 1 byte per color

BrushesList:
DD  BRUSH_GRID         ; Grid with horizontal and vertical lines 
DD  BRUSH_LINE         ; Draw Line Speed = F (Block Size)
DD  BRUSH_BACKGROUND   ; Draw window background

