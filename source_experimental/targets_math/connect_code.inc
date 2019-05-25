;------------------------------------------------------------------------------;
;   This performance patterns used in the "Vector brief" benchmark scenario.   ;
;           Library of performance patterns for mathematics benchmarks:        ;
;                    connect include files with subroutines.                   ;
;------------------------------------------------------------------------------;

;--- Get pointer to performance pattern subroutine by ID ---;
; INPUT:   AL  = Subroutine ID                              ;
; OUTPUT:  RBX = Subroutine entry point                     ;
;-----------------------------------------------------------; 
GetMathPattern:
movzx eax,al                           ; This clears bits RAX[63-8]
movzx eax,word [MathPatterns + rax*2]  ; Read 16-bit compact offset
lea rbx,[MathPatternsBase + rax]       ; Calculate 64-bit absolute address
ret

; Start offset for compact offsets encoding
MathPatternsBase:
; Math performance measurement
; Routines for mathematics (not memory read-write) benchmarks
include 'sqrt_sse128.inc'       ; Square root, SSE128, 2 x double 
include 'sqrt_avx256.inc'       ; Square root, AVX256, 4 x double
include 'sqrt_avx512.inc'       ; Square root, AVX512, 8 x double
include 'cos_x87.inc'           ; Cosine, x87
include 'sincos_x87.inc'        ; Sine + Cosine, x87

