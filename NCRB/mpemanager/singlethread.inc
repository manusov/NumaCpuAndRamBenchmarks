;---------- Memory performance measurement ------------------------------------;
; Note threads affinitization yet supported for MultiThread mode only          ;
; Note about target blocks addressing.                                         ;
; For Single-Thread, subroutine ExecuteBenchmarkST use input RSI from ICBB.    ;
; For Multi-Thread, subroutine ExecuteBenchmarkMT use Block Base from          ;
; ThreadsControl block, include support NUMA optimal block assignment.         ;
;                                                                              ; 
; INPUT:   RSI = Block #1 pointer (64-bit flat)                                ;
;          RDI = Block #2 pointer (64-bit flat)                                ;
;          R8  = Block #3 pointer (64-bit flat), reserved, not used yet        ;
;                For Read, Write, Modify use RSI as Source or Destination      ;
;                For Copy use RSI = Source , RDI = Destination                 ;
;          RCX = Block length, units = instructions                            ;
;          RBP = Number of measurement repeats                                 ;
;          RBX = Target subroutine address                                     ;
;                                                                              ;
; OUTPUT:  RAX = Delta-TSC, clocks                                             ;
;          Registers corrupted, but must save R15                              ;
;          Called procedure must save R14                                      ;
;------------------------------------------------------------------------------;

ExecuteBenchmarkST:   ; This entry point for single-thread
;--- Get start time ---
push rbx rcx
xor eax,eax        ; Function 0 for CPUID
cpuid              ; This CPUID for events serialization only, results ignored
pop rcx rbx
rdtsc              ; EDX:EAX = TSC value at start: EDX=High, EAX=Low
shl rdx,32         ; Positioning TSC high 32 bits to RDX.[63-32]
lea r14,[rax+rdx]  ; R14 = Interval start TSC value, integral 64 bits
;--- Call target routine ---
call rbx           ; Run target operation
;--- Store number of operations: FLOPs , this for Math. mode only ---
;- PATCH - mov qword [ThreadsControl+072],rax
;--- Get end time ---
rdtsc              ; EDX:EAX = TSC value at end: EDX=High, EAX=Low
shl rdx,32         ; Positioning TSC high 32 bits to RDX.[63-32]
or rax,rdx         ; RAX = Interval end TSC value, integral 64 bits
sub rax,r14        ; RAX = Delta TSC value
push rax
xor eax,eax        ; Function 0 for CPUID
cpuid              ; This CPUID for events serialization only, results ignored
pop rax
;--- Return ---
ret

