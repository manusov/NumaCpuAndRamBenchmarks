;---------- NUMA nodes scan and build list of nodes ---------------------------;
; INPUT:   RCX = Pointer to nodes list, destination for this subroutine        ;
;          EDX = Processor Groups option, used if value = PG_OPTIMAL (2)       ;
;          R8D = NUMA option, used for get nodes list scenario select          ;
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          EAX = Number of allocated nodes, valid if CF=0(NC)                  ;
;          empty nodes with affinity mask = 0 rejected when count EAX          ; 
;          NUMA enumeration list updated                                       ;
;                                                                              ;
; This subroutine not allocate memory, required additional NUMA allocator.     ;
; Can destroy registers, volatile by Microsoft x64 calling convention.         ; 
;------------------------------------------------------------------------------;

BuildNumaNodesList:
cld
push rbx rsi rdi rbp r12 r13 r14 0   ; Note push 0 is create local variable
mov rbx,rcx                          ; RBX = Base address of build nodes list
mov r14d,edx                         ; R14D = Number of processor groups
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h       ; Stack alignment because WinAPI calls
sub rsp,32                       ; Make parameters shadow because WinAPI calls 

; Select scenario = f( NUMA option )
cmp r8d,NUMA_NO_CONTROL
jbe .WithoutNuma

; Get maximum node number
mov rcx,rbp                          ; RCX = Parm #1 = Pointer to variable
call [_GetNumaHighestNodeNumber]     ; Return EAX=0 if error, EAX>0 if no errors
test rax,rax
jz .ErrorNumaBuild       ; Go if function status = error
mov eax,[rbp]            ; Get variable updated by OS API = highest node number
cmp eax,65536
jae .ErrorNumaBuild      ; Go error if potential nodes count >= 65536 

; Parameters for build list of nodes, RBX = Pointer to nodes list 
lea edi,[eax+1]                      ; RDI = Number of NUMA nodes, make 1-based
xor esi,esi                          ; RSI = Start node number, 0-based
mov r12,rbx                          ; R12 = Pointer to nodes list, copy
lea r13,[rbx + NUMA_LIST_SIZE]       ; R13 = Limit for nodes list

; Cycle for build list of nodes
.NumaNodesScan:
mov [rbx + NUMACTRL.NodeID],rsi        ; Store node number
mov ecx,esi                            ; RCX = Parm #1 = Node number
lea rdx,[rbx + NUMACTRL.NodeAffinity]  ; RDX = Parm #2 = Pointer Processor Mask
cmp r14d,PG_OPTIMAL                    ; R14D = PG_OPTIMAL if Processor Groups:  
jne .GetSmallMask                      ; supported and enabled              

.GetBigMask:
call [_GetNumaNodeProcessorMaskEx]  ; Write qword to [rdx], [rdx+8], mask+group
jmp .AfterGetMask

.GetSmallMask:
call [_GetNumaNodeProcessorMask]    ; Write qword to [rdx], affinity mask
mov [rbx + NUMACTRL.NodeGroup],0    ; Processor Group always 0 for this branch  

.AfterGetMask:
test  rax,rax           ; RAX = OS API status after get mask function
jz .SkipThisNode        ; Go skip validation if error returned

; this for count ACTIVE NODES ONLY, when affinity mask non zero
cmp qword [rbx + NUMACTRL.NodeAffinity],0
je .SkipThisNode        ; Go skip validation if affinity mask = 0

add rbx,NUMACTRL_SIZE   ; Shift to next entry, means validate stored

.SkipThisNode:
inc esi                 ; Select next node number
dec edi                 ; Node counter - 1
jz .NodesDone           ; Go exit if node counter done
cmp rbx,r13             ; Check size limit, maximum 64 used entries
jb .NumaNodesScan       ; Go cycle if entries available yet 

; End of cycle for build list of nodes
.NodesDone:
sub rbx,r12            ; RBX = Number of bytes stored, now pointer - start p.
xchg rax,rbx           ; RAX = Number of bytes stored 
xor edx,edx
mov ebx,NUMACTRL_SIZE  ; RBX = One entry size
div ebx                ; RAX = Total size / Entry size = entries stored,
                       ; note must count ACTIVE NODES if affinity mask non zero 
; Exit points, EAX = detected nodes count, after reject nodes with mask = 0 
.DoneNumaBuild:
clc                    ; CF=0 means no errors
.ExitNumaBuild:
mov rsp,rbp           ; Restore RSP at subroutine entry, because stack alignment
pop rcx r14 r13 r12 rbp rdi rsi rbx    ; first, pop rcx because local variable
ret
.ErrorNumaBuild:
stc                   ; CF=1 means error detected
jmp .ExitNumaBuild

; Branch without NUMA support, for platform without NUMA or
; rejected by NUMA option settings

; TODO.
; Bug if NUMA rejected, but Processor Groups supported

.WithoutNuma:
mov rdi,rbx                      ; RDI = RBX = Base address of build nodes list
mov ecx,NUMACTRL_SIZE / 4        ; RCX = One NUMA entry size, units = dwords
xor eax,eax                      ; EAX = write data = 0
rep stosd                        ; Blank entry
mov rax,[SystemParameters.SystemInfo.dwActiveProcessorMask]  ; RAX = mask 
mov [rbx + NUMACTRL.NodeAffinity],rax  ; set affinity mask = all system mask
mov al,1               ; Set return nodes count = 1, this branch without NUMA
jmp .DoneNumaBuild

