;--- This subroutine runs as parallel background thread, make measurements ----; 
;    and store results. Used for draw performance = f( block size ) function.  ;
;                                                                              ;
; INPUT:   None at CPU registers                                               ;
;          But reserve space for one DWORD parameter in the stack              ;
;                                                                              ;
; OUTPUT:  EAX = Return code = 0                                               ;
;          Non-volatile regs. unchanged for Microsoft ia32  calling convention ;
;                                                                              ;
; CPU registers volatile / non volatile by Microsoft ia32 calling convention,  ;
; note about this subroutine is object for OS callback.                        ;
;------------------------------------------------------------------------------;

DrawSessionThread:

; Entry point, must be compliant Microsoft x64 calling convention.
push ebx esi edi ebp

; Drawings window invalidation for silent mode: show at drawings start 
cmp [SilentMode],0
je .SkipSilent
mov eax,[Handle_Win1]
test eax,eax
jz .SkipSilent
push 0                 ; Parm#3 = Background erase flag, 0=Background unchanged
push 0                 ; Parm#2 = Pointer to invalidated region, 0=All window
push eax               ; Parm#1 = Window handle
call [InvalidateRect]  ; This cause callback WM_PAINT operation
.SkipSilent:

; Signaling thread START
push [DrawThreadParms + DTHP.EventStart]   ; Parm#1 = Handle for thread start event
call [SetEvent]                            ; Call WinAPI function

; Measurement initialization, pre-heat
lea esi,[InputParms]                 ; ESI = Pointer to IPB
lea ecx,[ThreadsList]                ; ECX = Pointer to Threads List
mov edx,[esi + IPB.UpdatedThreads]   ; EDX = Threads count
call ThreadsReStart
jc .BackgroundSkip  ; Go skip operations if error, note this is separate thread

; Measurement cycle
.BackgroundProcess:

cmp [DrawThreadParms + DTHP.MeasureCounter],DRAW_POINTS_COUNT
je .SilentBranch
ja .BackgroundSkip     ; Can replace this to wait with minimum utilization ?

lea esi,[InputParms]       ; ESI = Pointer to Input Parameters Block (IPB)
lea edi,[OutputParms]      ; EDI = Pointer to Output Parameters Block (OPB)

; Update block size at threads control block
mov edx,esi                          ; EDX = Pointer to IPB
mov ecx,[esi + IPB.UpdatedThreads]   ; ECX = Number of threads
mov eax,[esi + IPB.StartBlockSize]   ; EAX = Block size per thread, bytes
lea ebx,[ThreadsList]                ; EBX = Pointer to threads list
call ThreadsUpdateBlockSize

cmp [esi + IPB.UpdatedMeasure],MEASURE_B_ADAPTIVE
jb .NoAdaptive                       ; Go skip if adaptive mode not used
lea ebx,[ThreadsList]                ; EBX = Pointer to threads list 
mov edx,esi                          ; EDX = Pointer to IPB 
mov ecx,[esi + IPB.UpdatedThreads]   ; ECX = Number of threads 
call ThreadsUpdateAdaptive
.NoAdaptive:

; Select Bandwidth or Latency measurement mode
cmp [esi + IPB.UpdatedAsm],LATENCY_MODE
jae .LatencyMode   ; Go if LATENCY mode, otherwise BANDWIDTH mode 

.BandwidthMode:
; Measurement pattern
lea ecx,[ThreadsList]
mov edx,[esi + IPB.UpdatedThreads]
call ThreadsReStart
jc .BackgroundSkip  ; Go skip operations if error, note this is separate thread
jmp .DoneBandwidthOrLatency

.LatencyMode:
; Setup pattern for prepare walk list
lea ebx,[ThreadsList]                      ; EBX = Pointer to threads list
mov ecx,[esi + IPB.UpdatedThreads]         ; ECX = Number of threads
mov eax,[esi + IPB.PatternRoutine]         ; EAX = Pointer to pattern routine
call ThreadsUpdatePattern
; Run service pattern, prepare walk list, delta TSC ignored here
lea ecx,[ThreadsList]                      ; ECX = Pointer to threads list
mov edx,[esi + IPB.UpdatedThreads]         ; EDX = Number of threads
call ThreadsReStart
jc .BackgroundSkip  ; Go skip operations if error, note this is separate thread

; Setup pattern for walk with list, prepared on previous step
lea ebx,[ThreadsList]
mov ecx,[esi + IPB.UpdatedThreads]
mov eax,[esi + IPB.WalkRoutine]
call ThreadsUpdatePattern
; Run measurement pattern, walk by prepared walk list, get delta TSC in the RAX
lea ecx,[ThreadsList]
mov edx,[esi + IPB.UpdatedThreads]
call ThreadsReStart
jc .BackgroundSkip  ; Go skip operations if error, note this is separate thread

.DoneBandwidthOrLatency:

; Calculte and store CPI = Clocks per Instruction,
; CPI = f ( Delta TSC, Instructions count )
mov ecx,[DrawThreadParms + DTHP.MeasureCounter]
shl ecx,3      ; 2^3 = 8 bytes per store (QWORD)
lea edi,[DrawThreadParms + DTHP.MeasureArray + ecx]

push edx eax   ; EDX:EAX = Delta TSC per target operation, integer 64-bit
finit
fild qword [esp]
fild [esi + IPB.MeasureRepeats]   ; Multiply by total bytes per measurement cycle
fimul [esi + IPB.StartBlockSize]  ; Block size, units = bytes 
mov eax,[esi + IPB.OperandWidth]  ; EAX = bits per instruction
shr eax,3                         ; EAX = bytes per instruction
mov [esp],eax
fidiv dword [esp]                 ; Divide by BPI, ST0 = Instructions count, ST1 = Delta TSC
fdivp st1,st0     ; ST0 = CPI
fstp qword [esp]
pop eax       ; EDX:EAX = Clocks per Instruction (CPI), floating point, double
stosd         ; CPI, Low DWORD, store this for communication with parent thread
pop eax
stosd         ; CPI, High DWORD

; Update block size for performance = f(size)
mov eax,[esi + IPB.DeltaBlockSize]
add [esi + IPB.StartBlockSize],eax

; Cycle for measurement draw, increment counter by 1 drawing pixel
inc [DrawThreadParms + DTHP.MeasureCounter]

; Support approximation mode, x16 and x32. One pixel already saved, required
; it copy  16-1=15  or  32-1=31  times.
mov eax,[esi + IPB.UpdatedApprox]
test eax,eax
jz .NoApproxMode       ; go if no approximation mode
mov ecx,15             ; 1 already + 15 duplicated = 16 total stores
cmp eax,APPROX_X16
je .YesApproxMode
mov cl,31              ; 1 already + 31 duplicated = 32 total stores
.YesApproxMode:        ; approximation duplication cycle
mov eax,[edi-8]
stosd                  ; one duplication per loop
mov eax,[edi-8]
stosd                  ; one duplication per loop
mov ebx,[esi + IPB.DeltaBlockSize]
add [esi + IPB.StartBlockSize],ebx
inc [DrawThreadParms + DTHP.MeasureCounter]
loop .YesApproxMode    ; approximation duplication cycle
.NoApproxMode:

; Cycle with support termination by break
.BackgroundSkip:
cmp [DrawThreadParms + DTHP.MeasureBreak],0
je .BackgroundProcess

; Signaling thread STOP
push [DrawThreadParms + DTHP.EventDone]    ; Parm#1 = Handle for thread done event
call [SetEvent]                            ; Call WinAPI function

; Exit point
pop ebp edi esi ebx
xor eax,eax                                ; EAX = 0, return code = 0
ret 4

; Drawings window invalidation for silent mode: show at drawings end
.SilentBranch:
mov al,0
xchg [SilentMode],al
cmp al,0
je .BackgroundSkip
mov eax,[Handle_Win1]
test eax,eax
jz .BackgroundSkip
push 0                 ; Parm#3 = Background erase flag, 0=Background unchanged
push 0                 ; Parm#2 = Pointer to invalidated region, 0=All window
push eax               ; Parm#1 = Window handle
call [InvalidateRect]  ; This cause callback WM_PAINT operation
jmp .BackgroundSkip 
