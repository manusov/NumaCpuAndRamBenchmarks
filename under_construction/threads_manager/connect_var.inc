;------------------------------------------------------------------------------;
;                  Library for memory and threads management:                  ;
;                   variables without pre-defined values.                      ;
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
;     MPECSB = Memory Performance Engine control and status block (CSB).       ;
;------------------------------------------------------------------------------;
; Test input and output operands located by alignment-optimal ordering
; and 64-byte cache line allocation
; Threads management lists, must be sequentally located, this fact used in code
align 64
ThreadsList      DB  THREADS_LIST_SIZE DUP (?) 
StartEventsList  DB  EVENTS_LIST_SIZE  DUP (?)
DoneEventsList   DB  EVENTS_LIST_SIZE  DUP (?)
NumaNodesList    DB  NUMA_LIST_SIZE    DUP (?)
