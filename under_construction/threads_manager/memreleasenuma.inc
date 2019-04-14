;---------- NUMA-aware memory release -----------------------------------------;
; INPUT:   RCX = Pointer to nodes list                                         ;
;          EDX = Number of nodes                                               ; 
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;                                                                              ;
; Can destroy registers, volatile by Microsoft x64 calling convention.         ; 
;------------------------------------------------------------------------------;

MemReleaseNuma:
cld
push rbx rsi rdi rbp r12
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h         ; Stack alignment because WinAPI call
sub rsp,32                         ; Make parameters shadow because WinAPI call 
; Reload input parameters to non-volatile registers
mov rsi,rcx                        ; RSI = Pointer to NUMA nodes list
mov edi,edx                        ; EDI = Number of used NUMA nodes
; Get current process handle
call [GetCurrentProcess]           ; Return RAX = Handle
xchg rbx,rax                       ; Copy to non-volatile register, XCHG for compact
; Set nodes list limit
mov r12d,MAX_NODES                 ; R12D = Maximum number of NUMA nodes

; Cycle for release memory, allocated at all nodes
.NumaRelease:
mov rcx,rbx                        ; Parm#1 = RCX = Process Handle
mov rdx,[rsi + NUMACTRL.TrueBase]  ; Parm#2 = RDX = Address
xor r8d,r8d                        ; Parm#3 = R8 = Release size, here must be 0 = all block
mov r9d,MEM_RELEASE	               ; Parm#4 = R9 = Release type
test rdx,rdx
jz .SkipFree                       ; Skip this entry if memory not allocated for it
call [VirtualFreeEx]               ; Call memory release WinAPI, return RAX = Status
test rax,rax
jz .ErrorReleaseNuma               ; Go if error detected
.SkipFree:
; Cycle for NUMA nodes
dec edi
jz .NodesDone
add rsi,NUMACTRL_SIZE              ; Select next entry                    
dec r12
jnz .NumaRelease                   ; Cycle for all (typical 64) entries 

; Exit points
.NodesDone:
clc                  ; CF=0 means status = no errors 
.ExitReleaseNuma:
mov rsp,rbp          ; Restore RSP at subroutine entry, because stack alignment
pop r12 rbp rdi rsi rbx
ret
.ErrorReleaseNuma:
stc                  ; CF=1 means status = error detected
jmp .ExitReleaseNuma
