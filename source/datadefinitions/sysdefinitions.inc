;------------------------------------------------------------------------------;
;  System support data definitions: constants, structures, macro definitions.  ;
;     Make this module as separate file for useable by samples under debug     ;
;------------------------------------------------------------------------------;

; Geometry parameters: Window 0 (parent), system information and options
WIN0_XBASE = 100    ; Parent window start X position, pixels, if no auto-center
WIN0_YBASE = 100    ; Parent window start Y position, pixels, if no auto-center
WIN0_XSIZE = 750    ; Parent window start X size, pixels
WIN0_YSIZE = 550+36 ; Parent window start Y size, pix., +36 for 2 debug routines

; Geometry parameters: Window 1 (daughter), drawings Speed = F(Block Size)
WIN1_XBASE = 150    ; Daughter (drawings) window same parameters set 
WIN1_YBASE = 150     
WIN1_XSIZE = 760     
WIN1_YSIZE = 560     

; Geometry parameters: Area with drawings Speed = F(Block Size)
SUBWINX    = 750+2  ; Plot sub-window X size, pixels
SUBWINY    = 480+2  ; Plot sub-window Y size, pixels
GRIDX      = 15     ; Divisor for drawing coordinate X-grid, vertical lines
GRIDY      = 10     ; Divisor for drawing coordinate Y-grid, horizontal lines
SHIFTX     = 1      ; X-shift plot sub window in the dialogue window, pixels
SHIFTY     = 1      ; X-shift plot sub window in the dialogue window, pixels
GRIDSTEPX  = 50     ; Addend for X-grid step
GRIDSTEPY  = 48     ; Addend for Y-grid step

; Color parameters of Sub-Window with drawings Speed = F(Block Size)
; Brush color values = 00bbggrrh, bb=blue, gg=green, rr=red, 1 byte per color
BRUSH_GRID       = 00017001h        ; Grid with horizontal and vertical lines 
BRUSH_LINE       = 0001F0F0h        ; Draw Line Speed = F (Block Size)
BRUSH_BACKGROUND = 00250101h        ; Draw window background
COLOR_TEXT_UNITS = 0001F001h        ; Text front color, units print
COLOR_TEXT_BACK  = BRUSH_BACKGROUND ; Text back color
COLOR_TEXT_INFO  = 00E0E0E0h        ; Text front color, system info print
COLOR_TEXT_DUMP  = 00F04040h        ; Text front color, instruction dump print

; WinAPI equations for DIB (Device Independent Bitmap)
DIB_RGB_COLORS    = 0x00  ; Means mode: color table contain RGB values
DIB_PAL_COLORS    = 0x01  ; Means mode: color tab. is indexes in the pal. table
DIB_PAL_INDICES   = 0x02  ; Means mode: no color table exist, use default
CLEARTYPE_QUALITY = 5     ; Quality code for create font and draw at grap. win. 

; Benchmarks deault Y-sizing parameters
; This parameters set for first pass, 
; auto adjusted as F(Maximum Detected Speed) for next passes,
; if don't close Window 1 and press Run (Resize) button 
; Settings for Cache&RAM mode
Y_RANGE_MAX              = 300000               ; Default Y maximum 
DEFAULT_Y_MBPS_PER_GRID  = Y_RANGE_MAX/10       ; Def. units (MBPS) per greed Y 
DEFAULT_Y_MBPS_PER_PIXEL = Y_RANGE_MAX/SUBWINY  ; Def. units (MBPS) per pixel Y

; Benchmarks visualization timings parameters
TIMER_TICK       = 50     ; Milliseconds per tick, benchmarks progress timer
