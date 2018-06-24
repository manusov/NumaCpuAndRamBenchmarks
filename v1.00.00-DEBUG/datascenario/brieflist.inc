;--- Control structures for Vector Brief test ---
; VE = Legacy term "Visual Entry"

; Macro for CPUID features encoding 
; x1 = bits[5-0] = bit number in the 64-bit register
; x2 = bits[7-6] = CPUID function entry:
;                  0 = List stop or ignore (when stop control by strings list)
;                  1 = Standard features   (CPUID#00000001h, ECX:EDX),
;                  2 = Extended features   (CPUID#80000001h, ECX:EDX), 
;                  3 = Additional features (CPUID#00000007h, ECX:EBX)

MACRO VECPU x1,x2
{
DB  x2 shl 6 + x1
}
; CPUID features
VectorBriefControl:
VECPU  28+32 , 1      ; AVX            , CPUID#00000001h , ECX.28 
VECPU  05+00 , 3      ; AVX2           , CPUID#00000007h , EBX.5  
VECPU  16+00 , 3      ; AVX512F (AVX3) , CPUID#00000007h , EBX.16  
VECPU  28+00 , 3      ; AVX512CD       , CPUID#00000007h , EBX.28 
VECPU  26+00 , 3      ; AVX512PF       , CPUID#00000007h , EBX.26
VECPU  27+00 , 3      ; AVX512ER       , CPUID#00000007h , EBX.27 
VECPU  31+00 , 3      ; AVX512VL       , CPUID#00000007h , EBX.31
VECPU  30+00 , 3      ; AVX512BW       , CPUID#00000007h , EBX.30  
VECPU  17+00 , 3      ; AVX512DQ       , CPUID#00000007h , EBX.17
VECPU  21+00 , 3      ; AVX512IFMA     , CPUID#00000007h , EBX.21
VECPU  01+32 , 3      ; AVX512BM       , CPUID#00000007h , ECX.1

; Macro for OS context features encoding
; x1 = bits[5-0] = bit number in the 64-bit register
; x2 = bits[7-6] = skip check flag (if XCR0 not supported, but SSE supported)
;                  0 = skip , 1 = verify , when used as part of VECLK
MACRO VECNT x1,x2
{
DB  x2 shl 6 + x1
}
; OS context features
; No label, addressing continued after previous
VECNT  01+00 , 1      ; SSE128, XMM[0-15], bits [0-127]   , XCR0.1
VECNT  02+00 , 1      ; AVX256, YMM[0-15], bits [128-255] , XCR0.2  
VECNT  06+00 , 1      ; AVX512, ZMM[0-15], bits [256-511] , XCR0.6 
VECNT  07+00 , 1      ; AVX512, ZMM[16-31], bits [0-511]  , XCR0.7 
VECNT  05+00 , 1      ; AVX512, predicates K[0-7]         , XCR0.5

; Macro for instruction timings measurement control encoding
MACRO VECLK x1,x2,x3,x4,x5,x6,x7 
{
DQ  x1                ; Pointer to measurement routine
DD  x2                ; Instruction count per work cycle
DD  x3                ; Measurement cycle count
VECPU  x4,x5          ; CPU (CPUID) check criteria to run this
VECNT  x6,x7          ; OS (XCR0) check criteria to run this
}
; Terminator
MACRO VEEND
{
DQ  0
}
; Instruction timings measurement control list
InstructionTimingControl:
VECLK  Pattern_Read_SSE128  , 256 , 1600000 , 26+00 , 1 , 01+00 , 0   
VECLK  Pattern_Write_SSE128 , 256 , 1600000 , 26+00 , 1 , 01+00 , 0  
VECLK  Pattern_Copy_SSE128  , 128 , 1600000 , 26+00 , 1 , 01+00 , 0 
VECLK  Pattern_Read_AVX256  , 128 , 1600000 , 28+32 , 1 , 02+00 , 1
VECLK  Pattern_Write_AVX256 , 128 , 1600000 , 28+32 , 1 , 02+00 , 1  
VECLK  Pattern_Copy_AVX256  , 64  , 1600000 , 28+32 , 1 , 02+00 , 1  
VECLK  Pattern_Read_AVX512  , 64  , 1600000 , 16+00 , 3 , 06+00 , 1 
VECLK  Pattern_Write_AVX512 , 64  , 1600000 , 16+00 , 3 , 06+00 , 1  
VECLK  Pattern_Copy_AVX512  , 32  , 1600000 , 16+00 , 3 , 06+00 , 1   
VECLK  Pattern_Sqrt_SSE128  , 256 , 400000  , 26+00 , 1 , 01+00 , 0 
VECLK  Pattern_Sqrt_AVX256  , 256 , 400000  , 28+32 , 1 , 02+00 , 1   
VECLK  Pattern_Sqrt_AVX512  , 256 , 400000  , 16+00 , 3 , 06+00 , 1 
VECLK  Pattern_Cos_X87      , 256 , 40000   , 00+00 , 1 , 00+00 , 0     
VECLK  Pattern_SinCos_X87   , 256 , 40000   , 00+00 , 1 , 00+00 , 0 
VEEND
  

