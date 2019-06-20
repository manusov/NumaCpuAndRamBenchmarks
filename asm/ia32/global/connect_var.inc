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
;   for Windows XP compatibility reasons. Otherwise abort when interpreting    ;
;    import section under Windows XP, because some API functions not found.    ;
;------------------------------------------------------------------------------;
align 8
FunctionsPointersAll:
; Entry points at KERNEL32.DLL
_GlobalMemoryStatusEx            DD  ?
_GetNumaHighestNodeNumber        DD  ?
_GetNumaNodeProcessorMask        DD  ?
_VirtualAllocExNuma              DD  ?
_SetThreadAffinityMask           DD  ?                
_GetSystemFirmwareTable          DD  ?
_GetLogicalProcessorInformation  DD  ?
_GetLargePageMinimum             DD  ?
_GetActiveProcessorCount         DD  ?    ; added for support > 64 processors 
_GetActiveProcessorGroupCount    DD  ?    ; added for support > 64 processors 
_GetNumaNodeProcessorMaskEx      DD  ?    ; added for support > 64 processors
_SetThreadGroupAffinity          DD  ?    ; added for support > 64 processors    
; Entry points at ADVAPI32.DLL
_OpenProcessToken                DD  ?    ; added for Large Pages support
_AdjustTokenPrivileges           DD  ?    ; added for Large Pages support
; Handle for ADVAPI32.DLL
HandleAdvapi32                   DD  ?

;------------------------------------------------------------------------------;
; This function used for show warning when 32-bit NCRB runs under Win64,       ;
; this is possible, but non-optimal configuration, can use NCRB x64            ;
; for this platform.                                                           ;
;------------------------------------------------------------------------------;
FunctionPointerWoW64:
_IsWow64Process                  DD  ?

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
