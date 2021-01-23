;--- Get Processors and Memory information by Windows API ---------------;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save system info                ;
;                                                                        ;
; OUTPUT:  CF = Error flag, 0(NC)=Executed OK, 1(C)=Error                ;
;          Output RDI valid only if CF=0(NC)                             ;
;                                                                        ;
;          EDI = Input EDI + 100 , buffer size fixed = 100 bytes         ;
;          Destination memory at [input EDI] = Results, 0=Not detected   ;
;          --- 48 Bytes SYSTEM_INFO (see MSDN) for x64                   ;
;          --- note ia32 version uses 36 bytes                           ;
;          wProcessorArchitecture      dw ?                              ;
;          wReserved                   dw ?                              ;
;          dwPageSize                  dd ?                              ;
;          lpMinimumApplicationAddress dd ?   ; different for ia32/x64   ;
;          lpMaximumApplicationAddress dd ?   ; different for ia32/x64   ;   
;          dwActiveProcessorMask       dd ?   ; different for ia32/x64   ;   
;          dwNumberOfProcessors        dd ?                              ;
;          dwProcessorType             dd ?                              ;
;          dwAllocationGranularity     dd ?                              ;
;          wProcessorLevel             dw ?                              ;
;          wProcessorRevision          dw ?                              ;
;          --- 64 bytes MEMORYSTATUSEX (see MSDN)                        ;
;          dwLength                    dd ?                              ;
;          dwMemoryLoad                dd ?                              ;
;          ullTotalPhys                dq ?   ; 64 bit both for ia32/x64 ;
;          ullAvailPhys                dq ?   ; .                        ;
;          ullTotalPageFile            dq ?   ; .                        ;
;          ullAvailPageFile            dq ?   ; .                        ;
;          ullTotalVirtual             dq ?   ; .                        ;
;          ullAvailVirtual             dq ?   ; .                        ;
;          ullAvailExtendedVirtual     dq ?   ; 64 bit both for ia32/x64 ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

GetOsMemoryInfo:
push ebx esi ebp

; Here buffer not blanked by REP STOSB, and EDI valid only if no errors,
; because errors in this function is fatal and application exit

; Get System Info
push edi                     ; Parm#1 = Output buffer address
call [GetSystemInfo]

; Get pointer to Win API function, result of dynamical import
add edi,36                   ; Sub buffer #1 size is 48 bytes for x64 , 36 bytes for IA32
mov eax,[_GlobalMemoryStatusEx]
test eax,eax
jz .ErrorOsInfo              ; Go skip if function not declared by OS

; Get Memory Info
push edi                     ; Parm#1 = Output buffer address
mov dword [edi],64           ; Set structure length 
call eax

; Check status
test eax,eax                 ; Check for RAX=0 means error
.ErrorOsInfo:
stc                          ; CF=1(C) means error
jz .ExitOsInfo               ; Go exit if error returned 

; Exit points
add edi,64                   ; Sub buffer #2 size is 64 both for x64 and IA32 
clc
.ExitOsInfo:
pop ebp esi ebx
ret

