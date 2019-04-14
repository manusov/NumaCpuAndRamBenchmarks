; Required 48 Kilobytes miscellaneous buffer
TEMP_BUFFER_SIZE      EQU 49152

; Constant for memory allocation option
MEM_LARGE_PAGES       EQU  020000000h

; Selector for all processor groups
ALL_PROCESSOR_GROUPS  EQU 0000FFFFh 

; Thread control entry, or entire benchmark control if single thread
; Note keep 128 bytes per entry, see ThreadEntry, can be non parametrized coding
; Note keep all pairs: Affinity Mask + Affinity Group as 2 sequental qwords,
; this rule required because WinAPI use with direct output store to this qwords 
struct THCTRL
EventStart      dq  ?   ; Event Handle for operation START signal
EventDone       dq  ?   ; Event Handle for operation COMPLETE signal 
ThreadHandle    dq  ?   ; Thread Handle for execution thread
ThreadAffinity  dq  ?   ; Affinity Mask = F (True Affinity Mask, Options) 
ThreadGroup     dq  ?   ; Processor group, associated with affinity mask
EntryPoint      dq  ?   ; Entry point to operation subroutine
Base1           dq  ?   ; Source for read, destination for write and modify
Base2           dq  ?   ; Destination for copy
SizeBytes       dq  ?   ; Block size, units = bytes, for benchmarking, not for memory allocation (REDUNDANT FIELD RESERVED) 
SizeInst        dq  ?   ; Block size, units = instructions, for benchmarking (MUST BE UPDATED PER DRAW ITERATION)
LargePages      dq  ?   ; 0=not supported, 1=supported but not used, 2=used (REDUNDANT FIELD RESERVED)
Repeats         dq  ?   ; Number of measurement repeats for calibrate
AffinityMode    dq  ?   ; Affinity mode: 0=None, 1=Without groups, 2=With groups 
OrigBase        dq  ?   ; True (before alignment) memory block base for release (REDUNDANT FIELD RESERVED)
OrigAffinity    dq  ?   ; Store original affinity mask, also storage for return (STORED BY THREAD, NOT BY INITIALIZER) 
OrigGroup       dq  ?   ; Store processor group for original offinity mask (STORED BY THREAD, NOT BY INITIALIZER) 
ends


; Test USER parameters layout declaration
struct UPB
OptionAsm         dd  ?    ; Select test ASM method, one of procedures
OptionTarget      dd  ?    ; Objects { L1, L2, L3, L4, DRAM, STORAGE, CUSTOM }
OptionThreads     dd  ?    ; Number of threads, 0=auto SMP, 1-256 value
OptionHT          dd  ?    ; HT { GRAY_NOT_SUP, DISABLED, ENABLED }
OptionPG          dd  ?    ; PG { GRAY_NOT_SUP, DISABLED, ENABLED } 
OptionNUMA        dd  ?    ; { GRAY_NOT_SUP, UNAWARE, FORCE LOCAL, FORCE REMOTE }
OptionLP          dd  ?    ; LP { GRAY_NOT_SUP, DISABLED, ENABLED }  
OptionMeasure     dd  ?    ; 0=fast, 1=slow, 2=fast adaptive, 3=slow adaptive 
CustomBlockStart  dq  ?    ; Override start block size, or 0=default
CustomBlockEnd    dq  ?    ; Override end block size, or 0=default
CustomBlockDelta  dq  ?    ; Override delta block size, or 0=default
ends


; Test INPUT parameters layout declaration,
struct IPB
; First 8 items of IPB associated with first 8 items of UPB
UpdatedAsm        dd  ?    ; Routine selector, set after detect features
UpdatedTarget     dd  ?    ; Objects { L1, L2, L3, L4, DRAM, STORAGE, CUSTOM }
UpdatedThreads    dd  ?    ; Number of threads, set after detect features
UpdatedHT         dd  ?    ; 0=Not sup. by platform, 1=sup. but not used, 2=used
UpdatedPG         dd  ?    ; 0=Not sup. by platform, 1=sup. but not used, 2=used 
UpdatedNUMA       dd  ?    ; 0=None, 1=No control, 2=Optimal, 3=Non optim. 
UpdatedLP         dd  ?    ; 0=Not sup. by platform, 1=sup. but not used, 2=used
UpdatedMeasure    dd  ?    ; 0=fast, 1=slow, 2=fast adaptive, 3=slow adaptive 
; Memory allocation results
AllocatedBlock1   dq  ?
AllocatedBlock2   dq  ?
; This items generated as f( first 8 items, platform configuration )
StartBlockSize    dq  ?    ; Start block size, or 0=default
EndBlockSize      dq  ?    ; End block size, or 0=default
DeltaBlockSize    dq  ?    ; Delta block size, or 0=default
OperandWidth      dd  ?    ; Used instructions operand width, bits 
MeasureRepeats    dd  ?    ; Number of measurement repeats
GroupsCount       dd  ?    ; Number of Processor Groups
DomainsCount      dd  ?    ; Number of NUMA domains
PageSize          dq  ?    ; Memory page size, bytes
MemoryTotal       dq  ?    ; Memory allocated, per all threads
MemoryPerThread   dq  ?    ; Memory allocated, per each thread 
; Benchmarks select and routine dump support
PatternRoutine    dq  ?    ; Pointer to target benchmark routine
DumpMethodStart   dq  ?    ; Start of fragment, visualized as dump during benchmarks
DumpMethodLength  dd  ?    ; Length of fragment, visualized as dump during benchmarks, bytes
ends


; Test OUTPUT parameters layout declaration
struct OPB
DeltaTSC          dq  ?    ; TSC measured clock, Hz, 64-bit long integer
TSCfrequencyHz    dq  ?    ; TSC frequency, Hz, as double precision 
TSCfrequencyMHz   dq  ?    ; TSC frequency, MHz, as double precision
TSCperiodS        dq  ?    ; TSC period, seconds, as double precision
TSCperiodNs       dq  ?    ; TSC period, nanoseconds, as double precision 
OStimerDelta      dq  ?    ; delta time at units = 100 ns, 64-bit long integer 
TSCtimerDelta     dq  ?    ; delta time at units = 1 TSC clk., 64-bit long int. 
ends


; Assembler Methods (AM) option values count and Bandwidth/Latency criteria
AM_BYTE_COUNT     EQU  22     ; 22 bytes per methods primary list
AM_LIMIT          EQU  36+1   ; 36 names for assembler methods
LATENCY_MODE      EQU  35     ; modes ID = 35, 36 for latency measurement

; Options limits
HT_LIMIT          EQU  2
LP_LIMIT          EQU  2
NUMA_LIMIT        EQU  3
TARGET_LIMIT      EQU  5 
MEASURE_LIMIT     EQU  3

 
