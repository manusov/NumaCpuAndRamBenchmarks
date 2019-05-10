;--- Update GUI visualized parameters and benchmarks options availability -----;
;    (gray or black) by system information results                             ; 
;                                                                              ;
; INPUT:   None at registers                                                   ;
;          System Parameters Block = results of system information             ; 
;                                                                              ;
; OUTPUT:  None at registers                                                   ;
;          GUI control structures                                              ;  
;                                                                              ;
; All CPU registers can be corrupted                                           ;
;------------------------------------------------------------------------------;

; TODO.
; Required input-output parameters detalization.

SysParmsToGui:

; Options, Method Selector = F (CPU features)
lea rbx,[METHODS_BASE]          ; RBX local label for compact offsets
lea rdi,[SystemParameters]
mov eax,[rdi + SYSPARMS.CpuSpecFlags]  ; EAX = Flags = F(CPU Features)
mov edx,WS_DISABLED             ; EDX = Operand
test ax,100000000b
jnz @f
or [rbx+MMX64_METHOD_1],edx     ; Disable SSE128 tests if not supported
or [rbx+MMX64_METHOD_2],edx
or [rbx+MMX64_METHOD_3],edx
@@:
test al,00000001b
jnz @f                          ; Skip disable operation if SSE128 supported
or [rbx+SSE128_METHOD_1],edx    ; Disable SSE128 tests if not supported
or [rbx+SSE128_METHOD_2],edx
or [rbx+SSE128_METHOD_3],edx
@@:
test al,00000010b
jnz @f                          ; Skip disable operation if AVX256 supported
or [rbx+AVX256_METHOD_1],edx    ; Disable AVX256 tests if not supported
or [rbx+AVX256_METHOD_2],edx
or [rbx+AVX256_METHOD_3],edx
@@:
test al,00000100b
jnz @f                          ; Skip disable operation if FMA256 supported
or [rbx+FMA256_METHOD_1],edx    ; Disable FMA256/512 tests if not supported
or [rbx+FMA512_METHOD_1],edx
@@:
test al,00010000b
jnz @f                          ; Skip disable operation if AVX512 supported
or [rbx+AVX512_METHOD_1],edx    ; Disable AVX512/FMA512 tests if not supported
or [rbx+AVX512_METHOD_2],edx
or [rbx+AVX512_METHOD_3],edx
or [rbx+FMA512_METHOD_1],edx
@@:
test al,01000000b
jnz @f
or [rbx+RDRAND_METHOD],edx
@@:

; Parallel threads and Hyper-Threading options
; Note at this point, RDI, EAX, EDX valid from previous step
lea rbx,[PARALLEL_NUMA_PG_BASE]  ; RBX = Pointer to buttons list entry
mov ecx,[rdi + SYSPARMS.SystemInfo.dwNumberOfProcessors]    ; ECX = Number of processors by OS
test al,80h                      ; AL = Processor flags
jz .ClearRHT                     ; Go clear HT checkbox if not supported
cmp ecx,2
jae @f                           ; Skip clear HT checkbox if NCPU>1
.ClearRHT:
or [rbx+E_HYPER_THREADING],edx   ; Disable (clear) Hyper-Threading checkbox
@@:
cmp ecx,2                        ; Go skip clear MC checkbox if NCPU => 2
jae @f                          
or [rbx+E_PARALLEL_THREADS],edx  ; Disable (clear) Parallel Threads checkbox
@@:

; Processor Groups option
; Note at this point, RDI, RBX, EAX, EDX valid from previous step
cmp [rdi + SYSPARMS.ProcessorGroups],2
jae @f
or [rbx+E_PROCESSOR_GROUPS],edx
@@: 

; NUMA options
; Note at this point, RDI, RBX, EAX, EDX valid from previous step
cmp [rdi + SYSPARMS.NumaNodes],2    ; Check number of NUMA nodes
jae @f                          ; Skip clear MD checkbox if Domains>1
or [rbx+NUMA_UNAWARE],edx       ; Disable No NUMA checkbox, but can be selected
or [rbx+NUMA_FORCE_SINGLE],edx  ; Disable Force Local checkbox
or [rbx+NUMA_FORCE_LOCAL],edx   ; Disable Force Local checkbox
or [rbx+NUMA_FORCE_REMOTE],edx  ; Disable Force Remote checkbox
@@:

; Options, size select: L1, L2, L3, L4 cache analysing
; Note at this point, RDI, EAX, EDX valid from previous step
lea rbx,[CACHES_BASE]           ; RBX = Pointer to Buttons list entry
lea rsi,[rdi + SYSPARMS.CpuCache]    ; RSI = Pointer to Sys. Info, Caches list entry 
xor eax,eax                     ; RAX = 0 for size compare
cmp [rsi + CPUCACHE.CpuidL1Data],rax
jnz @f                          ; Go skip if L1 Data Cache exist
or [rbx+L1_CACHE],edx           ; Disable (clear) L1 Cache checkbox
@@:
cmp [rsi + CPUCACHE.CpuidL2Unified],rax
jnz @f                          ; Go skip if L2 Unified Cache exist
or [rbx+L2_CACHE],edx           ; Disable (clear) L2 Cache checkbox
@@:
cmp [rsi + CPUCACHE.CpuidL2Unified],rax
jnz @f                          ; Go skip if L3 Unified Cache exist
or [rbx+L3_CACHE],edx           ; Disable (clear) L3 Cache checkbox
@@:
; TODO. L4 support add required, yet disabled = const
or [rbx+L4_CACHE],edx

; Large Pages option
; Note at this point, RDI, EAX, EDX valid from previous step
lea rbx,[LARGE_PAGES_BASE]
cmp [rdi + SYSPARMS.LargePageFlag],1
je @f
or [rbx+E_LARGE_PAGES],edx
@@:

; Done
ret

