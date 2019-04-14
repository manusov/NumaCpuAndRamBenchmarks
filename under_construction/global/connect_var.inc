
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

;------------------------------------------------------------------------------;
;     MPECSB = Memory Performance Engine control and status block (CSB).       ;
;------------------------------------------------------------------------------;

; Test input and output operands located by alignment-optimal ordering
; and 64-byte cache line allocation

; Test USER parameters
align 64
UserParms        UPB
; Test INPUT parameters
align 64
InputParms       IPB
; Test OUTPUT parameters
align 64
OutputParms      OPB


; Continue data section, multifunctional buffer
align 4096                  ; Align by page, actual for Vector brief test 
TEMP_BUFFER  DB  TEMP_BUFFER_SIZE DUP (?)
