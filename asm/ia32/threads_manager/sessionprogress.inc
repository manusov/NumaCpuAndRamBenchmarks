;--- Execute measurement session --------------------------------------------;
; Execute measurements (run target performance pattern),                     ; 
; use allocated memory and threads resources handles,                        ; 
; allocated when session starts (see sessionstart.inc).                      ;
; See complementar routines: sessionstart.inc , sessionstop.inc.             ;
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
; All CPU registers can be corrupted, this rule for complex modules:         ;
; directory threads_manager\:                                                ;
;   sessionstart.inc, sessionstop.inc, sessionprogress.inc.                  ;
; directory gui\:                                                            ;
;   drawsessionstart.inc, drawsessionstop.inc                                ;
;   but not drawsessionthread.inc because OS callback                        ;       
; Remember about PUSH-POP in the callers.                                    ;
;----------------------------------------------------------------------------;

SessionProgress:
push 0 0

; Setup pointers
lea esi,[InputParms]   ; ESI = Pointer to IPB, benchmarks scenario
lea edi,[OutputParms]  ; EDI = Pointer to OPB, benchmarks results

; Pre-heat phase for result stabilization
; Note this pre-heat for simple only, not use this for drawings
lea ecx,[ThreadsList]
mov edx,[esi + IPB.UpdatedThreads]
call ThreadsReStart

; Replace fill pattern to walk pattern for latency mode
lea ebx,[ThreadsList]
cmp [esi + IPB.UpdatedAsm],LATENCY_MODE
jb .L20                ; Go skip this update if not a LATENCY MEASUREMENT mode
mov ecx,[esi + IPB.UpdatedThreads]   ; ECX = Threads count
mov eax,[esi + IPB.WalkRoutine]      ; EAX  = Pointer to pattern routine
call ThreadsUpdatePattern  ; replace list build pattern to list walk pattern
.L20:

; Get start time point, by OS timer API (alternative of TSC)
mov eax,esp        ; EAX = Pointer to updated variable, OS timer, 64-bit
push eax           ; Parm#1 = Pointer to updated variable, OS timer, 64-bit 
call [GetSystemTimeAsFileTime]

; Measurement pattern
lea ecx,[ThreadsList]               ; ECX = Pointer to threads list
mov edx,[esi + IPB.UpdatedThreads]  ; EDX = number of threads
call ThreadsReStart
mov al,2  ; AL = error status, valid if CF=1 only
jc .Exit
mov dword [edi + OPB.TSCtimerDelta + 0],eax   ; Store delta-TSC, units = TSC clocks
mov dword [edi + OPB.TSCtimerDelta + 4],edx

; Get stop time point, by OS timer API (alternative of TSC)
lea eax,[edi + OPB.OStimerDelta]  ; EAX = Pointer to updated variable, OS timer, 64-bit
push eax                          ; Parm#1 = Pointer to updated variable, OS timer, 64-bit 
call [GetSystemTimeAsFileTime]
mov eax,[esp + 0]  ; EDX:EAX = Time at this point
mov edx,[esp + 4]
sub dword [edi + OPB.OStimerDelta + 0],eax    ; Calculate delta-OS timer, units = 100 ns 
sbb dword [edi + OPB.OStimerDelta + 4],edx

; Done
clc            ; CF=0 means no errors
.Exit:         ; Can go to this point if CF=1 when errors detected
pop ecx ecx    ; Note POP for remove stack variable 
ret

