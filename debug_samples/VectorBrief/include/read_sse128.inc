;---------- Memory performance patterns ---------------------------------------;
; INPUT:   RSI = Block #1 pointer (64-bit flat)                                ;
;          RDI = Block #2 pointer (64-bit flat)                                ;
;          R8  = Block #3 pointer (64-bit flat) , reserved and not used yet    ;
;                For Read, Write, Modify use RSI as Source or Destination      ;
;                For Copy use RSI = Source , RDI = Destination                 ;
;          RCX = Block length, units = instructions                            ;
;          RBP = Number of measurement repeats                                 ;
; OUTPUT:  None                                                                ;
;          Registers corrupted, but must save R14, R15                         ;
;------------------------------------------------------------------------------;

Pattern_Read_SSE128:
Measurement_Read_SSE128:
; Prepare big cycle
; Set pointer to middle of 256-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea rax,[rsi+128]            ; RAX = Reload source address
mov rdx,rcx                  ; RDX = Reload length
shr rdx,4                    ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION iterations, required future make 1 instr. iteration (!)
jz Small_Read_SSE128         ; Go if Length < 16 instructions
mov ebx,256                  ; RBX = Addend, this operation also clear bits RBX[63-32]
; Big cycle                  ; 256 bytes per cycle with 16 128-bit SSE registers
DumpStart_Read_SSE128:
Block_Read_SSE128:
movapd xmm0, [rax-16*08]
movapd xmm1, [rax-16*07]
movapd xmm2, [rax-16*06]
movapd xmm3, [rax-16*05]
movapd xmm4, [rax-16*04]
movapd xmm5, [rax-16*03]
movapd xmm6, [rax-16*02]
movapd xmm7, [rax-16*01]
movapd xmm8, [rax+16*00]
movapd xmm9, [rax+16*01]
movapd xmm10,[rax+16*02]
movapd xmm11,[rax+16*03]
movapd xmm12,[rax+16*04]
movapd xmm13,[rax+16*05]
movapd xmm14,[rax+16*06]
movapd xmm15,[rax+16*07]
add rax,rbx                  ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Read_SSE128        ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Read_SSE128:
; Prepare tail cycle
Small_Read_SSE128:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_Read_SSE128
mov ebx,16                   ; 16 bytes per cycle with one 128-bit SSE register
; Tail cycle
Tail_Read_SSE128:
movapd xmm0, [rax-16*08]
add rax,rbx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Read_SSE128         ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
; Measurement cycle
Measure_Read_SSE128:
dec rbp
jnz Measurement_Read_SSE128  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
