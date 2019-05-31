;------------------------------------------------------------------------------;
;         GUI module: definitions for support GUI objects by WinAPI.           ;
;      Formats defined by Microsoft Windows and this application design.       ;
;------------------------------------------------------------------------------;

; Note about used structures:
; UPB  = User Parameters Block, contains user options settings, from widgets
; IPB  = Input Parameters Block, scenario build IPB = f ( UPB, System Parms. )  
; OPB  = Output Parameters Block, results of measurements, benchmarks + TSC
; DRPM = Draw Parameters: visualization control
; DTHP = Draw Thread Parameters: handles, measurements, statistics 

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
; Geometry parameters: Area with drawings Speed = F(Block Size), X parameters
SUBWINX    = 773    ; Plot sub-window X size, pixels
GRIDX      = 11     ; Number of vertical lines in the coordinate X-grid
GRIDBLANKX = 70     ; Coordinate X blank offset for point X=0
GRIDSTEPX  = 64     ; Coordinate addend for X-grid step
SHIFTX     = 0      ; X-shift plot sub window in the dialogue window, pixels
; Continue geometry parameters, Y parameters 
SUBWINY    = 502    ; Plot sub-window Y size, pixels
GRIDY      = 10     ; Number of horizontal lines in the coordinate Y-grid
GRIDBLANKY = 25     ; Coordinate Y blank offset for point Y=0
GRIDSTEPY  = 48     ; Coordinate addend for Y-grid step
SHIFTY     = 0      ; Y-shift plot sub window in the dialogue window, pixels
; Color parameters of Sub-Window with drawings Speed = F(Block Size)
; Brush color values = 00bbggrrh, bb=blue, gg=green, rr=red, 1 byte per color
BRUSH_GRID        = 00C8C8C8h         ; Grid with horizontal and vertical lines 
BRUSH_LINE        = 000101F0h         ; Draw Line Speed = F (Block Size)
BRUSH_BACKGROUND  = 00F9F9F9h         ; Draw window background
BRUSH_STATISTICS  = 00E82121h         ; Statistics table lines 
COLOR_TEXT_VALUES = 00414141h         ; Text front color, grid numeric values print
COLOR_TEXT_UNITS  = BRUSH_STATISTICS  ; Text front color, units print
COLOR_TEXT_BACK   = BRUSH_BACKGROUND  ; Text back color
COLOR_TEXT_INFO   = COLOR_TEXT_UNITS  ; Text front color, system info print
COLOR_TEXT_DUMP   = 0001B001h         ; Text front color, instruction dump print

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
Y_RANGE_MAX_BANDWIDTH = 300000
Y_RANGE_MAX_LATENCY = 100
Y_DIV = 10
DEFAULT_Y_MBPS_PER_GRID = Y_RANGE_MAX_BANDWIDTH / Y_DIV  ; Default units per grid Y , megabytes per second
DEFAULT_Y_NS_PER_GRID = Y_RANGE_MAX_LATENCY / Y_DIV      ; Default units per grid Y , nanoseconds

; Benchmarks measurement and visualization timings parameters
; Unused timer constants disabled.
;- TIMER_MEASURE = 15     ; Milliseconds per tick, measurement constant interval  
;- TIMER_VISUAL  = 50     ; Milliseconds per tick, benchmarks progress visual

; Benchmarks visualization timings parameters
TIMER_TICK       = 50     ; Milliseconds per tick, benchmarks progress timer

; Draw parameters layout declaration
; Note optimal layout is qwords alignment 8
; DRPM = Draw Parameters: visualization control
struct DRPM
; Benchmarks units control
; Used indexed access from this base, include next groups, don't reorder variables!
ValueGridX         dd  ?      ; Units per horizontal grid cell
ValueGridY         dd  ?      ; Units per vertical grid cell
SelectUnits        dd  ?      ; Units select: 0=Bytes, 1=Kilobytes, 2=Megabytes
SelectMode         dd  ?      ; Measurement mode: 0=Bandwidth, 1=Latency
; Drawings X-counter and X-drawings support
TimerCount         dd  ?      ; Timer ticks count ; OLD = Pixels counter for X-progress when drawing
DrawPreviousY      dd  ?      ; Previous coordinate for vertical lines draw if required, when ABS(X(i)-X(i+1)) > 1
; Benchmark drawings scale parameters
; Better store value for multiply (not divide) at each iteration, for minimize CPU resources utilization
; A / B  replace to:  A * C , when C = 1/B. Store C.
; Also, vertical offset must be negative, upper means smaller offset, biggest MBPS/ns value
Ymultiplier        dq  ?      ; Y pixels scale factor, floating point, double
ends

; Number of pixels by X, used for drawings, means number of measurements per draw
DRAW_POINTS_COUNT  EQU  640

; Benchmarking drawings measurement parallel thread state structure.
; DTHP = Draw Thread Parameters: handles, measurements, statistics
struct DTHP

EventStart       dq  ?   ; Event handle for signaling this thread starts
EventDone        dq  ?   ; Event handle for signaling this thread terminates 
ThreadHandle     dq  ?   ; Event handle for this thread 
MeasureCounter   dd  ?   ; Number of stored results, 0 means no stores before run, maximum MEASURE_POINTS_COUNT (640)
VisualCounter    dd  ?   ; Number of visualized results, required because results generation and timer ticks is asynchronous
MeasureBreak     dd  ?   ; Flag for measurement break, 0=None,   BIDIRECTIONAL SIGNALING IS REJECTED BECAUSE RESIZE BUTTON BUG: 1=Break or Done, signaling is BIDIRECTIONAL: Break and Done
MeasureAlign     dd  ?   ; This required for QWORD alignment

; Note qwords Min, Max, Average, Median must be SEQUENTAL for pointers advance,
; see gui\win1.inc statistics values write 

StatCpiMin       dq  ?   ; Statistics for CPI (Clocks per Instruction), CPI minimum
StatCpiMax       dq  ?   ; CPI maximum 
StatCpiAverage   dq  ?   ; CPI average value
StatCpiMedian    dq  ?   ; CPI median value, detected by numbers ordering
StatCpiSum       dq  ?   ; Service parameter: values sum for averaging CPI

StatNspiMin      dq  ?   ; Statistics for nsPI (Nanoseconds per Instruction)
StatNspiMax      dq  ?
StatNspiAverage  dq  ?
StatNspiMedian   dq  ?
StatNspiSum      dq  ?

StatMbpsMax      dq  ?   ; Statistics for MBPS (Megabytes per Second)
StatMbpsMin      dq  ?   ; Note Min/Max here swapped, because min. time means max. bandwidth
StatMbpsAverage  dq  ?
StatMbpsMedian   dq  ?
StatMbpsSum      dq  ?

MeasureArray     dq  DRAW_POINTS_COUNT dup (?)  ; Array of measurements results, double precision floating point, 64-bit, [delta TSC]
                                                ; under measurement ordered for median, remember it before get value for drawings     
ends

