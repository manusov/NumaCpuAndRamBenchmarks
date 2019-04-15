
MACRO MATH_ENTRY x1
{
DW  x1 - MathPatternsBase 
}

align 2
MathPatterns:
; Include files connect for performance patterns
; Math performance measurement
MATH_ENTRY  Pattern_Sqrt_SSE128       ; ID = 0
MATH_ENTRY  Pattern_Sqrt_AVX256
MATH_ENTRY  Pattern_Sqrt_AVX512
MATH_ENTRY  Pattern_Cos_X87
MATH_ENTRY  Pattern_SinCos_X87       ; ID = 4

; Data operands alignment is speed optimization condition.
; Note for SSE/AVX, if used aligned form of instructions, alignment is required,
; unaligned access at VMOVAPD cause exception.
; For classic x86 instructional, alignment is optional, but improve speed.
; Used align 64 , this maximum , for AVX512 512-bit = 64-byte operands.
; Constants for floating point operations

align 64
MathData:
DQ  11.9 , 22.8 , 33.7 , 44.6
DQ  55.5 , 66.4 , 77.3 , 88.2

