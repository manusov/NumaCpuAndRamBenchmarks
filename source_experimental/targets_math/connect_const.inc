;------------------------------------------------------------------------------;
;   This performance patterns used in the "Vector brief" benchmark scenario.   ;
;           Library of performance patterns for mathematics benchmarks:        ;
;               constants and variables with pre-defined values.               ;
;------------------------------------------------------------------------------;

; Macro for compact (16-bit) encode of 64-bit offsets, use base-relative
MACRO MATH_ENTRY x1
{
DW  x1 - MathPatternsBase 
}

align 2
MathPatterns:
; Include files connect for performance patterns
; Math performance measurement
MATH_ENTRY  Pattern_Sqrt_SSE128      ; SSE128 vector sqrt, ID = 0
MATH_ENTRY  Pattern_Sqrt_AVX256      ; AVX256 vector sqrt
MATH_ENTRY  Pattern_Sqrt_AVX512      ; AVX512 vector sqrt
MATH_ENTRY  Pattern_Cos_X87          ; X87 FPU scalar cosine
MATH_ENTRY  Pattern_SinCos_X87       ; X87 FPU scalar sine + cosine, ID = 4

; Data operands alignment is speed optimization condition.
; Note for SSE/AVX, if used aligned form of instructions, alignment is required,
; unaligned access at VMOVAPD cause exception.
; For classic x86 instructional, alignment is optional, but improve speed.
; Used align 64 , this maximum , for AVX512 512-bit = 64-byte operands.
; Constants for floating point operations, note for complex finctions
; (sqrt, sine, cosine) performance can depend on data values.
align 64
MathData:
DQ  11.9 , 22.8 , 33.7 , 44.6
DQ  55.5 , 66.4 , 77.3 , 88.2

