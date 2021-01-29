;--- Update GUI visualized parameters and benchmarks options availability -----;
;    (gray or black) by system information results                             ; 
;                                                                              ;
; INPUT:   None at registers                                                   ;
;          System Parameters Block = results of system information             ;
;          see global\connect_equ.inc for layout                               ; 
;                                                                              ;
; OUTPUT:  None at registers                                                   ;
;          GUI control structures                                              ;  
;                                                                              ;
; All CPU registers can be corrupted                                           ;
;------------------------------------------------------------------------------;

SysParmsToGui:

; Options, Method Selector = F (CPU features)
lea ebx,[METHODS_BASE]          ; EBX local label for compact offsets
lea edi,[SystemParameters]
mov eax,[edi + SYSPARMS.CpuSpecFlags]  ; EAX = Flags = F(CPU Features)
mov edx,WS_DISABLED             ; EDX = Operand
test ax,100000000b
jnz @f
or [ebx+MMX64_METHOD_1],edx     ; Disable SSE128 tests if not supported
or [ebx+MMX64_METHOD_2],edx
or [ebx+MMX64_METHOD_3],edx
@@:
test al,00000001b
jnz @f                          ; Skip disable operation if SSE128 supported
or [ebx+SSE128_METHOD_1],edx    ; Disable SSE128 tests if not supported
or [ebx+SSE128_METHOD_2],edx
or [ebx+SSE128_METHOD_3],edx
; Also for prefetch distance control
or [ebx+DISTANCE_DEFAULT_OPTION],edx
or [ebx+DISTANCE_MEDIUM_OPTION],edx
or [ebx+DISTANCE_LONG_OPTION],edx
@@:
test al,00000010b
jnz @f                          ; Skip disable operation if AVX256 supported
or [ebx+AVX256_METHOD_1],edx    ; Disable AVX256 tests if not supported
or [ebx+AVX256_METHOD_2],edx
or [ebx+AVX256_METHOD_3],edx
@@:
test al,00000100b
jnz @f                          ; Skip disable operation if FMA256 supported
or [ebx+FMA256_METHOD_1],edx    ; Disable FMA256/512 tests if not supported
or [ebx+FMA512_METHOD_1],edx
@@:
test al,00010000b
jnz @f                          ; Skip disable operation if AVX512 supported
or [ebx+AVX512_METHOD_1],edx    ; Disable AVX512/FMA512 tests if not supported
or [ebx+AVX512_METHOD_2],edx
or [ebx+AVX512_METHOD_3],edx
or [ebx+FMA512_METHOD_1],edx
@@:
test al,01000000b
jnz @f
or [ebx+RDRAND_METHOD_32],edx
or [ebx+RDRAND_METHOD_32x2],edx
@@:

; Parallel threads and Hyper-Threading options
; Note at this point, EDI, EAX, EDX valid from previous step
lea ebx,[PARALLEL_NUMA_PG_BASE]  ; RBX = Pointer to buttons list entry
mov ecx,[edi + SYSPARMS.SystemInfo.dwNumberOfProcessors]    ; ECX = Number of processors by OS
test al,80h                      ; AL = Processor flags
jz .ClearRHT                     ; Go clear HT checkbox if not supported
cmp ecx,2
jae @f                           ; Skip clear HT checkbox if NCPU>1
.ClearRHT:
or [ebx+E_HYPER_THREADING],edx   ; Disable (clear) Hyper-Threading checkbox
@@:
cmp ecx,2                        ; Go skip clear MC checkbox if NCPU => 2
jae @f                          
or [ebx+E_PARALLEL_THREADS],edx  ; Disable (clear) Parallel Threads checkbox
@@:

; Processor Groups option
; Note at this point, RDI, RBX, EAX, EDX valid from previous step
cmp [edi + SYSPARMS.ProcessorGroups],2
jae @f
or [ebx+E_PROCESSOR_GROUPS],edx
@@: 

; NUMA options
; Note at this point, EDI, EBX, EAX, EDX valid from previous step
cmp [edi + SYSPARMS.NumaNodes],2    ; Check number of NUMA nodes
jae @f                          ; Skip clear MD checkbox if Domains>1
or [ebx+NUMA_UNAWARE],edx       ; Disable No NUMA checkbox, but can be selected
or [ebx+NUMA_FORCE_SINGLE],edx  ; Disable Force Local checkbox
or [ebx+NUMA_FORCE_LOCAL],edx   ; Disable Force Local checkbox
or [ebx+NUMA_FORCE_REMOTE],edx  ; Disable Force Remote checkbox
@@:

; Options, size select: L1, L2, L3, L4 cache analysing
; Note at this point, EDI, EAX, EDX valid from previous step
lea ebx,[CACHES_BASE]           ; RBX = Pointer to Buttons list entry
lea esi,[edi + SYSPARMS.CpuCache]    ; ESI = Pointer to Sys. Info, Caches list entry 
xor eax,eax                     ; RAX = 0 for size compare
cmp dword [esi + CPUCACHE.CpuidL1Data],eax
jnz @f                          ; Go skip if L1 Data Cache exist
or [ebx+L1_CACHE],edx           ; Disable (clear) L1 Cache checkbox
@@:
cmp dword [esi + CPUCACHE.CpuidL2Unified],eax
jnz @f                          ; Go skip if L2 Unified Cache exist
or [ebx+L2_CACHE],edx           ; Disable (clear) L2 Cache checkbox
@@:
cmp dword [esi + CPUCACHE.CpuidL3Unified],eax
jnz @f                          ; Go skip if L3 Unified Cache exist
or [ebx+L3_CACHE],edx           ; Disable (clear) L3 Cache checkbox
@@:
; Note L4 support add required, yet disabled = const
or [ebx+L4_CACHE],edx

; Large Pages option
; Note at this point, RDI, EAX, EDX valid from previous step
lea ebx,[LARGE_PAGES_BASE]
cmp [edi + SYSPARMS.LargePageFlag],1
je @f
or [ebx+E_LARGE_PAGES],edx
@@:

; Done
ret

