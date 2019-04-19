
; TODO. 
; Regularize simple/brief/drawings calling conventions, what registers destroyed.

; TODO.
; This is NUMA-mode branch, support and check fot NUMA absent, PG absent

; TODO.
; Better special simplified branch for single-thread, for precision reasons ? 


SessionStop:

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