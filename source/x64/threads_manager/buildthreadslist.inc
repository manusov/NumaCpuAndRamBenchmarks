;---------- Build threads list: events, threads, memory ranges ----------------;
; INPUT:   RCX = Pointer to Threads Control List +                             ;
;                Signaled Start-Done Events List + NUMA nodes list             ;
;          RDX = Pointer to Input Parameters Block (IPB)                       ;
;                                                                              ;             
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;                                                                              ;
; Can destroy registers, volatile by Microsoft x64 calling convention.         ;
;------------------------------------------------------------------------------;

BuildThreadsList:
cld
push rbx rsi rdi rbp r12 r13 r14 r15 0 0  ; Note push 0,0 for local variables
mov rbp,rsp                         ; RBP = Save RSP at procedure entry
and rsp,0FFFFFFFFFFFFFFF0h          ; Stack alignment
sub rsp,32                          ; Make parameters shadow because WinAPI calls
; Reload parameters to non-volatile registers
mov rsi,rcx                         ; RSI  = Pointer to Threads List
lea rdi,[rcx + THREADS_LIST_SIZE]   ; RDI  = Pointer to 2 x Events Lists 
lea rbx,[rcx + THREADS_LIST_SIZE + EVENTS_LIST_SIZE * 2]  ; RBX = NUMA nodes list  
mov r12,rdx                         ; R12  = Pointer to Input Param. Block (IPB)
mov r13d,[rdx + IPB.UpdatedThreads] ; R13D = Number of threads, counter
xor r14d,r14d                       ; R14D = Threads counter
; Initialize THREADS per NODE counter
mov eax,r13d                        ; EAX = Number of threads
xor edx,edx
mov ecx,[r12 + IPB.DomainsCount]
test rcx,rcx                        ; Note can't use JRCXZ because out of range
jz .ErrorBuildThreads               ; Go error if number of domains = 0 
div rcx                             ; EAX = R15D = Threads / Node, means
mov r15d,eax                        ;       frequency of next NODE selection
mov [rbp+00],eax                    ;       when build THREADS


; Threads build cycle
.ThreadsBuild:

; Create event for thread start, store event handle
xor ecx,ecx                            ; RCX = Parm#1 = lpEventAttributes  
mov edx,1                              ; RDX = Parm#2 = bManualReset
xor r8d,r8d                            ; R8  = Parm#3 = bInitialState
xor r9d,r9d                            ; R9  = Parm#4 = Name, 0=None
call [CreateEvent]                     ; Call WinAPI function
test rax,rax                           ; Status=0 means error
jz .ErrorBuildThreads                  ; Go if error creating event
mov [rsi + THCTRL.EventStart],rax      ; Store handle in the threads block
stosq                                  ; Copy to sequental list, used by OS API

; Create event for thread done, store event handle
xor ecx,ecx                            ; RCX = Parm#1 = lpEventAttributes  
mov edx,1                              ; RDX = Parm#2 = bManualReset
xor r8d,r8d                            ; R8  = Parm#3 = bInitialState
xor r9d,r9d                            ; R9  = Parm#4 = Name, 0=None
call [CreateEvent]                     ; Call WinAPI function
test rax,rax                           ; Status=0 means error
jz .ErrorBuildThreads                  ; Go if error creating event
mov [rsi + THCTRL.EventDone],rax       ; Store handle in the threads block
mov [rdi + EVENTS_LIST_SIZE - 8],rax   ; Copy to sequental list, used by OS API 

; Create thread, store event handle
xor ecx,ecx                            ; RCX = Parm#1 = Security
xor edx,edx                            ; RDX = Parm#2 = Stack size, 0=default
cmp [r12 + IPB.UpdatedPG],PG_OPTIMAL   ; Processor Group option, 2=used
lea r8,[ThreadRoutineSingleGroup]      ; R8  = Parm#3 = Execute routine pointer
mov [rsi + THCTRL.AffinityMode],1      ; This added for single-thread affinitization
jne .AfterSetRoutine                   ; Go if simple system, single group 
lea r8,[ThreadRoutineMultiGroup]       ; This routine with affinitization with groups
mov [rsi + THCTRL.AffinityMode],2      ; This added for single-thread affinitization
.AfterSetRoutine:
mov r9d,r14d                           ; R9  = Parm#4 = Subroutine parameter, 0-63 or 0-255(if Groups)
push 0                                 ; Parm#6 = Thread ID return , 0=None
push CREATE_SUSPENDED                  ; Parm#5 = Creating Flags = Suspended
sub rsp,32                             ; Make parameters shadow
call [CreateThread]                    ; Call WinAPI function
add rsp,32+16                          ; Remove parameters shadow + 2 parameters
test rax,rax                           ; Status=0 means error
jz .ExitBuildThreads                   ; Go if error creating thread
mov [rsi + THCTRL.ThreadHandle],rax    ; Store thread handle in the per-thread list

; Setup affinity mask (include processor group) from NUMA to Thread Entry
; include special affinization for HT supported but not used
mov rax,[rbx + NUMACTRL.NodeAffinity]
cmp [r12 + IPB.UpdatedHT],HT_NOT_USED  ; detect means HT supported but not used
mov rcx,rax
jne .AfterMasking                      ; Go skip masking for other variants
mov rcx,05555555555555555h
and rcx,rax
jnz .AfterMasking
mov rcx,rax                            ; Restore unmasked if masked to zero 
.AfterMasking:
mov [rsi + THCTRL.ThreadAffinity],rcx 
mov rax,[rbx + NUMACTRL.NodeGroup]
mov [rsi + THCTRL.ThreadGroup],rax 

; Setup asm method subroutine entry point address from IPB to Thread Entry
mov rax,[r12 + IPB.PatternRoutine]
mov [rsi + THCTRL.EntryPoint],rax

; Setup base address for block1, block2, note 2 blocks because block copy sup.
mov rax,[rbx + NUMACTRL.AlignedBase]   ; RAX = base address allocated at node
add rax,[rbp+08]                       ; add offset
mov [rsi + THCTRL.OrigBase],rax        ; store base address
mov rcx,[r12 + IPB.MemoryPerThread]    ; RCX = ALLOCATED size
add [rbp+08],rcx                       ; modify offset by this thread memory
shr rcx,1
mov [rsi + THCTRL.Base1],rax           ; Operational base 1 = base
add rax,rcx
mov [rsi + THCTRL.Base2],rax           ; Operational base 2 = base + size / 2

; Setup block size, bytes, this field must be updated per draw iteration
mov [rsi + THCTRL.SizeBytes],rcx       ; Operational block size, units = BYTES

; Setup block size, instructions, this field must be updated per draw iteration
mov ecx,[r12 + IPB.OperandWidth]    ; RCX = bits per instruction
shr rcx,3                           ; RCX = bytes per instruction, BPI 
jz .ErrorBuildThreads
mov rax,[r12 + IPB.StartBlockSize]  ; here OPERATIONAL size
xor edx,edx
div rcx                          ; Instruction count = Work length / BPI 
mov [rsi + THCTRL.SizeInst],rax  ; Operational block size, units = INSTRUCTIONS

; Setup Large Page option, note get 32-bit value, set zero-extended 64-bit value
mov eax,[r12 + IPB.UpdatedLP]
mov [rsi + THCTRL.LargePages],rax

; Setup number of repeat measurement iterations, note copy 64-bit value
mov rax,[r12 + IPB.MeasureRepeats]
mov [rsi + THCTRL.Repeats],rax 

; Select next NUMA node if THREADS per NODES counter done
dec r15d                ; R15D = Threads per NUMA node ratio
jnz .AfterCountNode     ; Go skip reload if continue threads per this node
mov r15d,[rbp+00]       ; R15D = Reload threads per this node
mov qword [rbp+08],0    ; Blank block offset because next NUMA node
add rbx,NUMACTRL_SIZE   ; RBX = NUMA nodes entries pointer
.AfterCountNode:

; Cycle for Threads
add rsi,THCTRL_SIZE     ; RSI  = Threads entries pointer
inc r14d                ; R14D = Threads counter for Thread ID
dec r13d                ; R13D = Threads counter for cycle
jnz .ThreadsBuild


; Exit points
clc                     ; CF=0 means status = no errors 
.ExitBuildThreads:
mov rsp,rbp             ; Restore RSP at subroutine entry, because stack alignment
pop rax rax r15 r14 r13 r12 rbp rdi rsi rbx  ; First, second pop rax remove local variables
ret
.ErrorBuildThreads:
stc                     ; CF=1 means status = error detected
jmp .ExitBuildThreads





