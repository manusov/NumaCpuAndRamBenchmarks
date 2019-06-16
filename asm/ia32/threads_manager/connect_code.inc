;------------------------------------------------------------------------------;
;   Library for memory and threads management, include adjust measurements:    ;
;                    connect include files with subroutines.                   ;
;------------------------------------------------------------------------------;

include 'memallocationnuma.inc'   ; NUMA-aware and unaware memory allocation
include 'memreleasenuma.inc'      ; NUMA-aware and unaware memory release

include 'buildnumanodeslist.inc'  ; Detect topology, build list of NUMA nodes
include 'buildthreadslist.inc'    ; Detect topology, build threads list
include 'releasethreadslist.inc'  ; Release threads handles

include 'runtarget.inc'           ; Run performance target routine 
include 'threadroutine.inc'       ; Child thread routine
include 'threadsstart.inc'        ; Start child threads execution, first run
include 'threadsrestart.inc'      ; Re-start child threads execution 
include 'updatethreadslist.inc'   ; Modify threads parameters for exec. cycle

include 'sessionstart.inc'        ; Initializing, start benchmark session 
include 'sessionstop.inc'         ; Close benchmark session
include 'sessionprogress.inc'     ; Execute benchmark session

