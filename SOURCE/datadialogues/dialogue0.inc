;--- Window 0 elements list ---
; Window 0 = Main window of application, brief info and menu of operations.

; If x2=BasePoint, Result=StringsNull-StringsNull=0
START_HANDLE_WIN0 = 16
DialogueContent_Win0:
; All frames by vertical floors: Sysinfo, Settings, Buttons                                                ; Handle
DIALELEMENT  ClassButton , BasePoint , 0007 , 0001    , 0731 , 0168    , WS_VISIBLE+WS_CHILD+BS_GROUPBOX   ; 16 = Frame for sysinfo		    
DIALELEMENT  ClassButton , BasePoint , 0007 , 0169    , 0731 , 0291+36 , WS_VISIBLE+WS_CHILD+BS_GROUPBOX   ; 17 = Frame for options, size Y+36 when add 2 debug routines
DIALELEMENT  ClassButton , BasePoint , 0007 , 0460+36 , 0731 , 0053    , WS_VISIBLE+WS_CHILD+BS_GROUPBOX   ; 18 = Frame for buttons, Y+36 when add 2 debug routines
; Sysinfo strings
DIALELEMENT  ClassStatic , CpuVendorString , 0017 , 0015 , 0110 , 0020 , WS_VISIBLE+WS_CHILD    ; 19 = CPU Vendor String
DIALELEMENT  ClassStatic , TEMP_BUFFER+000  , 0115 , 0015 , 0220 , 0020 , WS_VISIBLE+WS_CHILD   ; 20 = CPU TFMS
DIALELEMENT  ClassStatic , CpuNameString   , 0250 , 0015 , 0348 , 0020 , WS_VISIBLE+WS_CHILD    ; 21 = CPU Name String
DIALELEMENT  ClassStatic , TEMP_BUFFER+080  , 0610 , 0015 , 0110 , 0020 , WS_VISIBLE+WS_CHILD   ; 22 = CPU TSC Frequency, items 1-4 same string
DIALELEMENT  ClassStatic , TEMP_BUFFER+160  , 0017 , 0036 , 0710 , 0020 , WS_VISIBLE+WS_CHILD   ; 23 = Cache types/levels list
DIALELEMENT  ClassStatic , TEMP_BUFFER+320  , 0017 , 0057 , 0710 , 0020 , WS_VISIBLE+WS_CHILD   ; 24 = ACPI MADT String
DIALELEMENT  ClassStatic , TEMP_BUFFER+480  , 0017 , 0078 , 0710 , 0020 , WS_VISIBLE+WS_CHILD   ; 25 = ACPI SRAT String
DIALELEMENT  ClassStatic , TEMP_BUFFER+640  , 0017 , 0099 , 0710 , 0020 , WS_VISIBLE+WS_CHILD   ; 26 = NUMA API String
DIALELEMENT  ClassStatic , TEMP_BUFFER+800  , 0017 , 0120 , 0710 , 0020 , WS_VISIBLE+WS_CHILD   ; 27 = OS Processors String
DIALELEMENT  ClassStatic , TEMP_BUFFER+960  , 0017 , 0141 , 0710 , 0020 , WS_VISIBLE+WS_CHILD   ; 28 = OS Memory String
; Method options (CPU instruction set)
METHODS_BASE = (35-START_HANDLE_WIN0)*16 + DialogueContent_Win0 + 8
SSE128_METHOD_1 = (35-35)*16
SSE128_METHOD_2 = (36-35)*16
SSE128_METHOD_3 = (37-35)*16
AVX256_METHOD_1 = (38-35)*16
AVX256_METHOD_2 = (39-35)*16
AVX256_METHOD_3 = (40-35)*16
AVX512_METHOD_1 = (41-35)*16
AVX512_METHOD_2 = (42-35)*16
AVX512_METHOD_3 = (43-35)*16
FMA256_METHOD_1 = (44-35)*16
FMA512_METHOD_1 = (45-35)*16
; Handles
HANDLE_X86_READ    = 29   ; This must be FIRST
HANDLE_SSE128_READ = 35
HANDLE_AVX256_READ = 38
HANDLE_AVX512_READ = 41
HANDLE_FMA256      = 44      
HANDLE_FMA512      = 45
HANDLE_LAST_METHOD = 45   ; This must be LAST
; Methods names strings
; "AM" label name prefix means "Assembler Method" means CPU Instruction Set
DIALELEMENT  ClassButton , AM_01 , 0017 , 0184 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP+WS_GROUP  ; 29 = Read x86 MOV64
DIALELEMENT  ClassButton , AM_02 , 0017 , 0202 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 30 = Write x86 MOV64
DIALELEMENT  ClassButton , AM_03 , 0017 , 0220 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 31 = Copy x86 MOV64
DIALELEMENT  ClassButton , AM_04 , 0017 , 0238 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 32 = Modify x86 MOV64
DIALELEMENT  ClassButton , AM_05 , 0017 , 0256 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 33 = Write x86 Strings
DIALELEMENT  ClassButton , AM_06 , 0017 , 0274 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 34 = Copy x86 Strings
DIALELEMENT  ClassButton , AM_07 , 0017 , 0292 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 35 = Read SSE128
DIALELEMENT  ClassButton , AM_08 , 0017 , 0310 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 36 = Write SSE128
DIALELEMENT  ClassButton , AM_09 , 0017 , 0328 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 37 = Copy SSE128
DIALELEMENT  ClassButton , AM_10 , 0017 , 0346 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 38 = Read AVX256
DIALELEMENT  ClassButton , AM_11 , 0017 , 0364 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 39 = Write AVX256
DIALELEMENT  ClassButton , AM_12 , 0017 , 0382 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 40 = Copy AVX256
DIALELEMENT  ClassButton , AM_13 , 0017 , 0400 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 41 = Read AVX512 
DIALELEMENT  ClassButton , AM_14 , 0017 , 0418 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 42 = Write AVX512 
DIALELEMENT  ClassButton , AM_15 , 0017 , 0436 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 43 = Copy AVX512 
DIALELEMENT  ClassButton , AM_16 , 0017 , 0454 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 44 = Dot product FMA256 
DIALELEMENT  ClassButton , AM_17 , 0017 , 0472 , 0260 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 45 = Dot product FMA512
; Parallel operations and hyper-threading
PARALLEL_NUMA_LARGE_PAGES_BASE = (46-START_HANDLE_WIN0)*16 + DialogueContent_Win0 + 8
E_PARALLEL_THREADS = (46-46)*16
E_HYPER_THREADING  = (47-46)*16
; Handles
HANDLE_PARALLEL_THREADS = 46
HANDLE_HYPER_THREADING  = 47
; Options
DIALELEMENT  ClassButton , PT_D_00         , 0380 , 0202 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTOCHECKBOX+WS_TABSTOP              ; 46 = Enable parallel threads
DIALELEMENT  ClassButton , HT_D_00         , 0380 , 0224 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTOCHECKBOX+WS_TABSTOP              ; 47 = Enable hyper-threading
; Large Pages Control, base is 46 because options group addressing
E_LARGE_PAGES = (48-46)*16
; Handle
HANDLE_LARGE_PAGES = 48
; Option
DIALELEMENT  ClassButton , LP_D_00         , 0380 , 0246 , 0180 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTOCHECKBOX+WS_TABSTOP              ; 48 = Enable large pages
; NUMA Control, base is 46 because options group addressing
NUMA_ABSENT       = (49-46)*16
NUMA_FORCE_LOCAL  = (50-46)*16
NUMA_FORCE_REMOTE = (51-46)*16
; Handles
HANDLE_NUMA_ABSENT = 49
HANDLE_NUMA_LOCAL  = 50
HANDLE_NUMA_REMOTE = 51
; Options
DIALELEMENT  ClassButton , NUMA_D_00       , 0380 , 0282 , 0310 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP+WS_GROUP  ; 49 = No NUMA control
DIALELEMENT  ClassButton , NUMA_D_01       , 0380 , 0300 , 0310 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 50 = Force Local
DIALELEMENT  ClassButton , NUMA_D_02       , 0380 , 0318 , 0310 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 51 = Force Remote
; Block size selections, radiobuttons and edit field
CACHES_BASE = (52-START_HANDLE_WIN0)*16 + DialogueContent_Win0 + 8
L1_CACHE    = (52-52)*16
L2_CACHE    = (53-52)*16
L3_CACHE    = (54-52)*16
CUSTOM_EDIT = (57-52)*16
; Handles
HANDLE_L1_CACHE     = 52
HANDLE_L2_CACHE     = 53
HANDLE_L3_CACHE     = 54
HANDLE_RAM          = 55
HANDLE_CUSTOM_BLOCK = 56
HANDLE_CUSTOM_EDIT  = 57
; Options
DIALELEMENT  ClassButton , Target_00       , 0380 , 0350 , 0115 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP+WS_GROUP  ; 52 = L1 analyse 
DIALELEMENT  ClassButton , Target_01       , 0380 , 0368 , 0115 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 53 = L2 analyse
DIALELEMENT  ClassButton , Target_02       , 0380 , 0386 , 0115 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 54 = L3 analyse
DIALELEMENT  ClassButton , Target_03       , 0380 , 0404 , 0115 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 55 = RAM analyse
DIALELEMENT  ClassButton , Target_D_04     , 0380 , 0422 , 0155 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTORADIOBUTTON+WS_TABSTOP           ; 56 = Custom size
DIALELEMENT  ClassEdit   , Edit_Size       , 0536 , 0420 , 0075 , 0020 , WS_VISIBLE+WS_CHILD+WS_BORDER+WS_TABSTOP+ES_AUTOHSCROLL     ; 57 = Custom size value edit
; Slow mode test
HANDLE_SLOW_MODE = 58
; Option
DIALELEMENT  ClassButton , Measure_D_00    , 0380 , 0458 , 0280 , 0018 , WS_VISIBLE+WS_CHILD+BS_AUTOCHECKBOX+WS_TABSTOP              ; 58 = Slow carefully mode
; Buttons Run Simple, Run Drawings, Vector Brief Defaults, Cancel, About
HANDLE_BUTTON_RUN_SIMPLE   = 59
HANDLE_BUTTON_RUN_DRAWINGS = 60
HANDLE_BUTTON_BRIEF        = 61
HANDLE_BUTTON_DEFAULTS     = 62
HANDLE_BUTTON_CANCEL       = 63
HANDLE_BUTTON_ABOUT        = 64
; Buttons
DIALELEMENT  ClassButton , TextRunSimple   , 0234 , 0516 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 59 = Run simple test (note CMP/JB limit at event handler)                 
DIALELEMENT  ClassButton , TextRunDrawings , 0356 , 0516 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 60 = Run drawings
DIALELEMENT  ClassButton , TextBrief       , 0112 , 0516 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 61 = Vector brief  
DIALELEMENT  ClassButton , TextDefaults    , 0478 , 0516 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 62 = Defaults               
DIALELEMENT  ClassButton , TextCancel      , 0600 , 0516 , 0110 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 63 = Cancel
DIALELEMENT  ClassButton , TextAbout       , 0030 , 0516 , 0070 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON   ; 64 = About (note CMP/JA limit at event handler)
; Terminator
DIALSTOP
; End of list

