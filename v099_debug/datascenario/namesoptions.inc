;--- Strings: names for modes and options ---

; System Information strings
TextTfms         DB  'TFMS=',0
TextTsc          DB  'TSC=',0
TextMHz          DB  'MHz',0
TextCache        DB  'Cache (KB)',0
TextTrace        DB  'Trace=',0
TextL1Code       DB  'L1Code=',0
TextL1Data       DB  'L1Data=',0
TextL2Unified    DB  'L2Unified=',0
TextL3Unified    DB  'L3Unified=',0
TextKuOps        DB  'KuOps',0
TextMadt         DB  'MADT tags',0
TextOemRev       DB  'OEM Rev=',0
TextLocalApics   DB  'Local APICs=',0
TextIoApics      DB  'I/O APICs=',0
TextSrat         DB  'SRAT tags',0
TextDomains      DB  'Domains=',0
TextCPUs         DB  'CPUs=',0
TextRAMs         DB  'RAMs=',0
TextNoMadt       DB  'ACPI MADT table not found',0
TextNoSrat       DB  'ACPI SRAT table not found',0
TextNumaNodes    DB  'NUMA Nodes=',0
TextOsCpuCount   DB  'OS processors count=',0
TextOsCpuMasks   DB  'mask=',0
TextPhysMemory   DB  'OS physical memory (MB)=',0
TextAvlMemory    DB  'Available (MB)=',0
TextLargePage    DB  'Minimal large page (MB)=',0

; Buttons strings
TextRunSimple    DB  'Run simple',0
TextRunDrawings  DB  'Run drawings',0
TextDefaults     DB  'Defaults',0
TextCancel       DB  'Cancel',0
TextAbout        DB  'About',0
TextRestart      DB  'Resize',0

; Options strings, also used for result strings and under drawings
; Changes for version 0.93 fix bug: "(V)MOVAPD" visualized INSTEAD "(V)MOVNTPD"
; if non-temporal mode selected for Tested Object = RAM.
; Required more carefully support non-temporal Write (V)MOVNTPD
; and prepare for support non-temporal Read (V)MOVNTDQA
; "AM" label name prefix means "Assembler Method" means CPU Instruction Set
; "NT" means "Non-temporal"
; Methods selector bytes
AM_Selector:
; 18 bytes per line
;                                *   *   *       *   *       *  *
;                                *   *   *   *   *   *       *  *   
DB  00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17
;DB 00, 01, 02, 03, 04, 05, 06, 07, 18, 19, 10, 20, 21, 13, 22, 23, 16, 17  ; v0.98.2 patch
;DB 00, 01, 02, 03, 04, 05, 06, 30, 18, 31, 10, 20, 21, 13, 22, 23, 16, 17  ; v0.99.0 patch
DB  00, 01, 02, 03, 04, 05, 06, 30, 18, 31, 32, 20, 21, 13, 22, 23, 16, 17
;                                *   *   *       *   *       *   *
DB  00, 01, 02, 03, 04, 05, 06, 24, 18, 25, 10, 20, 21, 13, 22, 23, 16, 17
;                                *   *   *   *   *   *   *   *   *   
DB  00, 01, 02, 03, 04, 05, 06, 24, 18, 25, 26, 20, 27, 28, 22, 29, 16, 17
; Methods names
AM_Strings:
; Base methods set                                                     ; #  | * means Can NT
AM_00  DB  '?',0                                                       ; 0
AM_01  DB  'Read x86-64 (MOV)',0                                       ; 1
AM_02  DB  'Write x86-64 (MOV)',0                                      ; 2
AM_03  DB  'Copy x86-64 (MOV)',0                                       ; 3
AM_04  DB  'Modify x86-64 (NOT)',0                                     ; 4
AM_05  DB  'Write x86-64 strings (REP STOSQ)',0                        ; 5 
AM_06  DB  'Copy x86-64 strings (REP MOVSQ)',0                         ; 6
AM_07  DB  'Read SSE-128 (MOVAPD)',0                                   ; 7  
AM_08  DB  'Write SSE-128 (MOVAPD)',0                                  ; 8    *
AM_09  DB  'Copy SSE-128 (MOVAPD)',0                                   ; 9    *
AM_10  DB  'Read AVX-256 (VMOVAPD)',0                                  ; 10
AM_11  DB  'Write AVX-256 (VMOVAPD)',0                                 ; 11   *
AM_12  DB  'Copy AVX-256 (VMOVAPD)',0                                  ; 12   *
AM_13  DB  'Read AVX-512 (VMOVAPD)',0                                  ; 13   
AM_14  DB  'Write AVX-512 (VMOVAPD)',0                                 ; 14   *
AM_15  DB  'Copy AVX-512 (VMOVAPD)',0                                  ; 15   *
AM_16  DB  'Dot product FMA-256 (VFMADD231)',0                         ; 16 
AM_17  DB  'Dot product FMA-512 (VFMADD231)',0                         ; 17
; Additions for non-temporal Write
AM_18  DB  'Non-temporal write SSE-128 (MOVNTPD)',0                    ; 18   
AM_19  DB  'Non-temporal copy SSE-128 (MOVAPD+MOVNTPD)',0              ; 19
AM_20  DB  'Non-temporal write AVX-256 (VMOVNTPD)',0                   ; 20
AM_21  DB  'Non-temporal copy AVX-256 (VMOVAPD+VMOVNTPD)',0            ; 21
AM_22  DB  'Non-temporal write AVX-512 (VMOVNTPD)',0                   ; 22
AM_23  DB  'Non-temporal copy AVX-512 (VMOVAPD+VMOVNTPD)',0            ; 23
; Additions for non-temporal Read
AM_24  DB  'Non-temporal read SSE-128 (MOVNTDQA)',0                    ; 24   
AM_25  DB  'Non-temporal copy SSE-128 (MOVNTDQA+MOVNTPD)',0            ; 25
AM_26  DB  'Non-temporal read AVX-256 (VMOVNTDQA)',0                   ; 26
AM_27  DB  'Non-temporal copy AVX-256 (VMOVNTDQA+VMOVNTPD)',0          ; 27
AM_28  DB  'Non-temporal read AVX-512 (VMOVNTDQA)',0                   ; 28
AM_29  DB  'Non-temporal copy AVX-512 (VMOVNTDQA+VMOVNTPD)',0          ; 29
; Additions for non-temporal Read if CPU not supports (V)MOVNTDQA, v0.98.2
; also if (V)MOVNTDQA is supported, but non-optimal
AM_30  DB  'Non-temporal read SSE-128 (PREFETCHNTA+MOVAPD)',0          ; 30   
AM_31  DB  'Non-temporal copy SSE-128 (PREFETCHNTA+MOVAPD+MOVNTPD)',0  ; 31
; Additions for non-temporal Read if CPU not supports (V)MOVNTDQA, v0.99.0
; also if (V)MOVNTDQA is supported, but non-optimal
AM_32  DB  'Non-temporal read AVX-256 (PREFETCHNTA+VMOVAPD)',0         ; 32

; Parallel threads, for dialogue
PT_D_00          DB  'Use parallel operations',0

; Hyper-threading option strings, for simple result window
HT_00            DB  'disabled by platform',0
HT_01            DB  'enabled by platform, not used by test',0
HT_02            DB  'enabled by platform, used by test',0

; Hyper-threading, for dialogue window
HT_D_00          DB  'Use hyper-threading',0

; Large pages, for simple result window
LP_00            DB  'not available',0
LP_01            DB  'available, not used by test',0
LP_02            DB  'available, used by test',0

; Large pages, for dialogue window
LP_D_00          DB  'Use large pages',0

; NUMA option strings
NUMA_00          DB  'not supported',0
NUMA_01          DB  'no control (under OS control)',0
NUMA_02          DB  'force optimal (local domains)',0
NUMA_03          DB  'force non-optimal (remote domains)',0

; For dialogue
NUMA_D_00        DB  'No NUMA aware',0
NUMA_D_01        DB  'Force NUMA optimal (local domains)',0
NUMA_D_02        DB  'Force NUMA not optimal (remote domains)',0

; Target Object option strings
Unknown_Object   DB  '?',0
Target_00        DB  'L1 cache',0
Target_01        DB  'L2 cache',0
Target_02        DB  'L3 cache',0
Target_03        DB  'RAM',0
Target_04        DB  'Custom block',0

; For dialogue
Target_D_04      DB  'Custom size (hex) =',0

; For dialogue, initial state of edit string
Edit_Size        DB  '00000000',0

; Measurement options strings
Measure_00       DB  'brief',0
Measure_01       DB  'careful',0

; For dialogue
Measure_D_00     DB  'Carefully measurement mode (slow)',0

; String for block=none 
Block_00         DB  'not used',0

; Memory size and speed units
U_B              DB  'Bytes',0
U_KB             DB  'KB',0
U_MB             DB  'MB',0
U_GB             DB  'GB',0
U_TB             DB  'TB',0
U_MBPS           DB  'MBPS',0

; Text strings for write timings in the drawings window
; 12 (not 9) for blank area with variable (font char defined) Xsize
; CPI = Clocks Per Instruction
TextTSC          DB  'TSC clock (MHz)  = '
TextTSC1         DB  7 DUP (' '), 0
TextMinimum      DB  'Minimum (CPI) = '
TextMinimum1     DB  12 DUP (' '),0
TextMaximum      DB  'Maximum (CPI) = '
TextMaximum1     DB  12 DUP (' '),0
TextAverage      DB  'Average (CPI) = '
TextAverage1     DB  12 DUP (' '),0
TextMBPS         DB  'MBPS = '
TextMBPS1        DB  12 DUP (' '),0  

; Text strings for number of threads
Threads_String   DB  'Threads='
Threads_Number   DB  2+1 DUP (0)

NPages_String    DB ' ',0
LPages_String    DB 'Large pages',0

; Text strings for number of repeats
Repeats_String   DB  'Repeats='
Repeats_Number   DB  9+1 DUP (0) 
