;--- Start measurement session ----------------------------------------------;
; Analysing user settings and system configuration, build test scenario      ;
; Main actions is:                                                           ;
;  1) Build IPB = f ( UPB, SYSPARMS ).                                       ;
;  2) Build NUMA domains list or single domain entry if NUMA not supported.  ;
;  3) Swap NUMA domains list if non-optimal remote access mode selected.     ;
;  4) Allocate memory for NUMA nodes.                                        ;
;  5) Build execution threads list = f ( NUMA nodes list ).                  ;
;  6) TSC frequency measurement.                                             ;
;  7) Run execution threads.                                                 ;
; See complementar routines: sessionstop.inc , sessionprogress.inc.          ; 
;                                                                            ;
; INPUT:   None in the registers                                             ;
;          UPB = User Parms. Block = user options settings                   ;
;          SYSPARMS = SystemParameters = system configuration                ;
;                                                                            ;
; OUTPUT:  CF = Error flag, 0(NC) = No errors, 1(C) = Error detected.        ;
;          AL = Error code, valid if CF=1(C)                                 ;
;               00 = Memory allocation error                                 ;
;               01 = Memory release error                                    ;
;               02 = Time measurement error                                  ;
;               04 = Address arithmetics error                               ;
;               04-0FFh = Reserved                                           ;
;          IPB = Input Parms. Block = Benchmarks arguments data, as transit  ;
;          OPB = Output Parms. Block = Benchmark results data                ;
;          Threads List, NUMA nodes list                                     ;
;                                                                            ;
;          IPB = InputParms = result of this routine, benchmark scenario     ;
;          Threads List, NUMA nodes list                                     ;
;          OPB = OutputParms = partially modified by this subroutine         ;
;          (TSC measurements), prepared for SessionProgress subroutine.      ;
;                                                                            ;
; All CPU registers can be corrupted, this rule for complex modules:         ;
; directory threads_manager\:                                                ;
;   sessionstart.inc, sessionstop.inc, sessionprogress.inc.                  ;
; directory gui\:                                                            ;
;   drawsessionstart.inc, drawsessionstop.inc                                ;
;   but not drawsessionthread.inc because OS callback                        ;       
; Remember about PUSH-POP in the callers.                                    ;
;----------------------------------------------------------------------------;

SessionStart:

IF 0

; Build IPB ( Input Parameters Block ) for benchmarking, 
; IPB = f ( UPB, SYSPARMS ),  IPB content depends on:
; 1) UPB ( User Parameters Block ), user settings by options widgets.
; 2) SYSPARMS ( System Parameters ), platform configuration, detected when
;    NCRB application starts.

lea rsi,[UserParms]         ; RSI = Pointer to user parameters block (UPB)
lea rdi,[InputParms]        ; RDI = Pointer to input parameters block (IPB)  
lea rbx,[SystemParameters]  ; RBX = Pointer for sys. info detected at NCRB start 

; Setup benchmark method, 
; assembler routine = f ( user settings method, test object, system config. )
; test object = f ( user settings object )
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

; Setup asm. routine address, set address and memory dump region, 
; note here EAX valid after previous fragment
push rbx
cmp eax,LATENCY_MODE
jae .L3                   ; Go if measurement mode = LATENCY
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
shl ecx,3                              ; convert bytes to bits, means * 8
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
jne .L8    ; Skip divide if not a variant "HT supported but not used" 
cmp eax,1
je .L8     ; Skip divide if threads count = 1
shr eax,1  ; Threads count / 2  if Hyper-Threading supported but not used
.L8:
mov [rdi + IPB.UpdatedThreads],eax  ; Setup number of threads

; Setup Hyper-Threading mode, 
; plus, see above dependences on HT for threads count
mov eax,[rsi + UPB.OptionHT]
mov [rdi + IPB.UpdatedHT],eax       ; Copy Hyper-Threading option (UPB to IPB)

; Setup Processor Group mode, 
; plus, see above dependences on PG for threads count
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
lea rcx,[BenchmarksRepeats]  ; select measurements constants
; Note for "Run simple" mode, no differentiation adaptive/non-adaptive,
; see also gui\drawsessionstart.inc, gui\drawsessionthread.inc
; for adaptive modes support
cmp al,MEASURE_BRIEF
je .L10                      ; Go with constants for fast (non precision) mode 
cmp al,MEASURE_B_ADAPTIVE
je .L10                      ; Go with constants for fast (non precision) mode
add rcx,REPEAT_SERIES_BYTES  ; use constants for slow (precision) mode
.L10:
mov eax,[rdi + IPB.UpdatedTarget]
mov eax,[rcx + rax * REPEAT_ENTRY_BYTES]  ; note after this, bits RAX.63-32 = 0
cmp [rdi + IPB.UpdatedAsm],LATENCY_MODE
jb .L13
xor edx,edx
mov ecx,50     ; Important DIVISOR CONSTANT for latency mode
div ecx        ; If Latency mode, make measurement repeats / 50 ,
.L13:          ; note after this, bits RAX.63-32 = 0
mov [rdi + IPB.MeasureRepeats],rax  ; Setup number of measurement repeats
; Note this measurement repeats value can be updated in the adaptive modes,
; for drawings, at subroutine gui\drawsessionstart.inc,
; at this subroutine sessionstart.inc set default approximation only 

; Setup measurement scenario as block sizes sequence,
; for "Run simple" mode required one block size only 
; Detect L1 cache
mov eax,[rdi + IPB.UpdatedTarget]
xor edx,edx    ; Previous level for L1 is 0 , not exist
mov rcx,[rbx + SYSPARMS.CpuCache + CPUCACHE.CpuidL1Data]
cmp al,TARGET_L1
je .L12 
; Detect L2 cache
mov rdx,rcx    ; Previous level for L2 is L1
mov rcx,[rbx + SYSPARMS.CpuCache + CPUCACHE.CpuidL2Unified]
cmp al,TARGET_L2
je .L12 
; Detect L3 cache
mov rdx,rcx    ; Previous level for L3 is L2
mov rcx,[rbx + SYSPARMS.CpuCache + CPUCACHE.CpuidL3Unified]
cmp al,TARGET_L3
je .L12 
; Detect L4 cache
mov rdx,rcx    ; Previous level for L4 is L3
mov rcx,DEFAULT_DRAM_BLOCK
cmp al,TARGET_L4               ; Note required add detect L4 cache here
je .L12
; Detect DRAM
cmp al,TARGET_DRAM
je .L9
; Support Custom block
mov rax,[rsi + UPB.CustomBlockStart]  ; Note requir. check for wrong block size
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
; Block size arithmetic for cache
.L12:
shr rcx,1              ; ( Size/2 - 512 ) for "Run simple", but
cmp rcx,rdx            ; other logic required for "Run drawings"
ja .L16                ; Go skip correction if ( RCX = L(i)/2 ) > ( RDX = L(i-1) )
;- lea rcx,[rcx*2+rdx] ; Support for exclusive cache L(i) / L(i-1) sizes ratio
;- shr rcx,1
add rcx,rdx
;-
.L16:
sub rcx,512 
; Done arithmetic, store block size
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
mov eax,[rbx + SYSPARMS.SystemInfo + SYSTEM_INFO.dwPageSize]  ; RAX = Std. page
cmp [rdi + IPB.UpdatedLP],LP_USED
jb .L11
mov rax,[rbx + SYSPARMS.LargePageSize]    ; RAX = Large page size 
.L11:
mov [rdi + IPB.PageSize],rax    ; Setup used Standard or Large Page size, bytes

; Setup memory allocation parameters, align allocated size by page size
mov rax,[rdi + IPB.StartBlockSize]
shl rax,1             ; Memory size * 2 because source+destination for copy
mov rcx,[rdi + IPB.PageSize]
dec rcx               ; for example, make addend 0FFFh from 1000h
add rax,rcx           ; add this addend
not rcx               ; for example, make mask FFFFFFFFFFFFF000h from 0FFFh
and rax,rcx           ; AND-masking with this mask

; Check drawings mode, big block required for 
; service all sizes when draw Performance = F(Size)
cmp [rsi + UPB.RunContext],0
je .L14   ; Go skip multiplication if simple measurement mode
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

; IPB initialization as  IPB = f ( UPB, SYSPARMS )  done at this point,
; next steps is resources allocation. 

; Build NUMA nodes list, 
; this is FIRST step before memory allocation
; NUMA unaware mode by option and by system limitations,
; supported by transfer R8D = option value to subroutine BuildNumaNodesList 
lea rcx,[NumaNodesList]          ; RCX = Pointers to NUMA nodes list for build
mov edx,[rdi + IPB.UpdatedPG]    ; EDX = Processor Groups option
mov r8d,[rdi + IPB.UpdatedNUMA]  ; R8D = NUMA option
call BuildNumaNodesList          ; CF returned, build list of NUMA nodes
jnc .NumaValid                 ; Go if no errors, means EAX = NUMA nodes count
mov al,0                       ; otherwise AL = error status
jmp .Exit                      ; Go for error handling
.NumaValid:

; Update NUMA nodes count after rejection nodes with affinity mask = 0
mov [rdi + IPB.DomainsCount],eax

; Note verify required).
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
jne .L30                         ; Go if not a single NUMA domain mode
mov [rdi + IPB.DomainsCount],1
cmp [rdi + IPB.UpdatedThreads],1
jbe .L30                         ; Go if not a multi-thread mode
mov eax,[SystemParameters.SystemInfo.dwNumberOfProcessors]  ; Change required: Get this from system or from one domain ?
mov [rdi + IPB.UpdatedThreads],eax  ; Update number of threads = f ( NUMA mode )
.L30:

; Swap affinity masks for NUMA Non Optimal scenario, force remote access
cmp [rdi + IPB.UpdatedNUMA],NUMA_NON_OPTIMAL
jne .L31         ; Go if NUMA mode not a "force non-optimal"
lea rdx,[NumaNodesList + NUMACTRL.NodeAffinity]
mov r8,rdx
mov ecx,[rdi + IPB.DomainsCount]
jrcxz .L31       ; skip swap if domains count = 0
dec ecx          ; domains count - 1
jrcxz .L31       ; skip swap if domains count = 1
mov rax,[rdx]    ; read mask from first NUMA domain entry
.L32:
add rdx,NUMACTRL_SIZE
xchg rax,[rdx]  ; rotate masks at domains entries, example 1-2-3-4 to 4-1-2-3
loop .L32       ; cycle for swapped domains
mov [r8],rax    ; write mask to last NUMA domain entry 
.L31:

; Note possible bug if assymetric domains (processors per domain).

; Both NUMA-aware / unaware memory allocation 
; inside subroutine MemAllocationNuma, 
; this si SECOND step with memory allocation,
; after NUMA nodes list build and optionally swapped
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
; this is THIRD step after NUMA domains list build and its memory allocated
lea rcx,[ThreadsList]
lea rdx,[InputParms]
call BuildThreadsList    ; CF returned 
mov al,0                 ; AL = error status, valid if CF=1
jc .Exit                 ; Go if error

; Update visualized block#1, block#2 bases at IPB from Threads List first entry
lea rcx,[ThreadsList]
mov rax,[rcx + THCTRL.Base1]
mov [rdi + IPB.AllocatedBlock1],rax
mov rax,[rcx + THCTRL.Base2]
mov [rdi + IPB.AllocatedBlock2],rax

; Now IPB initialized, 
; NUMA domains listed, its memory allocated, execution threads listed
; Next steps is TSC frequency measurements and benchmarks calibration
; Note repeat TSC frequency measurement after NCRB application start,
; for better adaptation to run conditions.

; Re-measure TSC clock before start benchmarks
push rdi
lea rdi,[OutputParms.DeltaTSC]   ; RDI = Pointer to destination area
call MeasureTsc    ; CF returned
pop rdi
mov al,2           ; AL = error status, valid if CF=1
jc .Exit           ; Go if error

; Start execution threads before per-iterations calls 
lea rcx,[ThreadsList]
mov edx,[rdi + IPB.UpdatedThreads]
call ThreadsStart  ; CF returned
mov al,2           ; AL = error status, valid if CF=1 only

END IF

; Initialization done, next phase is per-iterations calls, CF actual here
.Exit:
ret

