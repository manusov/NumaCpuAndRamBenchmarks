;--- Stop measurement session -----------------------------------------------;
; Release allocated memory and resources handles when session stops          ;
;                                                                            ;
; INPUT:   None in the registers                                             ;
;          IPB = Memory and Threads allocation data, used for release        ;
;          Threads List, NUMA nodes list                                     ;
;                                                                            ;
; OUTPUT:  CF = Error flag, 0(NC) = No errors, 1(C) = Error detected.        ;
;          AL = Error code, valid if CF=1(C)                                 ;
;               00 = Memory allocation error                                 ;
;               01 = Memory release error                                    ;
;               02 = Time measurement error                                  ;
;               04 = Address arithmetics error                               ;
;               04-0FFh = Reserved                                           ;
;          OPB = Benchmark results data                                      ;
;          Threads List, NUMA nodes list                                     ;
;                                                                            ;
; All CPU registers can be corrupted                                         ;
;----------------------------------------------------------------------------;

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
mov al,1  ; AL = error status
jc .Exit  ; Go if error

; TODO. Add "Threads release error" yet supported as "Memory release error"

; NUMA-aware memory release
; TODO. Check status required after call ( CF flag )
lea rcx,[NumaNodesList]
mov edx,[rdi + IPB.DomainsCount]
call MemReleaseNuma
mov al,1  ; AL = error status, valid if CF=1 only

; Done
.Exit:
ret
