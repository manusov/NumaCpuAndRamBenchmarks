;------------------------------------------------------------------------------;
; MPECSB = Memory Performance Engine control and status block                  ;
;------------------------------------------------------------------------------;

; Test input and output operands located by alignment-optimal ordering
; and 64-byte cache line allocation

;--- Test input ---

align 64
ICBB:                     ; ICBB = Input control block base, 
                          ; label ICBB used for relative offsets, keep layout
BlockBase1       DQ  ?    ; [00] Base address of target memory block 
BlockBase2       DQ  ?    ; [08] Base address of second target memory block, if used
BlockSize        DQ  ?    ; [16] Size of each memory block, bytes
OperandWidth     DD  ?    ; [24] Used instructions operand width, bits 
NumThreads       DD  ?    ; [28] Number of execution threads, 1 if single thread
NumRepeats       DD  ?    ; [32] Number of measurement repeats
SelectMethod     DB  ?    ; [36] Select test ASM method, one of procedure
OptionHT         DB  ?    ; [37] Hyper-threading: 0=None, 1=Present, not used, 2=Used
OptionNUMA       DB  ?    ; [38] NUMA: 0=None, 1=No control, 2=Optimal, 3=Non optim.
OptionTarget     DB  ?    ; [39] Target object: 0=L1, 1=L2, 2=L3, 3=RAM, 4=Custom
OptionMeasure    DB  ?    ; [40] Measurement: 0=brief, 1=careful

;--- Test output ---

align 64
OCBB:                     ; OCBB = Output control block base,
                          ; label OCBB used for relative offsets, keep layout 
dTms             DD  ?    ; [00] Test time (without 1 sec measure Fclk), units=ms
dTSCsec          DD  ?    ; [04] Delta-TSC per second, units = MHz/10 for visual
dTSCiteration    DD  ?    ; [08] Delta-TSC per iteration, units = Clk/1000 for visual - REJECTED
dTSCinstruction  DD  ?    ; [12] Delta-TSC per instruction, units = Clk/1000 for v.
SpeedMBPS        DD  ?    ; [16] Data transfer speed, units = MBPS

;--- Transit ---

;--- TSC Frequency (Hz) and Period (fs), 1 fs = 10^-15 seconds ---
; Transit version for MPE, see also SYSPARMS.INC for same fields
;---
MpeTscClockHz    DQ  ?    ; Time Stamp Counter frequency, Hz
MpeTscPeriodFs   DQ  ?    ; Time Stamp Counter period, Fs

;--- OS time (filetime) ---
MpeOsTime        DQ  ?    ; Variable updated by OS API

;--- Field for custom block size edit ---
BlockSizeEdit    DQ  ?    ; CUSTOM for selection: L1/L2/L3/Custom

;--- Performance pattern internal parameters ---
; SSE4.1 validation required for 128-bit MOVNTDQA
; AVX2 validation required for 256-bit VMOVNTDQA
; Note non-temporal read recommended for AMD CPU only,
; Note non-temporal write recommended for all CPUs vendors
NonTemporalRead  DB  ?    ; 0 = Default NT write only, 1 = SSE4.1 modify, 2 = AVX2 modify

;--- Memory allocation control ---
MemoryRequestSize  DQ  ?  ; Requested block size
MemoryHeapID       DQ  ?  ; Allocated memory heap ID
MemoryTrueBase     DQ  ?  ; Allocated memory true base, used for release
MemoryAlignedBase  DQ  ?  ; Allocated memory aligned base, align=4KB, 0=Error

;--- Benchmarks select and routine dump support ---
PatternSelector    DQ  ?  ; Address for call performance pattern
DumpMethodStart    DQ  ?  ; Start of fragment, visualized as dump during benchmarks
DumpMethodLength   DD  ?  ; Length of fragment, visualized as dump during benchmarks, bytes

;--- Threads control blocks for 16 threads ---
; 1 block per thread, all parameters = QWORDS
; Offset    Value
;--------------------------------------------------------------------
;  000      Event Handle
;  008      Thread Handle
;  016      Affinity Mask = F (True Affinity Mask, Options)
;  024      Subroutine entry point
;  032      Allocated block base, also Block #1 base address
;  040      Block #2 base address  
;  048      Block #3 base address
;  056      Block size, units = Instructions
;  064      Number of measurement iterations
;  072      Store output RAX = Clocks per one target instruction * Nrepeats
;  080      Store output RDX = Bitmap with "1" at logical CPU number position
;           This feature reserved for affinitization test 
;  088      Reserved
;  096      Reserved
;  104      Storage for true AFFINITY MASK, because modified as f(options)
;  112      Storage for true BASE, allocated by NUMA API (used for release)
;  120      Storage for true SIZE, allocated by NUMA API (used for release)
;--------------------------------------------------------------------
align 64
ThreadsControl     DB 2048 DUP (?)   ; 16 blocks x 128 bytes = 2048 bytes
MtEventsHandles    DQ 16 DUP (?)     ; 128 bytes, separate list requied for WaitForMultipleObjects function


