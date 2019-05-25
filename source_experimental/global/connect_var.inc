;------------------------------------------------------------------------------;
;            Global Library: variables without pre-defined values.             ;     
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
; Multifunctional buffer.                                                      ;
;------------------------------------------------------------------------------;
align 4096   ; Align by page, actual for Vector brief test 
TEMP_BUFFER  DB  TEMP_BUFFER_SIZE DUP (?)

;------------------------------------------------------------------------------;
;  Data for load some system functions as dynamical import - variables pool.   ;
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
;     User settings, input parameters, output parameters structures            ;
;------------------------------------------------------------------------------;
; Test input and output operands located by alignment-optimal ordering
; and 64-byte cache line allocation
; Test USER parameters, loaded from GUI widgets settings
align 8
UserParms  UPB
; Test INPUT parameters, build IPB = F ( UPB, SYSPARMS )
align 8
InputParms  IPB
; Test OUTPUT parameters, store OPB = F ( Benchmarks and TSC meas. results )
align 8
OutputParms  OPB
; System information paramters, store = F ( Platform detection results )
; This fields use sequental store (STOS), must be sequentally,
; without internal language-defined hidden alignments
align 8
SystemParameters  SYSPARMS
