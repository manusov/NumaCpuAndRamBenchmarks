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

; Build IPB ( Input Parameters Block ) for benchmarking, 
; IPB = f ( UPB, SYSPARMS ),  IPB content depends on:
; 1) UPB ( User Parameters Block ), user settings by options widgets.
; 2) SYSPARMS ( System Parameters ), platform configuration, detected when
;    NCRB application starts.

lea esi,[UserParms]         ; ESI = Pointer to user parameters block (UPB)
lea edi,[InputParms]        ; EDI = Pointer to input parameters block (IPB)  
lea ebx,[SystemParameters]  ; EBX = Pointer for sys. info detected at NCRB start 

; Setup benchmark method, 
; assembler routine = f ( user settings method, test object, system config. )
; test object = f ( user settings object )
mov eax,[esi + UPB.OptionAsm]
mov edx,[esi + UPB.OptionTarget]
cmp eax,ASM_LIMIT
ja .L0
cmp edx,TARGET_LIMIT
ja .L0
mov ecx,[ebx + SYSPARMS.NonTemporalRead]
inc ecx
imul ecx,ecx,AM_BYTE_COUNT
cmp edx,TARGET_DRAM
je .L2
xor ecx,ecx
.L2:
movzx eax,byte [AM_Selector + eax + ecx]
jmp .L1
.L0:
xor eax,eax
.L1:
mov [edi + IPB.UpdatedAsm],eax       ; Select assembler method ID
; Setup tested object
mov [edi + IPB.UpdatedTarget],edx    ; Select target object (Cache/DRAM/Custom)

; Setup asm. routine address, set address and memory dump region, 
; note here EAX valid after previous fragment
push ebx
cmp eax,LATENCY_MODE
jae .L3                   ; Go if measurement mode = LATENCY
push eax                  ; this branch for bandwidth patterns
call GetBandwidthPattern  
pop eax
call GetBandwidthDump
jmp .L4
.L3:                      ; this branch for latency patterns
sub eax,LATENCY_MODE      ; first latency method ID must be 0
push eax
call GetLatencyPattern
mov [edi + IPB.WalkRoutine],eax  
pop eax
call GetLatencyDump
.L4:
mov [edi + IPB.PatternRoutine],ebx     ; Pointer to performance pattern 
mov [edi + IPB.DumpMethodStart],eax    ; Pointer to hex-dumped part of pattern
mov [edi + IPB.DumpMethodLength],edx   ; Hex-dumped part length
shl ecx,3                              ; convert bytes to bits, means * 8
mov [edi + IPB.OperandWidth],ecx       ; Select instruction operand width
pop ebx

; Setup number of threads, use platform total number of threads if auto
mov eax,1
cmp [esi + UPB.OptionParallel],PARALLEL_USED
jne .L5
mov ecx,[ebx + SYSPARMS.NumaNodesMasks]
mov edx,1
xor eax,eax
.L6:
test ecx,edx
jz .L7
inc eax     ; EAX = processors at first NUMA domain
.L7:
shl edx,1
jnz .L6
cmp [esi + UPB.OptionNUMA],NUMA_CURRENT_ONLY
je .L5      ; Go if threads count limited by first NUMA domain
mov edx,[ebx + SYSPARMS.ProcessorsTotal]                                ; EDX = processors at all groups
cmp [esi + UPB.OptionPG],PG_OPTIMAL
je .L5      ; Go if use all processors from all processor groups
mov eax,[ebx + SYSPARMS.SystemInfo + SYSTEM_INFO.dwNumberOfProcessors]  ; EAX = processors at current group
.L5:
cmp [esi + UPB.OptionHT],HT_NOT_USED
jne .L8    ; Skip divide if not a variant "HT supported but not used" 
cmp eax,1
je .L8     ; Skip divide if threads count = 1
shr eax,1  ; Threads count / 2  if Hyper-Threading supported but not used
.L8:
mov [edi + IPB.UpdatedThreads],eax  ; Setup number of threads

; Setup Hyper-Threading mode, 
; plus, see above dependences on HT for threads count
mov eax,[esi + UPB.OptionHT]
mov [edi + IPB.UpdatedHT],eax       ; Copy Hyper-Threading option (UPB to IPB)

; Setup Processor Group mode, 
; plus, see above dependences on PG for threads count
mov eax,[esi + UPB.OptionPG]
mov [edi + IPB.UpdatedPG],eax       ; Copy Processor Groups option 

; Setup NUMA mode, plus, see above dependences on NUMA for threads count
mov eax,[esi + UPB.OptionNUMA]
mov [edi + IPB.UpdatedNUMA],eax     ; Copy NUMA option

; Setup Large Page mode
mov eax,[esi + UPB.OptionLP]
mov [edi + IPB.UpdatedLP],eax       ; Copy Large Pages option

; Setup measurement precision ( calibration repeats ) mode
mov eax,[esi + UPB.OptionMeasure]
mov [edi + IPB.UpdatedMeasure],eax
lea ecx,[BenchmarksRepeats]  ; select measurements constants
; Note for "Run simple" mode, no differentiation adaptive/non-adaptive,
; see also gui\drawsessionstart.inc, gui\drawsessionthread.inc
; for adaptive modes support
cmp al,MEASURE_BRIEF
je .L10                      ; Go with constants for fast (non precision) mode 
cmp al,MEASURE_B_ADAPTIVE
je .L10                      ; Go with constants for fast (non precision) mode
add ecx,REPEAT_SERIES_BYTES  ; use constants for slow (precision) mode
.L10:
mov eax,[edi + IPB.UpdatedTarget]
mov eax,[ecx + eax * REPEAT_ENTRY_BYTES]
cmp [edi + IPB.UpdatedAsm],LATENCY_MODE
jb .L13
xor edx,edx
mov ecx,50     ; Important DIVISOR CONSTANT for latency mode
div ecx        ; If Latency mode, make measurement repeats / 50 ,
.L13:          ; note after this, bits RAX.63-32 = 0
mov dword [edi + IPB.MeasureRepeats + 0],eax  ; Setup number of measurement repeats
mov dword [edi + IPB.MeasureRepeats + 4],0
; Note this measurement repeats value can be updated in the adaptive modes,
; for drawings, at subroutine gui\drawsessionstart.inc,
; at this subroutine sessionstart.inc set default approximation only 

; Setup measurement scenario as block sizes sequence,
; for "Run simple" mode required one block size only 
; Detect L1 cache
mov eax,[edi + IPB.UpdatedTarget]
xor edx,edx    ; Previous level for L1 is 0 , not exist
mov ecx,dword [ebx + SYSPARMS.CpuCache + CPUCACHE.CpuidL1Data]
cmp al,TARGET_L1
je .L12 
; Detect L2 cache
mov edx,ecx    ; Previous level for L2 is L1
mov ecx,dword [ebx + SYSPARMS.CpuCache + CPUCACHE.CpuidL2Unified]
cmp al,TARGET_L2
je .L12 
; Detect L3 cache
mov edx,ecx    ; Previous level for L3 is L2
mov ecx,dword [ebx + SYSPARMS.CpuCache + CPUCACHE.CpuidL3Unified]
cmp al,TARGET_L3
je .L12 
; Detect L4 cache
mov edx,ecx    ; Previous level for L4 is L3
mov ecx,DEFAULT_DRAM_BLOCK
cmp al,TARGET_L4               ; Note required add detect L4 cache here
je .L12
; Detect DRAM
cmp al,TARGET_DRAM
je .L9
; Support Custom block
mov eax,[esi + UPB.CustomBlockStart]  ; Note requir. check for wrong block size
; Restrictions for custom block size
mov ecx,CUSTOM_BLOCK_ALIGN
xor edx,edx
div ecx
mul ecx
cmp eax,CUSTOM_BLOCK_MIN
jae @f
mov eax,CUSTOM_BLOCK_MIN
@@:
cmp eax,CUSTOM_BLOCK_MAX
jbe @f
mov eax,CUSTOM_BLOCK_MAX
@@:
; Restrictions done, set result at RCX
xchg ecx,eax
jmp .L9
; Block size arithmetic for cache
.L12:
shr ecx,1              ; ( Size/2 - 512 ) for "Run simple", but
cmp ecx,edx            ; other logic required for "Run drawings"
ja .L16                ; Go skip correction if ( ECX = L(i)/2 ) > ( EDX = L(i-1) )
;- lea rcx,[rcx*2+rdx] ; Support for exclusive cache L(i) / L(i-1) sizes ratio
;- shr rcx,1
add ecx,edx
;-
.L16:
sub ecx,512 
; Done arithmetic, store block size
.L9:
mov [edi + IPB.StartBlockSize],ecx  ; Setup Block Size

; Setup approximation option, yet value copy only
mov eax,[esi + UPB.OptionApprox]
mov [edi + IPB.UpdatedApprox],eax   ; Copy X-approximation option (UPB to IPB)

; Setup Processor Groups parameters
mov eax,[ebx + SYSPARMS.ProcessorGroups]
mov [edi + IPB.GroupsCount],eax     ; Setup number of Processor Groups

; Setup NUMA domains parameters
mov eax,[ebx + SYSPARMS.NumaNodes]
mov [edi + IPB.DomainsCount],eax    ; Setup number of NUMA domains

; Setup page size, select default (typical 4KB) or large (typical 2M for x64)
mov eax,[ebx + SYSPARMS.SystemInfo + SYSTEM_INFO.dwPageSize]  ; RAX = Std. page
cmp [edi + IPB.UpdatedLP],LP_USED
jb .L11
mov eax,[ebx + SYSPARMS.LargePageSize]    ; RAX = Large page size 
.L11:
mov [edi + IPB.PageSize],eax    ; Setup used Standard or Large Page size, bytes

; Setup memory allocation parameters, align allocated size by page size
mov eax,[edi + IPB.StartBlockSize]
shl eax,1             ; Memory size * 2 because source+destination for copy
mov ecx,[edi + IPB.PageSize]
dec ecx               ; for example, make addend 0FFFh from 1000h
add eax,ecx           ; add this addend
not ecx               ; for example, make mask FFFFF000h from 0FFFh
and eax,ecx           ; AND-masking with this mask

; Check drawings mode, big block required for 
; service all sizes when draw Performance = F(Size)
cmp [esi + UPB.RunContext],0
je .L14   ; Go skip multiplication if simple measurement mode
; This required for memory reservation per drawings session
bsr ecx,eax
mov edx,1
shl edx,cl
cmp eax,edx
jbe .L15
lea eax,[edx*2]  ; Make integer power of 2
.L15:
; X*5 for memory reservation per X axis values
imul eax,eax,5   ; Xmax = Xtarget * 5
.L14:

; Store memory size per one thread and total
mov [edi + IPB.MemoryPerThread],eax   ; Setup memory size per one thread
mov ecx,[edi + IPB.UpdatedThreads]
mul ecx                               ; multiply by number of threads
mov [edi + IPB.MemoryTotal],eax       ; Setup memory size per all threads

; IPB initialization as  IPB = f ( UPB, SYSPARMS )  done at this point,
; next steps is resources allocation. 

; Build NUMA nodes list, 
; this is FIRST step before memory allocation
; NUMA unaware mode by option and by system limitations,
; supported by transfer R8D = option value to subroutine BuildNumaNodesList 
lea ecx,[NumaNodesList]          ; ECX = Pointers to NUMA nodes list for build
mov edx,[edi + IPB.UpdatedPG]    ; EDX = Processor Groups option
mov eax,[edi + IPB.UpdatedNUMA]  ; EAX = NUMA option
call BuildNumaNodesList          ; CF returned, build list of NUMA nodes
jnc .NumaValid                   ; Go if no errors, means EAX = NUMA nodes count
mov al,0                         ; otherwise AL = error status
jmp .Exit                        ; Go for error handling
.NumaValid:

; Update NUMA nodes count after rejection nodes with affinity mask = 0
mov [edi + IPB.DomainsCount],eax

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
cmp [edi + IPB.UpdatedNUMA],NUMA_CURRENT_ONLY
jne .L30                         ; Go if not a single NUMA domain mode
mov [edi + IPB.DomainsCount],1
cmp [edi + IPB.UpdatedThreads],1
jbe .L30                         ; Go if not a multi-thread mode
mov eax,[SystemParameters.SystemInfo.dwNumberOfProcessors]  ; Change required: Get this from system or from one domain ?
mov [edi + IPB.UpdatedThreads],eax  ; Update number of threads = f ( NUMA mode )
.L30:

; Swap affinity masks for NUMA Non Optimal scenario, force remote access
cmp [edi + IPB.UpdatedNUMA],NUMA_NON_OPTIMAL
jne .L31         ; Go if NUMA mode not a "force non-optimal"
lea edx,[NumaNodesList + NUMACTRL.NodeAffinity]
mov esi,edx
mov ecx,[edi + IPB.DomainsCount]
jecxz .L31       ; skip swap if domains count = 0
dec ecx          ; domains count - 1
jecxz .L31       ; skip swap if domains count = 1
mov eax,[edx]    ; read mask from first NUMA domain entry
.L32:
add edx,NUMACTRL_SIZE
xchg eax,[edx]  ; rotate masks at domains entries, example 1-2-3-4 to 4-1-2-3
loop .L32       ; cycle for swapped domains
mov [esi],eax   ; write mask to last NUMA domain entry 
.L31:

; Note possible bug if assymetric domains (processors per domain).
; Note ESI destroyed by previous step, but not used at next steps.

; Both NUMA-aware / unaware memory allocation 
; inside subroutine MemAllocationNuma, 
; this si SECOND step with memory allocation,
; after NUMA nodes list build and optionally swapped
mov eax,[edi + IPB.MemoryTotal]   ; EAX = Total memory allocated 
mov ecx,[edi + IPB.DomainsCount]  ; ECX = Number of NUMA nodes
xor edx,edx
div ecx
lea ebx,[NumaNodesList]           ; EBX = Base address of NUMA nodes list
mov edx,ecx                       ; EDX = Number of NUMA nodes 
xchg ecx,eax                      ; ECX = Memory allocation size per one node
mov eax,[edi + IPB.UpdatedLP]     ; EAX = Large Pages option
call MemAllocationNuma   ; CF returned
mov al,0                 ; AL = error status
jc .Exit                 ; Go if error

; Note EBX destroyed by previous step, but not used at next steps.

; Build threads list
; this is THIRD step after NUMA domains list build and its memory allocated
lea ecx,[ThreadsList]
lea edx,[InputParms]
call BuildThreadsList    ; CF returned 
mov al,0                 ; AL = error status, valid if CF=1
jc .Exit                 ; Go if error

; Update visualized block#1, block#2 bases at IPB from Threads List first entry
lea ecx,[ThreadsList]
mov eax,[ecx + THCTRL.Base1]
mov [edi + IPB.AllocatedBlock1],eax
mov eax,[ecx + THCTRL.Base2]
mov [edi + IPB.AllocatedBlock2],eax

; Now IPB initialized, 
; NUMA domains listed, its memory allocated, execution threads listed
; Next steps is TSC frequency measurements and benchmarks calibration
; Note repeat TSC frequency measurement after NCRB application start,
; for better adaptation to run conditions.

; Re-measure TSC clock before start benchmarks
push edi
lea edi,[OutputParms.DeltaTSC]   ; EDI = Pointer to destination area
call MeasureTsc    ; CF returned
pop edi
mov al,2           ; AL = error status, valid if CF=1
jc .Exit           ; Go if error

; Start execution threads before per-iterations calls 
lea ecx,[ThreadsList]
mov edx,[edi + IPB.UpdatedThreads]
call ThreadsStart  ; CF returned
mov al,2           ; AL = error status, valid if CF=1 only

; Initialization done, next phase is per-iterations calls, CF actual here
.Exit:
ret

