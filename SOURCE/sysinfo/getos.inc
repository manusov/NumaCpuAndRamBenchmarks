;------------------------------------------------------------------------;
; Get NUMA information by Windows API                                    ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save ACPI SRAT info             ;
;                                                                        ;
; OUTPUT:  RDI = Input RDI + 516 , buffer size fixed = 516 bytes         ;
;          Destination memory at [input RDI] = Results, 0=Not detected	 ;
;          Dword [000-003] = Number of NUMA Domains, 0=Not detected      ;
;          Qword [004-011] = Domain (Node) #0 Affinity Mask              ;
;          ...                                                           ;
;          Qword [012-515] = Domain (Node) #63 Affinity Mask             ; 
;------------------------------------------------------------------------;
GetOsNumaInfo:
cld                          ; Clear direction, because STOSQ used
push rbx rcx rdx rax rsi rbp r8 r9 r10 r11 rax	; R8-R11 because API
mov rbp,rsp                  ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack because WinAPI used
sub rsp,32                   ; Make parameters shadow
; Blank results
mov rbx,rdi                  ; RBX = Backup output region address
mov ecx,516
mov al,0
rep stosb
; Get maximum node number
mov rcx,rbp                      ; RCX = Parm #1 = Pointer to variable
mov rax,[_GetNumaHighestNodeNumber]
test rax,rax
jz .NumaNotAvailable             ; Go skip if function not declared by OS
call rax                         ; Return EAX=0 if error, EAX>0 if no errors
test rax,rax
jz .NumaNotAvailable             ; Go skip if error returned
cmp qword [rbp],65535            ; [RBP] = Number of nodes
ja .NumaNotAvailable             ; Go skip if invalid number of nodes
; Scan nodes
xor esi,esi                      ; RSI = Start node number
lea rdi,[rbx+4]                  ; RDI = Pointer to nodes list, qwords sequence
.NumaScan:                       ; RBX = Pointer to nodes counter, dword
mov ecx,esi                      ; RCX = Parm #1 = Node number
mov rdx,rdi                      ; RDX = Parm #2 = Pointer to variable = Processor Mask
mov rax,[_GetNumaNodeProcessorMask]
test rax,rax
jz .NumaNotAvailable             ; Go skip if function not declared by OS 
call rax                         ; Write qword to [rdx]
test  rax,rax
jnz @f                           ; Go skip blank if this node valid
mov qword [rdi],0
@@:
jz @f                            ; Go skip pointer advance if this node not valid
inc dword [rbx]                  ; Memory at [RBX] = Valid nodes counter +1
add rdi,8                        ; Nodes list pointer +8
@@:
mov rax,rdi
sub rax,rbx
cmp eax,516
jae .NumaNotAvailable            ; Exit if 64 valid nodes stored
inc esi                          ; Scanned node number +1
dec dword [rbp]                  ; Nodes counter -1
jns .NumaScan                    ; Stop cycle when make [rbp]=-1
; Exit
lea rdi,[rbx+516]                ; Output RDI must be valid
.NumaNotAvailable:
jmp ExitCpuClk

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
;------------------------------------------------------------------------;
GetOsMemoryInfo:
cld                          ; Clear direction, because STOSQ used
push rbx rcx rdx rax rsi rbp r8 r9 r10 r11 rax	; R8-R11 because API
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
jmp ExitCpuClk

;------------------------------------------------------------------------;
; Get multiprocessing topology and cache information by Windows API      ;
; This function allows get topological information                       ;
; without Vendor-Specific CPUID methods.                                 ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save system info                ;
;	         RSI = Pointer to 48KB transit buffer for WinAPI return data   ;
;                                                                        ;
; OUTPUT:  CF = Error flag, 0(NC)=Executed OK, 1(C)=Error                ;
;          Output RDI valid only if CF=0(NC)                             ;
;                                                                        ;
;          RDI = Input RDI + 64 , buffer size fixed = 64 bytes           ;
;          Destination memory at [input RDI] = Results, 0=Not detected   ;
;          --- 32 bytes CACHE SIZE INFO ---                              ;
;          Qword [000-007] = L1 instruction cache size (bytes)           ;
;          Qword [008-015] = L1 data cache size (bytes)                  ;
;          Qword [016-023] = L2 unified cache size (bytes)               ;
;          Qword [024-031] = L3 unified cache size (bytes)               ;
;          --- 16 bytes CACHE TOPOLOGY INFO ---                          ;
;          Dword [032-035] = Threads per L1 instruction cache            ;
;          Dword [036-039] = Threads per L1 data cache                   ;
;          Dword [040-043] = Threads per L2 unified cache                ; 
;          Dword [044-047] = Threads per L3 unified cache                ; 
;          --- 16 bytes MULTIPROCESSING TOPOLOGY INFO ---                ;
;          Dword [048-051] = Threads per one core (SMT)                  ;
;          Dword [052-055] = Cores per one package                       ;
;          Dword [056-059] = Total NUMA nodes per platform               ;
;          Dword [060-063] = Total packages per platform                 ; 
;                                                                        ;
;---------- Information about WinAPI function output data ---------------;
; WinAPI function = GetLogicalProcessorInformation                       ;
;---                                                                     ; 
; Entry format:                                                          ;
; QWORD  ProcessorMask = Bitmap for select processors                    ;
; QWORD  Relationship (byte used)                                        ;
; QWORD  Flags | Node Number | Cache Descriptor | Reserved               ;
; QWORD  continue if size > 8 bytes                                      ;
;---                                                                     ;
; Relationship encoding (select content for qwords 3,4                   ;
; 0 = Relation processor core                                            ;
; 1 = Relation NUMA node                                                 ;
; 2 = Relation cache                                                     ;
; 3 = Relation processor package                                         ;
;---                                                                     ;
; Flags encoding                                                         ;
; 0 = No sharing resources by Hyper-Threading or SMT                     ;
; 1 = Sharing resources by Hyper-Threading or SMT                        ;
;---                                                                     ;
; Node number = Value for NUMA node number                               ;
;---                                                                     ;
; Cache descriptor encoding:                                             ;
; BYTE   Level: 1=L1, 2=L2, 3=L3                                         ;
; BYTE   Associativity, or FF=Full associative                           ;
; WORD   Line Size                                                       ;
; DWORD  Size                                                            ;
; DWORD  Cache type: 0=Unified, 1=Instruction, 2=Data, 3=Trace           ; 
;---                                                                     ;
; Can get info for Intel/AMD without processor-specific fragments        ;
;------------------------------------------------------------------------;
GetOsMpInfo:
cld
push rbx rcx rdx rax rsi rbp r8 r9 r10 r11 rax	; R8-R11 because API
mov rbp,rsp                  ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack because WinAPI used
sub rsp,32                   ; Make parameters shadow
; Blank buffer, because can be partially updated if function OK
mov rbx,rdi
mov ecx,16
xor eax,eax
rep stosd
; Call WinAPI target function
mov rcx,rsi                  ; RCX = Parm#1 = Pointer to output buffer 
mov rdx,rbp                  ; RDX = Parm#2 = Pointer to output variable = size
mov dword [rdx],TEMP_BUFFER_SIZE    ; Set output buffer size limit
mov rax,[_GetLogicalProcessorInformation]
test rax,rax
jz .SkipOsMpInfo             ; Go skip if function not declared by OS
call rax
test rax,rax                 ; Check for RAX=0 means error
jz .ErrorOsMpInfo            ; If declared, but failed, exit with error signal 
; Get and analysing returned data size
mov ecx,[rbp]                ; ECX = Size of output information
cmp ecx,TEMP_BUFFER_SIZE
ja .ErrorOsMpInfo            ; Go error if size too big
test cl,1Fh 
jnz .ErrorOsMpInfo           ; Go error if size not aligned by 32 bytes
shr ecx,5
jz .ErrorOsMpInfo            ; Go error if size too small 

push rdi
; Scan WinAPI output data and built control structure
; RCX = Number of entries
; RSI = Pointer to source data for scan
; RBX = Pointer to destination data for built
; Begin scan cycle
.ScanRelations:
lodsq
xchg rdi,rax    ; RDI = Processor mask
lodsq
xchg rdx,rax    ; RDX = Relationship
lodsq
xchg r8,rax     ; R8  = Entry-specific QWORD#1 = Flags|Node|CacheDesc|Reserved 
lodsq           ; RAX = Entry-specific QWORD#2 = Cache type  
; Calculate R9 = number of logical processors = F(Processors Mask)
xor r9d,r9d
mov r10d,1
.L10:
test rdi,r10
jz .L11
inc r9d
.L11:
shl r10,1
jnz .L10
; Select handler = F(Relation)
test dl,dl
jz .RelationCore
dec dl
jz .RelationNUMA
dec dl
jz .RelationCache
dec dl
jnz .SkipThis
; Handler for PACKAGE entry, tag=3
; Dword [060-063] = Total CPU packages per platform
.RelationPackage:
inc dword [rbx+60]
jmp .SkipThis
; Handler for CORE entry, tag=0
; Dword [052-055] = Cores per one package (or per one NUMA node)
.RelationCore:
inc dword [rbx+52]
mov [rbx+48],r9d
jmp .SkipThis
; Handler for NUMA entry, tag=1
; Dword [048-051] = Threads per one core (SMT)
; Dword [056-059] = Total NUMA nodes per platform
.RelationNUMA:
inc dword [rbx+56]
jmp .SkipThis
; Handler for CACHE entry, tag=2
; Qword [000-007] = L1 instruction cache size (bytes)
; Qword [008-015] = L1 data cache size (bytes)
; Qword [016-023] = L2 unified cache size (bytes)
; Qword [024-031] = L3 unified cache size (bytes)
; Dword [032-035] = Threads per L1 instruction cache
; Dword [036-039] = Threads per L1 data cache
; Dword [040-043] = Threads per L2 unified cache 
; Dword [044-047] = Threads per L3 unified cache 
.RelationCache:
mov ah,al        ; AH = Cache type, valid: 0=Unif., 1=Instr., 2=Data, 3=Trace    
mov al,r8b       ; AL = Cache Level, valid: 1,2,3
shr r8,32        ; R8 = Cache size, bytes
xor edx,edx
cmp ax,0101h           ; Instruction cache, L1
je .L20
mov dl,8
cmp ax,0201h           ; Data cache, L1
je .L20
mov dl,16
cmp ax,0002h           ; Unified cache, L2
je .L20
mov dl,24
cmp ax,0003h           ; Unified cache, L3
jne .SkipThis
.L20:
mov [rbx+rdx],r8d      ; Store this CACHE SIZE
shr rdx,1
mov [rbx+32+rdx],r9d   ; Store THREADS PER this CACHE
; Cycle
.SkipThis:
loop .ScanRelations
pop rdi

; Calculate number of cores per one processor package (or per NUMA node)
mov eax,[rbx+52]
xor edx,edx
mov ecx,[rbx+60]
test ecx,ecx
jnz .L0
mov ecx,[rbx+56]
jecxz .L1
.L0:
div ecx
mov [rbx+52],eax
.L1:
; Exit point, OK
.SkipOsMpInfo:

; add rdi,64      ; Buffer size is 64 bytes, also clear CF=0(NC)
clc
;

.ExitOsMpInfo:
jmp ExitCpuClk
; Exit point, Error
.ErrorOsMpInfo:
stc
jmp .ExitOsMpInfo
