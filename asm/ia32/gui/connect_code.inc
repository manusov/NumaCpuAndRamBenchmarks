;------------------------------------------------------------------------------;
;             GUI module: connect include files with subroutines.              ;
;------------------------------------------------------------------------------;

include 'createappwin.inc'          ; Create dialogue window
include 'createdialelem.inc'        ; Create widgets for dialogue window
include 'setdialdefaults.inc'       ; Set benchmarks options defaults
include 'waitevent.inc'             ; Wait user events with handling cycle 
;include 'sysparmstobuffer.inc'      ; Extract system parameters for visual
;include 'sysparmstogui.inc'         ; Extract system parameters for GUI
include 'guitouserparms.inc'        ; Extract GUI state for benchmarks options
include 'sendmsggc.inc'             ; Get state of GUI toggler
include 'win0.inc'                  ; Callback for main (parent) window events
include 'win1.inc'                  ; Callback for draw (child) window events

include 'drawsessionstart.inc'      ; Initializing measurement+draw session
include 'drawsessionstop.inc'       ; Close measurement+draw session
;include 'drawsessionthread.inc'     ; Child thread for measurements,
                                    ; runned by drawsessionstart.inc

include 'runtimeerror.inc'          ; Error message box for benchmarks errors