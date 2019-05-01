;------------------------------------------------------------------------------;
; This subroutine runs as parallel background thread, make measurements and    ;
; store results. Used for draw performance = f( block size ) function.         ;
;                                                                              ;
; INPUT:   None at CPU registers                                               ;
;                                                                              ;
; OUTPUT:  CF = status flag, 0(NC)=No errors, 1(C)=Error                       ;  
;                                                                              ;
; All CPU registers can be corrupted                                           ;
;------------------------------------------------------------------------------;

; TODO.
; Required input-output parameters detalization and error type status definition.

DrawSessionThread:

; Entry point, must be compliant Microsoft x64 calling convention.
push rbx rsi rdi rbp r12 r13 r14 r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
push rbp
sub rsp,32

; Load pointer to thread control structure
lea r15,[DrawThreadParms]

; Signaling thread START
mov rcx,[r15 + DTHP.EventStart]   ; RCX = Parm#1 = Handle for thread start event
call [SetEvent]                   ; Call WinAPI function

; Measurement initialization, pre-heat

lea rsi,[InputParms]
lea rcx,[ThreadsList]
mov edx,[rsi + IPB.UpdatedThreads]
call ThreadsReStart

; Measurement cycle
.BackgroundProcess:

cmp [r15 + DTHP.MeasureCounter],MEASURE_POINTS_COUNT  ; TODO. Replace this to wait with minimum utilization.
jae .BackgroundSkip

;- DEBUG DELAY -
; mov ecx,90000h
; loop $
;- DEBUG STORE RESULTS -
; mov eax,[r15 + DTHP.MeasureCounter]
; imul eax,eax,16
; lea rdi,[r15 + DTHP.MeasureArray + rax]
; mov eax,32 * 5000000 * 2 ; 2
; stosq
; mov eax,1 * 5000000
; stosq
;- DEBUG -

lea rsi,[InputParms]
lea rdi,[OutputParms]

; Update block size at threads control block
lea rcx,[ThreadsList]
mov rdx,rsi
mov r8d,[rsi + IPB.UpdatedThreads]
mov r9,[rsi + IPB.StartBlockSize]
call ThreadsUpdateBlockSize

; Get start time point
; lea rcx,[rsp+32]
; call [GetSystemTimeAsFileTime]

; Select Bandwidth or Latency measurement mode
cmp [rsi + IPB.UpdatedAsm],LATENCY_MODE
jae .LatencyMode 

.BandwidthMode:
; Measurement pattern
lea rcx,[ThreadsList]
mov edx,[rsi + IPB.UpdatedThreads]
call ThreadsReStart
jmp .DoneBandwidthOrLatency

.LatencyMode:
; Setup pattern for prepare walk list
lea rcx,[ThreadsList]
mov r8d,[rsi + IPB.UpdatedThreads]
mov r9,[rsi + IPB.PatternRoutine]
call ThreadsUpdatePattern
; Run service pattern, prepare walk list, delta TSC ignored here
lea rcx,[ThreadsList]
mov edx,[rsi + IPB.UpdatedThreads]
call ThreadsReStart
; Setup pattern for walk
lea rcx,[ThreadsList]
mov r8d,[rsi + IPB.UpdatedThreads]
mov r9,[rsi + IPB.WalkRoutine]
call ThreadsUpdatePattern
; Run measurement pattern, walk by prepared walk list, get delta TSC in the RAX
lea rcx,[ThreadsList]
mov edx,[rsi + IPB.UpdatedThreads]
call ThreadsReStart

.DoneBandwidthOrLatency:

; mov [rdi + OPB.TSCtimerDelta],rax   ; Store delta-TSC, units = TSC clocks
; ; Get stop time point
; lea rcx,[rsp+40]
; call [GetSystemTimeAsFileTime]
; mov rax,[rsp+40]
; sub rax,[rsp+32]
; mov [rdi + OPB.OStimerDelta],rax    ; Store delta-OS timer, units = 100 ns 

mov edx,[r15 + DTHP.MeasureCounter]
shl edx,4
lea rdi,[r15 + DTHP.MeasureArray + rdx]
stosq  ; Store delta TSC, clocks
mov rax,[rsi + IPB.StartBlockSize]   ; Block size, units = bytes
; mov edx,[rsi + IPB.OperandWidth]
; shr edx,3
; mul rdx                            ; Multiply by bytes per instruction
mov edx,[rsi + IPB.MeasureRepeats]
mul rdx                              ; Multiply by measurement repeats
; mov edx,[rsi + IPB.UpdatedThreads]
; mul rdx                              ; Multiply by number of threads
stosq  ; Store full length, bytes

; Update block size for performance = f(size)
mov rax,[rsi + IPB.DeltaBlockSize]
add [rsi + IPB.StartBlockSize],rax

; Cycle for measurement draw
inc [r15 + DTHP.MeasureCounter]
.BackgroundSkip:
cmp [r15 + DTHP.MeasureBreak],0
je .BackgroundProcess
 
.BackgroundBreak: 

; Signaling thread STOP
mov rcx,[r15 + DTHP.EventDone]    ; RCX = Parm#1 = Handle for thread done event
call [SetEvent]                   ; Call WinAPI function

; Exit point
add rsp,32
pop rbp
mov rsp,rbp
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret