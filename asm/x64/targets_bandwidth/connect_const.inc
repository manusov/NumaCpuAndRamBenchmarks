;------------------------------------------------------------------------------;
;                  This performance patterns used in the                       ; 
;              "Run simple", "Run drawings" benchmark scenarios.               ;
;          Library of performance patterns for bandwidth measurement:          ;
;               constants and variables with pre-defined values.               ;
;------------------------------------------------------------------------------;

; Entry for compact encoding performance method, 
; use 16-bit base-relative offsets instead full 64-bit offsets
MACRO BANDWIDTH_ENTRY  x1, x2, x3, x4   ; 6 bytes per entry, compact encoding
{
DW x1-BandwidthPatternsBase   ; Target routine entry point
DW x2-BandwidthPatternsBase   ; Dump fragment start, dump show at drawings Y=F(X)
DB x3-x2                      ; Dump fragment size
DB x4                         ; Data Bytes per Instruction,
}                             ; example YMM register op. = 32 bytes (256 bit) 

align 2
BandwidthPatterns:
; Temporal (cacheable read and write) mode
BANDWIDTH_ENTRY  Pattern_Read_MOV64     , DumpStart_Read_MOV64     , DumpStop_Read_MOV64     , 8     ; ID = 0
BANDWIDTH_ENTRY  Pattern_Write_MOV64    , DumpStart_Write_MOV64    , DumpStop_Write_MOV64    , 8
BANDWIDTH_ENTRY  Pattern_Copy_MOV64     , DumpStart_Copy_MOV64     , DumpStop_Copy_MOV64     , 8
BANDWIDTH_ENTRY  Pattern_Modify_NOT64   , DumpStart_Modify_NOT64   , DumpStop_Modify_NOT64   , 8 
BANDWIDTH_ENTRY  Pattern_Write_STOSQ    , DumpStart_Write_STOSQ    , DumpStop_Write_STOSQ    , 8
BANDWIDTH_ENTRY  Pattern_Copy_MOVSQ     , DumpStart_Copy_MOVSQ     , DumpStop_Copy_MOVSQ     , 8 
BANDWIDTH_ENTRY  Pattern_Read_MMX64     , DumpStart_Read_MMX64     , DumpStop_Read_MMX64     , 8
BANDWIDTH_ENTRY  Pattern_Write_MMX64    , DumpStart_Write_MMX64    , DumpStop_Write_MMX64    , 8
BANDWIDTH_ENTRY  Pattern_Copy_MMX64     , DumpStart_Copy_MMX64     , DumpStop_Copy_MMX64     , 8
BANDWIDTH_ENTRY  Pattern_Read_SSE128    , DumpStart_Read_SSE128    , DumpStop_Read_SSE128    , 16    ; ID = 9
BANDWIDTH_ENTRY  Pattern_Write_SSE128   , DumpStart_Write_SSE128   , DumpStop_Write_SSE128   , 16
BANDWIDTH_ENTRY  Pattern_Copy_SSE128    , DumpStart_Copy_SSE128    , DumpStop_Copy_SSE128    , 16
BANDWIDTH_ENTRY  Pattern_Read_AVX256    , DumpStart_Read_AVX256    , DumpStop_Read_AVX256    , 32    ; ID = 12
BANDWIDTH_ENTRY  Pattern_Write_AVX256   , DumpStart_Write_AVX256   , DumpStop_Write_AVX256   , 32
BANDWIDTH_ENTRY  Pattern_Copy_AVX256    , DumpStart_Copy_AVX256    , DumpStop_Copy_AVX256    , 32
BANDWIDTH_ENTRY  Pattern_Read_AVX512    , DumpStart_Read_AVX512    , DumpStop_Read_AVX512    , 64    ; ID = 15
BANDWIDTH_ENTRY  Pattern_Write_AVX512   , DumpStart_Write_AVX512   , DumpStop_Write_AVX512   , 64
BANDWIDTH_ENTRY  Pattern_Copy_AVX512    , DumpStart_Copy_AVX512    , DumpStop_Copy_AVX512    , 64
BANDWIDTH_ENTRY  Pattern_Dot_FMA256     , DumpStart_Dot_FMA256     , DumpStop_Dot_FMA256     , 32  
BANDWIDTH_ENTRY  Pattern_Dot_FMA512     , DumpStart_Dot_FMA512     , DumpStop_Dot_FMA512     , 64    ; ID = 19  
; Non-temporal (non-cacheable write) mode 
BANDWIDTH_ENTRY  Pattern_NtWrite_SSE128 , DumpStart_NtWrite_SSE128 , DumpStop_NtWrite_SSE128 , 16    ; ID = 20
BANDWIDTH_ENTRY  Pattern_NtCopy_SSE128  , DumpStart_NtCopy_SSE128  , DumpStop_NtCopy_SSE128  , 16    ; This duplicated 1
BANDWIDTH_ENTRY  Pattern_NtWrite_AVX256 , DumpStart_NtWrite_AVX256 , DumpStop_NtWrite_AVX256 , 32
BANDWIDTH_ENTRY  Pattern_NtCopy_AVX256  , DumpStart_NtCopy_AVX256  , DumpStop_NtCopy_AVX256  , 32
BANDWIDTH_ENTRY  Pattern_NtWrite_AVX512 , DumpStart_NtWrite_AVX512 , DumpStop_NtWrite_AVX512 , 64
BANDWIDTH_ENTRY  Pattern_NtCopy_AVX512  , DumpStart_NtCopy_AVX512  , DumpStop_NtCopy_AVX512  , 64    ; ID = 25
; Non-temporal (non-cacheable read and write) mode
BANDWIDTH_ENTRY  Pattern_NtRead_SSE128  , DumpStart_NtRead_SSE128  , DumpStop_NtRead_SSE128  , 16    ; ID = 26
BANDWIDTH_ENTRY  Pattern_NtRCopy_SSE128 , DumpStart_NtRCopy_SSE128 , DumpStop_NtRCopy_SSE128 , 16
BANDWIDTH_ENTRY  Pattern_NtRead_AVX256  , DumpStart_NtRead_AVX256  , DumpStop_NtRead_AVX256  , 32
BANDWIDTH_ENTRY  Pattern_NtRCopy_AVX256 , DumpStart_NtRCopy_AVX256 , DumpStop_NtRCopy_AVX256 , 32
BANDWIDTH_ENTRY  Pattern_NtRead_AVX512  , DumpStart_NtRead_AVX512  , DumpStop_NtRead_AVX512  , 64   
BANDWIDTH_ENTRY  Pattern_NtRCopy_AVX512 , DumpStart_NtRCopy_AVX512 , DumpStop_NtRCopy_AVX512 , 64    ; ID = 31
; Non-temporal (non-cacheable read-by-prefetch and write) mode
BANDWIDTH_ENTRY  Pattern_NtpRead_SSE128 , DumpStart_NtpRead_SSE128 , DumpStop_NtpRead_SSE128 , 16    ; ID = 32
BANDWIDTH_ENTRY  Pattern_NtCopy_SSE128  , DumpStart_NtCopy_SSE128  , DumpStop_NtCopy_SSE128  , 16    ; This duplicated 1
BANDWIDTH_ENTRY  Pattern_NtpRead_AVX256 , DumpStart_NtpRead_AVX256 , DumpStop_NtpRead_AVX256 , 32    ; ID = 34
; Reserved for same 256-512 bit operations
; FMA with non-temporal store
; ... reserved ...
