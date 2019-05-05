;--- Stop measurement session -------------------------------------------------;
; Release allocated memory and resources handles when session stops            ;
;                                                                              ;
; INPUT:   None in the registers                                               ;
;          IPB = Memory and Threads allocation data, used for release          ;
;          Threads List, NUMA nodes list                                       ;
;                                                                              ;
; OUTPUT:  None in the registers                                               ;
;                                                                              ;
; All CPU registers can be corrupted                                           ;
;------------------------------------------------------------------------------;

; TODO. 
; Regularize simple/brief/drawings calling conventions, what registers destroyed.
; TODO.
; This is NUMA-mode branch, support and check fot NUMA absent, PG absent
; TODO.
; Better special simplified branch for single-thread, for precision reasons ?
; TODO.
; Add CF flag for status, include comments. 
 

SessionStop:

lea rdi,[InputParms]

; Threads stop, release handles for Threads and Events
; TODO. Check status required after call ( CF flag )
lea rcx,[ThreadsList]
mov edx,[rdi + IPB.UpdatedThreads]
call ReleaseThreadsList
; NUMA-aware memory release
; TODO. Check status required after call ( CF flag )
lea rcx,[NumaNodesList]
mov edx,[rdi + IPB.DomainsCount]
call MemReleaseNuma
; Done
ret