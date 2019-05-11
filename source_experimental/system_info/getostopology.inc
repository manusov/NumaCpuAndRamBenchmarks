;--- Get multiprocessing topology and cache information by Windows API --;
; This function allows get topological information                       ;
; without Vendor-Specific CPUID methods.                                 ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save system info                ;
;          RSI = Pointer to 48KB transit buffer for WinAPI return data   ;
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
;                                                                        ;
; Can destroy registers, volatile by Microsoft x64 calling convention.   ; 
;------------------------------------------------------------------------;

GetOsTopologyInfo:
cld
push rbx rsi rbp rax         ; Note push rax for make local variable
mov rbp,rsp                  ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack because WinAPI used
sub rsp,32                   ; Make parameters shadow
; Blank buffer, because can be partially updated if function OK
mov rbx,rdi
mov ecx,16
xor eax,eax
rep stosd
; Call WinAPI target function
mov rcx,rsi                   ; RCX = Parm#1 = Pointer to output buffer 
mov rdx,rbp                   ; RDX = Parm#2 = Pointer to output var. = size
mov dword [rdx],TEMP_BUFFER_SIZE  ; Set output buffer size limit
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
; Scan WinAPI output data and build control structure
; RCX = Number of entries
; RSI = Pointer to source data for scan
; RBX = Pointer to destination data for build
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
mov rsp,rbp
pop rax rbp rsi rbx
ret
; Exit point, Error
.ErrorOsMpInfo:
stc
jmp .ExitOsMpInfo

