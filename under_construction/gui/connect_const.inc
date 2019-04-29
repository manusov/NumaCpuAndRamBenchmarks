;------------------------------------------------------------------------------;
;             GUI module: constants and pre-defined variables                  ;
;                    for support GUI objects by WinAPI.                        ;
;      Formats defined by Microsoft Windows and this application design.       ;
;------------------------------------------------------------------------------;

; TODO. Correct HANDLE numbers and numbers in comment, it shifted.
;------------------------------------------------------------------------------;
;                         Window 0 elements list.                              ;                             
;------------------------------------------------------------------------------;

; Window 0 = Main window of application, brief info and menu of operations.
; List of GUI window elements descriptors.

; If x2=BasePointGui, Result=StringsNull-StringsNull=0
START_HANDLE_WIN0 = 16
DialogueContent_Win0:
; All frames by vertical floors: Sysinfo, Settings, Buttons                                             ; Handle
DIALELEMENT  ClassButton , BasePointGui , 0003 , 0000 , 0743 , 0167 , WS_VISIBLE+WS_CHILD+BS_GROUPBOX   ; 16 = Frame for sysinfo		    
DIALELEMENT  ClassButton , BasePointGui , 0003 , 0162 , 0743 , 0192 , WS_VISIBLE+WS_CHILD+BS_GROUPBOX   ; 17 = Frame for options, size Y+36 when add 2 debug routines
DIALELEMENT  ClassButton , BasePointGui , 0003 , 0349 , 0743 , 0179 , WS_VISIBLE+WS_CHILD+BS_GROUPBOX   ; 18 = Frame for buttons, Y+36 when add 2 debug routines
; Sysinfo strings
; TODO. Make access CpuVendorString, CpuNameString by TEMP_BUFFER, single regular method for all accesses 
DIALELEMENT  ClassStatic , SystemParameters.CpuVendorString , 0010 , 0012 , 0110 , 0018 , WS_VISIBLE+WS_CHILD   ; 19 = CPU Vendor String
DIALELEMENT  ClassStatic , TEMP_BUFFER + 000                , 0113 , 0012 , 0220 , 0018 , WS_VISIBLE+WS_CHILD   ; 20 = CPU TFMS
DIALELEMENT  ClassStatic , SystemParameters.CpuNameString   , 0248 , 0012 , 0348 , 0018 , WS_VISIBLE+WS_CHILD   ; 21 = CPU Name String
DIALELEMENT  ClassStatic , TEMP_BUFFER + 080                , 0608 , 0012 , 0112 , 0018 , WS_VISIBLE+WS_CHILD   ; 22 = CPU TSC Frequency, items 1-4 same string
DIALELEMENT  ClassStatic , TEMP_BUFFER + 160                , 0010 , 0030 , 0730 , 0020 , WS_VISIBLE+WS_CHILD   ; 23 = Cache types/levels list
DIALELEMENT  ClassStatic , TEMP_BUFFER + 320                , 0010 , 0048 , 0730 , 0020 , WS_VISIBLE+WS_CHILD   ; 24 = ACPI MADT String + table-specific extended info
DIALELEMENT  ClassStatic , TEMP_BUFFER + 480                , 0010 , 0066 , 0730 , 0020 , WS_VISIBLE+WS_CHILD   ; 25 = ACPI SRAT String + table-specific extended info
DIALELEMENT  ClassStatic , TEMP_BUFFER + 640                , 0010 , 0084 , 0730 , 0020 , WS_VISIBLE+WS_CHILD   ; 26 = ACPI SLIT String 
DIALELEMENT  ClassStatic , TEMP_BUFFER + 800                , 0010 , 0102 , 0730 , 0020 , WS_VISIBLE+WS_CHILD   ; 27 = NUMA API String
DIALELEMENT  ClassStatic , TEMP_BUFFER + 960                , 0010 , 0120 , 0730 , 0020 , WS_VISIBLE+WS_CHILD   ; 28 = OS Processors String
DIALELEMENT  ClassStatic , TEMP_BUFFER + 1120               , 0010 , 0138 , 0730 , 0020 , WS_VISIBLE+WS_CHILD   ; 29 = OS Memory String
; Method options (CPU instruction set), started from first conditionally supported method: MMX
METHODS_BASE = (36-START_HANDLE_WIN0)*16 + DialogueContent_Win0 + 8
MMX64_METHOD_1  = (36-36)*16
MMX64_METHOD_2  = (37-36)*16
MMX64_METHOD_3  = (38-36)*16
SSE128_METHOD_1 = (39-36)*16
SSE128_METHOD_2 = (40-36)*16
SSE128_METHOD_3 = (41-36)*16
AVX256_METHOD_1 = (42-36)*16
AVX256_METHOD_2 = (43-36)*16
AVX256_METHOD_3 = (44-36)*16
AVX512_METHOD_1 = (45-36)*16
AVX512_METHOD_2 = (46-36)*16
AVX512_METHOD_3 = (47-36)*16
FMA256_METHOD_1 = (48-36)*16
FMA512_METHOD_1 = (49-36)*16
RDRAND_METHOD   = (51-36)*16
; Handles
HANDLE_X86_READ    = 30   ; This must be FIRST
HANDLE_MMX64_READ  = 36
HANDLE_SSE128_READ = 39
HANDLE_AVX256_READ = 42
HANDLE_AVX512_READ = 45
HANDLE_FMA256      = 48      
HANDLE_FMA512      = 49
HANDLE_RDRAND      = 51
HANDLE_LAST_METHOD = 51   ; This must be LAST
; Methods names strings
; "AM" label name prefix means "Assembler Method" means CPU Instruction Set
; Assembler methods, column 1
DIALELEMENT  ClassButton , AM_00 , 0010 , 0180 , 0250 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP+WS_GROUP  ; 30 = Read x86 MOV64
DIALELEMENT  ClassButton , AM_01 , 0010 , 0198 , 0250 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 31 = Write x86 MOV64
DIALELEMENT  ClassButton , AM_02 , 0010 , 0216 , 0250 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 32 = Copy x86 MOV64
DIALELEMENT  ClassButton , AM_03 , 0010 , 0234 , 0250 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 33 = Modify x86 MOV64
DIALELEMENT  ClassButton , AM_04 , 0010 , 0252 , 0250 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 34 = Write x86 Strings
DIALELEMENT  ClassButton , AM_05 , 0010 , 0270 , 0250 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 35 = Copy x86 Strings
DIALELEMENT  ClassButton , AM_06 , 0010 , 0288 , 0250 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 36 = Read MMX64
DIALELEMENT  ClassButton , AM_07 , 0010 , 0306 , 0250 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 37 = Write MMX64 
DIALELEMENT  ClassButton , AM_08 , 0010 , 0324 , 0250 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 38 = Copy MMX64  
; Assembler methods, column 2
DIALELEMENT  ClassButton , AM_09 , 0272 , 0180 , 0200 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 39 = Read SSE128
DIALELEMENT  ClassButton , AM_10 , 0272 , 0198 , 0200 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 40 = Write SSE128
DIALELEMENT  ClassButton , AM_11 , 0272 , 0216 , 0200 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 41 = Copy SSE128
DIALELEMENT  ClassButton , AM_12 , 0272 , 0234 , 0200 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 42 = Read AVX256 
DIALELEMENT  ClassButton , AM_13 , 0272 , 0252 , 0200 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 43 = Write AVX256 
DIALELEMENT  ClassButton , AM_14 , 0272 , 0270 , 0200 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 44 = Copy AVX256 
DIALELEMENT  ClassButton , AM_15 , 0272 , 0288 , 0200 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 45 = Read AVX512 
DIALELEMENT  ClassButton , AM_16 , 0272 , 0306 , 0200 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 46 = Write AVX512
DIALELEMENT  ClassButton , AM_17 , 0272 , 0324 , 0200 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 47 = Copy AVX512
; Assembler methods, column 3
DIALELEMENT  ClassButton , AM_18 , 0483 , 0180 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 48 = Dot product FMA256 
DIALELEMENT  ClassButton , AM_19 , 0483 , 0198 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 49 = Dot product FMA512
; Assembler methods,  column 3
DIALELEMENT  ClassButton , AM_35 , 0483 , 0234 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 50 = Latency (LCM)
DIALELEMENT  ClassButton , AM_36 , 0483 , 0252 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 51 = Latency (RDRAND) 
; Parallel operations and hyper-threading
PARALLEL_NUMA_PG_BASE = (52-START_HANDLE_WIN0)*16 + DialogueContent_Win0 + 8
E_PARALLEL_THREADS = (52-52)*16
E_HYPER_THREADING  = (53-52)*16
E_PROCESSOR_GROUPS = (54-52)*16
; Handles
HANDLE_PARALLEL_THREADS  = 52
HANDLE_HYPER_THREADING   = 53
HANDLE_PROCESSOR_GROUPS  = 54
; Options
DIALELEMENT  ClassButton , PT_D_00 , 0010 , 0368 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTOCHECKBOX+WS_TABSTOP              ; 52 = Use parallel threads
DIALELEMENT  ClassButton , HT_D_00 , 0010 , 0386 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTOCHECKBOX+WS_TABSTOP              ; 53 = Use hyper-threading
DIALELEMENT  ClassButton , PG_D_00 , 0010 , 0404 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTOCHECKBOX+WS_TABSTOP              ; 54 = Use processor groups
; NUMA Control, base is 46 because options group addressing
NUMA_UNAWARE      = (55-52)*16
NUMA_FORCE_SINGLE = (56-52)*16
NUMA_FORCE_LOCAL  = (57-52)*16
NUMA_FORCE_REMOTE = (58-52)*16
; Handles
HANDLE_NUMA_UNAWARE = 55
HANDLE_NUMA_SINGLE  = 56
HANDLE_NUMA_LOCAL   = 57
HANDLE_NUMA_REMOTE  = 58
; Options
DIALELEMENT  ClassButton , NUMA_D_00 , 0010 , 0440 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP+WS_GROUP  ; 55 = No NUMA control
DIALELEMENT  ClassButton , NUMA_D_01 , 0010 , 0458 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 56 = Force single domain
DIALELEMENT  ClassButton , NUMA_D_02 , 0010 , 0476 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 57 = Force Local
DIALELEMENT  ClassButton , NUMA_D_03 , 0010 , 0494 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 58 = Force Remote
; Block size selections, radiobuttons and edit field
CACHES_BASE = (59-START_HANDLE_WIN0)*16 + DialogueContent_Win0 + 8
L1_CACHE    = (59-59)*16
L2_CACHE    = (60-59)*16
L3_CACHE    = (61-59)*16
L4_CACHE    = (62-59)*16
CUSTOM_EDIT = (65-59)*16
; Handles
HANDLE_L1_CACHE     = 59
HANDLE_L2_CACHE     = 60
HANDLE_L3_CACHE     = 61
HANDLE_L4_CACHE     = 62
HANDLE_DRAM         = 63
HANDLE_CUSTOM_BLOCK = 64
HANDLE_CUSTOM_EDIT  = 65
; Options
DIALELEMENT  ClassButton , Target_00    , 0220 , 0368 , 0115 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP+WS_GROUP  ; 59 = L1 analyse 
DIALELEMENT  ClassButton , Target_01    , 0220 , 0386 , 0115 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 60 = L2 analyse
DIALELEMENT  ClassButton , Target_02    , 0220 , 0404 , 0115 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 61 = L3 analyse
DIALELEMENT  ClassButton , Target_03    , 0220 , 0422 , 0115 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 62 = L4 analyse
DIALELEMENT  ClassButton , Target_04    , 0220 , 0440 , 0115 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 63 = DRAM analyse
DIALELEMENT  ClassButton , Target_D_05  , 0220 , 0458 , 0155 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 64 = Custom size
DIALELEMENT  ClassEdit   , Edit_Size    , 0376 , 0456 , 0075 , 0020 , WS_VISIBLE+WS_CHILD+WS_BORDER+WS_TABSTOP+ES_AUTOHSCROLL     ; 65 = Custom size value edit
; Large Pages Control, base is 46 because options group addressing
LARGE_PAGES_BASE = (66-START_HANDLE_WIN0)*16 + DialogueContent_Win0 + 8
E_LARGE_PAGES = (66-66)*16
; Handle
HANDLE_LARGE_PAGES = 66
; Option
DIALELEMENT  ClassButton , LP_D_00      , 0220 , 0494 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTOCHECKBOX+WS_TABSTOP              ; 66 = Enable large pages
; Measurement repeats modes
MEASUREMNNT_BASE = (67-START_HANDLE_WIN0)*16 + DialogueContent_Win0 + 8
BRIEF_MEASUREMENT            = (67-67)*16
CAREFUL_MEASUREMENT          = (68-67)*16
BRIEF_ADAPTIVE_MEASUREMENT   = (69-67)*16
CAREFUL_ADAPTIVE_MEASUREMENT = (70-67)*16
; Handles
HANDLE_BRIEF_MEASUREMENT            = 67
HANDLE_CAREFUL_MEASUREMENT          = 68
HANDLE_BRIEF_ADAPTIVE_MEASUREMENT   = 69
HANDLE_CAREFUL_ADAPTIVE_MEASUREMENT = 70
; Options
DIALELEMENT  ClassButton , Measure_D_00 , 0490 , 0368 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP+WS_GROUP  ; 67 = Measure brief 
DIALELEMENT  ClassButton , Measure_D_01 , 0490 , 0386 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 68 = Measure careful 
DIALELEMENT  ClassButton , Measure_D_02 , 0490 , 0404 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 69 = Brief adaptive 
DIALELEMENT  ClassButton , Measure_D_03 , 0490 , 0422 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 70 = Careful adaptive 
; Measurement X-approximation modes
APPROXIMATION_NONE  = (71-67)*16
APPROXIMATION_X16   = (72-67)*16
APPROXIMATION_X32   = (73-67)*16
; Handles
HANDLE_APPROXIMATION_NONE = 71
HANDLE_APPROXIMATION_X16  = 72 
HANDLE_APPROXIMATION_X32  = 73 
; Options
DIALELEMENT  ClassButton , Approx_D_00 , 0490 , 0458 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP+WS_GROUP  ; 71 = No approximations 
DIALELEMENT  ClassButton , Approx_D_01 , 0490 , 0476 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 72 = Approximation X by 16 
DIALELEMENT  ClassButton , Approx_D_02 , 0490 , 0494 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 73 = Approximation X by 32 
; Buttons Run Simple, Run Drawings, Vector Brief Defaults, Cancel, About
HANDLE_BUTTON_ABOUT        = 74
HANDLE_BUTTON_BRIEF        = 75
HANDLE_BUTTON_RUN_SIMPLE   = 76
HANDLE_BUTTON_RUN_DRAWINGS = 77
HANDLE_BUTTON_DEFAULTS     = 78
HANDLE_BUTTON_CANCEL       = 79
; Buttons
DIALELEMENT  ClassButton , TextAbout       , 0058 , 0533 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 74 = About (note CMP/JA limit at event handler)
DIALELEMENT  ClassButton , TextBrief       , 0176 , 0533 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 75 = Vector brief
DIALELEMENT  ClassButton , TextRunSimple   , 0290 , 0533 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 76 = Run simple test (note CMP/JB limit at event handler)                 
DIALELEMENT  ClassButton , TextRunDrawings , 0404 , 0533 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 77 = Run drawings
DIALELEMENT  ClassButton , TextDefaults    , 0518 , 0533 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 78 = Defaults               
DIALELEMENT  ClassButton , TextCancel      , 0634 , 0533 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 79 = Cancel
; Terminator
DIALSTOP
; End of list

; _D_ means dialogue window purpose
; Parallel threads, for dialogue window
PT_D_00          DB  'Use parallel operations',0
; Hyper-threading, for dialogue window
HT_D_00          DB  'Use hyper-threading',0
; Processor groups, for dialogue window
PG_D_00          DB  'Use processor groups',0
; Large pages, for dialogue window
LP_D_00          DB  'Use large pages',0
; NUMA modes, for dialogue
; TODO. Update by new extended set of NUMA options
NUMA_D_00        DB  'NUMA unaware',0
NUMA_D_01        DB  'NUMA single domain',0
NUMA_D_02        DB  'NUMA optimal',0
NUMA_D_03        DB  'NUMA not optimal',0
; For dialogue
Target_D_05      DB  'Custom size (hex) =',0
; For dialogue, initial state of edit string
Edit_Size        DB  '00000000',0
; Measurement iterations modes, for dialogue
Measure_D_00     DB  'measure brief',0
Measure_D_01     DB  'measure careful',0
Measure_D_02     DB  'brief adaptive',0
Measure_D_03     DB  'careful adaptive',0
; Measurement X-approximation modes, for dialogue
Approx_D_00      DB  'No approximations',0
Approx_D_01      DB  'Approximation X by 16',0
Approx_D_02      DB  'Approximation X by 32',0

; Additions for drawings window
; Text strings for number of threads
Threads_String   DB  'Threads='
Threads_Number   DB  3+1 DUP (0)
; Text strings for paging options
NPages_String    DB  '4K pages',0
LPages_String    DB  'Large pages',0
; Text strings for NUMA options
NumaOff_String   DB  ' ',0
NumaU_String     DB  'NUMA unaware',0
NumaS_String     DB  'NUMA single',0
NumaL_String     DB  'NUMA local',0
NumaR_String     DB  'NUMA remote',0
; Text strings for number of repeats
Repeats_String   DB  'Repeats='
Repeats_Number   DB  12+1 DUP (0) 
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
TextSlit         DB  'SLIT tags',0
TextNoMadt       DB  'ACPI MADT table not found',0
TextNoSrat       DB  'ACPI SRAT table not found',0
TextNoSlit       DB  'ACPI SLIT table not found',0
TextNumaNodes    DB  'NUMA Nodes=',0
TextOsCpuCount   DB  'OS processors count=',0
TextOsCpuMasks   DB  'mask=',0
TextProcGroups   DB  'Processor groups=',0
TextProcTotal    DB  'Processors total=',0
TextPhysMemory   DB  'OS physical memory (MB)=',0
TextAvlMemory    DB  'Available (MB)=',0
TextLargePage    DB  'Minimal large page (MB)=',0
; Buttons strings
TextRunSimple    DB  'Run simple',0
TextRunDrawings  DB  'Run drawings',0
TextBrief        DB  'Vector brief',0
TextDefaults     DB  'Defaults',0
TextCancel       DB  'Cancel',0
TextAbout        DB  'About',0
TextRestart      DB  'Resize',0

;------------------------------------------------------------------------------;
;        Flags means Windows 0, 1 already initialized, pre-blanked to 0.       ;
;------------------------------------------------------------------------------;
Win0_Init     DB  0   ; Enable events handling by Window 0 = main
Win1_Init     DB  0   ; Enable events handling by Window 1 = drawings 
Win1_Running  DB  0   ; Flag for drawings and timer

;------------------------------------------------------------------------------; 
;            Windows Class Descriptors for user interface windows.             ;
;------------------------------------------------------------------------------;
; Used classes.
; ClassWindow0 = used at Dialogue_Win0, application main window 
; ClassWindow1 = used at Dialogue_Win1, drawings window Y=F(X)
; ClassButton  = used at dialogue descriptors, button
; ClassStatic  = used at dialogue descriptors, string
; ClassEdit    = used at dialogue descriptors, custom value edit 
; 
; WNDCLASSEX class descriptor structure (for Win64):
;--------------------------------------------------------------
; Name             Size	 Comments
;--------------------------------------------------------------
; cbSize           dd  Size of this structure, bytes
; style            dd  Window style info and bitmap
; lpfnWndProc      dq  Pointer to Window Procedure for call back
; cbClsExtra       dd  Number of extra cleared bytes after win. class structure
; cbWndExtra       dd  Number of extra cleared bytes after window instance
; hInstance        dq  Handle for instance, contain window proc. for the class
; hIcon            dq  Handle to the class icon (icon resource) , 0=default
; hCursor          dq  Handle to the class cursor (cursor resource) , 0=default
; hbrBackground    dq  Handle to the class background brush or color value
; lpszMenuName     dq  Pointer to resource name - class menu , 0=None
; lpszClassName    dq  Pointer to class name string or atom
; hIconSm          dq  Handle to the small icon
;-------------------------------------------------------------- 
Dialogue_Win0	  WNDCLASSEX sizeof.WNDCLASSEX,0,WindowProc_Win0,0,0,0,0,0,COLOR_BTNFACE+1,0,ClassWindow0,0
Dialogue_Win1	  WNDCLASSEX sizeof.WNDCLASSEX,0,WindowProc_Win1,0,0,0,0,0,COLOR_BTNFACE+1,0,ClassWindow1,0
; Used classes
ClassWindow0  DB  'FASM64WIN0',0  ; Class for Window 0 (my object)
ClassWindow1  DB  'FASM64WIN1',0  ; Class for Window 1 (my object)
ClassButton   DB  'BUTTON',0      ; Class BUTTON (reference to windows classes)
ClassStatic   DB  'STATIC',0      ; Class STATIC (reference to windows classes)
ClassEdit     DB  'EDIT',0        ; Class EDIT (reference to windows classes)

;------------------------------------------------------------------------------;
;                         Window 1 elements list.                              ;
;------------------------------------------------------------------------------;
; Window 1 = Measurement window, drawings Y=F(X).
; List of GUI window elements descriptors.
; If x2=BasePoint, Result=StringsNull-StringsNull=0
START_HANDLE_WIN1 = 16
DialogueContent_Win1:
; Buttons and ASM method string
HANDLE_BUTTON_RUN1    = 16
HANDLE_BUTTON_CANCEL1 = 17
HANDLE_TEXT_METHOD1   = 18
HANDLE_TEXT_OBJECT1   = 19
HANDLE_THREADS1       = 20
HANDLE_TEXT_REPEATS1  = 21
HANDLE_LARGE_PAGES1   = 22
HANDLE_NUMA_MODE1     = 23
; "AM" label name prefix means "Assembler Method" means CPU Instruction Set
; Drawings window control elements
DIALELEMENT  ClassButton , TextRestart     , 0605 , 0523 , 0080 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON  ; 16 = Restart button
DIALELEMENT  ClassButton , TextCancel      , 0690 , 0523 , 0080 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON  ; 17 = Cancel button
; Drawings window down comments strings
DIALELEMENT  ClassStatic , AM_00           , 0017 , 0512 , 0500 , 0020 , WS_VISIBLE+WS_CHILD                ; 18 = Selected ASM method string
DIALELEMENT  ClassStatic , Unknown_Object  , 0017 , 0530 , 0200 , 0020 , WS_VISIBLE+WS_CHILD                ; 19 = Selected analysed object string
DIALELEMENT  ClassStatic , Threads_String  , 0135 , 0530 , 0125 , 0020 , WS_VISIBLE+WS_CHILD                ; 20 = Number of threads string
DIALELEMENT  ClassStatic , Repeats_String  , 0232 , 0530 , 0125 , 0020 , WS_VISIBLE+WS_CHILD                ; 21 = Number of repeats string
DIALELEMENT  ClassStatic , NPages_String   , 0368 , 0530 , 0125 , 0020 , WS_VISIBLE+WS_CHILD                ; 22 = Large pages string
DIALELEMENT  ClassStatic , NumaOff_String  , 0458 , 0530 , 0108 , 0020 , WS_VISIBLE+WS_CHILD                ; 23 = NUMA mode string
; Terminator
DIALSTOP
; End of list

;------------------------------------------------------------------------------;
;            BITMAPINFOHEADRE descriptor structure (for Win64).                ;
;------------------------------------------------------------------------------;
; This structure used when initializing graphics window context, drawings Y=F(X)
;--------------------------------------------------------------
; Name		          Size    Comments
;--------------------------------------------------------------
; biSize           dd ?   Number of bytes
; biWidth          dd ?   Width
; biHeight         dd ?   Height
; biPlanes         dw ?   Bit Planes
; biBitCount       dw ?   Bits Per Pixel
; biCompression    dd ?   Compression Mode
; biSizeImage      dd ?   Image size, not required for uncompressed
; biXPelsPerMeter  dd ?   X pixels per meter, here not required
; biYPelsPerMeter  dd ?   Y pixels per meter, here not required
; biClrUsed        dd ?   Number of colors used, here not required (true color)
; biClrImportant   dd ?   Number of important colors, 0=All, here not used
;                         Important colors - used for visualization.
;                         0 means all colors used for visualization.
;--------------------------------------------------------------
BitmapInfo    BITMAPINFOHEADER 40,SUBWINX,SUBWINY,1,32,BI_RGB,0,0,0,0,0

;------------------------------------------------------------------------------;
;                             GUI color brushes.                               ;
;------------------------------------------------------------------------------;
; Used for graphics window, drawings Y=F(X)
; Brush color values = 00bbggrrh, bb=blue, gg=green, rr=red, 1 byte per color
BrushesList:
DD  BRUSH_GRID         ; Grid with horizontal and vertical lines 
DD  BRUSH_LINE         ; Draw Line Speed = F (Block Size)
DD  BRUSH_BACKGROUND   ; Draw window background

;------------------------------------------------------------------------------;
;               Strings for Window 1 - benchmarks drawings.                    ; 
;------------------------------------------------------------------------------;
; Window name, visualized at top of GUI window
WindowTitle_Win1  DB  'Performance = F (Block Size)',0
; Text strings for write timings in the drawings window
; 12 (not 9) for blank area with variable (font char defined) Xsize
; CPI = Clocks Per Instruction
DrawTextTSC          DB  'TSC clock (MHz)  = '
DrawValueTSC         DB  12 DUP (' '), 0
DrawTextMinimum      DB  'Minimum (CPI) = '
DrawValueMinimum     DB  12 DUP (' '),0
DrawTextMaximum      DB  'Maximum (CPI) = '
DrawValueMaximum     DB  12 DUP (' '),0
DrawTextAverage      DB  'Average (CPI) = '
DrawValueAverage     DB  12 DUP (' '),0
DrawTextMedian       DB  'Median (CPI) = '
DrawValueMedian      DB  12 DUP (' '),0
DrawTextMBPS         DB  'MBPS = '
DrawValueMBPS        DB  12 DUP (' '),0  
