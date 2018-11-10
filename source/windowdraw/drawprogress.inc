;-------------------------------------------------------------------;
; Benchmarks drawings speed=f(size or address),                     ; 
; process execution phase                                           ;
; Procedure 2 of 3 = DrawStart, DrawProgress, DrawStop              ;
;                                                                   ;
; INPUT:   Threads management entries list.                         ;
; OUTPUT:  CF = Error flag: 0(NC)=OK, 1(C)=Execution error          ;
;          AL = Error code, valid if CF=1(C)                        ;
;               00 = Memory allocation error                        ;
;               01 = Memory release error                           ;
;               02 = Time measurement error                         ;
;               04 = Address arithmetics error                      ;
;               04-0FFh = Reserved                                  ;
;          Threads management entries list.                         ;
;-------------------------------------------------------------------;

DrawProgress:

mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
push rbp rbp

; Clear control structures, size/8 because REP STOSQ (QWORDS)
cld
lea rdi,[ThreadsControl]
mov rbx,rdi
xor eax,eax
mov ecx,( THREADS_CONTROL_SIZE + EVENTS_CONTROL_SIZE + NUMA_CONTROL_SIZE ) / 8
rep stosq

; Start build threads management block
lea rsi,[InputParms]

; Setup target benchmarks routine entry point, BasePointCode already added
mov rax,[PatternRoutine]
mov [rbx+THCTRL.EntryPoint],rax

; Prepare aligned memory offsets and sizes before allocation
mov rax,[rsi+IPB.BlockSize]  ; RAX = Block size, bytes, unaligned
push rax
test eax,0FFFh
jz @f
and rax,0FFFFFFFFFFFFF000h
add rax,00000000000001000h
@@:  
mov [rbx+THCTRL.Base1],0        ; Block1 single for Read, Write, Source for Copy
mov [rbx+THCTRL.Base2],rax      ; Block2 used only for Copy as Destination
shl rax,1 + 2                   ; This space reserve if Block2 used, if Copy + drawings reserve
mov [rbx+THCTRL.SizeBytes],rax  ; Total memory size for allocation   
pop rax

; Prepare instructions count as function of block size and instruction width
; Must be valid previous RAX = unaligned block size
mov ecx,[rsi+IPB.OperandWidth]
shr ecx,3                       ; Convert width, bits to width, bytes
jz .Error3                      ; Go error if width = 0, prevent divide by zero
xor edx,edx                     ; RDX = 0, dividend high qword
div rcx                         ; RAX = Number of instructions
test rdx,rdx                    ; Check RDX = mod after divide
jnz .Error3                     ; Go error if unaligned by instruction width 
mov [rbx+THCTRL.SizeInst],rax   ; Number of instructions per target block size

; Setup measurement repeats count
mov eax,[rsi+IPB.NumRepeats]
mov [rbx+THCTRL.Repeats],rax

; Check threads count validity, 
; select single-thread or multi-thread branch for open
mov eax,[rsi+IPB.NumThreads]
test eax,eax
jz .Error3                     ; Go error if number of threads = 0
cmp eax,64
ja .Error3                     ; Go error if number of threads > 64
cmp eax,1
ja .MultiThread                ; Go multi-thread branch if threads count > 1

; Single-thread branch
.SingleThread:

; Open (initializing) threads manager, include memory allocation
call SingleThreadOpen
jc .Error0

; Get memory allocation results for visual
mov rax,[rbx+THCTRL.Base1]
mov [rsi+IPB.BlockBase1],rax
mov rax,[rbx+THCTRL.Base2]
mov [rsi+IPB.BlockBase2],rax 

; Re-measure TSC clock before start benchmarks
lea rdi,[OutputParms.TSCfrequencyHz]
call MeasureCpuClk
jc .Error2

; Pre-heat phase for result stabilization
push rbx
call SingleThreadBenchmark
pop rbx
jc .Error2
jmp .DrawingsPhase

; Multi-thread branch
.MultiThread:

; Select number of threads = f( current threads, hyper-threading )
mov ecx,[InputParms.NumThreads] 
test ecx,1
jnz @f                      ; Skip if odd number of threads, count unchanged
cmp [InputParms.OptionHT],1
jne @f                      ; Skip if HT control not a "present but not used" 
shr ecx,1
mov [InputParms.NumThreads],ecx
@@:

; Print number of threads, see also at drawstart.inc
; This required for hyper-threading correction
push rbx rdi
lea rdi,[Threads_Number]  ; RDI = Pointer to text string
mov word [rdi],'? '
mov byte [rdi+2],0
xchg eax,ecx
test eax,eax
jz @f                 ; Skip if number of threads = 0
cmp eax,64
ja @f                 ; Skip if Number of threads > 64 (means error, not sup.)
mov bl,0
call DecimalPrint32
@@:
pop rdi rbx

; Scaling block size and instructions count by number of threads
mov rax,[rbx+THCTRL.SizeBytes]
mul rcx
mov [rbx+THCTRL.SizeBytes],rax
mov rax,[rbx+THCTRL.SizeInst]
mul rcx
mov [rbx+THCTRL.SizeInst],rax

; Open (initializing) threads manager, include memory allocation
; At this point valid RBX=Control list pointer, ECX=Threads count
push rbx rsi
call MultiThreadOpen
pop rsi rbx
jc .Error0

; Get memory allocation results for visual
mov rax,[rbx+THCTRL.Base1]
mov [rsi+IPB.BlockBase1],rax
mov rax,[rbx+THCTRL.Base2]
mov [rsi+IPB.BlockBase2],rax 

; Re-measure TSC clock before start benchmarks
lea rdi,[OutputParms.TSCfrequencyHz]
call MeasureCpuClk
jc .Error2

; Pre-heat phase for result stabilization
push rbx
mov ecx,[InputParms.NumThreads]
call MultiThreadBenchmark
pop rbx
jc .Error2

; DrawingsPhase
.DrawingsPhase:

; Create (Open) daughter window = Window 1
; Build child window for drawings Speed=F(Size)
; Terminology notes: WS = Window Style
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
; Wait events for Window 1
lea rdi,[DialogueMsg_Win1]	  ; RDI = MSG structure
call WaitEvent                ; Window 1 work as callbacks inside this

; Select single-thread or multi-thread branch for close
lea rbx,[ThreadsControl]
lea rsi,[InputParms]
cmp [rsi+IPB.NumThreads],1
ja .CloseMultiThread

; Close threads manager, include memory release, single-thread mode
.CloseSingleThread:
; Close threads manager, include memory release, single-thread mode
call SingleThreadClose
jc .Error1

; Exit point if no errors, single-thread branch
.ExitRestoreStack:
pop rbp rbp
mov rsp,rbp
ret
ret

; Close threads manager, include memory release, multi-thread mode
.CloseMultiThread:
; Close threads manager, include memory release, multi-thread mode
mov ecx,[InputParms.NumThreads]
call MultiThreadClose
jc .Error1

; Exit point if no errors, multi-thread branch
@@:
jmp .ExitRestoreStack

; Exit points for errors, used from both single-thread and multi-thread branches
.Error0:    ; Memory allocation error
mov al,0
jmp @b
.Error1:    ; Memory release error
mov al,1
jmp @b
.Error2:    ; Timings measurement error
mov al,2
jmp @b
.Error3:    ; Address arithmetic error
mov al,3
jmp @b



