;------------------------------------------------------------------------------;
;           Data for load some system functions - variables pool.              ;
; This functions not declared in the import section and required manually load ;
; for Windows XP x64 compatibility reasons. Otherwise abort when interpreting  ;
; import section under Windows XP x64, because some API functions not found.   ;
;------------------------------------------------------------------------------;

align 8
FunctionsPointersAll:

; Entry points at KERNEL32.DLL
_GlobalMemoryStatusEx            DQ  ?
_GetNumaHighestNodeNumber        DQ  ?
_GetNumaNodeProcessorMask        DQ  ?
_VirtualAllocExNuma              DQ  ?
_SetThreadAffinityMask           DQ  ?                
_GetSystemFirmwareTable          DQ  ?
_GetLogicalProcessorInformation  DQ  ?
_GetLargePageMinimum             DQ  ?
; _VirtualAllocEx                DQ  ?
_GetActiveProcessorCount         DQ  ?    ; added for support > 64 processors 
_GetActiveProcessorGroupCount    DQ  ?    ; added for support > 64 processors 
_GetNumaNodeProcessorMaskEx      DQ  ?    ; added for support > 64 processors
_SetThreadGroupAffinity          DQ  ?    ; added for support > 64 processors    

; Entry points at ADVAPI32.DLL
_OpenProcessToken                DQ  ?
_AdjustTokenPrivileges           DQ  ?

; Handle for ADVAPI32.DLL
HandleAdvapi32                   DQ  ?
