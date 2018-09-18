;-------------------------------------------------------------------;
; Simple Benchmarks mode run.                                       ;
; Procedure 2 of 3 = SimpleStart, SimpleProgress, SimpleStop        ;
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

SimpleProgress:

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
shl rax,1                       ; This space reserve if Block2 used, if Copy
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

; Setup large pages options
xor eax,eax
cmp [rsi+IPB.OptionLP],2        ; value 2 means available and used
jne @f
inc eax
@@:
mov [rbx+THCTRL.LargePages],rax

; Check threads count validity, select single-thread or multi-thread branch
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

; Get start time point
push 0
mov rcx,rsp
sub rsp,32+8
call [GetSystemTimeAsFileTime]
add rsp,32+8
pop r14

; Execute target performance
push r14
call SingleThreadBenchmark
pop r14
mov [OutputParms.TSCtimerDelta],rax
jc .Error2

; Get stop time point
push 0
mov rcx,rsp
sub rsp,32+8
call [GetSystemTimeAsFileTime]
add rsp,32+8
pop rax
sub rax,r14
mov [OutputParms.OStimerDelta],rax

; Close threads manager, include memory release
lea rbx,[ThreadsControl]
call SingleThreadClose
jc .Error1

; Exit point if no errors, single-thread branch
.ExitRestoreStack:
pop rbp rbp
mov rsp,rbp
ret

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

; Get start time point
push 0
mov rcx,rsp
sub rsp,32+8
call [GetSystemTimeAsFileTime]
add rsp,32+8
pop r14

; Execute target performance
push r14
mov ecx,[InputParms.NumThreads]
call MultiThreadRestart
pop r14
mov [OutputParms.TSCtimerDelta],rax
jc .Error2

; Get stop time point
push 0
mov rcx,rsp
sub rsp,32+8
call [GetSystemTimeAsFileTime]
add rsp,32+8
pop rax
sub rax,r14
mov [OutputParms.OStimerDelta],rax

; Close threads manager, include memory release
lea rbx,[ThreadsControl]
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
