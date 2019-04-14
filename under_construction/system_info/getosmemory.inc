;------------------------------------------------------------------------;
; Get Processors and Memory information by Windows API                   ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save system info                ;
;                                                                        ;
; OUTPUT:  CF = Error flag, 0(NC)=Executed OK, 1(C)=Error                ;
;          Output RDI valid only if CF=0(NC)                             ;
;                                                                        ;
;          RDI = Input RDI + 112 , buffer size fixed = 112 bytes         ;
;          Destination memory at [input RDI] = Results, 0=Not detected   ;
;          --- 48 Bytes SYSTEM_INFO (see MSDN)                           ;
;          wProcessorArchitecture      dw ?                              ;
;          wReserved                   dw ?                              ;
;          dwPageSize                  dd ?                              ;
;          lpMinimumApplicationAddress dq ?                              ;
;          lpMaximumApplicationAddress dq ?                              ;
;          dwActiveProcessorMask       dq ?                              ;
;          dwNumberOfProcessors        dd ?                              ;
;          dwProcessorType             dd ?                              ;
;          dwAllocationGranularity     dd ?                              ;
;          wProcessorLevel             dw ?                              ;
;          wProcessorRevision          dw ?                              ;
;          --- 64 bytes MEMORYSTATUSEX (see MSDN)                        ;
;          dwLength                    dd ?                              ;
;          dwMemoryLoad                dd ?                              ;
;          ullTotalPhys                dq ?                              ;
;          ullAvailPhys                dq ?                              ;
;          ullTotalPageFile            dq ?                              ;
;          ullAvailPageFile            dq ?                              ;
;          ullTotalVirtual             dq ?                              ;
;          ullAvailVirtual             dq ?                              ;
;          ullAvailExtendedVirtual     dq ?                              ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft x64 calling convention.   ; 
;------------------------------------------------------------------------;
GetOsMemoryInfo:
push rbx rsi rbp
mov rbp,rsp                  ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack because WinAPI used
sub rsp,32                   ; Make parameters shadow
; Here buffer not blanked by REP STOSB, and RDI valid only if no errors,
; because errors in this function is fatal and application exit
; Get Sysyem Info
mov rcx,rdi                  ; Parm#1 = RCX = Output buffer address
call [GetSystemInfo]
; Get Memory Info
add rdi,48                   ; Sub buffer #1 size is 48 bytes
mov rcx,rdi                  ; Parm#1 = RCX = Output buffer address
mov dword [rcx],64           ; Set structure length 
mov rax,[_GlobalMemoryStatusEx]
test rax,rax
jz .ErrorOsInfo              ; Go skip if function not declared by OS
call rax
test rax,rax                 ; Check for RAX=0 means error
.ErrorOsInfo:
stc                          ; CF=1(C) means error
jz .ExitOsInfo               ; Go exit if error returned 
; Exit points
add rdi,64                   ; Sub buf. #2 size is 64 bytes, also clear CF=0(NC)
.ExitOsInfo:
mov rsp,rbp
pop rbp rsi rbx
ret

