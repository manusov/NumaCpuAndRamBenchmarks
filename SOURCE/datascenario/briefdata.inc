;------------------------------------------------------------------------------;
;                   Data structures for Vector Brief test.                     ;
;------------------------------------------------------------------------------;

; Data operands alignment is speed optimization condition.
; Note for SSE/AVX, if used aligned form of instructions, alignment is required,
; unaligned access at VMOVAPD cause exception.
; For classic x86 instructional, alignment is optional, but improve speed.
; Used align 64 , this maximum , for AVX512 512-bit = 64-byte operands.

; Constants for floating point operations
align 64
VectorLoad  DQ  11.9 , 22.8 , 33.7 , 44.6
            DQ  55.5 , 66.4 , 77.3 , 88.2
DivisorMHz  DQ  1000000.0
DividendNs  DQ  1000000000.0


