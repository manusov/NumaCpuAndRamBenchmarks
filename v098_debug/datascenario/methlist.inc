;--- Benchmarks methods list by CPU instructions set ---

MACRO BMETHOD x1, x2, x3, x4   ; 6 bytes per entry, compact encoding
{
DW x1-BasePointCode    ; Target routine entry point
DW x2-BasePointCode    ; Dump fragment start, dump show at drawings Y=F(X)
DB x3-x2               ; Dump fragment size
DB x4                  ; Data Bytes per Instruction,
}                      ; example YMM register op. = 32 bytes (256 bit) 

;--- This methods set optimized for small blocks, temporal operations ---

MethodsTemporal:

BMETHOD  Pattern_Read_MOV64     , DumpStart_Read_MOV64     , DumpStop_Read_MOV64     , 8
BMETHOD  Pattern_Write_MOV64    , DumpStart_Write_MOV64    , DumpStop_Write_MOV64    , 8
BMETHOD  Pattern_Copy_MOV64     , DumpStart_Copy_MOV64     , DumpStop_Copy_MOV64     , 8
BMETHOD  Pattern_Modify_NOT64   , DumpStart_Modify_NOT64   , DumpStop_Modify_NOT64   , 8
BMETHOD  Pattern_Write_STOSQ    , DumpStart_Write_STOSQ    , DumpStop_Write_STOSQ    , 8
BMETHOD  Pattern_Copy_MOVSQ     , DumpStart_Copy_MOVSQ     , DumpStop_Copy_MOVSQ     , 8
BMETHOD  Pattern_Read_SSE128    , DumpStart_Read_SSE128    , DumpStop_Read_SSE128    , 16
BMETHOD  Pattern_Write_SSE128   , DumpStart_Write_SSE128   , DumpStop_Write_SSE128   , 16
BMETHOD  Pattern_Copy_SSE128    , DumpStart_Copy_SSE128    , DumpStop_Copy_SSE128    , 16
BMETHOD  Pattern_Read_AVX256    , DumpStart_Read_AVX256    , DumpStop_Read_AVX256    , 32
BMETHOD  Pattern_Write_AVX256   , DumpStart_Write_AVX256   , DumpStop_Write_AVX256   , 32
BMETHOD  Pattern_Copy_AVX256    , DumpStart_Copy_AVX256    , DumpStop_Copy_AVX256    , 32
BMETHOD  Pattern_Read_AVX512    , DumpStart_Read_AVX512    , DumpStop_Read_AVX512    , 64
BMETHOD  Pattern_Write_AVX512   , DumpStart_Write_AVX512   , DumpStop_Write_AVX512   , 64
BMETHOD  Pattern_Copy_AVX512    , DumpStart_Copy_AVX512    , DumpStop_Copy_AVX512    , 64
BMETHOD  Pattern_Dot_FMA256     , DumpStart_Dot_FMA256     , DumpStop_Dot_FMA256     , 32
BMETHOD  Pattern_Dot_FMA512     , DumpStart_Dot_FMA512     , DumpStop_Dot_FMA512     , 64

;--- This methods set optimized for big blocks, non-temporal write operations ---
; If caching avoid required (for example, big blocks at DRAM benchmark),
; non-temporal WRITE recommended for both Intel and AMD CPUs.
; This fragments uses Normal Read, Non-Temporal Write
; IMPORTANT patch v0.98.0 = non-temporal read use BOTH FOR INTEL AND AMD

MethodsNonTemporal:

BMETHOD  Pattern_Read_MOV64     , DumpStart_Read_MOV64     , DumpStop_Read_MOV64     , 8
BMETHOD  Pattern_Write_MOV64    , DumpStart_Write_MOV64    , DumpStop_Write_MOV64    , 8
BMETHOD  Pattern_Copy_MOV64     , DumpStart_Copy_MOV64     , DumpStop_Copy_MOV64     , 8
BMETHOD  Pattern_Modify_NOT64   , DumpStart_Modify_NOT64   , DumpStop_Modify_NOT64   , 8
BMETHOD  Pattern_Write_STOSQ    , DumpStart_Write_STOSQ    , DumpStop_Write_STOSQ    , 8
BMETHOD  Pattern_Copy_MOVSQ     , DumpStart_Copy_MOVSQ     , DumpStop_Copy_MOVSQ     , 8
BMETHOD  Pattern_Read_SSE128    , DumpStart_Read_SSE128    , DumpStop_Read_SSE128    , 16
BMETHOD  Pattern_NtWrite_SSE128 , DumpStart_NtWrite_SSE128 , DumpStop_NtWrite_SSE128 , 16
BMETHOD  Pattern_NtCopy_SSE128  , DumpStart_NtCopy_SSE128  , DumpStop_NtCopy_SSE128  , 16
BMETHOD  Pattern_Read_AVX256    , DumpStart_Read_AVX256    , DumpStop_Read_AVX256    , 32
BMETHOD  Pattern_NtWrite_AVX256 , DumpStart_NtWrite_AVX256 , DumpStop_NtWrite_AVX256 , 32
BMETHOD  Pattern_NtCopy_AVX256  , DumpStart_NtCopy_AVX256  , DumpStop_NtCopy_AVX256  , 32
BMETHOD  Pattern_Read_AVX512    , DumpStart_Read_AVX512    , DumpStop_Read_AVX512    , 64
BMETHOD  Pattern_NtWrite_AVX512 , DumpStart_NtWrite_AVX512 , DumpStop_NtWrite_AVX512 , 64
BMETHOD  Pattern_NtCopy_AVX512  , DumpStart_NtCopy_AVX512  , DumpStop_NtCopy_AVX512  , 64
BMETHOD  Pattern_Dot_FMA256     , DumpStart_Dot_FMA256     , DumpStop_Dot_FMA256     , 32
BMETHOD  Pattern_Dot_FMA512     , DumpStart_Dot_FMA512     , DumpStop_Dot_FMA512     , 64

;--- Supports non-temporal read for 128-bit, MOVNTDQA ---
; If caching avoid required (for example, big blocks at DRAM benchmark),
; non-temporal READ recommended for AMD CPUs.
; This fragments uses Non-Temporal Read, Non-Temporal Write.
; IMPORTANT patch v0.98.0 = non-temporal read use BOTH FOR INTEL AND AMD

MethodsNonTemporal_SSE41:

BMETHOD  Pattern_Read_MOV64     , DumpStart_Read_MOV64     , DumpStop_Read_MOV64     , 8
BMETHOD  Pattern_Write_MOV64    , DumpStart_Write_MOV64    , DumpStop_Write_MOV64    , 8
BMETHOD  Pattern_Copy_MOV64     , DumpStart_Copy_MOV64     , DumpStop_Copy_MOV64     , 8
BMETHOD  Pattern_Modify_NOT64   , DumpStart_Modify_NOT64   , DumpStop_Modify_NOT64   , 8
BMETHOD  Pattern_Write_STOSQ    , DumpStart_Write_STOSQ    , DumpStop_Write_STOSQ    , 8
BMETHOD  Pattern_Copy_MOVSQ     , DumpStart_Copy_MOVSQ     , DumpStop_Copy_MOVSQ     , 8
BMETHOD  Pattern_NtRead_SSE128  , DumpStart_NtRead_SSE128  , DumpStop_NtRead_SSE128  , 16   ; Non temporal read used
BMETHOD  Pattern_NtWrite_SSE128 , DumpStart_NtWrite_SSE128 , DumpStop_NtWrite_SSE128 , 16
BMETHOD  Pattern_NtRCopy_SSE128 , DumpStart_NtRCopy_SSE128 , DumpStop_NtRCopy_SSE128 , 16   ; Non temporal read used 
BMETHOD  Pattern_Read_AVX256    , DumpStart_Read_AVX256    , DumpStop_Read_AVX256    , 32
BMETHOD  Pattern_NtWrite_AVX256 , DumpStart_NtWrite_AVX256 , DumpStop_NtWrite_AVX256 , 32
BMETHOD  Pattern_NtCopy_AVX256  , DumpStart_NtCopy_AVX256  , DumpStop_NtCopy_AVX256  , 32
BMETHOD  Pattern_Read_AVX512    , DumpStart_Read_AVX512    , DumpStop_Read_AVX512    , 64
BMETHOD  Pattern_NtWrite_AVX512 , DumpStart_NtWrite_AVX512 , DumpStop_NtWrite_AVX512 , 64
BMETHOD  Pattern_NtCopy_AVX512  , DumpStart_NtCopy_AVX512  , DumpStop_NtCopy_AVX512  , 64
BMETHOD  Pattern_Dot_FMA256     , DumpStart_Dot_FMA256     , DumpStop_Dot_FMA256     , 32
BMETHOD  Pattern_Dot_FMA512     , DumpStart_Dot_FMA512     , DumpStop_Dot_FMA512     , 64

;--- Supports non-temporal read for 256-bit and 512-bit, VMOVNTDQA ---

MethodsNonTemporal_AVX2:

BMETHOD  Pattern_Read_MOV64     , DumpStart_Read_MOV64     , DumpStop_Read_MOV64     , 8
BMETHOD  Pattern_Write_MOV64    , DumpStart_Write_MOV64    , DumpStop_Write_MOV64    , 8
BMETHOD  Pattern_Copy_MOV64     , DumpStart_Copy_MOV64     , DumpStop_Copy_MOV64     , 8
BMETHOD  Pattern_Modify_NOT64   , DumpStart_Modify_NOT64   , DumpStop_Modify_NOT64   , 8
BMETHOD  Pattern_Write_STOSQ    , DumpStart_Write_STOSQ    , DumpStop_Write_STOSQ    , 8
BMETHOD  Pattern_Copy_MOVSQ     , DumpStart_Copy_MOVSQ     , DumpStop_Copy_MOVSQ     , 8
BMETHOD  Pattern_NtRead_SSE128  , DumpStart_NtRead_SSE128  , DumpStop_NtRead_SSE128  , 16   ; Non temporal read used
BMETHOD  Pattern_NtWrite_SSE128 , DumpStart_NtWrite_SSE128 , DumpStop_NtWrite_SSE128 , 16
BMETHOD  Pattern_NtRCopy_SSE128 , DumpStart_NtRCopy_SSE128 , DumpStop_NtRCopy_SSE128 , 16   ; Non temporal read used
BMETHOD  Pattern_NtRead_AVX256  , DumpStart_NtRead_AVX256  , DumpStop_NtRead_AVX256  , 32   ; Non temporal read used
BMETHOD  Pattern_NtWrite_AVX256 , DumpStart_NtWrite_AVX256 , DumpStop_NtWrite_AVX256 , 32
BMETHOD  Pattern_NtRCopy_AVX256 , DumpStart_NtRCopy_AVX256 , DumpStop_NtRCopy_AVX256 , 32   ; Non temporal read used
BMETHOD  Pattern_NtRead_AVX512  , DumpStart_NtRead_AVX512  , DumpStop_NtRead_AVX512  , 64   ; Non temporal read used
BMETHOD  Pattern_NtWrite_AVX512 , DumpStart_NtWrite_AVX512 , DumpStop_NtWrite_AVX512 , 64
BMETHOD  Pattern_NtRCopy_AVX512 , DumpStart_NtRCopy_AVX512 , DumpStop_NtRCopy_AVX512 , 64   ; Non temporal read used
BMETHOD  Pattern_Dot_FMA256     , DumpStart_Dot_FMA256     , DumpStop_Dot_FMA256     , 32
BMETHOD  Pattern_Dot_FMA512     , DumpStart_Dot_FMA512     , DumpStop_Dot_FMA512     , 64

;--- End of methods list ---
