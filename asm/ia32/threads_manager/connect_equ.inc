;------------------------------------------------------------------------------;
;   Library for memory and threads management, include adjust measurements:    ;
;                               data definitions.                              ;
;------------------------------------------------------------------------------;

; Benchmarks repeat parameters, precision=f(repeats), for Cache&RAM mode
; Calibrate iterations for objects, normal mode
; this constants must be multiple of LATENCY DIVISOR (50)
L1_REPEATS_FAST         EQU  100000
L2_REPEATS_FAST         EQU  50000
L3_REPEATS_FAST         EQU  1000
L4_REPEATS_FAST         EQU  200
DRAM_REPEATS_FAST       EQU  100
CUSTOM_REPEATS_FAST     EQU  100000
; Calibrate iterations for objects, slow mode
; this constants must be multiple of LATENCY DIVISOR (50)
L1_REPEATS_SLOW         EQU  2000000
L2_REPEATS_SLOW         EQU  500000
L3_REPEATS_SLOW         EQU  10000
L4_REPEATS_SLOW         EQU  2000
DRAM_REPEATS_SLOW       EQU  200
CUSTOM_REPEATS_SLOW     EQU  1000000

; Repeats options definitions: 
; objects entries count (L1-L4, DRAM, Custom),
; bytes per entry (dword=4 bytes), 
; list size
REPEAT_OBJECTS          EQU  6
REPEAT_ENTRY_BYTES      EQU  4
REPEAT_SERIES_BYTES     EQU  REPEAT_OBJECTS * REPEAT_ENTRY_BYTES    

; Constants for adaptive measurement mode, target measurement time, seconds,
; for fast and slow (precision) modes,
; adaptive logic criteria is:
; number of iterations must be adjusted for measurement time equal this time
ADAPTIVE_FAST           EQU  0.01
ADAPTIVE_SLOW           EQU  0.05

; Constants for custom block size restrictions
CUSTOM_BLOCK_MIN        EQU  16 * 1024
CUSTOM_BLOCK_MAX        EQU  64 * 1024 * 1024
CUSTOM_BLOCK_ALIGN      EQU  16 * 1024

