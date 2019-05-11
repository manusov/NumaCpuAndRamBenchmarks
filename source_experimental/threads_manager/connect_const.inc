;------------------------------------------------------------------------------;
;                  Library for memory and threads management:                  ;
;               constants and variables with pre-defined values.               ;
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
;                 Benchmarks calibration repeats select table.                 ;
;------------------------------------------------------------------------------;

; Number of measurement iterations N = F ( target object , careful mode )
; Target Objects: L1, L2, L3, L4 caches, DRAM, Storage, Custom (different sizes)
; Careful mode: Fast(inaccurate), Slow(precision)

BenchmarksRepeats:

; Number of measurement iterations for objects, for fast (inaccurate) mode,
; for Cache, RAM, Custom modes
; Group must be 24 bytes size for indexed access, 6 dwords
DD  L1_REPEATS_FAST , L2_REPEATS_FAST , L3_REPEATS_FAST , L4_REPEATS_FAST
DD  DRAM_REPEATS_FAST , CUSTOM_REPEATS_FAST

; Number of measurement iterations for objects, for slow (precision) mode,
;; for Cache, RAM, Custom modes
; Group must be 24 bytes size for indexed access, 6 dwords
DD  L1_REPEATS_SLOW , L2_REPEATS_SLOW , L3_REPEATS_SLOW , L4_REPEATS_SLOW
DD  DRAM_REPEATS_SLOW , CUSTOM_REPEATS_SLOW

; Constants for adaptive measurement mode, target measurement time, seconds
; double floating point constants

AdaptiveRepeats:
DQ  ADAPTIVE_FAST
DQ  ADAPTIVE_SLOW