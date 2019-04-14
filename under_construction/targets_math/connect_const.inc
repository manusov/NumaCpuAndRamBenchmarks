
align 8
MathPatterns:
; Include files connect for performance patterns
; Math performance measurement
DW  Pattern_Sqrt_SSE128 - MathPatternsBase           ; ID = 0
DW  Pattern_Sqrt_AVX256 - MathPatternsBase
DW  Pattern_Sqrt_AVX512 - MathPatternsBase
DW  Pattern_Cos_X87     - MathPatternsBase
DW  Pattern_SinCos_X87  - MathPatternsBase           ; ID = 4

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

