;------------------------------------------------------------------------------;
;          Memory performance measurement, support Multi-Thread mode.          ;
;------------------------------------------------------------------------------;

;---------- Open threads session: allocate memory, events, threads handles ----;
; INPUT:   RBX = Pointer to Threads Control List + Signaled Events List        ;
;          ECX = Number of Threads                                             ;
;          First entry of Threads Control List contains operational data       ;   
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          Registers corrupted, except R15 = saved                             ;
;          Threads control list updated                                        ; 
;------------------------------------------------------------------------------;
MultiThreadOpen:
cld
; Stack alignment, required because OS API used
mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
sub rsp,32                     ; Build parameters shadow, 4 qwords

; Divide total size (bytes) by number of threads
mov rax,[rbx+THCTRL.SizeBytes]  ; RAX = Total memory size for benchmark
xor edx,edx                     ; Make 128-bit dividend RDX:RAX
test rcx,rcx                    ; RCX = Number of threads, check for 0=error
jz OpenMultiThreadError         ; Go error if number of threads = 0
cmp rcx,MAX_THREADS             ; Check for threads count limit
ja OpenMultiThreadError         ; Go error if number of threads > maximum  
div rcx                         ; RAX = Work memory per thread
test rdx,rdx                    ; Check RDX = remainder after division
jnz OpenMultiThreadError        ; Go error if fractional memory allocation
xchg r8,rax                     ; R8 = Bytes per thread, XCHG for compact code
; Divide total size (instructions) by number of threads 
mov rax,[rbx+THCTRL.SizeInst]
div rcx                         ; RAX = Instructions per thread (really used buffer)
test rdx,rdx                    ; Check RDX = remainder after division
jnz OpenMultiThreadError        ; Go error if fractional instructions allocation
xchg r9,rax                     ; R9 = Instructions per thread

; Select NUMA or non-NUMA mode
mov al,[InputParms.OptionNUMA]  ; AL = NUMA option selected value
cmp al,2
jb .ModeNonNUMA    ; Go if <2, means 0 = None, 1 = Supported by platform, but unaware by application
cmp al,3
ja .ModeNonNUMA    ; Go if >3, means unknown value of option
                   ; NUMA branch if 2 = force local, 3 = force remote
                   
;---------- Branch for NUMA mode ----------------------------------------------;

.ModeNUMA:
; Build NUMA nodes list
push rbx rcx r8 r9 rbp
lea rbx,[NumaControl]
call BuildNumaList              ; Return EAX = Active nodes count, 
pop rbp r9 r8 rcx rbx           ; not count nodes with affinity mask = 0
jc OpenMultiThreadError         ; Go error if build NUMA nodes list error
; Allocate all memory
push rbx rcx r8 r9 rbp
mov ecx,eax                     ; RCX = active (nonzero) nodes count
mov rdx,[rbx+THCTRL.SizeBytes]  ; Full size at EACH node, TODO: divide 
push rcx
mov r8,[rbx+THCTRL.LargePages]
lea rbx,[NumaControl]
call MemoryAllocationNuma
pop rsi                         ; RSI = Nodes count, restored from RCX push
pop rbp r9 r8 rcx rbx           ; RCX = Threads count 
jc OpenMultiThreadError         ; Go error if memory allocation error

; Calculate threads per node
mov rax,rcx                    ; RAX = Threads count
xor edx,edx                    ; RDX = 0 , threads count extension for divide
test rsi,rsi                   ; RSI = Nodes count, check for 0=error
jz OpenMultiThreadError        ; Go error if number of nodes = 0
cmp rsi,MAX_NODES
ja OpenMultiThreadError        ; Go error if number of nodes > maximum  
div rsi                        ; RAX = Threads / Nodes = Threads per node
test rax,rax
jz OpenMultiThreadError        ; Go error if threads per node = 0
test rdx,rdx                   ; Check RDX = Remainder after division
jnz OpenMultiThreadError       ; Go error if threads per node is fractional 

; Initializing cycle parameters for build control structures
push rbx rcx              ; Save RBX = threads list pointer, RCX = threads count

; Load initial parameters, RBX = Threads pointer , R14 = Nodes pointer 
mov ecx,esi                         ; ECX = NUMA nodes count
xchg esi,eax                        ; ESI = Threads per node count
mov r10,[rbx+THCTRL.EntryPoint]     ; R10 = Selected performance pattern entry point 
mov r11,[rbx+THCTRL.Base1]          ; R11 = Base address 1 (source/destination) 
mov r12,[rbx+THCTRL.Base2]          ; R12 = Base address 2 (destination if dual address)
mov r13,[rbx+THCTRL.Repeats]        ; R13 = Measurement repeats count 
lea r14,[rbx + THREADS_CONTROL_SIZE + EVENTS_CONTROL_SIZE]  ; R14 = Nodes pointer

; External cycle for NUMA nodes
.NextNode:

xor edi,edi                    ; EDI = Counter for threads per node allocation
push r11 r12                   ; Save blocks offsets by test scenario, example 0, L1/2
mov rax,[r14+NUMACTRL.AlignedBase]
add r11,rax                         ; R11 = Base1 = Allocated base + Block1 offset
add r12,rax                         ; R12 = Base2 = Allocated base + Block2 offset
; Affinity mask
mov rax,[r14+NUMACTRL.NodeAffinity]  ; RAX = Affinity mask for this node
cmp [InputParms.OptionNUMA],3        ; Need invert affinity if NUMA-REMOTE mode
jne @f                               ; Go skip if not a remote mode
xor rax,[SystemInfo.dwActiveProcessorMask]  ; Invert used part of affinity mask
@@:

; Internal cycle for threads
.NextThread:

mov [rbx+THCTRL.EntryPoint],r10        ; Set selected performance pattern entry point
mov [rbx+THCTRL.Base1],r11             ; Set base address 1 (source/destination)
mov [rbx+THCTRL.Base2],r12             ; Set base address 2 (destination if dual address) 
mov [rbx+THCTRL.Repeats],r13           ; Set measurement repeats count
mov [rbx+THCTRL.SizeBytes],r8          ; Set block allocation size, units = bytes
mov [rbx+THCTRL.SizeInst],r9           ; Set block performance size, units = instructions 
mov [rbx+THCTRL.ThreadAffinity],rax    ; Set thread affinity mask, 64-bit KAFFINITY

; INT3
push qword [r14+NUMACTRL.NodeGroup]
pop qword [rbx+THCTRL.ThreadGroup]     ; Set Processor Group associated with affinity mask
;

; Make internal cycle for threads
add r11,r8               ; R11 = memory base1, add R8 = bytes per thread
add r12,r8               ; R12 = memory base1, add R8 = bytes per thread
add rbx,THCTRL_SIZE      ; Advance pointer to next thread (internal cycle)
inc edi                  ; EDI = count thread per nodes, current thread per node 
cmp edi,esi              ; EDI = current thread per node, ESI = total threads per node  
jb .NextThread           ; Internal cycle for THREADS, if current < limit 

; Make external cycle for NUMA nodes
pop r12 r11              ; Restore blocks offsets by test scenario, example 0, L1/2
add r14,NUMACTRL_SIZE    ; Advance pointer to next node (external cycle)
loop .NextNode           ; External cycle for NODES, RCX = counter

; End of build cycles
pop rcx rbx              ; Restore RCX = threads count, RBX = threads list pointer
; End of NUMA-specific part
jmp .EventsAndThreads 

;---------- Branch for non-NUMA mode ------------------------------------------;

.ModeNonNUMA:
; Allocate all memory, for all threads, this method possible at non-NUMA branch
push rbx rcx rbp r8 r9
call MemoryAllocation
pop r9 r8 rbp rcx rbx
jc OpenMultiThreadError        ; Go error if memory allocation error

; Load parameters for build control structures
push rbx rcx                       ; Save RBX = threads list pointer, RCX = threads count
mov r10,[rbx+THCTRL.EntryPoint]    ; R10 = Selected performance pattern entry point
mov r11,[rbx+THCTRL.Base1]         ; R11 = Block1 memory buffer base address
mov r12,[rbx+THCTRL.Base2]         ; R12 = Block1 memory buffer base address
mov r13,[rbx+THCTRL.Repeats]       ; R13 = Measurement repeats count

; Cycle for build threads entries
.BuildControls:
mov [rbx+THCTRL.EntryPoint],r10   ; Set selected performance pattern entry point 
mov [rbx+THCTRL.Base1],r11        ; Set thread buffer block 1 base address
mov [rbx+THCTRL.Base2],r12        ; Set thread buffer block 2 base address
mov [rbx+THCTRL.Repeats],r13      ; Set measurement repeats count 
mov [rbx+THCTRL.SizeBytes],r8     ; Set allocated block size, units = bytes
mov [rbx+THCTRL.SizeInst],r9      ; Set performance block size, units = instructions
; INT3
mov qword [rbx+THCTRL.ThreadAffinity],0  ; Blank affinity
mov qword [rbx+THCTRL.ThreadGroup],0     ; Blank group, associated with affinity
;
add r11,r8            ; R11 = Advance block 1 pointer, to next thread buffer 
add r12,r8            ; R12 = Advance block 2 pointer, to next thread buffer
add rbx,THCTRL_SIZE   ; RBX = Advance threads list pointer
loop .BuildControls   ; RCX = Cycle counter, by number of threads

pop rcx rbx           ; Restore RCX = threads count, RBX = threads list pointer

;---------- Continue both non-NUMA and NUMA branches --------------------------;

; Create events and threads in the common cycle
.EventsAndThreads:
mov rsi,rbx                          ; RSI = Pointer to threads list
lea rdi,[rbx+THREADS_CONTROL_SIZE]   ; RDI = Pointer for sequentally store handles
mov r12,rcx                          ; R12 = Threads count
xor ebx,ebx                          ; RBX = Threads number for create threads

; Cycle for events and threads, include set affinity mask = f(HT option)
.CreateEventsThreads:                                    
; Set affinity mask if Hyper-Threading supported by platform but not used by application
; INT3
cmp [InputParms.OptionHT],1   ; 1 means HT/SMT supported by platform but not enabled by option
jne .SkipHtMask               ; Go if this situation not detected 
mov rax,[rsi+THCTRL.ThreadAffinity]         ; Sub-mask for NUMA optimization
test rax,rax
jnz .NumaAwareHt                            ; Go if NUMA-aware mask exist
mov rax,[SystemInfo.dwActiveProcessorMask]  ; Full system mask
.NumaAwareHt:
mov rcx,05555555555555555h            ; Mask for disable ODD logical processors 
and rax,rcx                           ; Disable ODD logical processors
mov [rsi+THCTRL.ThreadAffinity],rax   ; Update affinity mask
.SkipHtMask:
;
; Create events
xor ecx,ecx                        ; RCX = Parm#1 = lpEventAttributes  
mov edx,1                          ; RDX = Parm#2 = bManualReset
xor r8d,r8d                        ; R8  = Parm#3 = bInitialState
xor r9d,r9d                        ; R9  = Parm#4 = Name, 0=None
call [CreateEvent]                 ; Call WinAPI function
test rax,rax                       ; Status=0 means error
jz OpenMultiThreadError            ; Go if error creating event
mov [rsi+THCTRL.EventHandle],rax   ; Store handle in the threads block
stosq                              ; Store handle in the handles sequental list
; Create threads
xor ecx,ecx                        ; RCX = Parm#1 = Security
xor edx,edx                        ; RDX = Parm#2 = Stack size, 0=default
; INT3
lea r8,[ThreadsEntry]              ; R8  = Parm#3 = Execute routine pointer
mov [rsi+THCTRL.AffinityMode],1    ; This added for single-thread affinitization
cmp [ProcessorsTotal],MAX_THREADS_PER_GROUP
jbe .AfterSetEntry                 ; Go if simple system, single group 
lea r8,[ThreadsEntryGroupMode]     ; This routine with affinitization with groups
mov [rsi+THCTRL.AffinityMode],2    ; This added for single-thread affinitization

.AfterSetEntry:
;
mov r9d,ebx                        ; R9  = Parm#4 = Subroutine parameter, 0-63 or 0-255(if Groups)
push 0                             ; Parm#6 = Thread ID return , 0=None
push CREATE_SUSPENDED              ; Parm#5 = Creating Flags = Suspended
sub rsp,32                         ; Make parameters shadow
call [CreateThread]                ; Call WinAPI function
add rsp,32+16                      ; Remove parameters shadow + 2 parameters
test rax,rax                       ; Status=0 means error
jz OpenMultiThreadError            ; Go if error creating thread
mov [rsi+THCTRL.ThreadHandle],rax  ; Store thread handle in the per-thread list
; Cycle
inc ebx                            ; RBX = Thread number +1
add rsi,THCTRL_SIZE                ; RSI = Advance threads list pointer
dec r12                            ; R12 = Threads count -1
jnz .CreateEventsThreads           ; Cycle for all events and threads handles

; Exit points, use previous add CF=0(NC)
OpenMultiThreadExit:
mov rsp,rbp
ret
OpenMultiThreadError:
stc                               ; CF=1 means error detected
jmp OpenMultiThreadExit

;---------- Close threads session: release memory, events, threads handles ----;
; INPUT:   RBX = Pointer to Threads Control List + Signaled Events List        ;
;          ECX = Number of threads                                             ;   
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          Registers corrupted, except R15 = saved                             ;
;------------------------------------------------------------------------------;
MultiThreadClose:
; Stack alignment, required because OS API used
mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
sub rsp,32                     ; Build parameters shadow, 4 qwords
; Release all memory
push rbx rcx rbp
mov al,[InputParms.OptionNUMA]
cmp al,2
jb .L0    ; Go to non-NUMA branch if 0=not supported, 1=not selected
cmp al,3
ja .L0    ; Go to non-NUMA branch if >3 means unknown option value
; This branch for NUMA mode, release memory, allocated for NUMA nodes
lea rbx,[NumaControl]
call MemoryReleaseNuma 
jmp .L1
.L0:
; This branch for non-NUMA mode, release memory allocated by single thread
call MemoryRelease
.L1:
pop rbp rcx rbx
jc OpenMultiThreadError        ; Go error if memory allocation error
; Scan list of threads
mov rsi,rbx                        ; RSI = Pointer to threads list
mov edi,ecx                        ; RDI = Number of threads 

; Cycle for release threads
ReleaseThreads:
; Terminate threads
mov rcx,[rsi+THCTRL.ThreadHandle]  ; RCX = Parm#1 = THREAD handle
jrcxz @f                           ; Skip if this entry not allocated
xor edx,edx                        ; RDX = Parm#2 = Thread exit code
call [TerminateThread]             ; Call WinAPI function
test rax,rax                       ; Status=0 means error
jz OpenMultiThreadError            ; Go exit if error
@@:
; Close Events
mov rcx,[rsi+THCTRL.EventHandle]   ; RCX = Parm#1 = Load EVENT handle
jrcxz @f                           ; Skip if this entry not allocated
call [CloseHandle]                 ; Call WinAPI function
test rax,rax                       ; Status=0 means error
jz OpenMultiThreadError            ; Go exit if error 
@@:
; Close Threads
mov rcx,[rsi+THCTRL.ThreadHandle]  ; RCX = Parm#1 = Load THREAD handle
jrcxz @f                           ; Skip if this entry not allocated
call [CloseHandle]                 ; Call WinAPI function
test rax,rax                       ; Status=0 means error
jz OpenMultiThreadError            ; Go exit if error
@@:
; Cycle for release threads
add rsi,THCTRL_SIZE                ; Select next thread entry
dec edi                            ; Count pairs of entries
jnz ReleaseThreads                 ; Cycle for all entries

; Exit
mov eax,1                  ; Status = no errors, if all skipped and RAX not set
jmp OpenMultiThreadExit    ; Here CF=0 

;---------- First call benchmark, include initialize context ------------------;
; INPUT:   RBX = Pointer to Threads Control List + Signaled Events List        ;
;          ECX = Number of threads                                             ;   
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          RAX = Delta TSC                                                     ;  
;          Registers corrupted, except R15 = saved                             ;
;------------------------------------------------------------------------------;
MultiThreadBenchmark:     ; This entry point for multi-thread benchmark mode
; Stack alignment, required because OS API used, save number of threads
mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
sub rsp,32                     ; Build parameters shadow, 4 qwords
mov rsi,rbx                    ; RSI = Store control list pointer
mov edi,ecx                    ; RDI = Store number of threads
; Serializing and get TSC, measurement interval start
xor eax,eax
cpuid                          ; This for serializing only
rdtsc                          ; Get TSC to EDX=High, EAX=Low
shl rdx,32
lea r12,[rax+rdx]              ; R12 = TSC value, 64-bit
; Scan list of threads
lea r13,[rsi+THREADS_CONTROL_SIZE] ; now RSI = Pointer to threads control block
xor ebx,ebx                        ; EBX = count active threads
; Cycle for run (resume) threads
.RunThreads:
mov rcx,[rsi+THCTRL.ThreadHandle]  ; RCX = Parm#1 = Load THREAD handle
jrcxz @f                           ; Skip if this entry not allocated
call [ResumeThread]                ; Call WinAPI function
cmp rax,-1                         ; Status=-1 means error
mov eax,0                          ; RAX=0, can't XOR EAX,EAX, must save flags
je OpenMultiThreadError            ; Go exit if error
inc ebx                            ; EBX = Count active threads
@@:
; Cycle
add rsi,THCTRL_SIZE            ; Select next thread entry
dec edi                        ; Count threads
jnz .RunThreads                ; Cycle for all entries
; Wait for N threads executed, count at EBX
WaitThreads:
mov ecx,ebx                    ; RCX = Parm#1 = Number of objects (threads)
mov rdx,r13                    ; RDX = Parm#2 = Pointer to events list
mov r8d,1                      ; R8  = Parm#3 = Wait All flag
mov r9d,60000                  ; R9  = Parm#4 = Timeout, milliseconds                
call [WaitForMultipleObjects]  ; Call WinAPI function
test rax,rax                   ; Status=0 means wait OK
mov rax,-1
jnz OpenMultiThreadError
; Serializing and get TSC, measurement interval end 
xor eax,eax
cpuid                          ; This for serializing only
rdtsc                          ; Get TSC to EDX=High, EAX=Low
shl rdx,32
add rax,rdx                    ; RAX = TSC value, 64-bit
sub rax,r12                    ; RAX = Delta TSC = Current TSC - Start TSC
; Exit points
jmp OpenMultiThreadExit

;---------- Restart benchamrk, called in the initialized context --------------;
; INPUT:   RBX = Pointer to Threads Control List + Signaled Events List        ;
;          ECX = Number of threads                                             ;   
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          RAX = Delta TSC                                                     ;  
;          Registers corrupted, except R15 = saved                             ;
;------------------------------------------------------------------------------;
MultiThreadRestart:
; Stack alignment, required because OS API used, save number of threads
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h    ; Stack alignment
sub rsp,32                    ; Make parameters shadow 
mov rsi,rbx                   ; RSI = Store control list pointer
mov edi,ecx                   ; RDI = Store number of threads
; Serializing and get TSC, measurement interval start
xor eax,eax
cpuid                         ; This for serializing only
rdtsc                         ; Get TSC to EDX=High, EAX=Low
shl rdx,32
lea r12,[rax+rdx]             ; R12 = TSC value, 64-bit
; Scan list of threads
lea r13,[rsi+THREADS_CONTROL_SIZE] ; now RSI = Pointer to threads control block
mov ebx,edi                        ; EBX = count active threads
RestartThreads:
; Restart threads by clear events
mov rcx,[rsi+THCTRL.EventHandle]    ; RCX = Parm#1 = Event handle
jrcxz @f                            ; Go skip if this entry not used
call [ResetEvent]                   ; Reset event cause repeat benchmark call
@@:
; Cycle
add rsi,THCTRL_SIZE                 ; Select next thread entry
dec edi                             ; Count threads
jnz RestartThreads                  ; Cycle for all entries
; Go to wait for execution
jmp WaitThreads

;---------- Thread routine, called by OS callback mechanism -------------------;
; Variant without support Processor Groups, systems with <64 logical CPUs      ;
;                                                                              ;
; INPUT:   RCX = Parm#1 = Thread ID, 0-63                                      ;
; OUTPUT:  None, because no return                                             ;
;------------------------------------------------------------------------------;
ThreadsEntry:
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                    ; Build parameters shadow, 4 qwords
; Calculate thread control entry offset = f(thread id)
and ecx,0000003Fh             ; Initialize entire 64-bit RCX
shl ecx,7                     ; Shift because 128 bytes per block
lea r15,[ThreadsControl+rcx]  ; R15 = Address for block for this thread

; This point for restart thread
.ThreadRepeat:
mov rbx,r15
; Set affinity mask for this thread
mov rcx,[rbx+THCTRL.ThreadHandle]     ; Parm#1 = RCX = Thread handle
mov rdx,[rbx+THCTRL.ThreadAffinity]   ; Parm#2 = RDX = Thread Affinity mask

; INT3
; This point for check affinitization

test rdx,rdx
jz @f                ; Skip set affinity mask if memory allocated without NUMA
call [_SetThreadAffinityMask]
@@:
; Target memory operation
call SingleThreadBenchmark
; Signaling thread terminated
mov rcx,[r15+THCTRL.EventHandle]    ; RCX = Parm#1 = Handle for this thread
call [SetEvent]                     ; Call WinAPI function
; Wait for thread re-run
@@:
mov rcx,[r15+THCTRL.EventHandle]  ; RCX = Parm#1 = Handle for this thread
xor edx,edx                       ; RDX = Parm#2 = 0, means return without wait
call [WaitForSingleObject]
test rax,rax                  ; RAX = Return = 0 if object state = signaled
jz @b                         ; Wait if object signaled
; Restart thread
jmp .ThreadRepeat

;---------- Thread routine, called by OS callback mechanism -------------------;
; Variant with support Processor Groups, systems with >64 logical CPUs         ;
;                                                                              ;
; INPUT:   RCX = Parm#1 = Thread ID, 0-255                                     ;
; OUTPUT:  None, because no return                                             ;
;------------------------------------------------------------------------------;
ThreadsEntryGroupMode:
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                    ; Build parameters shadow, 4 qwords
; Calculate thread control entry offset = f(thread id)
and ecx,0000003Fh             ; Initialize entire 64-bit RCX
shl ecx,7                     ; Shift because 128 bytes per block
lea r15,[ThreadsControl+rcx]  ; R15 = Address for block for this thread

; This point for restart thread
.ThreadRepeat:
mov rbx,r15
; Set affinity mask for this thread
mov rcx,[rbx+THCTRL.ThreadHandle]     ; Parm#1 = RCX = Thread handle
lea rdx,[rbx+THCTRL.ThreadAffinity]   ; Parm#2 = RDX = Pointer to mask + group
lea r8,[rbx+THCTRL.OrigAffinity]      ; Parm#3 = R8 = Pointer to get original
cmp qword [rdx],0
je @f                ; Skip set affinity mask if memory allocated without NUMA
call [_SetThreadGroupAffinity]      ; This changed for Processor Groups
@@:
; Target memory operation
call SingleThreadBenchmark
; Signaling thread terminated
mov rcx,[r15+THCTRL.EventHandle]    ; RCX = Parm#1 = Handle for this thread
call [SetEvent]                     ; Call WinAPI function
; Wait for thread re-run
@@:
mov rcx,[r15+THCTRL.EventHandle]  ; RCX = Parm#1 = Handle for this thread
xor edx,edx                       ; RDX = Parm#2 = 0, means return without wait
call [WaitForSingleObject]
test rax,rax                  ; RAX = Return = 0 if object state = signaled
jz @b                         ; Wait if object signaled
; Restart thread
jmp .ThreadRepeat

