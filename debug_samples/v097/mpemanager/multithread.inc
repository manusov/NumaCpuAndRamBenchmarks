;---------- Memory performance measurement (Multi-Thread) ---------------------;
; Yet multi-thread mode uses 16 threads = const only.                          ; 
;                                                                              ;
; Note threads affinitization yet supported for MultiThread mode only          ;
; Note about target blocks addressing.                                         ;
; For Single-Thread, subroutine ExecuteBenchmarkST use input RSI from ICBB.    ;
; For Multi-Thread, subroutine ExecuteBenchmarkMT use Block Base from          ;
; ThreadsControl block, include support NUMA optimal block assignment.         ;
;                                                                              ; 
; INPUT:   RSI = Block #1 pointer (64-bit flat), ignored for multi-thread      ;
;          RDI = Block #2 pointer (64-bit flat)                                ;
;          R8  = Block #3 pointer (64-bit flat), reserved, not used yet        ;
;                For Read, Write, Modify use RSI as Source or Destination      ;
;                For Copy use RSI = Source , RDI = Destination                 ;
;          RCX = Block length, units = instructions                            ;
;          RBP = Number of measurement repeats                                 ;
;          RBX = Target subroutine address                                     ;
;          RAX = Block size for allocation, units = bytes                      ;
;                                                                              ;
; OUTPUT:  RAX = Delta-TSC, clocks                                             ;
;          Registers corrupted, but must save R15                              ;
;          Called procedure must save R14                                      ;
;                                                                              ;
;------------------------------------------------------------------------------;

; Changes required:
; Functionality: Yet fixed 16 threads, make variable.
; Optimization: Create threads in suspend state, run later for better timings.

ExecuteBenchmarkMT:   ; This entry point for multi-thread
;--- Store threads input parameters ---
; Note affinity masks initialized at memory allocation procedure (not by this),
; if NUMA API support.: mask for node, passed as argument when memory allocation 
; if NUMA API not supported, mask=0, thread must skip set mask, if mask=0 
lea r12,[ThreadsControl+00]        ; R12 = Pointer to Threads Control Blocks
;--- Prepare affinity masks disjunction ---
xor r8d,r8d                        ; R8D = Threads counter
xor r9d,r9d                        ; R9  = Affinity masks disjunction
.BuiltDisjunction:
or r9,[r12+r8+104]                 ; R9 = Affinity masks disjunction
add r8d,128                        ; 128 bytes per thread
cmp r8d,128*CONST_THREADS
jb .BuiltDisjunction               ; Cycle for N threads, example = 16
;--- Take Hyper-Threading ---
;- cmp dword [r15-BasePoint+MadtLocalApics],000000002h
;- jb .NotChangeMask             ; Go skip masking if NCPU<2
;- test byte [r15-BasePoint+CpuSpecFlags],080h
;- jz .NotChangeMask             ; Go skip masking if HT not supported
;---
;- cmp byte [r15-BasePoint+OptionRht],1
;- jne .NotChangeMask            ; Go skip masking if Reject HT = Off by options
;---
cmp [OptionHT],1                 ; 1 means HT present but not used
jne .NotChangeMask 
mov r8,05555555555555555h
and r9,r8                        ; This masking disable HT if it active
.NotChangeMask:
;--- Built affinity masks = f(options) ---
xor r8d,r8d                      ; R8D = Threads counter
.BuiltAffinity:
mov r10,[r12+104]                ; Get mask from FIRST thread
;cmp [r15-BasePoint+OptionMd],1  ; Go if Multi-Domain disabled: FIRST NODE MASK
;jne .NumaOn
;---
;- cmp [r15-BasePoint+OptionRrd],1 ; Check option: Reject Remote Domains
;- je .NumaOn                      ; Go if Rejected: FIRST NODE MASK
;---
;
; YET BUG OR SPECIFIC: REJECT REMOTE DOMAINS NOT USED, SEE CHECK ABOVE 
;
;---
mov r10,[r12+r8+104]           ; Get mask from CURRENT thread
mov r11l,[OptionNUMA]

;- PATCH, VALUE+1 -
; cmp r11l,0                                  
; je .NumaOff            ; Go if No NUMA Control: Set DISJUNCTED mask 
; cmp r11l,1
; je .NumaOn             ; Go if NUMA force local: Set CURRENT mask
;-
cmp r11l,1                                  
jbe .NumaOff             ; Go if No NUMA Control: Set DISJUNCTED mask 
cmp r11l,2
je .NumaOn               ; Go if NUMA force local: Set CURRENT mask
;- PATCH -

not r10                ; Otherwise NUMA force remote: Set INVERTED CURRENT mask
.NumaOn:                                     
and r10,r9
mov [r12+r8+016],r10           ; Set current or first mask if NUMA on
jmp .NumaAfter
.NumaOff:
mov [r12+r8+016],r9            ; Set disjuncted mask if NUMA off
.NumaAfter:
add r8d,128                    ; 128 bytes per thread
cmp r8d,128*CONST_THREADS
jb .BuiltAffinity              ; Cycle for N threads, example = 16
;--- Built other threads parameters ---
xor r8d,r8d                    ; R8D = Threads counter
.BuiltThreads:
mov [r12+r8+24],rbx            ; Store subroutine base address for indirect call
mov r9,[r12+r8+32]             ; Load pre-allocated Block #1 address
add r9,rax                     ; Add block size in RAX
mov [r12+r8+40],r9             ; Store Block #2 address

;- Disabled at v0.94 because Block#3 not used -
;- add r9,rax                     ; Add block size in RAX
;- mov [r12+r8+48],r9             ; Store Block #3 address
;-

mov [r12+r8+56],rcx            ; Store Block size, units=instructions
mov [r12+r8+64],rbp            ; Store Number of measurement iterations
add r8d,128                    ; 128 bytes per thread
cmp r8d,128*CONST_THREADS
jb .BuiltThreads               ; Cycle for N threads, example = 16
;--- Threads control ---
; Additional parameters required: R15 = Global data pointer
;                                 RAX = Inter-block offset
; Return: CF = Error flag: 0(NC)=Executed OK, 1(C)=Error
;         RAX = Delta-TSC for all parallel threads if NO ERRORS (CF=0)
;         RAX = 0 or Status after WaitForMultipleObjects if ERROR (CF=1)
;---
mov rbp,rsp                         ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h          ; Align stack because WinAPI used
sub rsp,32                          ; Built parameters shadow, 4 qwords
;--- Create 16 events for 16 threads ---
; Two duplicated events handles list required because:
; 1) API function WaitForMultipleObjects required sequental list
; 2) For fast access from threads, required data per threads list 
; Here R12 = Pointer to Threads Control Blocks
;---
xor esi,esi         ; RSI = Pointer to EVENTS Entries at Threads Control Blocks
lea rdi,[MtEventsHandles]      ; RDI = Pointer to EVENTS List
.CreateEvents:                                    
xor ecx,ecx                    ; RCX = Parm#1 = lpEventAttributes  
xor edx,edx                    ; RDX = Parm#2 = bManualReset
xor r8d,r8d                    ; R8  = Parm#3 = bInitialState
xor r9d,r9d                    ; R9  = Parm#4 = Name, 0=None
call [CreateEvent]             ; Call WinAPI function
test rax,rax                   ; Status=0 means error
jz .ThreadsError               ; Go if error creating event
mov [r12+rsi],rax              ; Store handle in the threads block
mov [rdi],rax                  ; Store handle in the handles list
add rdi,8                      ; Select next handle in the EVENTS HANDLES list
add esi,128                    ; Select next handle in the PER THREADS list
cmp esi,128*CONST_THREADS      ; Cycle limit = N handles, example = 16
jb .CreateEvents               ; Cycle for all handles
;--- Get current TSC --- Time Measurement Start -------------------------------;
xor eax,eax         ; Function 0 for CPUID
cpuid               ; This CPUID for events serialization only, results ignored
rdtsc               ; EDX:EAX = TSC value at start: EDX=High, EAX=Low
shl rdx,32          ; Positioning TSC high 32 bits to RDX.[63-32]
lea r14,[rax+rdx]   ; R14 = TSC value, integral 64 bits
;--- Create N threads, example = 16 ---
mov esi,8           ; RSI = Pointer to THREADS Entries at Threads Control Blocks
lea rdi,[Thread_00] ; RDI = Pointer to first of N threads subroutines entries
.CreateThreads:
xor ecx,ecx            ; RCX = Parm#1 = Security
xor edx,edx            ; RDX = Parm#2 = Stack size, 0=default
mov r8,rdi             ; R8  = Parm#3 = Execute routine pointer, threads 1-16
xor r9d,r9d            ; R9  = Parm#4 = Subroutine parameter, 0=None
xor eax,eax
push rax                       ; Parm#6 = Thread ID return , 0=None
push rax                       ; Parm#5 = Creating Flags
sub rsp,32      ; Make parameters shadow, this required because > 4 parameters
call [CreateThread]            ; Call WinAPI function
add rsp,32+16                  ; Remove parameters shadow + 2 parameters
test rax,rax                   ; Status=0 means error
jz .ThreadsError               ; Go if error creating event
mov [r12+rsi],rax              ; Store handle
add rdi,4                      ; 4 bytes per entry point: MOV AL,xx / JMP SHORT
add esi,128                    ; Select next handle
cmp esi,128*CONST_THREADS      ; Cycle limit = N handles, example = 16
jb .CreateThreads              ; Cycle for all handles
;--- Wait for N threads executed, example = 16 ---
mov ecx,CONST_THREADS          ; RCX = Parm#1 = Number of objects
lea rdx,[MtEventsHandles]      ; RDX = Parm#2 = Pointer to events list
mov r8d,1                      ; R8  = Parm#3 = Wait All flag
mov r9d,60000                  ; R9  = Parm#4 = Timeout, milliseconds                
call [WaitForMultipleObjects]  ; Call WinAPI function
test rax,rax                   ; Status=0 means wait OK
jnz .ThreadsError              ; Go if error waiting event
;--- Get current TSC --- Time Measurement Stop --------------------------------;
rdtsc               ; EDX:EAX = TSC value at end: EDX=High, EAX=Low
shl rdx,32          ; Positioning TSC high 32 bits to RDX.[63-32]
or rax,rdx          ; RAX = TSC value, integral 64 bits
sub rax,r14         ; RAX = Delta TSC value
xchg r14,rax        ; R14 = Delta TSC value
xor eax,eax         ; Function 0 for CPUID
cpuid               ; This CPUID for events serialization only, results ignored
;--- Close 16 handles for N events and N threads, example = 16 ---
; Here valid R12 = Pointer to both lists
xor esi,esi                    ; RSI = Index in the both lists
.CloseHandles:
mov rcx,[r12+rsi+00]           ; RCX = Parm#1 = Load EVENT handle
call [CloseHandle]             ; Call WinAPI function
test rax,rax                   ; Status=0 means error
jz .ThreadsError               ; Go if error creating event
mov rcx,[r12+rsi+08]           ; RCX = Parm#1 = Load THREAD handle
call [CloseHandle]             ; Call WinAPI function
test rax,rax                   ; Status=0 means error
jz .ThreadsError               ; Go if error creating event
add esi,128                    ; Select next handle
cmp esi,128*CONST_THREADS      ; Cycle limit = N*2 handles, example = 16*2
jb .CloseHandles               ; Cycle for all handles
;--- Load RAX=Delta-TSC, restore stack and exit, include errors handling ---
xchg rax,r14                   ; RAX = Delta-TSC, here CF=0(NC) means no errors
@@:
mov rsp,rbp                    ; Here CF=0(NC) means no errors
ret                            ; Restore input RSP, RBP must be non-volatile
.ThreadsError:                 ; CF=1(C) means error
stc
jmp @b

;--- Threads subroutines ---
; Receive parameters in the threads block, 128 bytes per thread
; For built routines list must be 4 bytes per entry

Thread_00:
mov al,00
jmp @f
Thread_01:
mov al,01
jmp @f
Thread_02:
mov al,02
jmp @f
Thread_03:
mov al,03
jmp @f
Thread_04:
mov al,04
jmp @f
Thread_05:
mov al,05
jmp @f
Thread_06:
mov al,06
jmp @f
Thread_07:
mov al,07
jmp @f
Thread_08:
mov al,08
jmp @f
Thread_09:
mov al,09
jmp @f
Thread_10:
mov al,10
jmp @f
Thread_11:
mov al,11
jmp @f
Thread_12:
mov al,12
jmp @f
Thread_13:
mov al,13
jmp @f
Thread_14:
mov al,14
jmp @f
Thread_15:
mov al,15
@@:
;--- Save RSP, align stack, make parameters shadow
push rbx rsi rdi rbp r12 r13 r14 r15
mov r14,rsp                          ; R14 = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h           ; Align stack because WinApi
sub rsp,32                           ; Built parameters shadow, 4 qwords
;--- Set R14 = Base address of Thread Control Block ---
; Note R15 not valid here, use direct load address to R15
;- mov r15,ThreadsControl            ; R15 = Base address of blocks array
lea r15,[ThreadsControl]
movzx eax,al                         ; Initialize entire 64-bit RAX
shl eax,7                            ; Shift because 128 bytes per block
add r15,rax                          ; R15 = Address for block for this thread
;--- Set affinity mask ---
mov rcx,[r15+008]                    ; Parm#1 = RCX = Thread handle
mov rdx,[r15+016]                    ; Parm#2 = RDX = Affinity mask
test rdx,rdx
jz @f                 ; Skip set affinity mask if memory allocated without NUMA
;---
; Check Affinity mask = F ( Options )
; INT3 ; see RDX = Affinity mask
;---
call [SetThreadAffinityMask]
@@:
;--- Load input parameters ---
mov rsi,[r15+032]                  ; RSI = Block #1 base address
mov rdi,[r15+040]                  ; RDI = Block #2 base address
mov r8, [r15+048]                  ; R8  = Block #3 base address
mov rcx,[r15+056]                  ; RCX = Block size, units=instructions
mov rbp,[r15+064]                  ; RBP = Number of measurement repeats
;--- Call target routine ---
call qword [r15+024]               ; Call target subroutine by prepared address
;--- Store output parameters ---
; Also reserved for additions
;--- Store number of operations: FLOPs , this for Math. mode only --- 
;- PATCH - mov [r15+072],rax
;--- Store Logical CPU mask , this optional, note variable not pre-cleared ---
; or [r15+080],rdx
;--- Signaling thread terminated ---
mov rcx,[r15+000]                    ; RCX = Handle for this thread
call [SetEvent]                      ; Call WinAPI function
;--- Restore stack, restore registers and return ---
mov rsp,r14
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret

