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

Pattern_Copy_SSE128:
Measurement_Copy_SSE128:
;--- Prepare big cycle ---
;- v0.92 optimization note
; Set pointer to middle of 256-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
;-
lea rax,[rsi+128]            ; RAX = Reload source address
lea rbx,[rdi+128]            ; RBX = Reload destination address
mov rdx,rcx                  ; RDX = Reload length
shr rdx,4                    ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION iterations, required future make 1 instr. iteration (!)
jz Small_Copy_SSE128         ; Go if Length < 16 instructions
mov r8d,256                  ; R8 = Addend, this operation also clear bits R8[63-32]
;--- Big cycle ---
DumpStart_Copy_SSE128:
Block_Copy_SSE128:
movapd xmm0,[rax-16*08]
movapd [rbx-16*08],xmm0
movapd xmm1,[rax-16*07]
movapd [rbx-16*07],xmm1
movapd xmm2,[rax-16*06]
movapd [rbx-16*06],xmm2
movapd xmm3,[rax-16*05]
movapd [rbx-16*05],xmm3
movapd xmm4,[rax-16*04]
movapd [rbx-16*04],xmm4
movapd xmm5,[rax-16*03]
movapd [rbx-16*03],xmm5
movapd xmm6,[rax-16*02]
movapd [rbx-16*02],xmm6
movapd xmm7,[rax-16*01]
movapd [rbx-16*01],xmm7
movapd xmm8,[rax+16*00]
movapd [rbx+16*00],xmm8
movapd xmm9,[rax+16*01]
movapd [rbx+16*01],xmm9
movapd xmm10,[rax+16*02]
movapd [rbx+16*02],xmm10
movapd xmm11,[rax+16*03]
movapd [rbx+16*03],xmm11
movapd xmm12,[rax+16*04]
movapd [rbx+16*04],xmm12
movapd xmm13,[rax+16*05]
movapd [rbx+16*05],xmm13
movapd xmm14,[rax+16*06]
movapd [rbx+16*06],xmm14
movapd xmm15,[rax+16*07]
movapd [rbx+16*07],xmm15
add rax,r8                   ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8                   ; Modify destination address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Copy_SSE128        ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Copy_SSE128:
;--- Prepare tail cycle ---
Small_Copy_SSE128:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_Copy_SSE128
mov r8d,16
;--- Tail cycle ---
Tail_Copy_SSE128:
movapd xmm0,[rax-16*08]
movapd [rbx-16*08],xmm0
add rax,r8                   ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8                   ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Copy_SSE128         ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;--- Measurement cycle ---
Measure_Copy_SSE128:
dec rbp
jnz Measurement_Copy_SSE128  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret

;
; Pattern_Copy_SSE128:
; Measurement_Copy_SSE128:
; ;--- Prepare big cycle ---
; mov rax,rsi                  ; RAX = Reload source address
; mov rbx,rdi                  ; RBX = Reload destination address
; mov rdx,rcx                  ; RDX = Reload length
; shr rdx,4                    ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION iterations, required future make 1 instr. iteration (!)
; jz Small_Copy_SSE128         ; Go if Length < 16 instructions
; mov r8d,256                  ; R8 = Addend, this operation also clear bits R8[63-32]
; ;--- Big cycle ---
; DumpStart_Copy_SSE128:
; Block_Copy_SSE128:
; movapd xmm0,[rax+16*00]
; movapd [rbx+16*00],xmm0
; movapd xmm1,[rax+16*01]
; movapd [rbx+16*01],xmm1
; movapd xmm2,[rax+16*02]
; movapd [rbx+16*02],xmm2
; movapd xmm3,[rax+16*03]
; movapd [rbx+16*03],xmm3
; movapd xmm4,[rax+16*04]
; movapd [rbx+16*04],xmm4
; movapd xmm5,[rax+16*05]
; movapd [rbx+16*05],xmm5
; movapd xmm6,[rax+16*06]
; movapd [rbx+16*06],xmm6
; movapd xmm7,[rax+16*07]
; movapd [rbx+16*07],xmm7
; movapd xmm8,[rax+16*08]
; movapd [rbx+16*08],xmm8
; movapd xmm9,[rax+16*09]
; movapd [rbx+16*09],xmm9
; movapd xmm10,[rax+16*10]
; movapd [rbx+16*10],xmm10
; movapd xmm11,[rax+16*11]
; movapd [rbx+16*11],xmm11
; movapd xmm12,[rax+16*12]
; movapd [rbx+16*12],xmm12
; movapd xmm13,[rax+16*13]
; movapd [rbx+16*13],xmm13
; movapd xmm14,[rax+16*14]
; movapd [rbx+16*14],xmm14
; movapd xmm15,[rax+16*15]
; movapd [rbx+16*15],xmm15
; add rax,r8                   ; Modify source address, addend=register (not constant) for optimization!
; add rbx,r8                   ; Modify destination address, addend=register (not constant) for optimization!
; dec rdx
; jnz Block_Copy_SSE128        ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
; DumpStop_Copy_SSE128:
; ;--- Prepare tail cycle ---
; Small_Copy_SSE128:
; mov edx,ecx
; and edx,00001111b            ; ECX = Extract tail length
; jz Measure_Copy_SSE128
; mov r8d,16
; ;--- Tail cycle ---
; Tail_Copy_SSE128:
; movapd xmm0,[rax+16*00]
; movapd [rbx+16*00],xmm0
; add rax,r8                   ; Modify source address, addend=register (not constant) for optimization!
; add rbx,r8                   ; Modify destination address, addend=register (not constant) for optimization!
; dec edx
; jnz Tail_Copy_SSE128         ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
; ;--- Measurement cycle ---
; Measure_Copy_SSE128:
; dec rbp
; jnz Measurement_Copy_SSE128  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
; ret
;
