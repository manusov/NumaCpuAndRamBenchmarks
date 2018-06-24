;--- BITMAPINFOHEADRE descriptor structure (for Win64) ---
; Used when initializing graphics window context, drawings Y=F(X)
;--------------------------------------------------------------
; Name		          Size    Comments
;--------------------------------------------------------------
; biSize           dd ?   Number of bytes
; biWidth          dd ?   Width
; biHeight         dd ?   Height
; biPlanes         dw ?   Bit Planes
; biBitCount       dw ?   Bits Per Pixel
; biCompression    dd ?   Compression Mode
; biSizeImage      dd ?   Image size, not required for uncompressed
; biXPelsPerMeter  dd ?   X pixels per meter, here not required
; biYPelsPerMeter  dd ?   Y pixels per meter, here not required
; biClrUsed        dd ?   Number of colors used, here not required (true color)
; biClrImportant   dd ?   Number of important colors, 0=All, here not used
;--------------------------------------------------------------

BitmapInfo    BITMAPINFOHEADER 40,SUBWINX,SUBWINY,1,32,BI_RGB,0,0,0,0,0

