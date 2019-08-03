;--- This subroutine runs as parallel background thread, make measurements ----; 
;    and store results. Used for draw performance = f( block size ) function.  ;
;                                                                              ;
; INPUT:   None at CPU registers                                               ;
;                                                                              ;
; OUTPUT:  RAX = Return code = 0                                               ;
;          Non-volatile regs. unchanged for Microsoft x64  calling convention  ;
;                                                                              ;
; CPU registers volatile / non volatile by Microsoft x64 calling convention,   ;
; note about this subroutine is object for OS callback.                        ;
;------------------------------------------------------------------------------;

DrawSessionThread:

; Entry point, must be compliant Microsoft x64 calling convention.
push rbx rsi rdi rbp r12 r13 r14 r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
push rbp
sub rsp,32+8

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
jc .BackgroundSkip  ; Go skip operations if error, note this is separate thread

; Measurement cycle
.BackgroundProcess:

cmp [r15 + DTHP.MeasureCounter],DRAW_POINTS_COUNT
jae .BackgroundSkip    ; Can replace this to wait with minimum utilization ?

lea rsi,[InputParms]
lea rdi,[OutputParms]

; Update block size at threads control block
lea rcx,[ThreadsList]
mov rdx,rsi
mov r8d,[rsi + IPB.UpdatedThreads]
mov r9,[rsi + IPB.StartBlockSize]
call ThreadsUpdateBlockSize

cmp [rsi + IPB.UpdatedMeasure],MEASURE_B_ADAPTIVE
jb .NoAdaptive
lea rcx,[ThreadsList]
mov rdx,rsi
mov r8d,[rsi + IPB.UpdatedThreads]
call ThreadsUpdateAdaptive
.NoAdaptive:

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
jc .BackgroundSkip  ; Go skip operations if error, note this is separate thread
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
jc .BackgroundSkip  ; Go skip operations if error, note this is separate thread

; Setup pattern for walk
lea rcx,[ThreadsList]
mov r8d,[rsi + IPB.UpdatedThreads]
mov r9,[rsi + IPB.WalkRoutine]
call ThreadsUpdatePattern
; Run measurement pattern, walk by prepared walk list, get delta TSC in the RAX
lea rcx,[ThreadsList]
mov edx,[rsi + IPB.UpdatedThreads]
call ThreadsReStart
jc .BackgroundSkip  ; Go skip operations if error, note this is separate thread

.DoneBandwidthOrLatency:

; mov [rdi + OPB.TSCtimerDelta],rax   ; Store delta-TSC, units = TSC clocks
; ; Get stop time point
; lea rcx,[rsp+40]
; call [GetSystemTimeAsFileTime]
; mov rax,[rsp+40]
; sub rax,[rsp+32]
; mov [rdi + OPB.OStimerDelta],rax    ; Store delta-OS timer, units = 100 ns 

;
;mov edx,[r15 + DTHP.MeasureCounter]
;shl edx,4
;lea rdi,[r15 + DTHP.MeasureArray + rdx]
;stosq  ; Store delta TSC, clocks
;mov rax,[rsi + IPB.StartBlockSize]   ; Block size, units = bytes
;mov rdx,[rsi + IPB.MeasureRepeats]
;mul rdx                              ; Multiply by measurement repeats
;stosq  ; Store full length, bytes
;

; Calculte and store CPI = Clocks per Instruction,
; CPI = f ( Delta TSC, Instructions count )
mov edx,[r15 + DTHP.MeasureCounter]
shl edx,3      ; 2^3 = 8 bytes per store (QWORD)
lea rdi,[r15 + DTHP.MeasureArray + rdx]
push rax       ; RAX = Delta TSC per target operation, integer 64-bit
finit
fild qword [rsp]
mov rax,[rsi + IPB.StartBlockSize]  ; Block size, units = bytes
mul [rsi + IPB.MeasureRepeats]      ; Multiply by total bytes per measurement cycle
mov ecx,[rsi + IPB.OperandWidth]    ; RCX = bits per instruction
shr ecx,3                           ; RCX = bytes per instruction
xor edx,edx
div rcx                             ; Note can use x87 to prevent exception 0
mov [rsp],rax
fild qword [rsp]  ; ST0 = Instruction cound, ST1 = Delta TSC
fdivp st1,st0     ; ST0 = CPI
fstp qword [rsp]
pop rax        ; RAX = Clocks per Instruction (CPI), floating point, double
stosq          ; Store this for communication with parent thread

; Update block size for performance = f(size)
mov rax,[rsi + IPB.DeltaBlockSize]
add [rsi + IPB.StartBlockSize],rax

; Cycle for measurement draw, increment counter by 1 drawing pixel
inc [r15 + DTHP.MeasureCounter]

; Support approximation mode, x16 and x32. One pixel already saved, required
; it copy  16-1=15  or  32-1=31  times.
mov eax,[rsi + IPB.UpdatedApprox]
test eax,eax
jz .NoApproxMode       ; go if no approximation mode
mov ecx,15             ; 1 already + 15 duplicated = 16 total stores
cmp eax,APPROX_X16
je .YesApproxMode
mov cl,31              ; 1 already + 31 duplicated = 32 total stores
.YesApproxMode:        ; approximation duplication cycle
mov rax,[rdi-8]
stosq                  ; one duplication per loop
mov r8,[rsi + IPB.DeltaBlockSize]
add [rsi + IPB.StartBlockSize],r8
inc [r15 + DTHP.MeasureCounter]
loop .YesApproxMode    ; approximation duplication cycle
.NoApproxMode:

; Cycle with support termination by break
.BackgroundSkip:
cmp [r15 + DTHP.MeasureBreak],0
je .BackgroundProcess
 
.BackgroundBreak: 

; Signaling thread STOP
mov rcx,[r15 + DTHP.EventDone]    ; RCX = Parm#1 = Handle for thread done event
call [SetEvent]                   ; Call WinAPI function

; Exit point
add rsp,32+8
pop rbp
mov rsp,rbp
pop r15 r14 r13 r12 rbp rdi rsi rbx
xor eax,eax                       ; RAX = 0, return code = 0
ret
