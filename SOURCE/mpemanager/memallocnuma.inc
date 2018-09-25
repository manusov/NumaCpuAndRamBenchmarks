;------------------------------------------------------------------------------;
;      Memory allocation library, for NUMA platforms and test scenarios.       ;
;------------------------------------------------------------------------------;

;---------- Memory allocation with alignment = 4096 bytes, NUMA mode ----------;
; INPUT:   RBX = Pointer to nodes list                                         ;
;          ECX = Number of nodes                                               ;
;          RDX = Required memory size per node                                 ;
;          R8  = Large Pages Flag
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          Registers corrupted: assume all                                     ;
;------------------------------------------------------------------------------;
MemoryAllocationNuma:
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h   ; Stack alignment
; Store parameters to non-volatile registers
mov rsi,rdx                  ; RSI = Buffer block size per each NUMA node
mov edi,ecx                  ; RDI = Number of nodes

;- fix bug corrupt R8 -
; Get current process handle
; call [GetCurrentProcess]     ; Return RAX = Handle
; test rax,rax
; jz ErrorAllocationNuma       ; Go error if Handle=0
; xchg r12,rax                 ; R12 = Process Handle, XCHG for compact
;-

; Check for large pages
mov r14,MEM_RESERVE + MEM_COMMIT
test r8,0001h
jz .SkipLP
mov r14,MEM_RESERVE + MEM_COMMIT + MEM_LARGE_PAGES
mov r8,[LargePageSize]
test r8,r8
jz ErrorAllocationNuma         ; Go error to prevent divide by 0
mov rax,rsi
xor edx,edx
div r8
mul r8
lea rsi,[rax+r8]               ; Parm#3 = R8 = Block size + alignment reserved
.SkipLP:

;- fix bug corrupt r8 -
; Get current process handle
call [GetCurrentProcess]     ; Return RAX = Handle
test rax,rax
jz ErrorAllocationNuma       ; Go error if Handle=0
xchg r12,rax                 ; R12 = Process Handle, XCHG for compact
;-

; Cycle for nodes
NumaAllocation:
mov [rbx+NUMACTRL.AlignedSize],rsi  ; Store memory block size
mov rcx,r12			                    ; Parm#1 = RCX = Handle
xor edx,edx			                    ; Parm#2 = RDX = Base address, 0 means selected by function
mov r8,rsi			                    ; Parm#3 = R8  = Block size
mov r9,r14                          ; Parm#4 = R9  = Allocation type
push qword [rbx+NUMACTRL.NodeID]    ; Parm#6 = NUMA node preferred
push 04h			                      ; Parm#5 = Pages protection attributes = Read/Write
sub rsp,32			                    ; Create parameters shadow
call [_VirtualAllocExNuma]
add rsp,32+16			               ; Remove parameters shadow + parameters 5,6
test rax,rax
jz ErrorAllocationNuma           ; Go error if returned 0
mov [rbx+NUMACTRL.TrueBase],rax  ; Store true base address, it used for release
test eax,00000FFFh
jz @f                            ; Go if already aligned
and rax,0FFFFFFFFFFFFF000h
add rax,00000000000001000h
@@:
mov [rbx+NUMACTRL.AlignedBase],rax  ; Store aligned base address, it used for memory access
add rbx,NUMACTRL_SIZE               ; Select next entry in the nodes list 
dec edi                             ; Nodes count -1
jnz NumaAllocation                  ; Cycle for requested number of nodes 

; Exit points
clc                              ; Status = no errors 
ExitAllocationNuma:
mov rsp,rbp
ret
ErrorAllocationNuma:
stc
jmp ExitAllocationNuma

;---------- Memory release, NUMA mode -----------------------------------------;
; INPUT:   RBX = Pointer to nodes list                                         ;
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          Registers corrupted: assume all                                     ;
;------------------------------------------------------------------------------;
MemoryReleaseNuma:
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h       ; Stack alignment
sub rsp,32                       ; Make parameters shadow 
; Get current process handle
call [GetCurrentProcess]         ; Return RAX = Handle
xchg rdi,rax                     ; Copy to non-volatile register, XCHG for compact
lea r12,[rbx+NUMA_CONTROL_SIZE]  ; RBX = Control block base address, R12 = Limit

; Release memory for all nodes
NumaRelease:
mov rcx,rdi                      ; Parm#1 = RCX = Process Handle
mov rdx,[rbx+NUMACTRL.TrueBase]  ; Parm#2 = Address
xor r8d,r8d                      ; Parm#3 = R8 = Release size, here must be 0 = all block
mov r9d,MEM_RELEASE	             ; Parm#4 = Release type
test rdx,rdx
jz @f                     ; Skip this entry if memory not allocated for it
call [VirtualFreeEx]
test rax,rax
jz ErrorReleaseNuma       ; Go if error detected
@@:
add rbx,NUMACTRL_SIZE     ; Select next entry                    
cmp rbx,r12
jb NumaRelease            ; Cycle for 64 entries 

; Exit points
clc                       ; Status = no errors 
ExitReleaseNuma:
mov rsp,rbp
ret
ErrorReleaseNuma:
stc
jmp ExitReleaseNuma

;---------- NUMA nodes scan and build list of nodes ---------------------------;
; INPUT:   RBX = Pointer to nodes list, destination for this subroutine        ;
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          EAX = Number of allocated nodes, valid if CF=0(NC)                  ;
;          NUMA enumeration list updated                                       ;
;          Registers corrupted: assume all                                     ;  
;------------------------------------------------------------------------------;
BuildNumaList:
cld
push 0                           ; Create storage for local variable
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h       ; Stack alignment
sub rsp,32                       ; Make parameters shadow 
; Get maximum node number
mov rcx,rbp                       ; RCX = Parm #1 = Pointer to variable
call [_GetNumaHighestNodeNumber]  ; Return EAX=0 if error, EAX>0 if no errors
test rax,rax
jz ErrorNumaBuild
mov rax,[rbp]                    ; Get variable updated by OS API
cmp rax,65536
jae ErrorNumaBuild               ; Go error if potential nodes count > 65536 
; Parameters for build list of nodes, RBX = Pointer to nodes list 
lea rdi,[rax+1]                  ; RDI = Number of NUMA nodes, make 1-based
xor esi,esi                      ; RSI = Start node number
mov r12,rbx
lea r13,[rbx+NUMA_CONTROL_SIZE]

; Cycle for build list of nodes
NumaScan:
mov [rbx+NUMACTRL.NodeID],rsi        ; Store node number
mov ecx,esi                          ; RCX = Parm #1 = Node number
lea rdx,[rbx+NUMACTRL.NodeAffinity]  ; RDX = Parm #2 = Pointer Processor Mask
;
; INT3
cmp [ProcessorsTotal],MAX_THREADS_PER_GROUP
jbe .GetSmallMask 
.GetBigMask:
call [_GetNumaNodeProcessorMaskEx]   ; Write qword to [rdx], [rdx+8], mask+group
jmp .AfterGetMask
.GetSmallMask:
call [_GetNumaNodeProcessorMask]     ; Write qword to [rdx], affinity mask
mov [rbx+NUMACTRL.NodeGroup],0       ; Processor Group always 0 for this branch  
.AfterGetMask:
;- end of changed -
test  rax,rax
jz @f                            ; Go skip validation if error returned
;- add at v1.01.00 for count ACTIVE NODES ONLY, when affinity mask non zero
cmp qword [rbx+NUMACTRL.NodeAffinity],0
je @f                            ; Go skip validation if affinity mask = 0 
;- end of added -
add rbx,NUMACTRL_SIZE            ; Shift to next entry, means validate stored
@@:
inc esi               ; Select next node number
dec rdi               ; Node counter - 1
jz @f                 ; Go exit if node counter done
cmp rbx,r13           ; Check size limit, maximum 64 used entries
jb NumaScan           ; Go cycle if entries available yet 

; End of cycle for build list of nodes
@@:
sub rbx,r12           ; RDI = Number of bytes stored
shr rbx,6             ; RDI/64 because 64 bytes per entry
xchg rax,rbx          ; RAX = Number of entries stored,
; Exit points         ; note must count ACTIVE NODES if affinity mask non zero
clc
ExitNumaBuild:
mov rsp,rbp
pop rcx               ; This for restore stack
ret
ErrorNumaBuild:
stc
jmp ExitNumaBuild

