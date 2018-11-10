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

Pattern_Dot_FMA512:
Measurement_Dot_FMA512:
; Prepare big cycle
vzeroall
mov rax,rsi                  ; RAX = Reload source address
mov rbx,rdi                  ; RBX = Reload destination address
mov rdx,rcx                  ; RDX = Reload length
shr rdx,3                    ; RDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION iterations, required future make 1 instr. iteration (!)
jz Small_Dot_FMA512          ; Go if Length < 16 instructions
mov r8d,512                  ; R8 = Addend, this operation also clear bits R8[63-32]
; Big cycle
DumpStart_Dot_FMA512:
Block_Dot_FMA512:
vmovapd zmm0,[rax+64*00]
vmovapd zmm1,[rax+64*01]
vmovapd zmm2,[rax+64*02]
vmovapd zmm3,[rax+64*03]
vmovapd zmm4,[rax+64*04]
vmovapd zmm5,[rax+64*05]
vmovapd zmm6,[rax+64*06]
vmovapd zmm7,[rax+64*07]
vfmadd231pd zmm8 ,zmm0,[rbx+64*00]
vfmadd231pd zmm9 ,zmm1,[rbx+64*01]
vfmadd231pd zmm10,zmm2,[rbx+64*02]
vfmadd231pd zmm11,zmm3,[rbx+64*03]
vfmadd231pd zmm12,zmm4,[rbx+64*04]
vfmadd231pd zmm13,zmm5,[rbx+64*05]
vfmadd231pd zmm14,zmm6,[rbx+64*06]
vfmadd231pd zmm15,zmm7,[rbx+64*07]
add rax,r8                   ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8                   ; Modify destination address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Dot_FMA512        ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Dot_FMA512:
; Horizontal additions for main part, result is vector

;- redesigned: bug fix and optimizing -
; 512
; vaddpd zmm8,zmm8,zmm12
; vaddpd zmm9,zmm9,zmm13
; vaddpd zmm10,zmm10,zmm14 
; vaddpd zmm11,zmm11,zmm15
; vaddpd zmm0,zmm8,zmm10
; vaddpd zmm1,zmm9,zmm11
; vextractf64x4 ymm2,zmm0,1
; vextractf64x4 ymm3,zmm1,1
; 256
; vhaddpd ymm0,ymm8,ymm9
; vhaddpd ymm1,ymm10,ymm11
; vhaddpd ymm2,ymm12,ymm13
; vhaddpd ymm3,ymm14,ymm15
; vextractf128 xmm4,ymm0,1
; vextractf128 xmm5,ymm1,1
; vextractf128 xmm6,ymm2,1
; vextractf128 xmm7,ymm3,1
; vaddpd xmm0,xmm0,xmm4
; vaddpd xmm1,xmm1,xmm5
; vaddpd xmm2,xmm2,xmm6
; vaddpd xmm3,xmm3,xmm7
; vaddpd xmm8,xmm0,xmm1
; vaddpd xmm9,xmm2,xmm3
; vaddpd xmm8,xmm8,xmm9
;-
; add 512-bit operands
vaddpd zmm8,zmm8,zmm12
vaddpd zmm9,zmm9,zmm13
vaddpd zmm10,zmm10,zmm14 
vaddpd zmm11,zmm11,zmm15
vaddpd zmm0,zmm8,zmm9
vaddpd zmm1,zmm10,zmm11
vaddpd zmm0,zmm0,zmm1    ; ZMM0 = sum as 512-bit vector, 8 numbers x 64-bit
;- end of redesigned part -

; Prepare tail cycle
Small_Dot_FMA512:
mov edx,ecx
and edx,00000111b            ; ECX = Extract tail length
jz Measure_Dot_FMA512
mov r8d,64
; Tail cycle
Tail_Dot_FMA512:

;- changed -
; vmovapd zmm0,[rax+64*00]
; vfmadd231pd zmm8,zmm0,[rbx+64*00]
;-
vmovapd zmm1,[rax+64*00]
vfmadd231pd zmm0,zmm1,[rbx+64*00]
;- end of changed -

add rax,r8                   ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8                   ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Dot_FMA512          ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!

;- changed -
; Horizontal additions for tail, result is scalar
; 512
; vextractf64x4 ymm0,zmm8,1
; vaddpd ymm8,ymm8,ymm0
; 256
; vextractf128 xmm9,ymm8,1
; vaddpd xmm8,xmm8,xmm9
; vhaddpd xmm8,xmm8,xmm8
;-
; extract 2 x 256-bit operands from 512-bit operand, ZMM0 = vector source
vextractf64x4 ymm1,zmm0,1
vaddpd ymm0,ymm0,ymm1        ; add 256-bit vector operands
vhaddpd ymm0,ymm0,ymm0
vextractf128 xmm1,ymm0,1     ; extract 128-bit groups
vaddsd xmm8,xmm0,xmm1        ; add 64-bit operands, XMM8[0-63] = scalar result
;- end of changed -

; Measurement cycle
Measure_Dot_FMA512:
dec rbp
jnz Measurement_Dot_FMA512  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret

