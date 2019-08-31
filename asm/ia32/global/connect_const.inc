;------------------------------------------------------------------------------;
;                              Global Library:                                 ; 
;     globally used data constants and variables with pre-defined state.       ; 
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
; Base point for compact offsets encoding (16-bit relative offsets 0-65535),   ;
; note all used offsets must be POSITIVE relative this point.                  ;
;------------------------------------------------------------------------------;
BasePointGui:

;------------------------------------------------------------------------------;
;        Data definitions: application name, vendor and version strings.       ;
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
; Application, version and vendor description information.                     ;
;------------------------------------------------------------------------------;
PRODUCT_ID     DB  'NUMA CPU&RAM Benchmarks for Win32',0                                    
ABOUT_CAP      DB  'Program info',0
ABOUT_ID       DB  'NUMA CPU&RAM Benchmarks'    , 0Dh,0Ah
               DB  'v1.04.01 for Windows ia32'  , 0Dh,0Ah
               DB  '(C)2019 IC Book Labs'       , 0Dh,0Ah,0

;------------------------------------------------------------------------------;
; Memory size and speed units.                                                 ;
;------------------------------------------------------------------------------;
U_B            DB  'Bytes',0
U_KB           DB  'KB',0
U_MB           DB  'MB',0
U_GB           DB  'GB',0
U_TB           DB  'TB',0
U_MBPS         DB  'MBPS',0
U_NS           DB  'nanoseconds',0

;------------------------------------------------------------------------------;
; Options strings, used for 1)GUI, 2)result strings and 3)under drawings       ;
; "AM" label name prefix means "Assembler Method" means CPU Instruction Set    ;
; "NT" means "Non-temporal".                                                   ;
;------------------------------------------------------------------------------;
; Methods selector bytes
AM_Selector:
; AM_COUNT bytes per line
DB  00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 35, 36   ; Temporal, NT optimization not required
DB  00, 01, 02, 03, 04, 05, 06, 07, 08, 32, 20, 33, 34, 22, 23, 15, 24, 25, 18, 19, 35, 36   ; NT mode = NT Read by VMOVNTDQA not supported
DB  00, 01, 02, 03, 04, 05, 06, 07, 08, 26, 20, 27, 12, 22, 23, 15, 24, 25, 18, 19, 35, 36   ; NT mode = SSE4.1 only supported
DB  00, 01, 02, 03, 04, 05, 06, 07, 08, 26, 20, 27, 28, 22, 29, 30, 24, 31, 18, 19, 35, 36   ; NT mode = both SSE4.1 and AVX2 supported
; Methods names
AM_Strings_Prefix:
AM_XX  DB  '?',0
AM_Strings_Values:
; Base methods set                                                     ; #  | * means Can NT
AM_00  DB  'Read x86 (MOV)',0                                          ; 0
AM_01  DB  'Write x86 (MOV)',0                                         ; 1
AM_02  DB  'Copy x86 (MOV)',0                                          ; 2
AM_03  DB  'Modify x86 (NOT)',0                                        ; 3
AM_04  DB  'Write x86 strings (REP STOSD)',0                           ; 4 
AM_05  DB  'Copy x86 strings (REP MOVSD)',0                            ; 5
AM_06  DB  'Read MMX-64 (MOVQ)',0                                      ; 6
AM_07  DB  'Write MMX-64 (MOVQ)',0                                     ; 7
AM_08  DB  'Copy MMX-64 (MOVQ)',0                                      ; 8
AM_09  DB  'Read SSE-128 (MOVAPS)',0                                   ; 9    *  
AM_10  DB  'Write SSE-128 (MOVAPS)',0                                  ; 10   *
AM_11  DB  'Copy SSE-128 (MOVAPS)',0                                   ; 11   *
AM_12  DB  'Read AVX-256 (VMOVAPD)',0                                  ; 12   *
AM_13  DB  'Write AVX-256 (VMOVAPD)',0                                 ; 13   *
AM_14  DB  'Copy AVX-256 (VMOVAPD)',0                                  ; 14   *
AM_15  DB  'Read AVX-512 (VMOVAPD)',0                                  ; 15   *   
AM_16  DB  'Write AVX-512 (VMOVAPD)',0                                 ; 16   *
AM_17  DB  'Copy AVX-512 (VMOVAPD)',0                                  ; 17   *
AM_18  DB  'Dot product FMA-256 (VFMADD231)',0                         ; 18 
AM_19  DB  'Dot product FMA-512 (VFMADD231)',0                         ; 19
; Additions for non-temporal Write
AM_20  DB  'Non-temporal write SSE-128 (MOVNTPS)',0                    ; 20   
AM_21  DB  'Non-temporal copy SSE-128 (MOVAPS+MOVNTPS)',0              ; 21
AM_22  DB  'Non-temporal write AVX-256 (VMOVNTPD)',0                   ; 22
AM_23  DB  'Non-temporal copy AVX-256 (VMOVAPD+VMOVNTPD)',0            ; 23
AM_24  DB  'Non-temporal write AVX-512 (VMOVNTPD)',0                   ; 24
AM_25  DB  'Non-temporal copy AVX-512 (VMOVAPD+VMOVNTPD)',0            ; 25
; Additions for non-temporal Read
AM_26  DB  'Non-temporal read SSE-128 (MOVNTDQA)',0                    ; 26   
AM_27  DB  'Non-temporal copy SSE-128 (MOVNTDQA+MOVNTPD)',0            ; 27
AM_28  DB  'Non-temporal read AVX-256 (VMOVNTDQA)',0                   ; 28
AM_29  DB  'Non-temporal copy AVX-256 (VMOVNTDQA+VMOVNTPD)',0          ; 29
AM_30  DB  'Non-temporal read AVX-512 (VMOVNTDQA)',0                   ; 30
AM_31  DB  'Non-temporal copy AVX-512 (VMOVNTDQA+VMOVNTPD)',0          ; 31
; Additions for non-temporal Read if CPU not supports (V)MOVNTDQA, v0.98.2
; also if (V)MOVNTDQA is supported, but non-optimal
AM_32  DB  'Non-temporal read SSE-128 (PREFETCHNTA+MOVAPS)',0          ; 32   
AM_33  DB  'Non-temporal copy SSE-128 (PREFETCHNTA+MOVAPS+MOVNTPS)',0  ; 33
; Additions for non-temporal Read if CPU not supports (V)MOVNTDQA, v0.99.0
; also if (V)MOVNTDQA is supported, but non-optimal
AM_34  DB  'Non-temporal read AVX-256 (PREFETCHNTA+VMOVAPD)',0         ; 34
; Additions for latency measurement
AM_35  DB  'Measure latency (LCM)',0                                   ; 35  
AM_36  DB  'Measure latency (RDRAND)',0                                ; 36

; Hyper-threading option strings, for simple result window
HT_00            DB  'disabled by platform',0
HT_01            DB  'enabled by platform, not used by test',0
HT_02            DB  'enabled by platform, used by test',0
; Large pages, for simple result window
LP_00            DB  'not available',0
LP_01            DB  'available, not used by test',0
LP_02            DB  'available, used by test',0
; NUMA option strings
NUMA_00          DB  'not supported',0
NUMA_01          DB  'no control (under OS control)',0
NUMA_02          DB  'force current domain only',0
NUMA_03          DB  'force optimal (all local domains)',0
NUMA_04          DB  'force non-optimal (remote domains)',0
; PG option strings
PG_00            DB  'not supported',0
PG_01            DB  'no control (current group only)',0
PG_02            DB  'force optimal (all groups)',0  
; Unknown tested target object
Unknown_Object   DB  'Unknown target',0
; Target Object option strings, must be sequentally with previous string
Target_00        DB  'L1 cache',0
Target_01        DB  'L2 cache',0
Target_02        DB  'L3 cache',0
Target_03        DB  'L4 cache',0
Target_04        DB  'DRAM',0
Target_05        DB  'Custom block',0
; Measurement options strings
Measure_00       DB  'brief',0
Measure_01       DB  'careful',0
Measure_02       DB  'brief adaptive',0
Measure_03       DB  'careful adaptive',0
; String for block=none 
Block_00         DB  'not used',0

;------------------------------------------------------------------------------;
;                             Error messages.                                  ;
;      See also non-fatal warnings: file scenario_main\connect_const.inc       ;
;------------------------------------------------------------------------------;
; Error messages, visualized if can't start application
; Application cannot start with this errors
MessageNoCpuid	  DB  'CPUID instruction not supported or locked',0
MessageNoCpuid1   DB  'CPUID function 1 not supported or locked',0
MessageNoX87      DB  'x87 Floating Point Unit not supported or locked',0
MessageNoTsc      DB  'Time Stamp Counter not supported or locked',0
MessageClkError   DB  'Error measuring TSC frequency',0
MessageMApiError  DB  'Memory information API return error',0
MessageTApiError  DB  'CPU topological information API return error',0
MessageBadWindow  DB  'Error initializing dialogue window',0
; Error messages, visualized in runtime
; Cannot execute required test if this errors detected
RunErrorMemoryA   DB  'Benchmarks buffer memory allocation error',0  
RunErrorMemoryR   DB  'Benchmarks buffer memory release error',0
RunErrorTimings   DB  'Benchmarks timings measurement error',0 
RunErrorArithm    DB  'Benchmarks address arithmetic error',0

