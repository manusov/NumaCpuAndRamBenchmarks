
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
mov [rdi + IPB.UpdatedAsm],eax

; Setup tested object
mov [rdi + IPB.UpdatedTarget],edx

; Setup asm. routine address, set address and memory dump region, here EAX valid
push rbx
cmp eax,LATENCY_MODE
jae .L3
push rax
call GetBandwidthPattern  
pop rax
call GetBandwidthDump
jmp .L4
.L3:
push rax
call GetLatencyPattern  
pop rax
call GetLatencyDump
.L4:
mov [rdi + IPB.PatternRoutine],rbx
mov [rdi + IPB.DumpMethodStart],rax
mov [rdi + IPB.DumpMethodLength],edx
shl ecx,3     ; convert bytes to bits
mov [rdi + IPB.OperandWidth],ecx
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
mov [rdi + IPB.UpdatedThreads],eax

; Setup Hyper-Threading mode, plus, see above dependences on HT for threads count
mov eax,[rsi + UPB.OptionHT]
mov [rdi + IPB.UpdatedHT],eax

; Setup Processor Group mode, plus, see above dependences on PG for threads count
mov eax,[rsi + UPB.OptionPG]
mov [rdi + IPB.UpdatedPG],eax

; Setup NUMA mode, plus, see above dependences on NUMA for threads count
mov eax,[rsi + UPB.OptionNUMA]
mov [rdi + IPB.UpdatedNUMA],eax

; Setup Large Page mode
mov eax,[rsi + UPB.OptionLP]
mov [rdi + IPB.UpdatedLP],eax

; Setup measurement precision ( calibration repeats ) mode
mov eax,[rsi + UPB.OptionMeasure]
mov [rdi + IPB.UpdatedMeasure],eax
lea rcx,[BenchmarksRepeats]
cmp al,MEASURE_BRIEF
je .L10
cmp al,MEASURE_B_ADAPTIVE
je .L10
add rcx,REPEAT_SERIES_BYTES
mov eax,[rdi + IPB.UpdatedTarget]
mov eax,[rcx + rax * REPEAT_ENTRY_BYTES]
.L10:
mov [rdi + IPB.MeasureRepeats],eax

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
mov rcx,[rsi + UPB.CustomBlockStart]  ; TODO. Check for wrong block size
jmp .L9
.L12:
shr rcx,1          ; TODO. ( Size/2 - 512 ) for "Run simple", other logic required for "Run drawings" 
sub rcx,512
.L9:
mov [rdi + IPB.StartBlockSize],rcx
; TODO. mov [rdi + IPB.EndBlockSize],rcx
; TODO. mov [rdi + IPB.DeltaBlockSize],rax

; Setup Processor Groups parameters
mov eax,[rbx + SYSPARMS.ProcessorGroups]
mov [rdi + IPB.GroupsCount],eax

; Setup NUMA domains parameters
mov eax,[rbx + SYSPARMS.NumaNodes]
mov [rdi + IPB.DomainsCount],eax

; Setup page size, select default (typical 4KB) or large (typical 2M for x64)
mov eax,[rbx + SYSPARMS.SystemInfo + SYSTEM_INFO.dwPageSize]
cmp [rdi + IPB.UpdatedLP],LP_USED
jb .L11
mov rax,[rbx + SYSPARMS.LargePageSize]
.L11:
mov [rdi + IPB.PageSize],rax

; Setup memory allocation parameters, align allocated size by page size
mov rax,[rdi + IPB.StartBlockSize]
shl rax,1                 ; Memory size * 2 because source+destination for copy
mov rcx,[rdi + IPB.PageSize]
dec rcx
add rax,rcx
not rcx
and rax,rcx
mov [rdi + IPB.MemoryPerThread],rax
mov ecx,[rdi + IPB.UpdatedThreads]
mul rcx
mov [rdi + IPB.MemoryTotal],rax

; Build NUMA nodes list, 
; first step before memory allocation
; TODO. Check status required after call ( CF flag )
lea rcx,[NumaNodesList]
mov edx,[rdi + IPB.UpdatedPG]
call BuildNumaNodesList

; NUMA-aware memory allocation, 
; second step with memory allocation after nodes list build
; TODO. NUMA-unaware memory allocation branch add required
; TODO. Check status required after call ( CF flag )
mov rax,[rdi + IPB.MemoryTotal]
mov r8d,[rdi + IPB.DomainsCount]  ; R8 = Number of nodes
xor edx,edx
div r8
lea rcx,[NumaNodesList]           ; RCX = Base address of NUMA nodes list
mov rdx,rax                       ; RDX = Memory allocation size per one node
mov r9d,[rdi + IPB.UpdatedLP]     ; R9D = Large Pages option
call MemAllocationNuma

; Build threads list
; TODO. Check status required after call ( CF flag )
lea rcx,[ThreadsList]
lea rdx,[InputParms]
call BuildThreadsList 

; Update visualized block#1, block#2 bases at IPB from Threads List first entry
lea rcx,[ThreadsList]
mov rax,[rcx + THCTRL.Base1]
mov [rdi + IPB.AllocatedBlock1],rax
mov rax,[rcx + THCTRL.Base2]
mov [rdi + IPB.AllocatedBlock2],rax

; Start threads before per-iterations calls 
; TODO. Check status required after call ( CF flag )
lea rcx,[ThreadsList]
mov edx,[rdi + IPB.UpdatedThreads]
call ThreadsStart

; Re-measure TSC clock before start benchmarks
lea rdi,[OutputParms.DeltaTSC]
call MeasureTsc

; Initialization done, next phase is per-iterations calls
ret
