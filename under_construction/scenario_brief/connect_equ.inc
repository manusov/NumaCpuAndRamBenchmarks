
; Vector brief, test parameters
VECTOR_BRIEF_DATA_SIZE     EQU  4096     ; Part of L1 (bytes) used as buffer

; Vector brief, temporary buffer allocation parameters
VECTOR_BRIEF_TEMP_DATA     EQU  0        ; Buffer for performance patterns
VECTOR_BRIEF_TEMP_RESULTS  EQU  4096     ; Buffer for save results
VECTOR_BRIEF_TEMP_TRANSIT  EQU  8192     ; Buffer for transit data 

; Vector brief results description block, not supported data = 0
struct VECTORBRIEF
CpuName         db  48 DUP (?)  ; CPU name string
Reserved1       dd  ?           ; Reserved for string terminator byte + align
TscClockHzI     dq  ?           ; TSC clock, Hz, increments per second
TscClockHzF     dq  ?
TscClockMHzF    dq  ?
TscClockSecF    dq  ?
TscClockNsF     dq  ?           ; Nanoseconds per TSC clock, double precision
StdFeatures     dq  ?           ; EDX(low), ECX(high) after CPUID#00000001h
ExtFeatures     dq  ?           ; EDX(low), ECX(high) after CPUID#80000001h
AddFeatures     dq  ?           ; EBX(low), ECX(high) after CPUID#00000007h  
OsContext       dq  ?           ; XCR0
dtSse128read    dq  ?           ; TSC clocks per SSE128 Read pattern
dtSse128write   dq  ?           ; TSC clocks per SSE128 Write pattern
dtSse128copy    dq  ?           ; TSC clocks per SSE128 Copy pattern 
dtAvx256read    dq  ?           ; TSC clocks per AVX256 Read pattern
dtAvx256write   dq  ?           ; TSC clocks per AVX256 Write pattern
dtAvx256copy    dq  ?           ; TSC clocks per AVX256 Copy pattern 
dtAvx512read    dq  ?           ; TSC clocks per AVX512 Read pattern
dtAvx512write   dq  ?           ; TSC clocks per AVX512 Write pattern
dtAvx512copy    dq  ?           ; TSC clocks per AVX512 Copy pattern 
dtSse128sqrt    dq  ?           ; TSC clocks per SSE128 Square Root pattern
dtAvx256sqrt    dq  ?           ; TSC clocks per AVX256 Square Root pattern
dtAvx512sqrt    dq  ?           ; TSC clocks per AVX512 Square Root pattern
dtX87cos        dq  ?           ; TSC clocks per x87 Cosine (FCOS) pattern
dtX87sincos     dq  ?           ; TSC clocks per x87 Sine+Cosine (FSINCOS) pat. 
ends

; Measurement repeats scale for performance patterns
REPEATS_SCALE  EQU  10000

