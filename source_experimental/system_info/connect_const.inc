;------------------------------------------------------------------------------;
; System Information Library: constants and variables with pre-defined values. ; 
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
;                  System signatures for configuration detect.                 ;
;------------------------------------------------------------------------------;
Signature_Intel  DB  'GenuineIntel'
Signature_AMD    DB  'AuthenticAMD'

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
DB  'GetActiveProcessorCount',0             ; added for support > 64 processors 
DB  'GetActiveProcessorGroupCount',0        ; added for support > 64 processors
DB  'GetNumaNodeProcessorMaskEx', 0         ; added for support > 64 processors
DB  'SetThreadGroupAffinity',0              ; added for support > 64 processors
DB  0                           ; List must be terminared by additional 00 byte
; Functions names at ADVAPI32.DLL
FunctionsNamesAdvapi32:
DB  'OpenProcessToken',0
DB  'AdjustTokenPrivileges',0
DB  0                           ; List must be terminared by additional 00 byte

