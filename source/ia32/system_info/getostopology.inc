;--- Get multiprocessing topology and cache information by Windows API --;
; This function allows get topological information                       ;
; without Vendor-Specific CPUID methods.                                 ;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save system info                ;
;          ESI = Pointer to 48KB transit buffer for WinAPI return data   ;
;                                                                        ;
; OUTPUT:  CF = Error flag, 0(NC)=Executed OK, 1(C)=Error                ;
;          Output RDI valid only if CF=0(NC)                             ;
;                                                                        ;
;          EDI = Input EDI + 64 , buffer size fixed = 64 bytes           ;
;          Destination memory at [input EDI] = Results, 0=Not detected   ;
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
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

GetOsTopologyInfo:
cld
push ebx esi ebp eax    ; Note push eax for make local variable
mov ebp,esp             ; EBP = Pointer to local variable
; Blank buffer, because can be partially updated if function OK
mov ebx,edi
mov ecx,16
xor eax,eax
rep stosd            ; Buffer size is 64 bytes
; Call WinAPI target function
mov eax,[_GetLogicalProcessorInformation]
test eax,eax
jz .SkipOsMpInfo     ; Go skip if function not declared by OS
push ebp             ; Parm#2 = Pointer to output variable = size
push esi             ; Parm#1 = Pointer to output buffer 
mov dword [ebp],TEMP_BUFFER_SIZE  ; Set output buffer size limit
call eax
test eax,eax         ; Check for EAX = 0 means error
jz .ErrorOsMpInfo    ; If declared, but failed, exit with error signal 
; Get and analysing returned data size
mov eax,[ebp]        ; ECX = Size of output information
cmp eax,TEMP_BUFFER_SIZE
ja .ErrorOsMpInfo    ; Go error if size too big
mov ecx,24
xor edx,edx
div ecx
test eax,eax
jz .ErrorOsMpInfo    ; Go error if size too small or 0
test edx,edx
jnz .ErrorOsMpInfo   ; Go error if size not aligned by 24 bytes

push edi
; Scan WinAPI output data and build control structure
; ECX = Number of entries
; ESI = Pointer to source data for scan
; EBX = Pointer to destination data for build
; Begin scan cycle
.ScanRelations:
lodsd
xchg edi,eax    ; EDI = Processor mask
lodsd
xchg edx,eax    ; EDX = Relationship
; Next data at descriptor:
; QWORD#1 = Entry-specific = Flags|Node|CacheDesc|Reserved 
; QWORD#2 = Entry-specific = Cache type  
; Calculate EBP = number of logical processors = F(Processors Mask)
xor ebp,ebp
mov eax,1
.L10:
test edi,eax
jz .L11
inc ebp
.L11:
shl eax,1
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
inc dword [ebx + 60]
jmp .SkipThis
; Handler for CORE entry, tag=0
; Dword [052-055] = Cores per one package (or per one NUMA node)
.RelationCore:
inc dword [ebx + 52]
mov [ebx + 48],ebp
jmp .SkipThis
; Handler for NUMA entry, tag=1
; Dword [048-051] = Threads per one core (SMT)
; Dword [056-059] = Total NUMA nodes per platform
.RelationNUMA:
inc dword [ebx + 56]
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
mov al,[esi + 00]   ; AL = Cache Level, valid: 1,2,3 (note about L4 support)
mov ah,[esi + 08]   ; AH = Cache type, valid: 0=Unif., 1=Instr., 2=Data, 3=Trace
xor edx,edx         ; EDX = 0 , pre-blank
cmp ax,0101h               ; Instruction cache, L1
je .L20
mov dl,8
cmp ax,0201h               ; Data cache, L1
je .L20
mov dl,16
cmp ax,0002h               ; Unified cache, L2
je .L20
mov dl,24
cmp ax,0003h               ; Unified cache, L3
jne .SkipThis
.L20:
mov eax,[esi + 04]         ; EBP = Cache size, bytes
mov [ebx + edx],eax        ; Store this CACHE SIZE
shr edx,1
mov [ebx + 32 + edx],ebp   ; Store THREADS PER this CACHE
; Cycle
.SkipThis:
add esi,16
loop .ScanRelations
pop edi

; Calculate number of cores per one processor package (or per NUMA node)
mov eax,[ebx + 52]
xor edx,edx
mov ecx,[ebx + 60]
test ecx,ecx
jnz .L0
mov ecx,[ebx + 56]
jecxz .L1
.L0:
div ecx
mov [ebx + 52],eax
.L1:

; Exit point, OK, flag CF = 0 means no errors
.SkipOsMpInfo:
clc
.ExitOsMpInfo:
pop eax ebp esi ebx
ret
; Exit point, Error, flag CF = 1 means error
.ErrorOsMpInfo:
stc
jmp .ExitOsMpInfo

