;---------- Memory performance patterns ---------------------------------------;
; INPUT:   RSI = Block #1 pointer (64-bit flat)                                ;
;          RDI = Block #2 pointer (64-bit flat)                                ;
;          R8  = Block #3 pointer (64-bit flat) , used for load data vector    ;
;                For Read, Write, Modify use RSI as Source or Destination      ;
;                For Copy use RSI = Source , RDI = Destination                 ;
;          RCX = Block length, units = instructions                            ;
;          RBP = Number of measurement repeats                                 ;
; OUTPUT:  None                                                                ;
;          Registers corrupted, but must save R14, R15                         ;
;------------------------------------------------------------------------------;

Pattern_Sqrt_SSE128:
Measurement_Sqrt_SSE128:

; Prepare big cycle (without tail)
movapd xmm15,[r8]
mov rdx,rcx               ; RDX = Reload length
shr rdx,4                 ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION iterations, required future make 1 instr. iteration (!)
jz Small_Sqrt_SSE128      ; Go if Length < 16 instructions

; Big cycle
DumpStart_Sqrt_SSE128:
Block_Sqrt_SSE128:
sqrtpd xmm0,xmm15
sqrtpd xmm1,xmm15
sqrtpd xmm2,xmm15
sqrtpd xmm3,xmm15
sqrtpd xmm4,xmm15
sqrtpd xmm5,xmm15
sqrtpd xmm6,xmm15
sqrtpd xmm7,xmm15
sqrtpd xmm8,xmm15
sqrtpd xmm9,xmm15
sqrtpd xmm10,xmm15
sqrtpd xmm11,xmm15
sqrtpd xmm12,xmm15
sqrtpd xmm13,xmm15
sqrtpd xmm14,xmm15
sqrtpd xmm0,xmm15
addpd xmm15,xmm15
dec rdx
jnz Block_Sqrt_SSE128        ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Sqrt_SSE128:

; Prepare tail cycle
Small_Sqrt_SSE128:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_Sqrt_SSE128

; Tail cycle
Tail_Sqrt_SSE128:
sqrtpd xmm0,xmm15
dec edx           
jnz Tail_Sqrt_SSE128         ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!

; Measurement cycle
Measure_Sqrt_SSE128:
dec rbp
jnz Measurement_Sqrt_SSE128  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret

