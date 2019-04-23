
; TODO. 
; Regularize simple/brief/drawings calling conventions, what registers destroyed.

; TODO.
; Better special simplified branch for single-thread, for precision reasons ? 


SessionProgress:

; Stack alignment, required because OS API used, save number of threads
mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
sub rsp,32+16                  ; Build parameters shadow, 4 qwords

; Setup pointers
lea rsi,[InputParms]
lea rdi,[OutputParms]

; Pre-heat phase for result stabilization
; TODO. This pre-heat for simple only, not use this for drawings
lea rcx,[ThreadsList]
mov edx,[rsi + IPB.UpdatedThreads]
call ThreadsReStart

; Replace fill pattern to walk pattern for latency mode
lea rcx,[ThreadsList]
cmp [rsi + IPB.UpdatedAsm],LATENCY_MODE
jb .L20
mov r8d,[rsi + IPB.UpdatedThreads]
mov r9,[rsi + IPB.WalkRoutine]
call ThreadsUpdatePattern
.L20:

; Get start time point
lea rcx,[rsp+32]
call [GetSystemTimeAsFileTime]

; Measurement pattern
lea rcx,[ThreadsList]
mov edx,[rsi + IPB.UpdatedThreads]

call ThreadsReStart
mov [rdi + OPB.TSCtimerDelta],rax   ; Store delta-TSC, units = TSC clocks

; Get stop time point
lea rcx,[rsp+40]
call [GetSystemTimeAsFileTime]
mov rax,[rsp+40]
sub rax,[rsp+32]
mov [rdi + OPB.OStimerDelta],rax    ; Store delta-OS timer, units = 100 ns 

; Done
mov rsp,rbp
ret