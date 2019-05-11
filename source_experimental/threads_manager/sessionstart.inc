;--- Start measurement session ----------------------------------------------;
; Analysing user settings and system configuration, build test scenario      ;
;                                                                            ;
; INPUT:   None in the registers                                             ;
;          UPB = UserParms = user options settings                           ;
;          SYSPARMS = SystemParameters = system configuration                ;
;                                                                            ;
; OUTPUT:  CF = Error flag, 0(NC) = No errors, 1(C) = Error detected.        ;
;          AL = Error code, valid if CF=1(C)                                 ;
;               00 = Memory allocation error                                 ;
;               01 = Memory release error                                    ;
;               02 = Time measurement error                                  ;
;               04 = Address arithmetics error                               ;
;               04-0FFh = Reserved                                           ;
;          OPB = Benchmark results data                                      ;
;          Threads List, NUMA nodes list                                     ;
;                                                                            ;
;          IPB = InputParms = result of this routine, benchmark scenario     ;
;          Threads List, NUMA nodes list                                     ;                                                  
;                                                                            ;
; All CPU registers can be corrupted                                         ;
;----------------------------------------------------------------------------;

; TODO. 
; Regularize simple/brief/drawings calling conventions, what registers destroyed.
; TODO.
; Better special simplified branch for single-thread, for precision reasons ?

SessionStart:

lea rsi,[UserParms]
lea rdi,[InputParms]  
lea rbx,[SystemParameters] 

; Setup benchmark method, 
; assembler routine = f (test method, test object, system configuration)
mov eax,[rsi + UPB.OptionAsm]
mov edx,[rsi + UPB.OptionTarget]
cmp eax,ASM_LIMIT
ja .L0
cmp edx,TARGET_LIMIT
ja .L0
mov ecx,[rbx + SYSPARMS.NonTemporalRead]
inc ecx
imul ecx,ecx,AM_BYTE_COUNT
cmp edx,TARGET_DRAM
je .L2
xor ecx,ecx
.L2:
movzx eax,byte [AM_Selector + rax + rcx]
jmp .L1
.L0:
xor eax,eax
.L1:
mov [rdi + IPB.UpdatedAsm],eax       ; Select assembler method ID
; Setup tested object
mov [rdi + IPB.UpdatedTarget],edx    ; Select target object (Cache/DRAM/Custom)

; Setup asm. routine address, set address and memory dump region, here EAX valid
push rbx
cmp eax,LATENCY_MODE
jae .L3
push rax                  ; this branch for bandwidth patterns
call GetBandwidthPattern  
pop rax
call GetBandwidthDump
jmp .L4
.L3:                      ; this branch for latency patterns
sub eax,LATENCY_MODE      ; first latency method ID must be 0
push rax
call GetLatencyPattern
mov [rdi + IPB.WalkRoutine],rax  
pop rax
call GetLatencyDump
.L4:
mov [rdi + IPB.PatternRoutine],rbx     ; Pointer to performance pattern 
mov [rdi + IPB.DumpMethodStart],rax    ; Pointer to hex-dumped part of pattern
mov [rdi + IPB.DumpMethodLength],edx   ; Hex-dumped part length
shl ecx,3                              ; convert bytes to bits
mov [rdi + IPB.OperandWidth],ecx       ; Select instruction operand width
pop rbx

; Setup number of threads, use platform total number of threads if auto
mov eax,1
cmp [rsi + UPB.OptionParallel],PARALLEL_USED
jne .L5
mov rcx,[rbx + SYSPARMS.NumaNodesMasks]
mov edx,1
xor eax,eax
.L6:
test rcx,rdx
jz .L7
inc eax     ; EAX = processors at first NUMA domain
.L7:
shl rdx,1
jnz .L6
cmp [rsi + UPB.OptionNUMA],NUMA_CURRENT_ONLY
je .L5      ; Go if threads count limited by first NUMA domain
mov edx,[rbx + SYSPARMS.ProcessorsTotal]                                ; EDX = processors at all groups
cmp [rsi + UPB.OptionPG],PG_OPTIMAL
je .L5      ; Go if use all processors from all processor groups
mov eax,[rbx + SYSPARMS.SystemInfo + SYSTEM_INFO.dwNumberOfProcessors]  ; EAX = processors at current group
.L5:
cmp [rsi + UPB.OptionHT],HT_NOT_USED
jne .L8
cmp eax,1
je .L8
shr eax,1  ; Threads count / 2 if Hyper-Threading supported but not used
.L8:
mov [rdi + IPB.UpdatedThreads],eax  ; Setup number of threads

; Setup Hyper-Threading mode, plus, see above dependences on HT for threads count
mov eax,[rsi + UPB.OptionHT]
mov [rdi + IPB.UpdatedHT],eax       ; Copy Hyper-Threading option (UPB to IPB)

; Setup Processor Group mode, plus, see above dependences on PG for threads count
mov eax,[rsi + UPB.OptionPG]
mov [rdi + IPB.UpdatedPG],eax       ; Copy Processor Groups option 

; Setup NUMA mode, plus, see above dependences on NUMA for threads count
mov eax,[rsi + UPB.OptionNUMA]
mov [rdi + IPB.UpdatedNUMA],eax     ; Copy NUMA option

; Setup Large Page mode
mov eax,[rsi + UPB.OptionLP]
mov [rdi + IPB.UpdatedLP],eax       ; Copy Large Pages option

; Setup measurement precision ( calibration repeats ) mode
mov eax,[rsi + UPB.OptionMeasure]
mov [rdi + IPB.UpdatedMeasure],eax
lea rcx,[BenchmarksRepeats]
cmp al,MEASURE_BRIEF
je .L10
cmp al,MEASURE_B_ADAPTIVE
je .L10
add rcx,REPEAT_SERIES_BYTES
.L10:
mov eax,[rdi + IPB.UpdatedTarget]
mov eax,[rcx + rax * REPEAT_ENTRY_BYTES]  ; note after this, bits RAX.63-32 = 0
cmp [rdi + IPB.UpdatedAsm],LATENCY_MODE
jb .L13
xor edx,edx
mov ecx,50
div ecx        ; If Latency mode, make measurement repeats / 50 ,
.L13:          ; note after this, bits RAX.63-32 = 0
mov [rdi + IPB.MeasureRepeats],rax  ; Setup number of measurement repeats

; Setup measurement scenario as block sizes sequence,
; for "Run simple" mode required one block size only 
mov eax,[rdi + IPB.UpdatedTarget]
mov rcx,[rbx + SYSPARMS.CpuCache + CPUCACHE.CpuidL1Data]
cmp al,TARGET_L1
je .L12 
mov rcx,[rbx + SYSPARMS.CpuCache + CPUCACHE.CpuidL2Unified]
cmp al,TARGET_L2
je .L12 
mov rcx,[rbx + SYSPARMS.CpuCache + CPUCACHE.CpuidL3Unified]
cmp al,TARGET_L3
je .L12 
mov rcx,DEFAULT_DRAM_BLOCK
cmp al,TARGET_L4                 ; TODO. Required detect L4 cache
je .L12
cmp al,TARGET_DRAM
je .L9
; Support Custom block
mov rax,[rsi + UPB.CustomBlockStart]  ; TODO. Check for wrong block size
; Restrictions for custom block size
mov ecx,CUSTOM_BLOCK_ALIGN
xor edx,edx
div rcx
mul rcx
cmp rax,CUSTOM_BLOCK_MIN
jae @f
mov rax,CUSTOM_BLOCK_MIN
@@:
cmp rax,CUSTOM_BLOCK_MAX
jbe @f
mov rax,CUSTOM_BLOCK_MAX
@@:
; Restrictions done, set result at RCX
xchg rcx,rax
jmp .L9
.L12:
shr rcx,1          ; ( Size/2 - 512 ) for "Run simple", other logic required for "Run drawings" 
sub rcx,512
.L9:
mov [rdi + IPB.StartBlockSize],rcx  ; Setup Block Size

; Setup approximation option, yet value copy only
mov eax,[rsi + UPB.OptionApprox]
mov [rdi + IPB.UpdatedApprox],eax   ; Copy X-approximation option (UPB to IPB)

; Setup Processor Groups parameters
mov eax,[rbx + SYSPARMS.ProcessorGroups]
mov [rdi + IPB.GroupsCount],eax     ; Setup number of Processor Groups

; Setup NUMA domains parameters
mov eax,[rbx + SYSPARMS.NumaNodes]
mov [rdi + IPB.DomainsCount],eax    ; Setup number of NUMA domains

; Setup page size, select default (typical 4KB) or large (typical 2M for x64)
mov eax,[rbx + SYSPARMS.SystemInfo + SYSTEM_INFO.dwPageSize]
cmp [rdi + IPB.UpdatedLP],LP_USED
jb .L11
mov rax,[rbx + SYSPARMS.LargePageSize]
.L11:
mov [rdi + IPB.PageSize],rax        ; Setup Large Page size, bytes

; Setup memory allocation parameters, align allocated size by page size
mov rax,[rdi + IPB.StartBlockSize]
shl rax,1                 ; Memory size * 2 because source+destination for copy
mov rcx,[rdi + IPB.PageSize]
dec rcx
add rax,rcx
not rcx
and rax,rcx

; Check drawings mode, big block required for 
; service all sizes when draw Performance = F(Size)
cmp [rsi + UPB.RunContext],0
je .L14
; This required for memory reservation per drawings session
bsr rcx,rax
mov edx,1
shl rdx,cl
cmp rax,rdx
jbe .L15
lea rax,[rdx*2]  ; Make integer power of 2
.L15:
; X*5 for memory reservation per X axis values
imul rax,rax,5   ; Xmax = Xtarget * 5
.L14:

; Store memory size per one thread and total
mov [rdi + IPB.MemoryPerThread],rax   ; Setup memory size per one thread
mov ecx,[rdi + IPB.UpdatedThreads]
mul rcx                               ; multiply by number of threads
mov [rdi + IPB.MemoryTotal],rax       ; Setup memory size per all threads

; Build NUMA nodes list, 
; first step before memory allocation
; NUMA unaware mode by option and by system limitations,
; supported by transfer R8D to subroutine BuildNumaNodesList 
lea rcx,[NumaNodesList]
mov edx,[rdi + IPB.UpdatedPG]
mov r8d,[rdi + IPB.UpdatedNUMA]
call BuildNumaNodesList  ; CF returned, build list of NUMA nodes
mov al,0                 ; AL = error status
jc .Exit                 ; Go if error

; TODO (verify required).
; Support NUMA options by update IPB and NUMA nodes list

; Option = NUMA_NOT_SUPPORTED, NUMA_NO_CONTROL
;   NUMA unaware = not change prepared list 
; Option = NUMA_CURRENT_ONLY
;   NUMA single domain = reduce node count to 1, reduce CPU count = per node 0
; Option = NUMA_OPTIMAL 
;   NUMA optimal = not change prepared list
; Option = NUMA_NON_OPTIMAL 
;   NUMA not optimal = shift affinity masks by ring, example 1-2-3-4 to 4-1-2-3

; Reduce domains count for NUMA single domain scenario
cmp [rdi + IPB.UpdatedNUMA],NUMA_CURRENT_ONLY
jne .L30
mov [rdi + IPB.DomainsCount],1
cmp [rdi + IPB.UpdatedThreads],1
jbe .L30
mov eax,[SystemParameters.SystemInfo.dwNumberOfProcessors]  ; TODO. Get this from system or from one domain ?
mov [rdi + IPB.UpdatedThreads],eax  ; Update number of threads = f ( NUMA mode )
.L30:

; Swap affinity masks for NUMA Non Optimal scenario
cmp [rdi + IPB.UpdatedNUMA],NUMA_NON_OPTIMAL
jne .L31
lea rdx,[NumaNodesList + NUMACTRL.NodeAffinity]
mov r8,rdx
mov ecx,[rdi + IPB.DomainsCount]
jrcxz .L31    ; skip if domains count = 0
dec ecx
jrcxz .L31    ; skip if domains count = 1
mov rax,[rdx]
.L32:
add rdx,NUMACTRL_SIZE
xchg rax,[rdx]  ; Rotate masks at domains entries, example 1-2-3-4 to 4-1-2-3
loop .L32
mov [r8],rax
.L31:

; TODO.
; Possible bug if assymetric domains (processors per domain).

; Both NUMA-aware / unaware memory allocation 
; inside subroutine MemAllocationNuma, 
; second step with memory allocation after nodes list build
mov rax,[rdi + IPB.MemoryTotal]
mov r8d,[rdi + IPB.DomainsCount]  ; R8 = Number of nodes
xor edx,edx
div r8
lea rcx,[NumaNodesList]           ; RCX = Base address of NUMA nodes list
mov rdx,rax                       ; RDX = Memory allocation size per one node
mov r9d,[rdi + IPB.UpdatedLP]     ; R9D = Large Pages option
call MemAllocationNuma   ; CF returned
mov al,0                 ; AL = error status
jc .Exit                 ; Go if error

; Build threads list
lea rcx,[ThreadsList]
lea rdx,[InputParms]
call BuildThreadsList    ; CF returned 
mov al,0                 ; AL = error status
jc .Exit                 ; Go if error

; TODO. Add "Threads allocation error" yet supported as "Memory allocation error"

; Update visualized block#1, block#2 bases at IPB from Threads List first entry
lea rcx,[ThreadsList]
mov rax,[rcx + THCTRL.Base1]
mov [rdi + IPB.AllocatedBlock1],rax
mov rax,[rcx + THCTRL.Base2]
mov [rdi + IPB.AllocatedBlock2],rax

; Re-measure TSC clock before start benchmarks
push rdi
lea rdi,[OutputParms.DeltaTSC]
call MeasureTsc    ; CF returned
pop rdi
mov al,2           ; AL = error status
jc .Exit           ; Go if error

; Start threads before per-iterations calls 
lea rcx,[ThreadsList]
mov edx,[rdi + IPB.UpdatedThreads]
call ThreadsStart  ; CF returned
mov al,2           ; AL = error status, valid if CF=1 only

; TODO. Add "Threads run error", yet supported as "Timings measurement error"

; Initialization done, next phase is per-iterations calls, CF actual here
.Exit:
ret

