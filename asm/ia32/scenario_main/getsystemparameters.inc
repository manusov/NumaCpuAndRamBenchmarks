;--- Initialize system information data, for visual at main window and use ----;
;    in the benchmarks scenarios                                               ;
;                                                                              ;
; INPUT:   None at CPU registers                                               ;
;                                                                              ;
; OUTPUT:  CF flag = error indicator, 0(NC)=no errors, 1(C)=Error              ;
;          EBP = Pointer for error description string, valid only if CF=1      ;
;                                                                              ;
; All CPU registers can be corrupted                                           ;
;------------------------------------------------------------------------------;

GetSystemParameters:

; Note required check load status, check validity of required API functions
; check for pointer = NULL. 
; Detect CPU features, abort if x87 not found or TSC not found
lea edi,[SystemParameters]  ; Prepare destination pointer
lea ebp,[MessageNoCpuid]    ; Prepare error message pointer
call CheckCpuId             ; Return CPUID#0 parameters
jc ErrorSysInfo1            ; Go if CPUID not sup. or locked
call GetCpuName             ; Get and store CPU Name string
call GetCpuFeatures         ; Get TFMS and check features
jnc CpuFeaturesOk           ; Go skip error if CPU compatible
lea ebp,[MessageNoCpuid1]   ; Prepare error message pointer
cmp al,0                    ; AL=0 means CPUID#1 not sup.
je ErrorSysInfo1            ; AL=1 means x87 not sup.
lea ebp,[MessageNoX87]      ; Prepare error message pointer	
cmp al,1                    ; AL=0 means CPUID#1 not sup.
je ErrorSysInfo1            ; AL=2 means TSC not sup.
lea ebp,[MessageNoTsc]      ; Prepare error message pointer	
ErrorSysInfo1: 
jmp ErrorSysInfo
CpuFeaturesOk:				       
; Measure CPU clock by Time Stamp Counter
lea ebp,[MessageClkError]
call MeasureTsc            ; Measure TSC clock
jc ErrorSysInfo            ; Go error if TSC clock measurement failed
; Get cache information by CPUID 
call GetCpuCache
; Get topology and cache information by WinAPI
lea ebp,[MessageTApiError]
lea esi,[TEMP_BUFFER]
call GetOsTopologyInfo
jc ErrorSysInfo
; Overload topology and cache parameters, if WinAPI data valid
lea ebx,[SystemParameters + SYSPARMS.OsCache]
lea ecx,[SystemParameters + SYSPARMS.CpuCache]
; L1 instruction cache
mov eax,dword [ebx + OSCACHE.OsL1Code]      ; Get parms. L1C, received from WinAPI
mov edx,[ebx + OSCACHE.OsThreadsPerL1C]
test eax,eax                                ; Check OS parameter validity
jz @f                                       ; Go skip parm. update if not valid
mov dword [ecx + CPUCACHE.CpuidL1Code],eax  ; Overwrite L1 instruction cache size
mov [ecx + CPUCACHE.CpuidL1Csmp],dx         ; Overwrite threads per this cache
@@:
; L1 data cache
mov eax,dword [ebx + OSCACHE.OsL1Data]      ; Get parms. L1D, received from WinAPI
mov edx,[ebx + OSCACHE.OsThreadsPerL1D]
test eax,eax                                ; Check OS parameter validity
jz @f                                       ; Go skip parm. update if not valid
mov dword [ecx + CPUCACHE.CpuidL1Data],eax  ; Overwrite L1 data cache size
mov [ecx + CPUCACHE.CpuidL1Dsmp],dx         ; Overwrite threads per this cache
@@:
; L2 unified cache
mov eax,dword [ebx + OSCACHE.OsL2Unified]   ; Get parms. L2U, received from WinAPI
mov edx,[ebx + OSCACHE.OsThreadsPerL2U]
test eax,eax                                   ; Check OS parameter validity
jz @f                                          ; Go skip parm. update if not valid
mov dword [ecx + CPUCACHE.CpuidL2Unified],eax  ; Overwrite L2 unified cache size
mov [ecx + CPUCACHE.CpuidL2Usmp],dx            ; Overwrite threads per this cache
@@:
; L3 unified cache
mov eax,dword [ebx + OSCACHE.OsL3Unified]      ; Get parms. L3U, received from WinAPI
mov edx,[ebx + OSCACHE.OsThreadsPerL3U]
test eax,eax                                   ; Check OS parameter validity
jz @f                                          ; Go skip parm. update if not valid
mov dword [ecx + CPUCACHE.CpuidL3Unified],eax  ; Overwrite L3 unified cache size
mov [ecx + CPUCACHE.CpuidL3Usmp],dx            ; Overwrite threads per this cache
@@:
; Overload hyper-threading flag if by WinAPI NCPUs=1 
mov eax,[ebx + OSCACHE.OsThreadsPerCore] ; EAX = Threads per core
test eax,eax
jz @f                            ; Skip clear HT flag if argument not valid
cmp eax,1
jne @f                           ; Skip clear HT flag if number of threads <> 1
and [SystemParameters + SYSPARMS.CpuSpecFlags],01111111101111111b
@@:
; Get memory information by WinAPI
lea ebp,[MessageMApiError]
lea esi,[TEMP_BUFFER]
call GetOsMemoryInfo
jc ErrorSysInfo

; Get large pages information by WinAPI
call GetLargePagesInfo

; Get processor groups topological information
call GetProcessorGroups

; Get NUMA topological information include domains list
call GetOsNumaInfo

; Get topology information by firmware objects (ACPI tables)
lea esi,[TEMP_BUFFER]
call GetAcpiMadt
call GetAcpiSrat
call GetAcpiSlit

; This added for support non-temporal read, important for AMD
; SSE4.1 validation required for 128-bit MOVNTDQA
; AVX2 validation required for 256-bit VMOVNTDQA
; Note non-temporal operations used instead temporal if avoid caching required,
; for example DRAM access, when block size > cache size.
; Note non-temporal READ recommended for AMD CPU only,
; Note non-temporal WRITE recommended both for Intel and AMD CPU.
; Select non-temporal read performance patterns for AMD branch
; If re-use this mode, note unlock entries at methlist.inc file,
; and re-connect target performance include files

xor ebx,ebx                    ; BL = 0 means NT Read by VMOVNTDQA not supported
; patch v0.98.0 = non-temporal read use BOTH FOR INTEL AND AMD
; lea rsi,[r12 + SYSPARMS.CpuVendorString]    ; This string get from current CPU
; lea rdi,[Signature_AMD]      ; This is constant string
; mov ecx,12                   ; CPU Vendor String size is 12 bytes
; repe cmpsb
; jne @f                       ; Go because NT Read recommended for AMD,  
; end of patch v.0.98.0

; patch v0.99.0 = disable VMOVNTDQA 
; mov eax,[r12 + SYSPARMS.CpuSpecFlags]     ; even if supported by Intel
; mov bl,2                     ; BL = 2 means both SSE4.1 and AVX2 supported
; test al,00001000b            ; Check bit D3 = AVX2
; jnz @f                       ; Go if 128 and 256 bit NT Read supported
; mov bl,1                     ; BL = 1 means SSE4.1 only supported
; test al,00100000b            ; Check bit D5 = SSE4.1
; jnz @f                       ; Go if 128 bit NT Read supported 
; mov bl,0                     ; BL = 0 means NT Read not supported
; @@:
; end of patch v.0.99.0

mov [SystemParameters + SYSPARMS.NonTemporalRead],ebx   ; This variable selects patterns set

; Done without errors
clc
ret

; Exit point for error signaling
ErrorSysInfo:   ; if CF=1, register R15 = Pointer to error string
stc
ret


