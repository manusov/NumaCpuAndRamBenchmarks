;==============================================================================;
;                                                                              ;
;               NCRB (NUMA CPU&RAM Benchmarks). Win64 Edition.                 ; 
;                           (C)2018 IC Book Labs.                              ;
;                                                                              ;
;  This file is main module: translation object, interconnecting all modules.  ;
;                                                                              ;
;          Translation by Flat Assembler version 1.72 (Oct 10, 2017)           ;
;           Visit http://flatassembler.net/ for more information.              ;
;       Edit by FASM Editor 2.0, use this editor for correct tabulations.      ;
;                                                                              ;
;==============================================================================;

;--- Definitions and constants ---

; System control
; MAXIMUM_MEMORY  = 01001000h    ; OLD, 16MB + 4KB aligned, for limited version
; MAXIMUM_MEMORY  = 40001000h    ; OLD, 1GB + 4KB aligned request
MAXIMUM_MEMORY  = 80001000h      ; 2GB + 4KB aligned request
CONST_RAM_BLOCK = 32*1024*1024   ; 32MB one tested RAM block

; Criteria for 2GB note for v0.94 changes
; 1GB single type region
; 1GB region for 16 sub-regions, includes NUMA support
; 1GB / 16 threads = 64 GB per thread
; One tested block = 32 GB, 
; for copy required ( source=32GB ) + ( destination=32GB ) = 64GB

; When debug multi-thread remember 3 variants (not 2):
; a) Sufficient physical cores.
; b) Insufficient physical cores, resolved by Hyper-Threading.
; c) Insufficient physical cores, resolved by software multi threading.

CONST_THREADS  = 16            ; Number of execution threads, yet const=16
                               ; Control number of opened threads only,
                               ; don't control memory allocation
                               ; Yet memory reserved for maximum 16 threads

; Geometry parameters of Window 0 (parent), system information and options
WIN0_XBASE = 100    ; Parent window start X position, pixels, if no auto-center
WIN0_YBASE = 100    ; Parent window start Y position, pixels, if no auto-center
WIN0_XSIZE = 750    ; Parent window start X size, pixels
WIN0_YSIZE = 550+36 ; Parent window start Y size, pix., +36 for 2 debug routines

; Geometry parameters of Window 1 (daughter), drawings Speed = F(Block Size)
WIN1_XBASE = 150    ; Daughter (drawings) window same parameters set 
WIN1_YBASE = 150     
WIN1_XSIZE = 760     
WIN1_YSIZE = 560     

; Geometry parameters of Sub-Window with drawings Speed = F(Block Size)
SUBWINX    = 750+2  ; Plot sub-window X size, pixels
SUBWINY    = 480+2  ; Plot sub-window Y size, pixels
GRIDX      = 15     ; Divisor for drawing coordinate X-grid, vertical lines
GRIDY      = 10     ; Divisor for drawing coordinate Y-grid, horizontal lines
SHIFTX     = 1      ; X-shift plot sub window in the dialogue window, pixels
SHIFTY     = 1      ; X-shift plot sub window in the dialogue window, pixels
GRIDSTEPX  = 50     ; Addend for X-grid step
GRIDSTEPY  = 48     ; Addend for Y-grid step

; Color parameters of Sub-Window with drawings Speed = F(Block Size)
; Brush color values = 00bbggrrh, bb=blue, gg=green, rr=red, 1 byte per color
BRUSH_GRID       = 00017001h        ; Grid with horizontal and vertical lines 
BRUSH_LINE       = 0001F0F0h        ; Draw Line Speed = F (Block Size)
BRUSH_BACKGROUND = 00250101h        ; Draw window background
COLOR_TEXT_UNITS = 0001F001h        ; Text front color, units print
COLOR_TEXT_BACK  = BRUSH_BACKGROUND ; Text back color
COLOR_TEXT_INFO  = 00E0E0E0h        ; Text front color, system info print
COLOR_TEXT_DUMP  = 00F04040h        ; Text front color, instruction dump print

; WinAPI equations for DIB (Device Independent Bitmap)
DIB_RGB_COLORS    = 0x00  ; Means mode: color table contain RGB values
DIB_PAL_COLORS    = 0x01  ; Means mode: color tab. is indexes in the pal. table
DIB_PAL_INDICES   = 0x02  ; Means mode: no color table exist, use default
CLEARTYPE_QUALITY = 5     ; Quality code for create font and draw at grap. win. 

; Benchmarks visualization timings parameters
TIMER_TICK       = 50     ; Milliseconds per tick, benchmarks progress timer

; Benchmarks repeat parameters, precision=f(repeats), for Cache&RAM mode
L1_REPEATS       = 100000    ; Number of measur. iter. for objects, normal mode
L2_REPEATS       = 50000
L3_REPEATS       = 1000
MEM_REPEATS      = 100
CM_REPEATS       = 100000
L1_REPEATS_SLOW  = 2000000   ; Number of measur. iter. for objects, slow mode
L2_REPEATS_SLOW  = 500000
L3_REPEATS_SLOW  = 10000
MEM_REPEATS_SLOW = 200
CM_REPEATS_SLOW  = 1000000

; Benchmarks deault Y-sizing parameters
; This parameters set for first pass, 
; auto adjusted as F(Maximum Detected Speed) for next passes,
; if don't close Window 1 and press Run (Resize) button 
; Settings for Cache&RAM mode
Y_RANGE_CA               = 300000              ; Default Y maximum 
DEFAULT_Y_MBPS_PER_GRID  = Y_RANGE_CA/10       ; Def. units (MBPS) per greed Y 
DEFAULT_Y_MBPS_PER_PIXEL = Y_RANGE_CA/SUBWINY  ; Def. units (MBPS) per pixel Y

;--- FASM definitions ---

include 'win64a.inc'

;---------- Code section ------------------------------------------------------;

format PE64 GUI
entry start
section '.text' code readable executable
start:

;--- 32 byte parm. shadow and +8 alignment ---
sub rsp,8*5
;--- Load optional system functions ---
; Make it optional (not a static import) because Win XP (x64) compatibility
call SystemFunctionsLoad    ; This subroutine output CF=1 if error
call ShowWarningAPI         ; This subroutine input CF as error flag 
;--- Detect CPU features ---
lea rdi,[SystemParameters]  ; Prepare destination pointer
lea r15,[MessageNoCpuid]    ; Prepare error message pointer
call CheckCpuId             ; Return CPUID#0 parameters
jc ErrorProgram1            ; Go if CPUID not sup. or locked
mov al,0
stosb                       ; String terminator
call GetCpuName             ; Get and store CPU Name string
mov al,0
stosb                       ; String terminator
call GetCpuFeatures         ; Get TFMS and check features
jnc CpuFeaturesOk           ; Go skip error if CPU compatible
lea r15,[MessageNoCpuid1]   ; Prepare error message pointer
cmp al,0                    ; AL=0 means CPUID#1 not sup.
je ErrorProgram1            ; AL=1 means TSC not sup.
lea r15,[MessageNoTsc]      ; Prepare error message pointer	
ErrorProgram1: 
jmp ErrorProgram
CpuFeaturesOk:				       

;--- Check CPU TFMS signature ---
; Bug because don't check CPU vendor string, can match value for non-Intel.
; Remove this check for version with get topology info by WinAPI.
; lea r15,[MessageUnsupport]
; mov eax,[rdi-05]         ; EAX=TFMS (Type, Family, Model, Stepping)
; and al,0F0h              ; Clear stepping bits = D[3-0]
; cmp eax,000206D0h        ; Example #1 = Sandy Bridge, Xeon E5-2660 @ 2.2 GHz
; je @f                    ; Go continue if match
; cmp eax,000306C0h        ; Example #2 = Haswell, Core i7-4700HQ @ 2.4 GHz
; je @f                    ; Go continue if match
; cmp eax,00050650h        ; Example #3 = Skylake Xeon, Intel SDE emulator
; jne ErrorProgram1        ; Go error if mismatch
; @@:
;--- LOCKED ---

;--- Measure CPU clock by Time Stamp Counter ---
lea r15,[MessageClkError]
call MeasureCpuClk         ; Measure TSC clock
jc ErrorProgram1           ; Go error if TSC clock measurement failed
;--- Get CPU cache information ---
call GetCpuCache           ; Get Cache information
;--- Get ACPI info ---
lea rsi,[TempBuffer]     ; RSI = Transit buffer base, size = const
call GetAcpiMadt         ; Get ACPI MADT (Multiple APIC Description Table) info
call GetAcpiSrat         ; Get ACPI SRAT (System Resource Affinity Table) info
;--- Get OS info: NUMA, Memory, Processors ---
call GetOsNumaInfo       ; Get NUMA info by WinAPI
lea r15,[MessageMApiError]
call GetOsMemoryInfo
jc ErrorProgram1         ; Go error if get memory info failed
;--- Get MP and Cache topology info from OS --- 
; This method is universal: WinAPI used without CPU vendor-specific procedures,
; but less detail.
lea r15,[MessageTApiError]
lea rsi,[TempBuffer]     ; RSI = Transit buffer base, size = const
call GetOsMpInfo 
jc ErrorProgram1         ; Go error if get SMP info failed
;--- Overload topology and cache parameters, if WinAPI data valid ---
lea rsi,[TopologicalInfo]
lea rdi,[CacheL1Code]
;--- L1 instruction cache ---
mov rax,[rsi+00+00]  ; Get parameters for L1, received from WinAPI
mov edx,[rsi+32+00]
test rax,rax         ; Check new parameter validity
jz @f                ; Go skip parm. update if value for overwr. is not valid
mov [rdi+00+00],rax  ; Overwrite L1 instruction cache size
mov [rdi+00+08],dx   ; Overwrite L1i parameter: threads per this cache
@@:
;--- L1 data cache ---
mov rax,[rsi+00+08]
mov edx,[rsi+32+04]
test rax,rax
jz @f
mov [rdi+12+00],rax
mov [rdi+12+08],dx
@@:
;--- L2 unified cache ---
mov rax,[rsi+00+16]
mov edx,[rsi+32+08]
test rax,rax
jz @f
mov [rdi+24+00],rax
mov [rdi+24+08],dx
@@:
;--- L3 unified cache ---
mov rax,[rsi+00+24]
mov edx,[rsi+32+12]
test rax,rax
jz @f
mov [rdi+36+00],rax
mov [rdi+36+08],dx
@@:
;--- Overload hyper-threading flag if by WinAPI NCPUs=1 --- 
mov eax,[rsi+48]           ; EAX = Threads per core
test eax,eax
jz @f                      ; Skip clear HT flag if argument not valid
cmp eax,1
jne @f                     ; Skip clear HT flag if number of threads <> 1
and [CpuSpecFlags],01111111b
@@:
;--- Built Window 0 user interface elements = F (System Information) ---
;--- SysInfo Line 1, Processor ---
lea rsi,[TextTfms]         ; RSI = Text string "TFMS="
lea rdi,[TempBuffer+000]   ; RDI = Built position 0
call StringWrite           ; Print "TFMS="
mov eax,[CpuSignature]     ; TFMS = Type, Family, Model, Stepping
call HexPrint32            ; Print TFMS as hex number
mov ax,0000h+'h'
stosw                      ; "h" and string terminator 00h
;--- Line 1 continue, CPU Frequency ---
lea rdi,[TempBuffer+080]   ; RDI = Built position 1
call StringWrite           ; Print "TSC="
mov word [rdi],0000h+'?'
mov rax,[TscClockHz]       ; RAX = Frequency, Hz
test rax,rax
jz .EndTsc                 ; Go skip if frequency = 0
xor edx,edx                ; This clear entire RDX (not EDX only)
mov rcx,100000             ; Divisor=100000 for [Hz] to [MHz*10]
div rcx                    ; RAX = Frequency, [MHz*10]
shld rdx,rax,32
test edx,edx
jnz .EndTsc                ; Go skip if frequency > 2^32
call FloatPrintP1          ; Print Frequency as X.Y number
call StringWrite           ; Print "MHz", valid RSI=Src, RDI=Dst
mov al,0
stosb                      ; String terminator 00h
.EndTsc:
;--- SysInfo Line 2, Cache ---
lea rsi,[TextCache]        ; RSI = Text string "Cache"
lea rdi,[TempBuffer+160]   ; RDI = Built position 2
lea r8,[CacheTrace]        ; R8 = Pointer to sysinfo data
mov ecx,5                  ; RCX = Number of types/levels
call StringWrite           ; Print "Cache"
mov al,' '                 ; First interval
stosb
.CacheLevels:              ; Cycle for all cache types/levels
mov rbp,[r8]               ; Get RBP = Cache size, bytes
test rbp,rbp
jnz .MakeLevel             ; Go write if this type/level exist
@@:
lodsb                      ; Skip this type/level name string if not exist 
cmp al,0
jne @b
jmp .NextLevel
.MakeLevel:
mov ax,'  '
stosw
stosw
stosw
call StringWrite           ; Print Cache Level name
xchg rax,rbp
xor edx,edx
mov rbx,1024
div rbx
mov byte [rdi],'?'
shld rdx,rax,32
inc rdi
test edx,edx
jnz .NextLevel
dec rdi
mov bl,0
call DecimalPrint32
cmp ecx,5
jne .NextLevel             ; Skip write "KuOps" if no trace cache
push rsi
lea rsi,[TextKuOps]       
call StringWrite           ; Print Cache Level name
pop rsi
.NextLevel:
add r8,12
loop .CacheLevels          ; Cycle for all cache types/levels
mov al,0
stosb                      ; String terminator 00h
;--- SysInfo Line 3, ACPI MADT ---
lea rbx,[MadtText]         ; RBX = Pointer to System Information
lea rsi,[TextMadt]         ; RSI = Pointer to Name
lea rdi,[TempBuffer+320]   ; RDI = Built position 3
cmp dword [rbx+18],0
jne .PrintMadt
lea rsi,[TextNoMadt]
call StringWrite
jmp .EndMadt
.PrintMadt:
call AcpiHeaderPrint
lea rbx,[TextLocalApics]
call AcpiNumberPrint       ; Print Number of Local APICs
mov ax,'  '
stosw
stosw
stosb
lea rbx,[TextIoApics]
call AcpiNumberPrint       ; Print Number of I/O APICs
.EndMadt:
mov al,0
stosb                      ; String terminator 00h
;--- SysInfo Line 4, ACPI SRAT ---
lea rbx,[SratText]         ; RBX = Pointer to System Information
lea rsi,[TextSrat]         ; RSI = Pointer to Name
lea rdi,[TempBuffer+480]   ; RDI = Built position 4
cmp dword [rbx+18],0
jne .PrintSrat
lea rsi,[TextNoSrat]
call StringWrite
jmp .EndSrat
.PrintSrat:
call AcpiHeaderPrint
lea rbx,[TextDomains]
call AcpiNumberPrint       ; Print Number of NUMA Domains
mov ax,'  '
stosw
stosw
stosb
lea rbx,[TextCPUs]
call AcpiNumberPrint       ; Print Number of CPUs, NUMA-aware
mov ax,'  '
stosw
stosw
stosb
lea rbx,[TextRAMs]
call AcpiNumberPrint       ; Print Number of RAMs, NUMA-aware
.EndSrat:
mov al,0
stosb                      ; String terminator 00h
;--- SysInfo Line 5, NUMA Nodes ---
lea rsi,[TextNumaNodes]    ; RSI = Pointer to Name
lea rdi,[TempBuffer+640]   ; RDI = Built position 5
call StringWrite
lea rsi,[NumaNodes]
lodsd                      ; EAX = Number of NUMA Nodes
mov bl,0
call DecimalPrint32
cmp eax,4
jbe @f                     ; Print for first 4 NUMA nodes only
mov eax,4                  ; Set number of visualized nodes = 4
@@:
xchg ecx,eax               ; ECX = Number of NUMA Nodes
jecxz .EndNuma
mov ax,'  '
stosw
stosw
@@:
lodsq
call HexPrint64
mov al,'h'
stosb
dec ecx
jz .EndNuma
mov ax,', '
stosw
jmp @b                     ; Cycle for print first 4 NUMA nodes bitmap
.EndNuma:
mov al,0
stosb					             ; String terminator 00h
;--- SysInfo Line 6, Processors by OS ---
lea rcx,[SystemInfo]       ; RCX = Pointer to System Information
lea rsi,[TextOsCpuCount]	 ; RSI = Pointer to String name
lea rdi,[TempBuffer+800]   ; RDI = Built position 6
call StringWrite           ; Print "OS processors count="
mov eax,[rcx+SYSTEM_INFO.dwNumberOfProcessors]
mov bl,0                   ; BL = 0, mode for decimal print
mov byte [rdi],'?'
inc rdi
cmp eax,10000h             ; Validity limit = 65536
jae @f
dec rdi
call DecimalPrint32
@@:
mov ax,'  '
stosw
stosw
call StringWrite           ; Print "mask="
mov rax,[rcx+SYSTEM_INFO.dwActiveProcessorMask]
call HexPrint64
mov ax,0000h+'h'           ; "h" and string terminator 00h
stosw
;--- SysInfo Line 7, Physical memory by OS ---
; BL, RCX, RSI here valid from previous step
lea rdi,[TempBuffer+960]   ; RDI = Built position 7
mov bh,2                   ; 2 strings: Physical, Available
.PrintMemory:              ; Print "OS physical memory (MB)=
call StringWrite           ; "at second pass "Available (MB)="
mov rax,[rcx+48+08]        ; RAX = Physical memory by OS, Bytes
xor edx,edx                ; RDX = 0, Dividend high bits
mov rbp,1024*1024          ; RBP = Divisor for Bytes to Megabytes
div rbp                    ; RAX = Physical memory by OS, Megabytes
mov byte [rdi],'?'
inc rdi
shld rdx,rax,32
test edx,edx               ; Check bits [63-32] of Size, Megabytes
jnz @f                     ; Go skip print if overflow
dec rdi
call DecimalPrint32
@@:
mov ax,'  '
stosw
stosw
add rcx,8
dec bh
jnz .PrintMemory
mov al,0
stosb					             ; String terminator 00h
;--- Options, Method Selector = F (CPU features)  ---
mov al,[CpuSpecFlags]           ; AL = Flags = F(CPU Features)
mov edx,WS_DISABLED             ; EDX = Operand
lea rbx,[METHODS_BASE]          ; RBX local label for compact offsets
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
;--- Options, SMT, MP, NUMA control ---
; AL, EDX valid from previous step
lea rbx,[PARALLEL_NUMA_BASE]    ; RBX = Pointer to buttons list entry
;-- PATCH -
; mov ecx,[MadtLocalApics]      ; RCX = Number of Local APICs
mov ecx,[SystemInfo+32]         ; RCX = Number of processors by OS
;- PATCH -
test al,80h                     ; AL = Processor flags
jz .ClearRHT                    ; Go clear HT checkbox if not supported
cmp ecx,2
jae @f                          ; Skip clear HT checkbox if NCPU>1
.ClearRHT:
or [rbx+E_HYPER_THREADING],edx  ; Disable (clear) Hyper-Threading checkbox
@@:
;--- Parallel threads ---
cmp ecx,2                     ; Go skip clear MC checkbox if NCPU => 2
jae @f                          
;- PATCH -
; Old method is separate MC(Multicore) and HT (Hyperthreading)
; ja @f
; cmp ecx,1
; jbe .ClearEPT               ; Go clear MC checkbox if NCPU<2
; test al,80h
; jz @f                       ; Go skip clear MC checkbox if NCPU>=2 and HT=OFF
; .ClearEPT:
;- PATCH -
or [rbx+E_PARALLEL_THREADS],edx  ; Disable (clear) Parallel Threads checkbox
@@:
;--- NUMA ---
cmp dword [NumaNodes],2
jae @f                          ; Skip clear MD checkbox if Domains>1
or [rbx+NUMA_ABSENT],edx        ; Disable No NUMA checkbox, but can be selected
or [rbx+NUMA_FORCE_LOCAL],edx   ; Disable Force Remote checkbox
or [rbx+NUMA_FORCE_REMOTE],edx  ; Disable Force Local checkbox
@@:
;--- Options, size select: L1, L2, L3 analysing ---
lea rbx,[CACHES_BASE]           ; RBX = Pointer to Buttons list entry
lea rsi,[CacheL1Data]           ; RSI = Pointer to Sys. Info, Caches list entry 
xor eax,eax                     ; RAX = 0 for size compare
cmp [rsi+00],rax
jnz @f                          ; Go skip if L1 Data Cache exist
or [rbx+L1_CACHE],edx           ; Disable (clear) L1 Cache checkbox
@@:
cmp [rsi+12],rax
jnz @f                          ; Go skip if L2 Unified Cache exist
or [rbx+L2_CACHE],edx           ; Disable (clear) L2 Cache checkbox
@@:
cmp [rsi+24],rax
jnz @f                          ; Go skip if L3 Unified Cache exist
or [rbx+L3_CACHE],edx           ; Disable (clear) L3 Cache checkbox
@@:

;--- This added for support non-temporal read, important for AMD ---
; SSE4.1 validation required for 128-bit MOVNTDQA
; AVX2 validation required for 256-bit VMOVNTDQA
; Note non-temporal operations used instead temporal if avoid caching required,
; for example DRAM access, when block size > cache size.
; Note non-temporal READ recommended for AMD CPU only,
; Note non-temporal WRITE recommended both for Intel and AMD CPU.
;--- Select non-temporal read performance patterns for AMD branch ---

mov bl,0                     ; BL = 0 means NT Read not supported
lea rsi,[CpuVendorString]    ; This string get from current CPU
lea rdi,[Signature_AMD]      ; This is constant string
mov ecx,12                   ; CPU Vendor String size is 12 bytes
repe cmpsb
jne @f                       ; Go because NT Read recommended for AMD,  
mov al,[CpuSpecFlags]        ; even if supported by Intel
mov bl,2                     ; BL = 2 means both SSE4.1 and AVX2 supported
test al,00001000b            ; Check bit D3 = AVX2
jnz @f                       ; Go if 128 and 256 bit NT Read supported
mov bl,1                     ; BL = 1 means SSE4.1 only supported
test al,00100000b            ; Check bit D5 = SSE4.1
jnz @f                       ; Go if 128 bit NT Read supported 
mov bl,0                     ; BL = 0 means NT Read not supported
@@:
mov [NonTemporalRead],bl     ; Update variable, used for select patterns set

;--- Memory allocation ---
; 1GB+ request, yet fixed, need re-sizing = F(available memory).
; Need change logic, 1GB = HEAP or 1GB = 16*32GB+512GB (NUMA)
; for all allocate statically, dynamic allocation before each test 
; can cause unstable. Can be both Normal and NUMA-ready regions statically
; exist. 
; Memory request, must prepare both as Heap and as NUMA,
; error if request memory from OS callback window routine ?
;---
; DON'T USE   mov [MemoryRequestSize],MAXIMUM_MEMORY   because overflow
;---
mov rax,MAXIMUM_MEMORY
mov [MemoryRequestSize],rax
call MemoryAllocation                      
lea r15,[MessageMemError]  ; Prepare error message pointer                                          
jz ErrorProgram            ; Go error if memory allocation failed

;--- Warning about engineering sample ---
; Skipped from v0.94
; xor ecx,ecx	               ; Parm#1 = Parent window handle or 0
; lea rdx,[RevisionID]       ; Parm#2 = Pointer to string
; lea r8,[ProductID]         ; Parm#3 = Pointer to caption
; mov r9d,MB_ICONWARNING     ; Parm#4 = Message box icon type
; call [MessageBoxA]
;

;--- Create (Open) parent window = Window 0 ---
; Terminology notes: WS = Window Style
;---
lea rsi,[ProductID]        ; RSI = Window name string pointer
lea rdi,[Dialogue_Win0]    ; RDI = Dialogue descriptor pointer
mov r10d,WS_VISIBLE+WS_DLGFRAME+WS_SYSMENU ; R10 = Window Style
mov r11d,WIN0_XBASE        ; R11 = Window X-base
mov r12d,WIN0_YBASE        ; R12 = Window Y-base
mov r13d,WIN0_XSIZE        ; R13 = Window X-size
mov r14d,WIN0_YSIZE        ; R14 = Window Y-size
xor ebx,ebx                ; RBX = 0, means no parent handle
xor eax,eax                ; AH  = 0 , means parent = Application
call CreateApplicationWindow     ; AL  = 0 , means first call
lea r15,[MessageBadWindow]       ; R14 = Error string, used if abort
jz ErrorProgram 			           ; Go if error returned
;--- Get messages with wait user events for window from OS ---
; GetMessage subroutine parameters
; Parm#1 = Pointer to structure receive data = DialMsg
; Parm#2 = Window handle whose message retrieved, 0=any
; Parm#3 = Filter Min , both Min=Max=0 means no filtering
; Parm#4 = Filter Max , both Min=Max=0 means no filtering
; Output RAX = Message status, used values (but this is not a message code):
; 0, means returned WM_QUIT, returned when window subroutine
;    call PostQuitMessage subroutine, means quit
; 1, means process event
;---
mov byte [Win0_Init],1           ; Enable commands handling by callback routine
lea rdi,[DialogueMsg_Win0]       ; RDI = MSG structure
call WaitEvent                   ; Window 1 work as callbacks inside this

;--- Memory release and go exit ---
call MemoryRelease               ; Memory release, must both as Heap and as NUMA

;--- Restore system context ---
; Reserved for add functionality
;---
ExitProgram:
xor ecx,ecx	                     ; RCX = Parm#1 = Exit code
call [ExitProcess]               ; No return from this function

;--- Exit points for error exit and normal exit ---
; Terminology notes: MB = Message Box
;---
ErrorProgram:
;---
; Display OS message box, return button ID
; Parm#1 = RCX = Parent window handle
; Parm#2 = RDX = Pointer to message string must be valid at this point,
;	         value get from transit storage: R15
; Parm#3 = R8  = Caption=0 means error message, otherwise pointer to caption
; Parm#4 = R9  = Message Box Icon Error = MB_ICNERROR
; Output = RAX = Pressed button ID, not used at this call
; Note INVOKE replaced to instructions for code size optimization!
; invoke MessageBoxA,0,r14,0,MB_ICONERROR
;---
xor ecx,ecx	         ; Parm#1, this clear entire RCX by x64 architecture rules
mov rdx,r15	         ; Parm#2
xor r8d,r8d	         ; Parm#3
mov r9,MB_ICONERROR  ; Parm#4
call [MessageBoxA]
jmp ExitProgram

;--- Continue code section, INCLUDE connections ---
include 'stringwrite\stringwrite.inc'  ; String write, copy to buffer
include 'stringwrite\hexprint.inc'     ; Hex numbers write, ASCII string 
include 'stringwrite\decprint.inc'     ; Decimal numbers write, ASCII string 
include 'stringwrite\sizeprint.inc'    ; Memory size write, ASCII string  
include 'stringwrite\floatprint.inc'   ; Floating numbers write, ASCII string
include 'stringwrite\acpihprint.inc'   ; ACPI table header write
include 'stringwrite\acpinprint.inc'   ; Numbers write for ACPI tables info  
include 'sysinfo\fncload.inc'          ; Optional system functions loader
include 'sysinfo\checkcpuid.inc'       ; Check CPUID support
include 'sysinfo\getcpuname.inc'       ; Get CPU name string by CPUID
include 'sysinfo\getcpufeatures.inc'   ; Get CPU features by CPUID
include 'sysinfo\measurecpuclk.inc'    ; Measure CPU clock frequency by TSC
include 'sysinfo\getcpucache.inc'      ; Get CPU cache info by CPUID
include 'sysinfo\getacpi.inc'          ; Get ACPI tables information
include 'sysinfo\getos.inc'            ; Get Operating System information
include 'sysinfo\memalloc.inc'         ; Memory allocation
include 'sysinfo\memrelease.inc'       ; Memory release, after allocation
include 'windowone\resultone.inc'      ; Show simple measurement result 
include 'windowmsg\warningapi.inc'     ; Show warning about optional API 
include 'windowmain\win0.inc'          ; Main parent window callback handler
include 'windowdraw\win1.inc'          ; Draw child window callback handler
include 'windowmain\createwin.inc'     ; Create dialogue window subroutine
include 'windowmain\waitevent.inc'     ; Wait user event subroutine
include 'windowmain\createdial.inc'    ; Create dialogue elements subroutine
include 'windowmain\setdef0.inc'       ; Set defaults for options at win0
include 'windowmain\sendmsggc.inc'     ; Send message
include 'windowmain\modeset.inc'       ; Benchmarks mode set = F(Options)
;--- Performance patterns for CPU instructions ---
; See comments at include modules
;--- This point for compact offsets encoding ---
BasePointCode:
;---
include 'mpetargets\read_mov64.inc'          ; Read, General Purpose MOV 64-bit 
include 'mpetargets\write_mov64.inc'         ; Write, General Purp. MOV 64-bit
include 'mpetargets\copy_mov64.inc'          ; Read + Write, G. P. MOV 64-bit
include 'mpetargets\modify_not64.inc'        ; Read+Modify+Write, NOT 64-bit
include 'mpetargets\write_stosq.inc'         ; Write, Hardware Cycle 64-bit
include 'mpetargets\copy_movsq.inc'          ; Copy, Hardware Cycle 64-bit
include 'mpetargets\read_sse128.inc'         ; Read, SSE 128-bit
include 'mpetargets\write_sse128.inc'        ; Write, SSE 128-bit
include 'mpetargets\copy_sse128.inc'         ; Read + Write, SSE 128-bit
include 'mpetargets\read_avx256.inc'         ; Read, AVX 256-bit
include 'mpetargets\write_avx256.inc'        ; Write, AVX 256-bit
include 'mpetargets\copy_avx256.inc'         ; Read + Write, AVX 256-bit
include 'mpetargets\read_avx512.inc'         ; Read, AVX 512-bit
include 'mpetargets\write_avx512.inc'        ; Write, AVX 512-bit
include 'mpetargets\copy_avx512.inc'         ; Read + Write, AVX 512-bit
include 'mpetargets\dot_fma256.inc'          ; Fused Multiply-Add, AVX 256-bit 
include 'mpetargets\dot_fma512.inc'          ; Fused Multiply-Add, AVX 512-bit
;--- Additional routines for support non-temporal write (NT = Non-Temporal) ---
include 'mpetargets\ntwrite_sse128.inc'      ; NT Write, SSE 128-bit
include 'mpetargets\ntwrite_avx256.inc'      ; NT Write, AVX 256-bit      
include 'mpetargets\ntwrite_avx512.inc'      ; NT Write, AVX 512-bit
include 'mpetargets\ntcopy_sse128.inc'       ; NT Read + NT Write, SSE 128-bit
include 'mpetargets\ntcopy_avx256.inc'       ; NT Read + NT Write, AVX 256-bit
include 'mpetargets\ntcopy_avx512.inc'       ; NT Read + NT Write, AVX 512-bit
include 'mpetargets\ntdot_fma256.inc'        ; Reserved for FMA with NT Write
include 'mpetargets\ntdot_fma512.inc'        ; Reserved for FMA with NT Write
;--- Additional routines for support non-temporal read ---
include 'mpetargets_ntr\ntread_sse128.inc'   ; NT Read, SSE 128-bit 
include 'mpetargets_ntr\ntread_avx256.inc'   ; NT Read, AVX 256-bit
include 'mpetargets_ntr\ntread_avx512.inc'   ; NT Read, AVX 512-bit
include 'mpetargets_ntr\ntrcopy_sse128.inc'  ; NT Read + NT Write, SSE 128-bit
include 'mpetargets_ntr\ntrcopy_avx256.inc'  ; NT Read + NT Write, AVX 256-bit 
include 'mpetargets_ntr\ntrcopy_avx512.inc'  ; NT Read + NT Write, AVX 512-bit 

;--- Memory Performance Engine manager ---
include 'mpemanager\singlethread.inc'    ; Dispatcher for single thread
include 'mpemanager\multithread.inc'     ; Dispatcher for multi thread

;--- Data section ---
; For save executable file space, first locate initialized variables. 
; Not-initialized variables is not part of file, but make memory request
; when application start.  

section '.data' data readable writeable

;--- Base point for compact 16-bit offsets in the limited size 64KB block ---
BasePoint:

;--- Copyright and version info ---
ProductID   DB  'NUMA CPU&RAM Benchmarks for Win64',0                                    
AboutCap    DB  'Program info',0
AboutID     DB  'NUMA CPU&RAM Benchmarks' , 0Ah,0Dh
            DB  'v0.97 for Windows x64'   , 0Ah,0Dh
            DB  '(C)2018 IC Book Labs'    , 0Ah,0Dh,0
             
;--- Continue data section, constants pool ---
include 'datasys\sysconstants.inc'     ; Constants for system detection
include 'datasys\cfncdata.inc'         ; Optional system functions - constants 
include 'datavisual\errstart.inc'      ; Error messages used if can't start
include 'datavisual\warnapistr.inc'    ; String for warning about API 
include 'datavisual\resultstr.inc'     ; Strings for simple measurement result
include 'datavisual\namesoptions.inc'  ; Strings names of methods
include 'datavisual\winflags.inc'      ; Pre-blanked flags for dialogue windows
include 'datavisual\drawstr.inc'       ; Strings for Window 1 - benchmarks draw.
include 'datainterface\classes.inc'    ; Classes for user interface objects
include 'datainterface\brushes.inc'    ; Color brushes for user interface
include 'datainterface\bitmaps.inc'    ; Bitmaps for graphics output
include 'datainterface\dialmacro.inc'  ; Macro for dialogues descriptors
include 'datainterface\dialogue0.inc'  ; User dialogue - main window
include 'datainterface\dialogue1.inc'  ; User dialogue - drawings window
include 'datainterface\methlist.inc'   ; List of benchmarks methods by asm.
include 'datainterface\msrepeats.inc'  ; Measurement repeats control

;--- Continue data section, variables pool ---
include 'datasys\vfncdata.inc'         ; Optional system functions - variables
include 'datasys\sysparms.inc'         ; System parameters after detect
include 'datainterface\mpecsb.inc'     ; MPE control and status block
include 'datainterface\subhndl.inc'    ; Dialogue elements handles buffers 
include 'datainterface\exec1.inc'      ; Drawings window execution controls
include 'datavisual\dialmsg.inc'       ; Message descriptor structures
include 'datavisual\drawvars.inc'      ; Variables for Win. 1 - benchmarks draw. 

;--- Continue data section, multifunctional buffer ---
TBufSize  = 49152   ; Required 48 Kilobytes
align 64            ; Aligned by typical cache line size is 64 bytes
TempBuffer  DB  TBufSize DUP (?)

;--- Import section ---
section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, common

;--- Fixups section ---
; This not required for application because it first module, start at
; own address space. Declared load address free, no relocation executed.
; But used for compatibility/regularity.
; For example, relocation required for DLL, 
; because load address can be different  

data fixups
end data

