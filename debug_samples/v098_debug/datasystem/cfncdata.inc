;------------------------------------------------------------------------------;
; Data for load some system functions - constants pool.                        ;
; This functions not declared in the import section and required manually load ;
; for Windows XP x64 compatibility reasons. Otherwise abort when interpreting  ;
; import section under Windows XP x64, because some API functions not found.   ;
;------------------------------------------------------------------------------;

NameKernel32:                                ; Libraries names
DB  'KERNEL32.DLL',0

NameAdvapi32:
DB  'ADVAPI32.DLL',0

FunctionsNamesKernel32:                      ; Functions names
DB  'GlobalMemoryStatusEx',0
DB  'GetNumaHighestNodeNumber',0
DB  'GetNumaNodeProcessorMask',0
DB  'VirtualAllocExNuma',0
DB  'SetThreadAffinityMask',0                
DB  'GetSystemFirmwareTable',0
DB  'GetLogicalProcessorInformation',0
DB  'GetLargePageMinimum',0
DB  'VirtualAllocEx',0
DB  0

FunctionsNamesAdvapi32:
DB  'OpenProcessToken',0
DB  'AdjustTokenPrivileges',0
DB  0


