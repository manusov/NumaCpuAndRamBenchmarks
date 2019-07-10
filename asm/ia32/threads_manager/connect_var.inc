;------------------------------------------------------------------------------;
;   Library for memory and threads management, include adjust measurements:    ;
;                   variables without pre-defined values.                      ;
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
;     MPECSB = Memory Performance Engine control and status block (CSB).       ;
;------------------------------------------------------------------------------;
; Benchmarks input and output operands located by alignment-optimal ordering
; and 64-byte cache line allocation
; Threads management lists, must be sequentally located, this fact used in code,
; events list must be sequental, this fact used by OS events API. 
align 64
ThreadsList      DB  THREADS_LIST_SIZE DUP (?) 
StartEventsList  DB  EVENTS_LIST_SIZE  DUP (?)
DoneEventsList   DB  EVENTS_LIST_SIZE  DUP (?)
NumaNodesList    DB  NUMA_LIST_SIZE    DUP (?)


;------------------------------------------------------------------------------;
;                   Flag for child thread termination.                         ;
;------------------------------------------------------------------------------;

PrimaryChildsBreak  DB  ?