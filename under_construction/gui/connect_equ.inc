;------------------------------------------------------------------------------;
;            Element descriptor for user interface window element.             ;
;------------------------------------------------------------------------------;

; Used as dialogue descriptors entry
; Parms list: Class, Name, Xbase, Ybase, Xsize, Ysize, Style
; Short pointers with 16-bit relative offsets used for compact encoding. 

MACRO DIALELEMENT  x1, x2, x4, x5, x6, x7, x3
{
DW  x7, x6, x5, x4       ; Ysize, Xsize, Ybase, Xbase, if Ysize=0 means stop
DD  x3                   ; Window style, numeric constants combination
DW  x2-BasePointGui      ; Short pointer to Object Name, 0=None
DW  x1-BasePointGui      ; Short pointer to Object Class Name
}
; Terminator for list of structures
MACRO DIALSTOP
{
DW  0                    ; Ysize=0 means stop
}


; TODO.
; Change some constants functionality.
; Remove unused constants.

;------------------------------------------------------------------------------;
;       GUI data definitions: constants, structures, macro definitions.        ;
;     Make this module as separate file for useable by samples under debug     ;
;------------------------------------------------------------------------------;

; Geometry parameters: Window 0 (parent), system information and options
WIN0_XBASE = 100    ; Parent window start X position, pixels, if no auto-center
WIN0_YBASE = 100    ; Parent window start Y position, pixels, if no auto-center
WIN0_XSIZE = 755    ; Parent window start X size, pixels
WIN0_YSIZE = 590    ; Parent window start Y size, pixels

; Geometry parameters: Window 1 (daughter), drawings Speed = F(Block Size)
WIN1_XBASE = 150    ; Daughter (drawings) window same parameters set 
WIN1_YBASE = 150     
WIN1_XSIZE = 780     
WIN1_YSIZE = 580     

; Geometry parameters: Area with drawings Speed = F(Block Size)
SUBWINX    = 772    ; Plot sub-window X size, pixels
SUBWINY    = 502    ; Plot sub-window Y size, pixels
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
; auto adjusted as F(Maximum Detected Speed or Latency) for next passes,
; if don't close Window 1 and press Run (Resize) button 
; Settings for Cache&RAM mode
; Speed units = MBPS (Megabytes per Second), Latency units = ns (nanoseconds)
Y_RANGE_MAX              = 300000               ; Default Y maximum 
DEFAULT_Y_MBPS_PER_GRID  = Y_RANGE_MAX/10       ; Default units per greed Y 
DEFAULT_Y_MBPS_PER_PIXEL = Y_RANGE_MAX/SUBWINY  ; Default units per pixel Y

; Benchmarks measurement and visualization timings parameters
TIMER_MEASURE = 15     ; Milliseconds per tick, measurement constant interval  
TIMER_VISUAL  = 50     ; Milliseconds per tick, benchmarks progress visual

