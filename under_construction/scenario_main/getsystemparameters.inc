
GetSystemParameters:
; Load optional system functions
; Optional (not a static import) because Win XP (x64) compatibility

; TODO. Need show warnings after SystemFunctionsLoad,
; separate this operations

call SystemFunctionsLoad    ; This subroutine output CF=1 if error

; TODO.
; Need check load status, check validity of required API functions
; check for pointer = NULL. 

; Detect CPU features, abort if x87 not found or TSC not found
lea rdi,[SystemParameters]  ; Prepare destination pointer
mov r12,rdi
lea r15,[MessageNoCpuid]    ; Prepare error message pointer
call CheckCpuId             ; Return CPUID#0 parameters
jc ErrorSysInfo1            ; Go if CPUID not sup. or locked
call GetCpuName             ; Get and store CPU Name string
call GetCpuFeatures         ; Get TFMS and check features
jnc CpuFeaturesOk           ; Go skip error if CPU compatible
lea r15,[MessageNoCpuid1]   ; Prepare error message pointer
cmp al,0                    ; AL=0 means CPUID#1 not sup.
je ErrorSysInfo1            ; AL=1 means x87 not sup.
lea r15,[MessageNoX87]      ; Prepare error message pointer	
cmp al,1                    ; AL=0 means CPUID#1 not sup.
je ErrorSysInfo1            ; AL=2 means TSC not sup.
lea r15,[MessageNoTsc]      ; Prepare error message pointer	
ErrorSysInfo1: 
jmp ErrorSysInfo
CpuFeaturesOk:				       
; Measure CPU clock by Time Stamp Counter
lea r15,[MessageClkError]
call MeasureTsc            ; Measure TSC clock
jc ErrorSysInfo            ; Go error if TSC clock measurement failed
; Get cache information by CPUID 
call GetCpuCache
; Get topology and cache information by WinAPI
lea r15,[MessageTApiError]
lea rsi,[TEMP_BUFFER]
call GetOsTopologyInfo
jc ErrorSysInfo
; Overload topology and cache parameters, if WinAPI data valid
lea rbx,[r12 + SYSPARMS.OsCache]
lea rcx,[r12 + SYSPARMS.CpuCache]
; L1 instruction cache
mov rax,[rbx + OSCACHE.OsL1Code]         ; Get parms. L1C, received from WinAPI
mov edx,[rbx + OSCACHE.OsThreadsPerL1C]
test rax,rax                             ; Check OS parameter validity
jz @f                                    ; Go skip parm. update if not valid
mov [rcx + CPUCACHE.CpuidL1Code],rax     ; Overwrite L1 instruction cache size
mov [rcx + CPUCACHE.CpuidL1Csmp],dx      ; Overwrite threads per this cache
@@:
; L1 data cache
mov rax,[rbx + OSCACHE.OsL1Data]         ; Get parms. L1D, received from WinAPI
mov edx,[rbx + OSCACHE.OsThreadsPerL1D]
test rax,rax                             ; Check OS parameter validity
jz @f                                    ; Go skip parm. update if not valid
mov [rcx + CPUCACHE.CpuidL1Data],rax     ; Overwrite L1 data cache size
mov [rcx + CPUCACHE.CpuidL1Dsmp],dx      ; Overwrite threads per this cache
@@:
; L2 unified cache
mov rax,[rbx + OSCACHE.OsL2Unified]      ; Get parms. L2U, received from WinAPI
mov edx,[rbx + OSCACHE.OsThreadsPerL2U]
test rax,rax                             ; Check OS parameter validity
jz @f                                    ; Go skip parm. update if not valid
mov [rcx + CPUCACHE.CpuidL2Unified],rax  ; Overwrite L2 unified cache size
mov [rcx + CPUCACHE.CpuidL2Usmp],dx      ; Overwrite threads per this cache
@@:
; L3 unified cache
mov rax,[rbx + OSCACHE.OsL3Unified]      ; Get parms. L3U, received from WinAPI
mov edx,[rbx + OSCACHE.OsThreadsPerL3U]
test rax,rax                             ; Check OS parameter validity
jz @f                                    ; Go skip parm. update if not valid
mov [rcx + CPUCACHE.CpuidL3Unified],rax  ; Overwrite L3 unified cache size
mov [rcx + CPUCACHE.CpuidL3Usmp],dx      ; Overwrite threads per this cache
@@:
; Overload hyper-threading flag if by WinAPI NCPUs=1 
mov eax,[rbx + OSCACHE.OsThreadsPerCore] ; EAX = Threads per core
test eax,eax
jz @f                            ; Skip clear HT flag if argument not valid
cmp eax,1
jne @f                           ; Skip clear HT flag if number of threads <> 1
and [r12 + SYSPARMS.CpuSpecFlags],01111111b
@@:
; Get memory information by WinAPI
lea r15,[MessageMApiError]
lea rsi,[TEMP_BUFFER]
call GetOsMemoryInfo
; jc ErrorSysInfo

ErrorSysInfo:   ; if CF=1, register R15 = Pointer to error string
ret


; Helpers