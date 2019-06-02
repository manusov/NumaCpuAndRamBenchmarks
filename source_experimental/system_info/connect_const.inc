;------------------------------------------------------------------------------;
; System Information Library: constants and variables with pre-defined values. ; 
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
;                  System signatures for configuration detect.                 ;
;------------------------------------------------------------------------------;
Signature_Intel  DB  'GenuineIntel'
Signature_AMD    DB  'AuthenticAMD'

;------------------------------------------------------------------------------;
; Data structure for L1 trace cache detection,                                 ; 
; this actual for Intel Prescott CPUs.                                         ;
;------------------------------------------------------------------------------;
; x1 = Cache Descriptor value, for CPUID function 2
; x2 = Trace Cache size, KuOps (kilo micro-operations)
MACRO TRACE_CACHE_ID x1, x2
{
DB x1, x2
}
; List terminator
MACRO TRACE_CACHE_END
{
DB 0
}

IntelTraceCaches:
TRACE_CACHE_ID   070h , 12   ; { 0x70, "trace L1 cache, 12 K킣Ps, 8 ways" }
TRACE_CACHE_ID   071h , 16   ; { 0x71, "trace L1 cache, 16 K킣Ps, 8 ways" }
TRACE_CACHE_ID   072h , 32   ; { 0x72, "trace L1 cache, 32 K킣Ps, 8 ways" }
TRACE_CACHE_ID   073h , 64   ; { 0x73, "trace L1 cache, 64 K킣Ps, 8 ways" }
TRACE_CACHE_END

;------------------------------------------------------------------------------;
;   Data for load some system functions as dynamical import - constants pool.  ;
; This functions not declared in the import section and required manually load ;
; for Windows XP x64 compatibility reasons. Otherwise abort when interpreting  ;
; import section under Windows XP x64, because some API functions not found.   ;
;------------------------------------------------------------------------------;
; Library name KERNEL32.DLL
NameKernel32:                         ; Libraries names
DB  'KERNEL32.DLL',0
; Library name ADVAPI32.DLL
NameAdvapi32:
DB  'ADVAPI32.DLL',0
; Functions names at KERNEL32.DLL
FunctionsNamesKernel32:               ; Functions names
DB  'GlobalMemoryStatusEx',0
DB  'GetNumaHighestNodeNumber',0
DB  'GetNumaNodeProcessorMask',0
DB  'VirtualAllocExNuma',0
DB  'SetThreadAffinityMask',0                
DB  'GetSystemFirmwareTable',0
DB  'GetLogicalProcessorInformation',0
DB  'GetLargePageMinimum',0
DB  'GetActiveProcessorCount',0           ; added for support > 64 processors 
DB  'GetActiveProcessorGroupCount',0      ; added for support > 64 processors
DB  'GetNumaNodeProcessorMaskEx', 0       ; added for support > 64 processors
DB  'SetThreadGroupAffinity',0            ; added for support > 64 processors
DB  0                           ; List must be terminared by additional 00 byte
; Functions names at ADVAPI32.DLL
FunctionsNamesAdvapi32:
DB  'OpenProcessToken',0                  ; added for Large Pages support
DB  'AdjustTokenPrivileges',0             ; added for Large Pages support
DB  0                           ; List must be terminared by additional 00 byte

