
align 8
BandwidthPatterns:
; Temporal (cacheable read and write) mode
DW  Pattern_Read_MOV64      - BandwidthPatternsBase       ; ID = 0
DW  Pattern_Write_MOV64     - BandwidthPatternsBase
DW  Pattern_Copy_MOV64      - BandwidthPatternsBase
DW  Pattern_Modify_NOT64    - BandwidthPatternsBase
DW  Pattern_Write_STOSQ     - BandwidthPatternsBase
DW  Pattern_Copy_MOVSQ      - BandwidthPatternsBase
DW  Pattern_Read_MMX64      - BandwidthPatternsBase
DW  Pattern_Write_MMX64     - BandwidthPatternsBase
DW  Pattern_Copy_MMX64      - BandwidthPatternsBase
DW  Pattern_Read_SSE128     - BandwidthPatternsBase       ; ID = 9
DW  Pattern_Write_SSE128    - BandwidthPatternsBase
DW  Pattern_Copy_SSE128     - BandwidthPatternsBase
DW  Pattern_Read_AVX256     - BandwidthPatternsBase       ; ID = 12
DW  Pattern_Write_AVX256    - BandwidthPatternsBase
DW  Pattern_Copy_AVX256     - BandwidthPatternsBase
DW  Pattern_Read_AVX512     - BandwidthPatternsBase       ; ID = 15
DW  Pattern_Write_AVX512    - BandwidthPatternsBase
DW  Pattern_Copy_AVX512     - BandwidthPatternsBase
DW  Pattern_Dot_FMA256      - BandwidthPatternsBase  
DW  Pattern_Dot_FMA512      - BandwidthPatternsBase       ; ID = 19  
; Non-temporal (non-cacheable write) mode 
DW  Pattern_NtWrite_SSE128  - BandwidthPatternsBase       ; ID = 20
DW  Pattern_NtCopy_SSE128   - BandwidthPatternsBase       ; This duplicated 1
DW  Pattern_NtWrite_AVX256  - BandwidthPatternsBase
DW  Pattern_NtCopy_AVX256   - BandwidthPatternsBase
DW  Pattern_NtWrite_AVX512  - BandwidthPatternsBase
DW  Pattern_NtCopy_AVX512   - BandwidthPatternsBase       ; ID = 25
; Non-temporal (non-cacheable read and write) mode
DW  Pattern_NtRead_SSE128   - BandwidthPatternsBase       ; ID = 26
DW  Pattern_NtRCopy_SSE128  - BandwidthPatternsBase
DW  Pattern_NtRead_AVX256   - BandwidthPatternsBase
DW  Pattern_NtRCopy_AVX256  - BandwidthPatternsBase
DW  Pattern_NtRead_AVX512   - BandwidthPatternsBase   
DW  Pattern_NtRCopy_AVX512  - BandwidthPatternsBase       ; ID = 31
; Non-temporal (non-cacheable read-by-prefetch and write) mode
DW  Pattern_NtpRead_SSE128  - BandwidthPatternsBase       ; ID = 32
DW  Pattern_NtCopy_SSE128   - BandwidthPatternsBase       ; This duplicated 1
DW  Pattern_NtpRead_AVX256  - BandwidthPatternsBase       ; ID = 34
; Reserved for same 256-512 bit operations
; FMA with non-temporal store
; ... reserved ...
