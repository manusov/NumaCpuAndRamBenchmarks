;------------------------------------------------------------------------------;
;  System support data definitions: constants, structures, macro definitions.  ;
;     Make this module as separate file for useable by samples under debug     ;
;------------------------------------------------------------------------------;

; Block size for RAM benchmark
CONST_RAM_BLOCK       EQU  32*1024*1024

; Constant for memory allocation option
MEM_LARGE_PAGES = 020000000h

; Constant for processor groups management, as WinAPI input parameter means
; get total processors count, for all Processor Groups 
ALL_PROCESSOR_GROUPS = 0000FFFFh 

; Benchmarks repeat parameters, precision=f(repeats), for Cache&RAM mode
L1_REPEATS            EQU  100000   ; Number of measur. iter. for objects, normal mode
L2_REPEATS            EQU  50000
L3_REPEATS            EQU  1000
MEM_REPEATS           EQU  100
CM_REPEATS            EQU  100000
L1_REPEATS_SLOW       EQU  2000000  ; Number of measur. iter. for objects, slow mode
L2_REPEATS_SLOW       EQU  500000
L3_REPEATS_SLOW       EQU  10000
MEM_REPEATS_SLOW      EQU  200
CM_REPEATS_SLOW       EQU  1000000

; Thread control entry, or entire benchmark control if single thread
; Note keep 128 bytes per entry, see ThreadEntry, can be non parametrized coding
; Note keep all pairs: Affinity Mask + Affinity Group as 2 sequental qwords,
; this rule required because WinAPI use with direct output store to this qwords 
struct THCTRL
EventHandle     dq  ?   ; Event Handle for operation complete signal 
ThreadHandle    dq  ?   ; Thread Handle for execution thread
ThreadAffinity  dq  ?   ; Affinity Mask = F (True Affinity Mask, Options) 
ThreadGroup     dq  ?   ; Processor group, associated with affinity mask
EntryPoint      dq  ?   ; Entry point to operation subroutine
Base1           dq  ?   ; Source for read, destination for write and modify
Base2           dq  ?   ; Destination for copy
SizeBytes       dq  ?   ; Block size, units = bytes, for memory allocation 
SizeInst        dq  ?   ; Block size, units = instructions, for benchmarking
LargePages      dq  ?   ; Bit D0=Large Pages, other bits [1-63] = reserved
Repeats         dq  ?   ; Number of measurement repeats
AffinityMode    dq  ?   ; Affinity mode: 0=None, 1=Without groups, 2=With groups 
Reserved1       dq  ?   ; Reserved 
TrueBase        dq  ?   ; True (before alignment) memory block base for release
OrigAffinity    dq  ?   ; Store original affinity mask, also storage for return 
OrigGroup       dq  ?   ; Store processor group for original offinity mask 
ends

; NUMA node description entry, not a same as thread description enrty
; Note keep 64 bytes per entry, see ThreadEntry, can be non parametrized coding
; Note keep all pairs: Affinity Mask + Affinity Group as 2 sequental qwords,
; this rule required because WinAPI use with direct output store to this qwords 
struct NUMACTRL
NodeID          dq  ?   ; NUMA Node number, if no NUMA, this field = 0 for all entries
NodeAffinity    dq  ?   ; NUMA Node affinity mask
NodeGroup       dq  ?   ; Reserved for Processors Group number
AlignedBase     dq  ?   ; Block size after alignment, for read/write operation
AlignedSize     dq  ?   ; Block base address after alignment, for r/w operation 
TrueBase        dq  ?   ; Base address, returned when allocated, for release, 0=not used 
Reserved        dq  2 DUP (?)    ; Reserved for alignment
ends

; Maximum number of supported threads: total and per processor group
MAX_THREADS            EQU  256      ; old = 64, this extension is UNDER CONSTRUCTION
MAX_THREADS_PER_GROUP  EQU  64

; Maximum number of supported NUMA nodes
MAX_NODES             EQU  64

; 8192 bytes, 64 entries for (multi) thread management
THCTRL_SIZE           EQU  sizeof.THCTRL
THREADS_CONTROL_SIZE  EQU  MAX_THREADS * THCTRL_SIZE

; 512 bytes, 64 handles, separate list req. for WaitForMultipleObjects function  
EVENT_SIZE            EQU  8
EVENTS_CONTROL_SIZE   EQU  MAX_THREADS * EVENT_SIZE

; 4096 bytes, 64 entries for NUMA nodes description, not a same as THREAD
NUMACTRL_SIZE         EQU  sizeof.NUMACTRL
NUMA_CONTROL_SIZE     EQU  MAX_NODES * NUMACTRL_SIZE  

; Vector brief, test parameters
VECTOR_BRIEF_DATA_SIZE       EQU  4096     ; Part of L1 (bytes) used as buffer

; Vector brief, temporary buffer allocation parameters
VECTOR_BRIEF_TEMP_DATA       EQU  0        ; Buffer for performance patterns
VECTOR_BRIEF_TEMP_RESULTS    EQU  4096     ; Buffer for save results
VECTOR_BRIEF_TEMP_TRANSIT    EQU  8192     ; Buffer for transit data 

; Vector brief results description block, not supported data = 0
struct VECTORBRIEF
CpuName         db  48 DUP (?)  ; CPU name string
Reserved        dq  ?           ; Reserved for string terminator byte + align
TscClock        dq  ?           ; TSC clock, Hz, increments per second
TscClockNs      dq  ?           ; Nanoseconds per TSC clock, double precision
StdFeatures     dq  ?           ; EDX(low), ECX(high) after CPUID#00000001h
ExtFeatures     dq  ?           ; EDX(low), ECX(high) after CPUID#80000001h
AddFeatures     dq  ?           ; EBX(low), ECX(high) after CPUID#00000007h  
OsContext       dq  ?           ; XCR0
dtSse128read    dq  ?           ; TSC clocks per SSE128 Read pattern
dtSse128write   dq  ?           ; TSC clocks per SSE128 Write pattern
dtSse128copy    dq  ?           ; TSC clocks per SSE128 Copy pattern 
dtAvx256read    dq  ?           ; TSC clocks per AVX256 Read pattern
dtAvx256write   dq  ?           ; TSC clocks per AVX256 Write pattern
dtAvx256copy    dq  ?           ; TSC clocks per AVX256 Copy pattern 
dtAvx512read    dq  ?           ; TSC clocks per AVX512 Read pattern
dtAvx512write   dq  ?           ; TSC clocks per AVX512 Write pattern
dtAvx512copy    dq  ?           ; TSC clocks per AVX512 Copy pattern 
dtSse128sqrt    dq  ?           ; TSC clocks per SSE128 Square Root pattern
dtAvx256sqrt    dq  ?           ; TSC clocks per AVX256 Square Root pattern
dtAvx512sqrt    dq  ?           ; TSC clocks per AVX512 Square Root pattern
dtX87cos        dq  ?           ; TSC clocks per x87 Cosine (FCOS) pattern
dtX87sincos     dq  ?           ; TSC clocks per x87 Sine+Cosine (FSINCOS) pat. 
ends
