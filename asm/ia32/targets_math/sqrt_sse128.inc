;---------- Mathematics calculations performance patterns ---------------------;
; INPUT:   ESI = Block #1 pointer (32-bit flat) , used for load data vector    ;
;          EDI = Block #2 pointer (32-bit flat) , not used for math scenario   ;
;          ECX = Block length, units = instructions                            ;
;          EBX:EBP = Number of measurement repeats, EBX=High32, EBP=Low32      ;
; OUTPUT:  None                                                                ;
;          Registers corrupted                                                 ;
;------------------------------------------------------------------------------;

Pattern_Sqrt_SSE128:
Measurement_Sqrt_SSE128:

; Prepare big cycle (without tail)
movapd xmm7,[esi]
mov edx,ecx               ; EDX = Reload length
shr edx,4                 ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION iterations, required future make 1 instr. iteration (!)
jz Small_Sqrt_SSE128      ; Go if Length < 16 instructions

; Big cycle
DumpStart_Sqrt_SSE128:
Block_Sqrt_SSE128:
sqrtpd xmm0,xmm7
sqrtpd xmm1,xmm7
sqrtpd xmm2,xmm7
sqrtpd xmm3,xmm7
sqrtpd xmm4,xmm7
sqrtpd xmm5,xmm7
sqrtpd xmm6,xmm7
sqrtpd xmm0,xmm7
sqrtpd xmm1,xmm7
sqrtpd xmm2,xmm7
sqrtpd xmm3,xmm7
sqrtpd xmm4,xmm7
sqrtpd xmm5,xmm7
sqrtpd xmm6,xmm7
sqrtpd xmm0,xmm7
sqrtpd xmm1,xmm7
addpd xmm7,xmm7
dec edx
jnz Block_Sqrt_SSE128        ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Sqrt_SSE128:

; Prepare tail cycle
Small_Sqrt_SSE128:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_Sqrt_SSE128

; Tail cycle
Tail_Sqrt_SSE128:
sqrtpd xmm0,xmm7
dec edx           
jnz Tail_Sqrt_SSE128         ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!

; Measurement cycle
Measure_Sqrt_SSE128:

sub ebp,1
sbb ebx,0
mov eax,ebp
or eax,ebx
jnz Measurement_Sqrt_SSE128  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!

ret

