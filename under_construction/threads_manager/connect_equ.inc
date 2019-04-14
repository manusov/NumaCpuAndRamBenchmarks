
; NUMA node description entry, not a same as thread description enrty
; Note keep 64 bytes per entry, see ThreadEntry, can be non parametrized coding
; Note keep all pairs: Affinity Mask + Affinity Group as 2 sequental qwords,
; this rule required because WinAPI use with direct output store to this qwords 
struct NUMACTRL
NodeID          dq  ?   ; NUMA Node number, if no NUMA, this field = 0 for all entries
NodeAffinity    dq  ?   ; NUMA Node affinity mask
NodeGroup       dq  ?   ; Processors Group number, 0 if single group platform
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
THREADS_LIST_SIZE     EQU  MAX_THREADS * THCTRL_SIZE

; 512 bytes, 64 handles, separate list req. for WaitForMultipleObjects function  
EVENT_SIZE            EQU  8
EVENTS_LIST_SIZE      EQU  MAX_THREADS * EVENT_SIZE

; 4096 bytes, 64 entries for NUMA nodes description, not a same as THREAD
NUMACTRL_SIZE         EQU  sizeof.NUMACTRL
NUMA_LIST_SIZE        EQU  MAX_NODES * NUMACTRL_SIZE  

