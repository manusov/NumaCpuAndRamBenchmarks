;------------------------------------------------------------------------------;
; Callback handler for main-parent window                                      ;
; This subroutine called by OS,                                                ; 
; when application call DispatchMessage subroutine                             ;
; WM = Window Message                                                          ; 
; Callback procedure parameters:                                               ;
; Parm#1 = RCX = hwnd = Handle to the window                                   ;
; Parm#2 = RDX = uMsg = System defined message                                 ;
; Parm#3 = R8  = wParam = Additional message information (1), uMsg-dependent   ;
; Parm#4 = R9  = lParam = Additional message information (2), uMsg-dependent   ;
; Output = RAX = Status, encoding = f(message type)                            ; 
;------------------------------------------------------------------------------;

WindowProc_Win0:
push rbx rsi rdi rbp r12 r13 r14 r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
push rbp
sub rsp,32+8          ; Parameters shadow for call API + alignment
mov rbx,rcx           ; RBX = Storage for hwnd
;--- Select handler ---
cmp rdx,WM_CREATE
je WmCreate_Win0      ; Go if WM (Window Message) = OPEN WINDOW
cmp rdx,WM_DESTROY
je WmDestroy_Win0     ; Go if WM = CLOSE WINDOW
cmp rdx,WM_COMMAND
je WmCommand_Win0     ; Go if WM = USER INPUT
;--- Otherwise Default OS routine ---
; Here must be valid input RCX, RDX, R8, R9
DefWndProc_Win0:
call [DefWindowProc]  ; DEFAULT WINDOW PROCEDURE
;--- Exit points ---
Finish_Win0:  ; No EAX destroy here, otherwise window not closed at WinXP, Win7
add rsp,32+8  ; Remove parameters shadow + alignment
pop rbp       ; Can directly use POP RSP ?
mov rsp,rbp
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret
Status_Win0:
mov eax,1
jmp Finish_Win0

;--- Handler for create window (open) ---
WmCreate_Win0:		    
;--- Built user interface objects ---
lea rsi,[DialogueContent_Win0]     ; RSI = Pointer to elements descriptors
mov edi,START_HANDLE_WIN0          ; RDI = Start handle
mov r14,[Dialogue_Win0.hInstance]  ; R14 = Parent module handle = [wc.hInstance]
call CreateDialogueElements
call SetDefaults_Win0              ; Set defaults
;--- Exit ---
Finish1_Win0:
jmp Finish_Win0

;--- Handler for destroy window (close) ---
WmDestroy_Win0:
xor ecx,ecx                        ; Parm#1 = RCX = Exit code
call [PostQuitMessage]
xor eax,eax
Finish2_Win0:
jmp Finish1_Win0

;--- Handler for user input ---
; Buttons: Run Simple, Run Drawings, Defaults, Close buttons
; Also used for make active/inactive block size edit string.
; Also used for extract and store variables = F(options state)
;---
WmCommand_Win0:
;--- Check phase of execution ---
cmp byte [Win0_Init],1
jne DefWndProc_Win0     ; Run default OS handler if Window 0 not initialized yet
;--- Handling if Window 0 already initialized ---
mov r14,r8              ; R14 = Store event code
;--- Activate custom block radio button if edit string active ---
call [GetFocus]
cmp rax,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]
jne  @f
mov rcx,rbx
mov edx,HANDLE_L1_CACHE
mov r8d,HANDLE_CUSTOM_BLOCK
mov r9d,r8d
call [CheckRadioButton]
@@:
;--- Detect state of "Custom Size" radio button ---
; Use fact: BST_UNCHECKED=0, BST_CHECKED=1
mov esi,HANDLE_CUSTOM_BLOCK  ; RSI = Pointer to Handle
call SendMessageGetCheck
mov edi,0                 ; Can.t XOR EDI,EDI because must save ZF flag
jne @f                    ; Skip set EDI=1 if not a BST_CHECKED
inc edi                   ; BST_CHECKED=1, if CHECKED set EDI=1, otherwise EDI=0
@@:
;--- Set state for block size edit string ---
mov rcx,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]  ; RCX = Parm#1 = Handle
mov edx,WM_ENABLE         ; RDX = Parm#2 = Message
mov r8d,edi               ; R8  = Parm#3 = Select: 0=Disable, EDI from previous step
xor r9d,r9d               ; R9  = Parm#4 = Not used
call [SendMessage]                  
;--- Set or clear READ ONLY state for edit string
mov rcx,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]  ; RCX = Parm#1 = Handle
mov edx,EM_SETREADONLY    ; RDX = Parm#2 = Message
xor edi,0001b             ; Invert bit D0
mov r8d,edi               ; R8 = Parm#3 = Select: 0=Clear READ ONLY, 1=Set READ ONLY
xor r9d,r9d               ; R9 = Parm#4 = Not used = 0
call [SendMessage]
;--- Check event source ---
cmp r14,START_HANDLE_WIN0
jb DefWndProc_Win0     ; Skip if other handles, otherwise WM_SETTEXT cause error
cmp r14,HANDLE_BUTTON_ABOUT   ; - PATCH- HANDLE_BUTTON_CANCEL
ja DefWndProc_Win0
;--- Extract CUSTOM BLOCK SIZE as text string ---
;--- Get text ---
lea rsi,[BigTexts]  ; RSI = Buffer base address
mov rcx,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]  ; RCX = Parm#1 = Handle
mov edx,WM_GETTEXT  ; RDX = Parm#2 = Command
mov r8d,8+1         ; R8  = Parm#3 = Max. number of chars, include 0=terminator
mov r9,rsi          ; R9  = Parm#4 = Pointer to buffer
call [SendMessage]
;--- Extract editable block size for "Run" button ---
; Here RSI valid string base, Calculate CUSTOM BLOCK SIZE as number
push rsi
mov ecx,8           ; RCX = String Length
xor edx,edx         ; EDX = Value, this compact clear entire 64-bit RDX
cld
.ScanValue:
lodsb
cmp al,0
je .DoneValue
cmp al,'0'
jb .NotNum          ; Go if not in range "0-9"
cmp al,'9'
jna .StoreValue     ; Go if in range "0-9"
.NotNum:
and al,11011111b    ; Convert "a-f" to "A-F"
cmp al,'A'
jb .NotChar         ; Go if not in range "A-F"
cmp al,'F'
jna .StoreChar      ; Go if in range "A-F"
.NotChar:                 
mov al,'A'-10       ; This value produce 0
.StoreChar:
sub al,'A'-10
.StoreValue:
and eax,00001111b
shl edx,4           ; Positioning for next hex digit
or edx,eax          ; Store current hex digit
loop .ScanValue     ; Cycle for calculate value = F(ASCII string)
.DoneValue:
mov [BlockSizeEdit],rdx   ; Store value = custom block size
pop rdi
;--- Built ASCII hex number ---
mov r9,rdi          ; R9 = Parm#4 = Source buffer
xchg eax,edx
call HexPrint32
mov al,0
stosb
;--- Send back to edit field, for correct wrong input ---
mov rcx,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]  ; RCX=Parm#1=Dest. element handle
mov edx,WM_SETTEXT        ; RDX = Parm#2 = Command, "WM" means Window Message
xor r8d,r8d               ; R8  = Parm#3 = Don't used
call [SendMessage]
;--- Detect "Run Simple", "Run Drawings", "Defaults" and "Cancel" buttons ---
cmp r14,HANDLE_BUTTON_RUN_SIMPLE
je RunS_Win0              ; Go if "Run Simple" button pressed
cmp r14,HANDLE_BUTTON_RUN_DRAWINGS
je RunD_Win0              ; Go if "Run Drawings" button pressed
cmp r14,HANDLE_BUTTON_DEFAULTS
je Defaults_Win0          ; Go if "Defaults" button pressed
cmp r14,HANDLE_BUTTON_CANCEL
je WmDestroy_Win0         ; Go if "Cancel" button pressed
cmp r14,HANDLE_BUTTON_ABOUT
je About_Win0             ; Go if "About" button pressed
jmp DefWndProc_Win0       ; Otherwise default OS handler


;---------- Handling button #1 = Run Simple -----------------------------------;
RunS_Win0:
;--- Disable parent window when daughter window work --- Changed at v0.94 ---
mov rcx,rbx                   ; RCX = Parm#1 = Handle
xor edx,edx                   ; RDX = Parm#2 = 0, means DISABLE
call [EnableWindow]
push rbx                ; save handle for restore parent window activity v0.94

;--- Warning about engineering sample ---
;- xor ecx,ecx	              ; Parm#1 = Parent window handle or 0
;- lea rdx,[RevisionID]       ; Parm#2 = Pointer to string
;- lea r8,[ProductID]         ; Parm#3 = Pointer to caption
;- mov r9d,MB_ICONWARNING     ; Parm#4 = Message box icon type
;- call [MessageBoxA]
;--- Set benchmarks options = F (User dialogue results) ---
call ModeSet
;--- Measure CPU clock ---
; This operation before get current time, because:
; exclude 1000 ms interval and synchronization with ticks
;---
lea rdi,[MpeTscClockHz]
call MeasureCpuClk
mov rax,[rdi-16]
xor edx,edx
mov ecx,1000*100         ; RCX divisor for convert Hz to MHz/10
div rcx
mov [dTSCsec],eax        ; Write CPU Frequency
;--- Prepare input, outside critical operation ---
lea rbx,[ICBB]           ; RBX = ICBB, Input Control Block Base
mov rsi,[rbx+00]         ; RSI = Block #1 pointer, this for Single Thread only
mov rdi,[rbx+08]         ; RDI = Block #2 pointer
mov rax,[rbx+16]         ; RAX = Block size, bytes

; INT3 ; DEBUG for fix Multithread Size Bug
; This bug created at v0.90 (performance heating), fixed at v0.91.
; Note about target blocks addressing.
; For Single-Thread, subroutine ExecuteBenchmarkST use input RSI from ICBB.
; For Multi-Thread, subroutine ExecuteBenchmarkMT use Block Base from
; ThreadsControl block, include support NUMA optimal block assignment.

mov r12,rax                 ; R12 = Backup block per thread size,
                            ;       this actual for Multi-Thread
xor edx,edx                 ; RDX = 0 , dividend extension
mov ecx,[rbx+24]            ; RCX = Bits per Instruction
shr ecx,3                   ; RCX = Bytes per Instruction
div rcx                     ; RAX = Instructions per Block
xchg r14,rax                ; R14 = RAX = Block length, units = instructions 
mov ebp,[rbx+32]            ; RBP = Number of measurement repeats
mov rbx,[PatternSelector]   ; RBX = Target subroutine address
;--- Get start timepoint, enter to critical operation ---
mov r15,rsp                 ; R15 used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h  ; Align stack because WinAPI used
sub rsp,32                  ; Make parameters shadow because WinAPI used

;--- START HEATING: load parameters run performance pattern -------------------;
; This PUSH-POP possible because routines between don't require
; parameters shadow or create it internally.
;--- 
push rbx rdx rsi rdi rbp r8 r9 r10 r11 r12 r13 r14 r15
;--- 
mov rcx,r14                 ; RCX = Block length, units = instructions
mov rax,r12                 ; RAX = Block length, units = bytes,
                            ;       this actual for Multi-Thread blk. allocation
;--- Select Single/Multi Thread ---
cmp [NumThreads],1
ja .HeattingMultiThread     ; Go if multi-thread branch
;--- Single Thread branch ---
.HeattingSingleThread:
call ExecuteBenchmarkST     ; Single Thread: Return = RAX = Delta-TSC, clocks
jmp .HeattingAfterThreads 
;--- Multi Thread branch ---
.HeattingMultiThread:
call ExecuteBenchmarkMT     ; Multi Thread: Return = RAX = Delta-TSC, clocks
.HeattingAfterThreads:
;---
pop r15 r14 r13 r12 r11 r10 r9 r8 rbp rdi rsi rdx rbx
;--- END HEATING: Single/Multi Thread branches convergention point ------------;

lea rcx,[MpeOsTime]             ; Pointer to output updated variable
call [GetSystemTimeAsFileTime]  ; Call for interval BEGIN


;--- START MEASURE: load parameters run performance pattern -------------------;
; All registers volatile except R15
;---
mov rcx,r14                 ; RCX = Restore Block length, units = instructions
mov rax,r12                 ; RAX = Restore Block length, units = bytes
                            ;       this actual for Multi-Thread

;--- Select Single/Multi Thread ---
cmp [NumThreads],1
ja .MultiThread             ; Go if multi-thread branch
;--- Single Thread branch ---
.SingleThread:
call ExecuteBenchmarkST     ; Single Thread: Return = RAX = Delta-TSC, clocks
jmp .AfterThreads 
;--- Multi Thread branch ---
.MultiThread:
call ExecuteBenchmarkMT     ; Multi Thread: Return = RAX = Delta-TSC, clocks
.AfterThreads:
;--- END MEASURE: Single/Multi Thread branches convergention point ------------;

mov r14,rax                 ; R14 = Delta TSC
;--- Get stop timepoint, exit from critical operation ---
lea rcx,[MpeOsTime]
mov rsi,rcx
mov rdi,[rsi]                    ; RDI = Previous value, time BEFORE
call [GetSystemTimeAsFileTime]   ; Get next value, time AFTER

;--- Remove parameters shadow, restore RSP ---
mov rsp,r15
pop rbx                     ; From v0.94, handle for restore window

;--- Calculate and store: delta-time from units=100ns to units = 1 ms ---
; Note when try use units ms/10 occurred OS granularity = 1 ms
; even when units = 100 ns.
;---
mov rax,[rsi]            ; RAX = Time AFTER
sub rax,rdi              ; RAX = Delta T = AFTER - BEFORE
mov r12,rax              ; R12 = Delta T, units = 100 ns, save for later use
xor edx,edx
mov ecx,10000
div rcx
lea rsi,[ICBB]           ; RSI = ICBB = Input Control Block Base
lea rdi,[OCBB]           ; RDI = OCBB = Output Control Block Base
mov [rdi+00],eax         ; Write delta-T, units = ms/10
;--- Calculate and store: TSC clocks per Instruction ---
; Note item "clocks per iteration" rejected
;--- Blank ---
xor eax,eax
mov [rdi+12],eax         ; Pre-blank CPI for correct skip branches
mov [rdi+16],eax         ; Pre-blank MBPS for correct skip branches 
;--- Total size with measurement repeats, bytes ---
mov rax,[rsi+16]         ; RAX = Block size, bytes
mov ecx,[rsi+32]         ; RCX = Number of measurement repeats (zero-extended)
mul rcx                  ; RDX:RAX = Total number of instructions executed
test rdx,rdx
jnz @f                   ; Go skip if overflow, must be RDX=0
mov r13,rax              ; R13 = Total bytes, save for next operation
;--- Total number of instructions ---
mov ecx,[rsi+24]         ; RCX = Bits per instruction
shr ecx,3                ; RCX = Bytes per instruction 
div rcx
xchg rcx,rax             ; RCX = Total number of instructions ---
;--- Clocks*1000 ---
mov eax,1000
mul r14                  ; RDX:RAX = Clocks*1000
test rdx,rdx
jnz @f                   ; Go skip if overflow, must be RDX=0
;--- Clocks per Instructions * 1000 ---
div rcx          ; (RDX:RAX)/RCX = ( Total clocks / Total instructions ) * 1000
shld rdx,rax,32
test edx,edx
jnz @f                   ; Go skip if overflow
;--- Store ---
mov [rdi+12],eax         ; Store ( Clocks / Instruction ) * 1000
;--- Calculate and store: Speed, Megabytes per Second ---  
; At this point: R13=Total bytes, R12=Total units 100 ns
; Don't use TSC for real time measurement, because Turbo Boost
;--- Check for Multi-Thread
; YET BUG = NUMBER OF THREADS = 16 = CONST
;---
cmp [NumThreads],1
jbe @f
imul r13,r13,CONST_THREADS
@@:
;--- Pre checks ---
test r12,r12             ; R12 = Total time units = 100 ns
jz @f                    ; Go skip if error: if delta time = 0
test r13,r13             ; R13 = Total bytes transferred
jz @f                    ; Go skip if error: if transfer size = 0
;--- Divide with convert to MBPS (Megabytes per Second) ---
; Current size units = bytes, required = MB, need: Size / 1048576
; Current time units = 100 ns, required = seconds, need: Time / 10^7
; For convert value X = Size / Time , required:
; X / 1048576
; X * 10000000
; First make multiply for improve precision
;---
mov eax,10000000
mul r13                  ; RAX:RDX = Total size * 10^7
cmp rdx,r12
jae @f                   ; Go skip for prevent divide overflow
div r12                  ; RAX = Size / Time
mov ecx,1048576
xor edx,edx
div rcx
mov [rdi+16],eax         ; Store MBPS
@@:

;--- Show benchmarks results ---
call ShowSingleResult
;--- Exit after simple test works ---
; jmp Finish2_Win0
jmp RestoreActive_Win0        ; Changed at v0.94, maximum one window called

;---------- Handling button #2 = Run Drawings ---------------------------------;
RunD_Win0:
;--- Warning about engineering sample ---
;- xor ecx,ecx	              ; Parm#1 = Parent window handle or 0
;- lea rdx,[RevisionID]       ; Parm#2 = Pointer to string
;- lea r8,[ProductID]         ; Parm#3 = Pointer to caption
;- mov r9d,MB_ICONWARNING     ; Parm#4 = Message box icon type
;- call [MessageBoxA]
;--- Set benchmarks options = F (User dialogue results) ---
call ModeSet
;--- Set string visual under drawings sub-window: ASM method ---
cld
lea r15,[BasePoint]
;-
; Changed at v0.93 (method name selection for DRAWINGS mode: graphics output)
;-
; lea rsi,[Unknown_Method]      ; RSI = Pointer to methods strings sequence
; mov ah,[SelectMethod]         ; AH = OptionMethod
; cmp ah,16
; ja .BadMethod
; inc ah
; .SelectMethod:                ; First string always skipped
; lodsb
; cmp al,0
; jne .SelectMethod             ; Find zero for skip one string
; dec ah
; jnz .SelectMethod             ; Skip required number of strings
; .BadMethod:
;-
mov al,18
mul [NonTemporalRead]    ; Supports non-temporal read for AMD
mov cl,al
;-
push rbx
lea rsi,[AM_Strings]     ; RSI = Pointer to strings sequence
lea rbx,[AM_Selector]    ; RBX = Pointer to array of strings numbers
mov al,[SelectMethod]    ; RAX = OptionMethod, CPU instruction set
cmp al,16
ja .BadMethod
inc eax                  ; + 1 because  means first method, not a INVALID value
cmp [OptionTarget],3     ; If Object=RAM=3 force non-temporal mode
jne @f                   ; Skip for other types: L1/L2/L3/Custom
add al,18                ; Skip first 18 entries: Unknown method + 17 methods
add al,cl
;push rax  ; OPTIMIZE THIS
;mov al,18
;mul [NonTemporalRead]
;mov cl,al
;pop rax
;add al,cl
@@:
xlat [rbx]               ; AL = Load byte from [ RBX+AL ]
cmp al,29  ; 18+6-1      ; AH = Limit = last acceptable value
ja .BadMethod
mov ah,al
.SelectMethod:           ; First string always skipped
lodsb
cmp al,0
jne .SelectMethod        ; Find zero for skip one string
dec ah
jnz .SelectMethod        ; Skip required number of strings
.BadMethod:
pop rbx
;-
sub rsi,r15
mov [DialogueContent_Win1+HANDLE_TEXT_METHOD1*16-256+12],si
;--- Set string visual under drawings sub-window: Object (L1,L2,L3,RAM, Custom) ---
lea rsi,[Unknown_Object]      ; RSI = Pointer to objects strings sequence
mov ah,[OptionTarget]         ; AH = OptionBlock
movzx edx,ah                  ; DL = Copy for later use
cmp ah,4
ja .BadObject   
inc ah
.SelectObject:                ; First string always skipped
lodsb
cmp al,0
jne .SelectObject
dec ah
jnz .SelectObject 
.BadObject:
sub rsi,r15
mov [DialogueContent_Win1+HANDLE_TEXT_OBJECT1*16-256+12],si
;--- Set number of instructions for current X ---
; This default settings for prevent use undefined state of this variable,
; benchmark procedure overwrite it when drawing Y=F(X)
;---
mov [BenchInstructions],1
;--- Print number of repeats ---
push rbx rdi
lea rdi,[Repeats_Number]
push rdi 
mov ecx,9
mov al,' '
rep stosb
pop rdi 
mov bl,0
mov eax,[NumRepeats]
call DecimalPrint32
pop rdi rbx
;--- Set bytes per instruction(s) or hardware iteration cycle ---
mov eax,[OperandWidth]
shr eax,3                     ; Convert from bits to bytes
mov [BenchBPI],rax
;--- Set instructions per pixel X and other X-parameters ---
; Example#1 : 1 unstruction = 32 bytes , 32 * 2 = 64 bytes per pixel for X
; Example#2 : 1 unstruction = 8 bytes  , 8 * 8 = 64 bytes per pixel for X
;---
xchg ecx,eax                  ; ECX = Bytes per Instruction
;--- Calculating BPP = Bytes per pixel, get block size first ---
; Here valid RDX = OptionBlock
;---
mov rax,[BlockSizeEdit]                    ; RAX = Custom Block size
cmp dl,4
jae .SizeReady                             ; Go if selected object = Custom or 4+
mov rax,[r15-BasePoint+MemoryRequestSize]  ; RAX = RAM Block size
;--- Fix memory address overflow in the multithread mode ---
; This make effect if analysed object = memory only
cmp [NumThreads],1           ; Check Enable Parallel Threads, 1=Enabled
jbe @f                       ; Go skip if single thread, execute SHR if parallel threads
shr rax,4                    ; RAX/16, this for const only: CONST_THREADS = 16
@@:
;---
and rax,0FFFFFFFFFFFFE000h   ; Clear alignment addend , must be /2 because st/mt and /2 for centering
shr rax,2                    ; RAX = Block size / 4  , for multi-thread or single-thread region and maximum limit
cmp dl,3
je .SizeReady                ; Go if selected object = RAM
imul edx,edx,12              ; 12 bytes per cache entry
mov rax,[rdx+CacheL1Data]    ; RAX = Cache size, bytes, L1,L2,L3
.SizeReady:
;---
; RAX = Analysed object size, this size must be at X-center of screen
; BPP = Bytes per Pixel = RAX / Pixels at half X
; For prevent approximation, BPP must be aligned by 64
;---
mov esi,SUBWINX/2
xor edx,edx
div esi                      ; EAX = BPP, Bytes per pixel 
;--- Select units: Bytes, Kilobytes, Megabytes ---
lea rdi,[SelectUnits]
mov byte [rdi],0             ; BYTE [SelectUnits] = 0 , Bytes/KB/MB
cmp eax,512
jb @f                        ; Go if BPP < 0.5K , Units=Bytes 
inc byte [rdi]
add eax,0000003FFh
and eax,0FFFFFC00h
cmp eax,512*1024             ; Go if BPP < 0.5M , Units=Kilobytes
jb @f
inc byte [rdi]
add eax,0000FFFFFh
and eax,0FFF00000h
@@:                          ; Otherwise Units=Megabytes
;--- Here: EAX=BPP, ECX=BPI, set instructions per pixel ---
add eax,00000003Fh
and eax,0FFFFFFC0h           ; EAX = BPP, aligned by 64 
push rax
cdq                          ; Clear EDX = 0
div ecx                      ; EAX = BPP / BPI 
mov dword [Xmultiplier],eax
pop rax
;--- Set default Maximum MBPS for Y-sizing at first pass ---
; Note at next passes Y-size selected automatic as F(Maximum Detected MBPS)
;---
mov [MaximumMBPS],0 
;--- Set units per grid X ---
; .XSettings:
mov edx,GRIDSTEPX
;--- Override defaults for Cache&RAM mode, Set units per grid Y ---
mov [ValueGridY],DEFAULT_Y_MBPS_PER_GRID
;--- Set total MBPS per Y, for all sub-screen ---
mov [Ydivisor],DEFAULT_Y_MBPS_PER_PIXEL 
;--- Continue for Grid X ---
mov cl,[rdi]  ; [rdi+019]
imul ecx,ecx,10
shr eax,cl
mul edx
xchg edx,eax
mov [ValueGridX],edx
;--- Disable parent window when daughter window work ---
mov rcx,rbx                  ; RCX = Parm#1 = Handle
xor edx,edx                  ; RDX = Parm#2 = 0, means DISABLE
call [EnableWindow]
;--- Create (Open) daughter window = Window 1 ---
; Built child window for drawings Speed=F(Size)
; Terminology notes: WS = Window Style
;---
lea rsi,[WindowTitle_Win1]    ; RSI = Window name string pointer
lea rdi,[Dialogue_Win1]       ; RSI = Dialogue descriptor pointer
mov r10d,WS_VISIBLE+WS_DLGFRAME+WS_SYSMENU ; R10 = Window Style
mov r11d,WIN1_XBASE           ; R11 = Window X-base
mov r12d,WIN1_YBASE           ; R12 = Window Y-base
mov r13d,WIN1_XSIZE           ; R13 = Window X-size
mov r14d,WIN1_YSIZE           ; R14 = Window Y-size
mov ax,0101h                  ; AH  = 1 , Means parent=RBX
xchg al,[Win1_Init]           ; AL  = 1 , if window 1 already initialized
call CreateApplicationWindow  ; RBX = Parent Handle
;--- Wait events for Window 1 ---
lea rdi,[DialogueMsg_Win1]	  ; RDI = MSG structure
call WaitEvent                ; Window 1 work as callbacks inside this
;--- Enable and activate parent window ---
RestoreActive_Win0:
mov rcx,rbx                   ; RCX = Parm#1 = Handle
mov edx,1                     ; RDX = Parm#2 = 1, means ENABLE
call [EnableWindow]
mov rcx,rbx                   ; RCX = Parm#1 = Handle
mov edx,WM_ACTIVATE           ; RDX = Parm#2 = Message
mov r8d,WA_CLICKACTIVE        ; R8  = Parm#3 = Activation method
xor r9d,r9d                   ; R9  = Parm#4 = 0 , not used
call [SendMessage]
;--- Exit after drawings works ---
jmp Finish2_Win0


;---------- Handling button #3 = Defaults -------------------------------------;
Defaults_Win0:
call SetDefaults_Win0
jmp Finish2_Win0


;---------- Handling button #4 = Cancel ---------------------------------------;
; Already defined.


;---------- Handling button #5 = About ----------------------------------------;
About_Win0:
;--- Disable parent window when daughter window work --- Changed at v0.94 ---
mov rcx,rbx                   ; RCX = Parm#1 = Handle
xor edx,edx                   ; RDX = Parm#2 = 0, means DISABLE
call [EnableWindow]
;--- Message box and wait user input ---
xor ecx,ecx	                  ; RCX = Parm#1 = Parent window handle or 0
lea rdx,[AboutID]             ; RDX = Parm#2 = Pointer to "About" string
lea r8,[AboutCap]             ; R8  = Parm#3 = Pointer to caption
mov r9d,0040h                 ; R9  = Parm#4 = Message box icon type = Info
call [MessageBoxA]
; jmp Finish2_Win0
jmp RestoreActive_Win0        ; Changed at v0.94, maximum one window called

