;------------------------------------------------------------------------------;
;                 Global Library: global used data definitions.                ; 
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
;  System support data definitions: constants, structures, macro definitions.  ;
;     Make this module as separate file for useable by samples under debug     ;
;------------------------------------------------------------------------------;
; Block size for DRAM benchmarks, default values 
; must be corrected by platform configuration checks
DEFAULT_DRAM_BLOCK    EQU  32 * 1024 * 1024
; Constant for memory allocation option
MEM_LARGE_PAGES       EQU  020000000h
; Selector for all processor groups
ALL_PROCESSOR_GROUPS  EQU  0000FFFFh 
; Required 48 Kilobytes miscellaneous buffer
TEMP_BUFFER_SIZE      EQU  48 * 1024

; Thread control entry, or entire benchmark control if single thread
; Note keep 64 bytes per entry, see ThreadEntry, can be non parametrized coding
; Note keep all pairs: Affinity Mask + Affinity Group as 2 sequental qwords,
; this rule required because WinAPI use with direct output store to this qwords 
struct THCTRL
EventStart      dd  ?   ; Event Handle for operation START signal
EventDone       dd  ?   ; Event Handle for operation COMPLETE signal 
ThreadHandle    dd  ?   ; Thread Handle for execution thread
ThreadAffinity  dd  ?   ; Affinity Mask = F (True Affinity Mask, Options) 
ThreadGroup     dd  ?   ; Processor group, associated with affinity mask
EntryPoint      dd  ?   ; Entry point to operation subroutine
Base1           dd  ?   ; Source for read, destination for write and modify
Base2           dd  ?   ; Destination for copy
SizeBytes       dd  ?   ; Block size, units = bytes, for benchmarking, not for memory allocation (REDUNDANT FIELD RESERVED) 
SizeInst        dd  ?   ; Block size, units = instructions, for benchmarking (MUST BE UPDATED PER DRAW ITERATION)
Repeats         dq  ?   ; Number of measurement repeats for precision requirements
LargePages      dw  ?   ; 0=not supported, 1=supported but not used, 2=used (REDUNDANT FIELD RESERVED)
AffinityMode    dw  ?   ; Affinity mode: 0=None, 1=Without groups, 2=With groups 
OrigBase        dd  ?   ; True (before alignment) memory block base for release (REDUNDANT FIELD RESERVED)
OrigAffinity    dd  ?   ; Store original affinity mask, also storage for return (STORED BY THREAD, NOT BY INITIALIZER) 
OrigGroup       dd  ?   ; Store processor group for original offinity mask (STORED BY THREAD, NOT BY INITIALIZER) 
ends

; NUMA node description entry, not a same as thread description enrty
; Note keep 32 bytes per entry, see ThreadEntry, can be non parametrized coding
; Note keep all pairs: Affinity Mask + Affinity Group as 2 sequental qwords,
; this rule required because WinAPI use with direct output store to this qwords 
struct NUMACTRL
NodeID          dd  ?   ; NUMA Node number, if no NUMA, this field = 0 for all entries
NodeAffinity    dd  ?   ; NUMA Node affinity mask
NodeGroup       dd  ?   ; Processors Group number, 0 if single group platform
AlignedBase     dd  ?   ; Block size after alignment, for read/write operation
AlignedSize     dd  ?   ; Block base address after alignment, for r/w operation 
TrueBase        dd  ?   ; Base address, returned when allocated, for release, 0=not used 
Reserved        dd  2 DUP (?)    ; Reserved for alignment
ends

; Maximum number of supported threads: total and per processor group
MAX_THREADS            EQU  256      ; old = 64, this extension is UNDER CONSTRUCTION
MAX_THREADS_PER_GROUP  EQU  64
; Maximum number of supported NUMA nodes
MAX_NODES              EQU  64
; Thread entry size = 64 bytes, const because optimal shift by 6 can be used
; 16384 bytes, 256 entries * 64 bytes, used for (multi) thread management
THCTRL_SIZE            EQU  sizeof.THCTRL
THREADS_LIST_SIZE      EQU  MAX_THREADS * THCTRL_SIZE
; Event handle size = 4 bytes (32-bit)
; 1024 bytes, 256 handles * 4 bytes, 
; separate sequental list req. for WaitForMultipleObjects function  
EVENT_SIZE             EQU  4
EVENTS_LIST_SIZE       EQU  MAX_THREADS * EVENT_SIZE
; NUMA node entry size = 32 bytes, const because optimal shift by 5 can be used
; 8192 bytes, 256 entries * 32 bytes,
; for NUMA nodes description, not a same as THREAD
NUMACTRL_SIZE          EQU  sizeof.NUMACTRL
NUMA_LIST_SIZE         EQU  MAX_NODES * NUMACTRL_SIZE  

; Test USER parameters layout declaration
struct UPB
OptionAsm         dd  ?    ; Select test ASM method, one of procedures
OptionTarget      dd  ?    ; Objects { L1, L2, L3, L4, DRAM, CUSTOM }
OptionParallel    dd  ?    ; Parallel { GRAY_NOT_SUP, DISABLED, ENABLED }
OptionHT          dd  ?    ; HT { GRAY_NOT_SUP, DISABLED, ENABLED }
OptionPG          dd  ?    ; PG { GRAY_NOT_SUP, DISABLED, ENABLED } 
OptionNUMA        dd  ?    ; NUMA { GRAY_NOT_SUP, UNAWARE, SINGLE_DOMAIN, FORCE LOCAL, FORCE REMOTE }
OptionLP          dd  ?    ; LP { GRAY_NOT_SUP, DISABLED, ENABLED }  
OptionMeasure     dd  ?    ; Measurement repeats { 0=fast, 1=slow, 2=fast adaptive, 3=slow adaptive }
OptionApprox      dd  ?    ; Approximation for X { 0=none, 1=X16, 2=X32 }
RunContext        dd  ?    ; Run context: 0 = "Run simple" , 1 = "Run drawings" 
CustomBlockStart  dd  ?    ; Override start block size, or 0=default
; YET NOT USED FIELDS:
; CustomBlockEnd    dd  ?    ; Override end block size, or 0=default
; CustomBlockDelta  dd  ?    ; Override delta block size, or 0=default
ends

; Test INPUT parameters layout declaration,
struct IPB
; First 8 items of IPB associated with first 8 items of UPB
UpdatedAsm        dd  ?    ; Routine selector, set after detect features
UpdatedTarget     dd  ?    ; Objects { L1, L2, L3, L4, DRAM, CUSTOM }
UpdatedThreads    dd  ?    ; Number of threads, set after detect features
UpdatedHT         dd  ?    ; 0=Not sup. by platform, 1=sup. but not used, 2=used
UpdatedPG         dd  ?    ; 0=Not sup. by platform, 1=sup. but not used, 2=used 
UpdatedNUMA       dd  ?    ; 0=None, 1=No control, 2-Single domain, 3=Optimal, 4=Non optim. 
UpdatedLP         dd  ?    ; 0=Not sup. by platform, 1=sup. but not used, 2=used
UpdatedMeasure    dd  ?    ; 0=fast, 1=slow, 2=fast adaptive, 3=slow adaptive
UpdatedApprox     dd  ?    ; Approximation for X { 0=none, 1=X16, 2=X32 }  
; Two work blocks used by benchmark scenario
AllocatedBlock1   dd  ?
AllocatedBlock2   dd  ?
; This items generated as f( first 8 items, platform configuration )
StartBlockSize    dd  ?    ; Start block size, or 0=default, unite = bytes
EndBlockSize      dd  ?    ; End block size, or 0=default
DeltaBlockSize    dd  ?    ; Delta block size, or 0=default
MeasureRepeats    dq  ?    ; Number of measurement repeats, note 64-bit value
OperandWidth      dd  ?    ; Used instructions operand width, bits 
GroupsCount       dd  ?    ; Number of Processor Groups
DomainsCount      dd  ?    ; Number of NUMA domains
PageSize          dd  ?    ; Memory page size, bytes
; Memory allocation results
MemoryTotal       dd  ?    ; Memory allocated, per all threads
MemoryPerThread   dd  ?    ; Memory allocated, per each thread 
; Benchmarks select and routine dump support
PatternRoutine    dd  ?    ; Pointer to target benchmark routine, this is fill routine for latency mode
WalkRoutine       dd  ?    ; Second routine, required for latency measurement
DumpMethodStart   dd  ?    ; Start of fragment, visualized as dump during benchmarks
DumpMethodLength  dd  ?    ; Length of fragment, visualized as dump during benchmarks, bytes
; Adaptive measurement timings support
AdaptiveSeconds   dq  ?    ; Target measurement time for stabilization, seconds, floation point
AdaptiveProduct   dq  ?    ; Size * Repeats = Product , practic calc: Repeats = Product / Size 
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
AM_BYTE_COUNT       EQU  22     ; 22 bytes per methods primary list
LATENCY_MODE        EQU  35     ; modes ID = 35, 36 for latency measurement

; Options limits and values
; 36 names for assembler methods
ASM_LIMIT           EQU  36+1

; Parallel threads option
PARALLEL_LIMIT      EQU  2
PARALLEL_NOT_SUP    EQU  0
PARALLEL_NOT_USED   EQU  1
PARALLEL_USED       EQU  2
; Target Object: Cache, DRAM, Custom
TARGET_LIMIT        EQU  5 
TARGET_L1           EQU  0
TARGET_L2           EQU  1
TARGET_L3           EQU  2
TARGET_L4           EQU  3
TARGET_DRAM         EQU  4
TARGET_CUSTOM       EQU  5
; HT = Hyper Threading
HT_LIMIT            EQU  2
HT_NOT_SUPPORTED    EQU  0
HT_NOT_USED         EQU  1
HT_USED             EQU  2
; PG = Processor Group
PG_LIMIT            EQU  2
PG_NOT_SUPPORTED    EQU  0
PG_NO_CONTROL       EQU  1
PG_OPTIMAL          EQU  2
; NUMA topology
NUMA_LIMIT          EQU  4
NUMA_NOT_SUPPORTED  EQU  0
NUMA_NO_CONTROL     EQU  1
NUMA_CURRENT_ONLY   EQU  2
NUMA_OPTIMAL        EQU  3
NUMA_NON_OPTIMAL    EQU  4    
; LP = Large Pages
LP_LIMIT            EQU  2
LP_NOT_SUPPORTED    EQU  0
LP_NOT_USED         EQU  1
LP_USED             EQU  2
; Measure repeats selection modes
MEASURE_LIMIT       EQU  3
MEASURE_BRIEF       EQU  0
MEASURE_CAREFUL     EQU  1    
MEASURE_B_ADAPTIVE  EQU  2
MEASURE_C_ADAPTIVE  EQU  3
; Approximation by X modes
APPROX_LIMIT        EQU  2
APPROX_NONE         EQU  0
APPROX_X16          EQU  1
APPROX_X32          EQU  2

;------------------------------------------------------------------------------;
;                  System parameters, detected at start.                       ;
;------------------------------------------------------------------------------;
; Cache info format
; Qword=Size, bytes,
; Word=Maximum threads at this level
; Word=Maximum APIC IDs per package
; For Trace Cache size in micro operations, not bytes
struct CPUCACHE
CpuidTraceCache   dq  ?         ; Instruction trace cache
CpuidTraceSmp     dw  ? , ?
CpuidL1Code       dq  ?	        ; L1 instruction cache
CpuidL1Csmp       dw  ? , ?
CpuidL1Data       dq  ?	        ; L1 data cache
CpuidL1Dsmp       dw  ? , ?
CpuidL2Unified    dq  ?	        ; L2 unified cache
CpuidL2Usmp       dw  ? , ?
CpuidL3Unified    dq  ?	        ; L3 unified cache
CpuidL3Usmp       dw  ? , ?
ends

;--- 32 bytes CACHE SIZE INFO ---
; Qword [000-007] = L1 instruction cache size (bytes)
; Qword [008-015] = L1 data cache size (bytes)
; Qword [016-023] = L2 unified cache size (bytes)
; Qword [024-031] = L3 unified cache size (bytes)
;--- 16 bytes CACHE TOPOLOGY INFO ---
; Dword [032-035] = Threads per L1 instruction cache
; Dword [036-039] = Threads per L1 data cache
; Dword [040-043] = Threads per L2 unified cache 
; Dword [044-047] = Threads per L3 unified cache 
;--- 16 bytes MULTIPROCESSING TOPOLOGY INFO ---
; Dword [048-051] = Threads per one core (HT/SMT)
; Dword [052-055] = Cores per one package (or per one NUMA node)
; Dword [056-059] = Total NUMA nodes per platform
; Dword [060-063] = Total CPU packages per platform 
struct OSCACHE
OsL1Code          dq  ?         ; Cache size information
OsL1Data          dq  ?
OsL2Unified       dq  ?
OsL3Unified       dq  ?
OsThreadsPerL1C   dd  ?         ; Cache threads information
OsThreadsPerL1D   dd  ?
OsThreadsPerL2U   dd  ?
OsThreadsPerL3U   dd  ?
OsThreadsPerCore  dd  ?         ; Topology information
OsThreadsPerNode  dd  ?
OsNodesTotal      dd  ?
OsPackagesTotal   dd  ?
ends

;--- Memory info --
; dwLength                    dd ?
; dwMemoryLoad                dd ?
; ullTotalPhys                dq ?
; ullAvailPhys                dq ?
; ullTotalPageFile            dq ?
; ullAvailPageFile            dq ?
; ullTotalVirtual             dq ?
; ullAvailVirtual             dq ?
; ullAvailExtendedVirtual     dq ?
struct MEMORYSTATUSEX_DEF
dwLength                 dd ?
dwMemoryLoad             dd ?
ullTotalPhys             dq ?
ullAvailPhys             dq ?
ullTotalPageFile         dq ?
ullAvailPageFile         dq ?
ullTotalVirtual          dq ?
ullAvailVirtual          dq ?
ullAvailExtendedVirtual  dq ?
ends

; Root structure
struct SYSPARMS
; CPUID base parameters
CpuVendorString   db  12 DUP (?)     ; Result of CPUID#0
CVterminator      db  4  DUP (?)     ; Terminator byte + alignment for dwords 
CpuNameString     db  48 DUP (?)     ; Result of CPUID#80000002h-80000004h
CNterminator      db  4  DUP (?)     ; Terminator byte + alignment for dwords     
CpuSignature      dd  ?              ; Resut EAX of CPUID#1 = TFMS
; CPU feature flags, selected and ordered by NCRB functionality
; Note this bit flags is not sequental by x86 instructions generations:
; MMX-SSE-SSE4-AVX-AVX2-AVX512-...
; D0=SSE, D1=AVX, D2=FMA, D3=AVX2
; D4=AVX512F (foundation part of AVX512)
; D5=SSE4.1 (for 128-bit NT Read, VMOVNTDQA xmm)
; D6=RDRAND (for randomization address in latency test)
; D7=HT (Hyper Threading)
; D8=MMX (added at NCRB v1.02.xx generation)
; D[9-31]=Reserved bits
; HT bit can be updated by WinAPI results, note CPU can return HT support=1
; when CPU support HT but HT disabled by CMOS setup option or OS
CpuSpecFlags      dd  ?   ; Special flags
; TSC Frequency (Hz) and Period (Seconds),
; 1 MHz = 10^6 Hz , 1 nanosecond = 10^-9 seconds
DeltaTsc          dq  ?    ; Time Stamp Counter frequency, Hz, dTSC per second
TscHz             dq  ?    ; TSC frequency, Hz, floating point double precision
TscMHz            dq  ?    ; TSC frequency, MHz, fl. point double precision
TscSeconds        dq  ?    ; TSC period, Seconds, fl. point double precision
TscNs             dq  ?    ; TSC period, Nanoseconds, fl. point double precis.
; Cache info format
; Qword=Size, bytes,
; Word=Maximum threads at this level
; Word=Maximum APIC IDs per package
; For Trace Cache size in micro operations, not bytes
CpuCache          CPUCACHE
;--- 32 bytes CACHE SIZE INFO ---
; Qword [000-007] = L1 instruction cache size (bytes)
; Qword [008-015] = L1 data cache size (bytes)
; Qword [016-023] = L2 unified cache size (bytes)
; Qword [024-031] = L3 unified cache size (bytes)
;--- 16 bytes CACHE TOPOLOGY INFO ---
; Dword [032-035] = Threads per L1 instruction cache
; Dword [036-039] = Threads per L1 data cache
; Dword [040-043] = Threads per L2 unified cache 
; Dword [044-047] = Threads per L3 unified cache 
;--- 16 bytes MULTIPROCESSING TOPOLOGY INFO ---
; Dword [048-051] = Threads per one core (HT/SMT)
; Dword [052-055] = Cores per one package (or per one NUMA node)
; Dword [056-059] = Total NUMA nodes per platform
; Dword [060-063] = Total CPU packages per platform 
OsCache           OSCACHE
;--- Operating System Info (base) ---
; wProcessorArchitecture      dw ?
; wReserved                   dw ?
; dwPageSize                  dd ?
; lpMinimumApplicationAddress dq ?
; lpMaximumApplicationAddress dq ?
; dwActiveProcessorMask       dq ?
; dwNumberOfProcessors	      dd ?
; dwProcessorType             dd ?
; dwAllocationGranularity     dd ?
; wProcessorLevel             dw ?
; wProcessorRevision          dw ?
SystemInfo	      SYSTEM_INFO           ; Length = 48 bytes
;--- Memory info --
; dwLength                    dd ?
; dwMemoryLoad                dd ?
; ullTotalPhys                dq ?
; ullAvailPhys                dq ?
; ullTotalPageFile            dq ?
; ullAvailPageFile            dq ?
; ullTotalVirtual             dq ?
; ullAvailVirtual             dq ?
; ullAvailExtendedVirtual     dq ?
MemoryInfo	      MEMORYSTATUSEX_DEF    ; Length = 64 bytes
; Large page size and availability
LargePageSize     dd  ?              ; Used large page size, bytes
LargePageFlag     dd  ?              ; 0 = mapping not available, 1 = available
; Processor groups count for support systems with >64 logical processors
; 0 means unknown, supported as single group system
; 1 means detected single group system
; >1 means complex system with >1 groups, required processor group management
ProcessorGroups   dd  ?
; Total processors count at all groups
ProcessorsTotal   dd  ?
; NUMA = Non Uniform Memory Access info, maximum for 64 NUMA Nodes
NumaNodes         dd  ?              ; Number of NUMA Nodes
NumaNodesMasks    dd  64 DUP (?)     ; Affinity masks for 64 nodes maximum
; MADT = Multiple APICs Descripton Table extracted info
; Text and numbers format:
; 6 bytes [00-05] = OEM ID (ACPI MADT relative), text
; 8 bytes [06-13] = Manufacturer Model ID (ACPI MADT relative), text
; 4 bytes [14-17] = OEM revision (ACPI MADT relative), number
MadtText          db  14 DUP (?)      ; ACPI MADT Text Tags
MadtReserved      db  2  DUP (?)      ; Reserved for dword alignment 14 to 16
MadtOemRevision   dd  ?               ; OEM Revision, 32-bit number
MadtLocalApics	  dd  ?               ; Number of CPU Local APICs
MadtIoApics       dd  ?               ; Number of I/O APICs
; SRAT = System (Static) Resources Affinity Table extracted info
; Text header format same as MADT, see upper block
SratText          db  14 DUP (?)      ; ACPI SRAT Text Tags
SratReserved      db  2  DUP (?)      ; Reserved for dword alignment 14 to 16
SratOemRevision   dd  ?               ; OEM Revision, 32-bit number
SratDomains       dd  ?               ; Number of NUMA Domains
SratCpus          dd  ?               ; Number of CPUs (as Local APICs)
SratRams          dd  ?               ; Number of RAMs (as Regions)
; SLIT = System Locality Distance Information Table extracted info
; Text header format same as MADT, see upper block
SlitText          db  14 DUP (?)      ; ACPI SLIT Text Tags
SlitReserved      db  2  DUP (?)      ; Reserved for dword alignment 14 to 16
SlitOemRevision   dd  ?               ; OEM Revision, 32-bit number
; Performance pattern internal parameters
; SSE4.1 validation required for 128-bit MOVNTDQA
; AVX2 validation required for 256-bit VMOVNTDQA
; Note non-temporal read recommended for AMD CPU only,
; Note non-temporal write recommended for both Intel and AMD
; IMPORTANT patch v0.98.0 = non-temporal read use BOTH FOR INTEL AND AMD
; IMPORTANT patch v1.01.02 = this byte locked to 0, REJECT (V)MOVNTDQA
; selection because optimal variant is PREFETCHNTA from SSE1 
NonTemporalRead   dd  ?    ; 0 = Default (V)MOVNTPD write and PREFETCHNTA read 
                           ; 1 = SSE4.1 use MOVNTDQA for read 128-bit 
                           ; 2 = AVX2 use VMOVNTDQA for read 256-bit 
ends

 
