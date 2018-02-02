;------------------------------------------------------------------------------;
; Data for load some system functions - variables pool.                        ;
; This functions not declared in the import section and required manually load ;
; for Windows XP x64 compatibility reasons. Otherwise abort when interpreting  ;
; import section under Windows XP x64.                                         ;
;------------------------------------------------------------------------------;

FunctionsPointers:
_GlobalMemoryStatusEx            DQ  ?
_GetNumaHighestNodeNumber        DQ  ?
_GetNumaNodeProcessorMask        DQ  ?
_VirtualAllocExNuma              DQ  ?
_SetThreadAffinityMask           DQ  ?                
_GetSystemFirmwareTable          DQ  ?
_GetLogicalProcessorInformation  DQ  ?

