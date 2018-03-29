;--- Benchmarks repeats select table ---
; Number of measurement iterations N = F ( target object , careful mode )
; Target Objects: L1, L2, L3 caches, DRAM, Custom (different sizes)
; Careful mode: Fast, Slow

BenchmarksRepeats:
; Number of measurement iterations for objects, for normal mode, for Cache&RAM mode
; Group must be 20 bytes size for indexed access

DD  L1_REPEATS , L2_REPEATS , L3_REPEATS , MEM_REPEATS , CM_REPEATS

; Number of measurement iterations for objects, for slow mode, for Cache&RAM mode
; Group must be 20 bytes size for indexed access

DD  L1_REPEATS_SLOW , L2_REPEATS_SLOW , L3_REPEATS_SLOW , MEM_REPEATS_SLOW , CM_REPEATS_SLOW


