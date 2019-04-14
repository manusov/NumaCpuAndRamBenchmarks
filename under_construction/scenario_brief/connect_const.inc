
;------------------------------------------------------------------------------;
;                Control structures for Vector Brief test.                     ;
;------------------------------------------------------------------------------;

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
; x1 = bits[6-0] = Pattern ID
; x1 = bit[7] = Pattern type, 0=Memory bandwidth, 1=Math performance
;
MACRO VECLK x1,x2,x3,x4,x5,x6,x7 
{
DB  x1          ; Pointer to measurement routine
DB  x2          ; Instruction count per work cycle = per 4KB block, power of 2
DB  x3          ; Measurement cycle count, required * REPEATS_SCALE
VECPU  x4,x5    ; CPU (CPUID) check criteria to run this
VECNT  x6,x7    ; OS (XCR0) check criteria to run this
}
; Terminator for structures list, 
; value pointer to measurement routine = 0 means end of list 
MACRO VEEND
{
DB 5 DUP (0)
}

; Equations for performance patterns select
ID_Read_SSE128   EQU 00h + 9    
ID_Write_SSE128  EQU 00h + 10  
ID_Copy_SSE128   EQU 00h + 11 
ID_Read_AVX256   EQU 00h + 12 
ID_Write_AVX256  EQU 00h + 13  
ID_Copy_AVX256   EQU 00h + 14  
ID_Read_AVX512   EQU 00h + 15 
ID_Write_AVX512  EQU 00h + 16  
ID_Copy_AVX512   EQU 00h + 17   
ID_Sqrt_SSE128   EQU 80h + 0   
ID_Sqrt_AVX256   EQU 80h + 1   
ID_Sqrt_AVX512   EQU 80h + 2 
ID_Cos_X87       EQU 80h + 3     
ID_SinCos_X87    EQU 80h + 4 

; Instruction timings measurement control list
InstructionTimingControl:
VECLK  ID_Read_SSE128  , 8 , 160 , 26+00 , 1 , 01+00 , 0   
VECLK  ID_Write_SSE128 , 8 , 160 , 26+00 , 1 , 01+00 , 0  
VECLK  ID_Copy_SSE128  , 7 , 160 , 26+00 , 1 , 01+00 , 0 
VECLK  ID_Read_AVX256  , 7 , 160 , 28+32 , 1 , 02+00 , 1
VECLK  ID_Write_AVX256 , 7 , 160 , 28+32 , 1 , 02+00 , 1  
VECLK  ID_Copy_AVX256  , 6 , 160 , 28+32 , 1 , 02+00 , 1  
VECLK  ID_Read_AVX512  , 6 , 160 , 16+00 , 3 , 06+00 , 1 
VECLK  ID_Write_AVX512 , 6 , 160 , 16+00 , 3 , 06+00 , 1  
VECLK  ID_Copy_AVX512  , 5 , 160 , 16+00 , 3 , 06+00 , 1   
VECLK  ID_Sqrt_SSE128  , 8 ,  40 , 26+00 , 1 , 01+00 , 0 
VECLK  ID_Sqrt_AVX256  , 8 ,  40 , 28+32 , 1 , 02+00 , 1   
VECLK  ID_Sqrt_AVX512  , 8 ,  40 , 16+00 , 3 , 06+00 , 1 
VECLK  ID_Cos_X87      , 8 ,   4 , 00+00 , 1 , 00+00 , 0     
VECLK  ID_SinCos_X87   , 8 ,   4 , 00+00 , 1 , 00+00 , 0 
VEEND

; Example for instruction count per work cycle, means per 4KB block.
; Block size = 4096 bytes.
; One 128-bit SSE instruction has operand size 128 bits means 16 bytes
; Instructions per block = 4096 / 16 = 256,
; because power of 2 compact encoding: 2^8 = 256, value = 8.    

;--- Strings for vector brief test, with text result only ---

; Processor clock, this string after CPU name
MsgResultBrief  DB  'TSC clock (MHz)',0

; Processor CPUID features
                DB  0Dh,0Ah, 'Processor features, detect by CPUID',0
                DB  'AVX 256-bit',0
                DB  'AVX2 256-bit',0
                DB  'AVX3 512-bit = AVX512F (Foundation)',0
                DB  'AVX512CD (Conflict Detection)',0
                DB  'AVX512PF (Prefetch)',0
                DB  'AVX512ER (Exponential and Reciprocal)',0
                DB  'AVX512VL (Vector Length)',0
                DB  'AVX512BW (Byte and Word)',0
                DB  'AVX512DQ (Doubleword and Quadword)',0
                DB  'AVX512IFMA (Integer Fused Multiply and Add)',0
                DB  'AVX512VBM (Vector Byte Manipulation)',0
                DB  0  ; Second zero is sequence terminator

; OS context features
                DB  0Dh,0Ah, 'OS context management features, detect by XCR0',0
                DB  'SSE128 registers XMM[0-15] bits [0-127]',0
                DB  'AVX256 registers YMM[0-15] bits [128-255]',0
                DB  'AVX512 registers ZMM[0-15] bits[256-511]',0
                DB  'AVX512 registers ZMM[16-31] bits[0-511]',0
                DB  'AVX512 predicate registers K[0-7]',0
                DB  0  ; Second zero is sequence terminator

; Instructions timings
                DB  0Dh,0Ah, 'Instruction timings per 1 core (TSC clocks and nanoseconds)',0
                DB  'SSE128 read',0             
                DB  'SSE128 write',0             
                DB  'SSE128 copy',0             
                DB  'AVX256 read',0             
                DB  'AVX256 write',0             
                DB  'AVX256 copy',0             
                DB  'AVX512 read',0             
                DB  'AVX512 write',0             
                DB  'AVX512 copy',0             
                DB  'SQRTPD xmm',0             
                DB  'VSQRTPD ymm',0             
                DB  'VSQRTPD zmm',0             
                DB  'FCOS',0             
                DB  'FSINCOS',0             
                DB  0  ; Second zero is sequence terminator

; Strings for results
; Strings "supported" , "not supported"
Nsup            DB 'not supported',0
Sup             DB 'supported',0
NsupCPU         DB 'not supported by CPU',0
NsupOS          DB 'not supported by OS',0

; Note.
; Support by CPU validated by CPUID instruction,
; Support by OS validated by XCR0 register mask, 
; means OS context management support,
; save and restore CPU context for tasks switch.

;------------------------------------------------------------------------------;
;                   Data structures for Vector Brief test.                     ;
;------------------------------------------------------------------------------;

DivisorMHz  DQ  1000000.0
DividendNs  DQ  1000000000.0


