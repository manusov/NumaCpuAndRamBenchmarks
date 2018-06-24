;---------- Memory performance measurement (Multi-Thread) ---------------------;

;---------- Open threads session: allocate memory, events, threads handles ----;
; INPUT:   RBX = Pointer to Threads Control List + Signaled Events List        ;
;          ECX = Number of threads                                             ;
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
sub rsp,32                     ; Built parameters shadow, 4 qwords

; Divide total size (bytes) by number of threads
mov rax,[rbx+THCTRL.SizeBytes]
xor edx,edx
test rcx,rcx
jz OpenMultiThreadError        ; Go error if number of threads = 0
cmp rcx,MAX_THREADS
ja OpenMultiThreadError        ; Go error if number of threads > maximum  
div rcx                        ; RAX = Work memory per thread
test rdx,rdx
jnz OpenMultiThreadError       ; Go error if fractional memory allocation
xchg r8,rax                    ; R8 = Bytes per thread
; Divide total size (instructions) by number of threads 
mov rax,[rbx+THCTRL.SizeInst]
div rcx                        ; RAX = Work memory per thread
test rdx,rdx
jnz OpenMultiThreadError       ; Go error if fractional instructions allocation
xchg r9,rax                    ; R8 = Instructions per thread

; Select NUMA or non-NUMA mode
mov al,[InputParms.OptionNUMA]
cmp al,2
jb .ModeNonNUMA
cmp al,3
ja .ModeNonNUMA

; Branch for NUMA mode

.ModeNUMA:
; Build NUMA nodes list
push rbx rcx r8 r9
lea rbx,[NumaControl]
call BuildNumaList
pop r9 r8 rcx rbx
jc OpenMultiThreadError         ; Go error if build NUMA nodes list error
; Allocate all memory
push rbx rcx r8 r9
mov ecx,eax
mov rdx,[rbx+THCTRL.SizeBytes]  ; Full size at EACH node, TODO: divide 
push rcx
lea rbx,[NumaControl]
call MemoryAllocationNuma
pop rsi                         ; RSI = Nodes count
pop r9 r8 rcx rbx               ; RCX = Threads count 
jc OpenMultiThreadError         ; Go error if memory allocation error
; Calculate threads per node
mov rax,rcx
xor edx,edx
test rsi,rsi
jz OpenMultiThreadError        ; Go error if number of nodes = 0
cmp rsi,MAX_NODES
ja OpenMultiThreadError        ; Go error if number of nodes > maximum  
div rsi                        ; RAX = Threads per node
test rax,rax
jz OpenMultiThreadError        ; Go error if threads per node = 0
test rdx,rdx
jnz OpenMultiThreadError       ; Go error if threads per node is fractional 

; Initializing cycle parameters for build control structures
push rbx rcx

; Load initial parameters
mov ecx,esi                    ; ECX = NUMA nodes count
xchg esi,eax                   ; ESI = Threads per node count
mov r10,[rbx+THCTRL.EntryPoint]
mov r11,[rbx+THCTRL.Base1]
mov r12,[rbx+THCTRL.Base2]
mov r13,[rbx+THCTRL.Repeats]
lea r14,[rbx + THREADS_CONTROL_SIZE + EVENTS_CONTROL_SIZE]

; Cycle for NUMA nodes
.NextNode:
xor edi,edi                    ; EDI = Counter for threads per node allocation
push r11 r12
mov rax,[r14+NUMACTRL.AlignedBase]
add r11,rax
add r12,rax
; Affinity mask
mov rax,[r14+NUMACTRL.NodeAffinity]
cmp [InputParms.OptionNUMA],3       ; Need invert affinity if NUMA-REMOTE mode
jne @f
xor rax,[SystemInfo.dwActiveProcessorMask]
@@:
; Cycle for threads
.NextThread:
mov [rbx+THCTRL.EntryPoint],r10
mov [rbx+THCTRL.Base1],r11
mov [rbx+THCTRL.Base2],r12
mov [rbx+THCTRL.Repeats],r13
mov [rbx+THCTRL.SizeBytes],r8
mov [rbx+THCTRL.SizeInst],r9
mov [rbx+THCTRL.ThreadAffinity],rax
add r11,r8
add r12,r8
add rbx,THCTRL_SIZE
; Make cycle for threads
inc edi
cmp edi,esi
jb .NextThread 

; Make cycle for NUMA nodes
pop r12 r11
add r14,NUMACTRL_SIZE
loop .NextNode

; End of build cycles
pop rcx rbx
; End of NUMA-specific part
jmp .EventsAndThreads 

; Branch for non-NUMA mode

.ModeNonNUMA:
; Allocate all memory
push rbx rcx rbp r8 r9
call MemoryAllocation
pop r9 r8 rbp rcx rbx
jc OpenMultiThreadError        ; Go error if memory allocation error
; Cycle for build control structures
push rbx rcx
mov r10,[rbx+THCTRL.EntryPoint]
mov r11,[rbx+THCTRL.Base1]
mov r12,[rbx+THCTRL.Base2]
mov r13,[rbx+THCTRL.Repeats]
.BuildControls:
mov [rbx+THCTRL.EntryPoint],r10
mov [rbx+THCTRL.Base1],r11
mov [rbx+THCTRL.Base2],r12
mov [rbx+THCTRL.Repeats],r13
mov [rbx+THCTRL.SizeBytes],r8
mov [rbx+THCTRL.SizeInst],r9
add r11,r8
add r12,r8
add rbx,THCTRL_SIZE
loop .BuildControls
pop rcx rbx

; Continue both non-NUMA and NUMA branches

; Create events and threads in the common cycle
.EventsAndThreads:
mov rsi,rbx
lea rdi,[rbx+THREADS_CONTROL_SIZE]
mov r12,rcx
xor ebx,ebx
; Cycle
.CreateEventsThreads:                                    
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
lea r8,[ThreadsEntry]              ; R8  = Parm#3 = Execute routine pointer
mov r9d,ebx                        ; R9  = Parm#4 = Subroutine parameter, 0-63
push 0                             ; Parm#6 = Thread ID return , 0=None
push CREATE_SUSPENDED              ; Parm#5 = Creating Flags = Suspended
sub rsp,32                         ; Make parameters shadow
call [CreateThread]                ; Call WinAPI function
add rsp,32+16                      ; Remove parameters shadow + 2 parameters
test rax,rax                       ; Status=0 means error
jz OpenMultiThreadError            ; Go if error creating thread
mov [rsi+THCTRL.ThreadHandle],rax  ; Store thread handle in the per-thread list
; Cycle
inc ebx
add rsi,THCTRL_SIZE
dec r12
jnz .CreateEventsThreads          ; Cycle for all events and threads handles
; Exit points, use previous add CF=0(NC)
OpenMultiThreadExit:
mov rsp,rbp
ret
OpenMultiThreadError:
stc
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
sub rsp,32                     ; Built parameters shadow, 4 qwords
; Release all memory
push rbx rcx rbp
mov al,[InputParms.OptionNUMA]
cmp al,2
jb .L0
cmp al,3
ja .L0
; This branch for NUMA mode
lea rbx,[NumaControl]
call MemoryReleaseNuma 
jmp .L1
.L0:
; This branch for non-NUMA mode
call MemoryRelease
.L1:
pop rbp rcx rbx
jc OpenMultiThreadError        ; Go error if memory allocation error
; Scan list of threads
mov rsi,rbx
mov edi,ecx
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
;--- Cycle ---
add rsi,THCTRL_SIZE                ; Select next thread entry
dec edi                            ; Count pairs of entries
jnz ReleaseThreads                 ; Cycle for all entries
;--- Exit ---
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
sub rsp,32                     ; Built parameters shadow, 4 qwords
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
; INPUT:   RCX = Parm#1 = Thread ID, 0-63                                      ;
; OUTPUT:  None, because no return                                             ;
;------------------------------------------------------------------------------;
ThreadsEntry:
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                    ; Built parameters shadow, 4 qwords
; Calculate thread control entry offset = f(thread id)
and ecx,0000003Fh             ; Initialize entire 64-bit RCX
shl ecx,7                     ; Shift because 128 bytes per block
lea r15,[ThreadsControl+rcx]  ; R15 = Address for block for this thread
; This point for restart thread
ThreadRepeat:
mov rbx,r15
; Set affinity mask for this thread
mov rcx,[rbx+THCTRL.ThreadHandle]     ; Parm#1 = RCX = Thread handle
mov rdx,[rbx+THCTRL.ThreadAffinity]   ; Parm#2 = RDX = Affinity mask
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
jmp ThreadRepeat
