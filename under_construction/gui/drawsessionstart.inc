;--- Initializing and start measurement and drawings session, -----------------;
;    used for draw performance = f( block size ) function.                     ;
;    Include run  DrawSessionThread  subroutine as parallel thread             ;
;                                                                              ;
; INPUT:   None at registers                                                   ;
;          IPB = Input parameters block with benchmark scenario data           ;
;          DialogueContent with user options settings data                     ;
;                                                                              ;
; OUTPUT:  CF = status flag, 0(NC)=No errors, 1(C)=Error                       ;  
;                                                                              ;
; All CPU registers can be corrupted                                           ;
;------------------------------------------------------------------------------;

; TODO.
; Required input-output parameters detalization and error type status definition.

DrawSessionStart:

mov al,1  ; Run context = drawings, with memory allocation multiplication
call GuiToUserParms
call SessionStart     ; Initializing, allocate events, threads, memory
jc .Exit              ; Go if error returned, AL = Error code

; Initializing pointers
cld
lea rbx,[InputParms]
lea r14,[DialogueContent_Win1]
lea r15,[BasePointGui]

; Down comments strings, assembler method
lea rsi,[AM_Strings_Prefix]
mov ecx,[rbx + IPB.UpdatedAsm]
cmp ecx,ASM_LIMIT
jae .L10 
lea rsi,[AM_Strings_Values]
jrcxz .L10
.L11:
lodsb
cmp al,0
jne .L11
loop .L11
.L10:
sub rsi,r15
mov [r14 + HANDLE_TEXT_METHOD1*16 - 256 + 12],si

; Down comments strings, target object (Cache, DRAM, Custom block)
lea rsi,[Unknown_Object]
mov ecx,[rbx + IPB.UpdatedTarget]
cmp ecx,TARGET_LIMIT
jae .L12 
lea rsi,[Target_00]
jrcxz .L12
.L13:
lodsb
cmp al,0
jne .L13
loop .L13
.L12:
sub rsi,r15
mov [r14 + HANDLE_TEXT_OBJECT1*16 - 256 + 12],si

; Down comments strings, number of threads
lea rdi,[Threads_Number]
mov word [rdi],0000h + '?'
mov eax,[rbx + IPB.UpdatedThreads]
cmp eax,MAX_THREADS
ja .L14
push rbx    ; TODO> RBX can be non-optimal because this PUSH/POP requirement
mov bl,0
call DecimalPrint32
pop rbx
.L14:

; Down comments strings, number of measurement repeats
lea rdi,[Repeats_Number]
push rdi 
mov ecx,12
mov al,' '
rep stosb
pop rdi 
mov rax,[rbx + IPB.MeasureRepeats]   ; TODO. Required support overflows RAX=4GB, actual for adaptive modes
push rbx    ; TODO> RBX can be non-optimal because this PUSH/POP requirement
mov bl,0
call DecimalPrint32
pop rbx

; Down comments strings, normal or large pages
lea rax,[NPages_String]
cmp [rbx + IPB.UpdatedLP],LP_USED
jne .L15 
lea rax,[LPages_String]
.L15:
sub rax,r15
mov [r14 + HANDLE_LARGE_PAGES1*16 - 256 + 12],ax

; Down comments strings, NUMA optimization mode
lea rsi,[NumaOff_String]
mov ecx,[rbx + IPB.UpdatedNUMA]
cmp ecx,NUMA_LIMIT
jae .L16 
jrcxz .L16
.L17:
lodsb
cmp al,0
jne .L17
loop .L17
.L16:
sub rsi,r15
mov [r14 + HANDLE_NUMA_MODE1*16 - 256 + 12],si

; Support option, actual for drawings only: adaptive measurement modes
lea rsi,[AdaptiveRepeats]
mov eax,[rbx + IPB.UpdatedMeasure]
sub eax,MEASURE_B_ADAPTIVE
jc .L20
cmp eax,1
ja .L20
mov rax,[rsi + rax*8]
mov [rbx + IPB.AdaptiveSeconds],rax


; Run with recommended repeats count, TODO. Check CF flag, detect errors
; TODO. Repeats count must be 64-bit, see also threads list initialization and
;       performance patterns.
; TODO. Buggy no-interpreting adaptive options cases for "Run simple".
; TODO. Visual settings when adaptive modes.
; TODO. Overflow problems when latency measurement.


; Select Bandwidth or Latency measurement mode
cmp [rbx + IPB.UpdatedAsm],LATENCY_MODE
jae .L22

; Bandwidth mode
lea rcx,[ThreadsList]
mov edx,[rbx + IPB.UpdatedThreads]
call ThreadsReStart
jmp .L21

; Latency mode
; Setup pattern for prepare walk list
.L22:
lea rcx,[ThreadsList]
mov r8d,[rbx + IPB.UpdatedThreads]
mov r9,[rbx + IPB.PatternRoutine]
call ThreadsUpdatePattern
; Run service pattern, prepare walk list, delta TSC ignored here
lea rcx,[ThreadsList]
mov edx,[rbx + IPB.UpdatedThreads]
call ThreadsReStart
; Setup pattern for walk
lea rcx,[ThreadsList]
mov r8d,[rbx + IPB.UpdatedThreads]
mov r9,[rbx + IPB.WalkRoutine]
call ThreadsUpdatePattern
; Run measurement pattern, walk by prepared walk list, get delta TSC in the RAX
lea rcx,[ThreadsList]
mov edx,[rbx + IPB.UpdatedThreads]
call ThreadsReStart
.L21:

; Calculate seconds and ratio after calibration measurement
push rax                            ; RAX = Delta TSC after ThreadsReStart
finit
fld [rbx + IPB.AdaptiveSeconds]     ; ST0 = Target seconds
fild qword [rsp]                    ; ST0 = Measured delta TSC , ST1 = Target seconds
fmul [OutputParms.TSCperiodS]       ; ST0 = Measured seconds
fdivp st1,st0                       ; ST0 = Target / Measured 
fild [rbx + IPB.MeasureRepeats]     ; ST0 = Product repeats
fmulp st1,st0                       ; ST0 = Corrected product
fild [rbx + IPB.StartBlockSize]
fmulp st1,st0                       ; ST0 = Corrected product for divide per bytes count 
fistp [rbx + IPB.AdaptiveProduct]
pop rax
; Skip point for non-adaptive modes
.L20:

; Support option, actual for drawings only: X-approximation modes

; UNDER CONSTRUCTION


; TODO. Optimize RSI/RBX usage.

lea rsi,[InputParms]
lea rdi,[DrawParms]

; Calculate X grid step values, must be integer power of 2 for next step correct 
mov rax,[rsi + IPB.StartBlockSize]
bsr rcx,rax
mov eax,1
shl eax,cl

; Special support for DRAM and Custom modes
; TODO. Make this at block setup, for detect mode single time ?
mov ecx,[rsi + IPB.UpdatedTarget]
cmp ecx,TARGET_DRAM
je .L1
cmp ecx,TARGET_CUSTOM
jne .L2
.L1:
shr eax,3
.L2:

; Set Xstart, Xdelta for drawings
push rax
xor edx,edx
mov ecx,GRIDSTEPX
div rcx
mov [rsi + IPB.StartBlockSize],rax
mov [rsi + IPB.DeltaBlockSize],rax 
pop rax

; Update and store X grid step values
; Select and store X grid units: Bytes, Kilobytes, Megabytes
xor edx,edx                ; EDX = SelectUnits, 0=Bytes / 1=KB / 2=MB
cmp eax,1024
jb @f                      ; Go with Units=Bytes if grid step < 1 KB  
inc edx
shr eax,10
cmp eax,1024
jb @f                      ; Go with Units=KB if grid step < 1 MB
inc edx
shr eax,10
@@:                        ; Otherwise Units=MB
mov [rdi + DRPM.ValueGridX],eax
mov [rdi + DRPM.SelectUnits],edx

; Set first approximation constant for Y grid step values
mov eax,DEFAULT_Y_MBPS_PER_GRID
xor edx,edx
cmp [rsi + IPB.UpdatedAsm],LATENCY_MODE
jb @f
mov eax,DEFAULT_Y_NS_PER_GRID
inc edx
@@:
mov [rdi + DRPM.ValueGridY],eax
mov [rdi + DRPM.SelectMode],edx

; Set Ymultiplier scale constant for conversion
; measurement results to Y-coordinate

; TODO. Move this to win1.inc for re-scaling, 
; correct RDI based reference by new context.

finit
push rax -GRIDSTEPY
fild dword [rsp]      ; ST0 = - grid step, pixels
fidiv dword [rsp+8]   ; ST0 = ( - grid step, pixels ) / ( Y units per grid )      
fstp [rdi + DRPM.Ymultiplier]
pop rax rax

; Align stack and create parameters shadow, required for use WinAPI
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32

; Initializing handles and run thread with DrawSessionThread  subroutine 
lea rsi,[DrawThreadParms]
mov rdi,rsi
cld
; Create event for thread start, store event handle
xor ecx,ecx          ; RCX = Parm#1 = lpEventAttributes  
mov edx,1            ; RDX = Parm#2 = bManualReset
xor r8d,r8d          ; R8  = Parm#3 = bInitialState
xor r9d,r9d          ; R9  = Parm#4 = Name, 0=None
call [CreateEvent]   ; Call WinAPI function
test rax,rax         ; Status=0 means error
jz .Error            ; Go if error creating event
stosq                ; Store handle in the thread parameters, EventStart 
; Create event for thread done, store event handle
xor ecx,ecx          ; RCX = Parm#1 = lpEventAttributes  
mov edx,1            ; RDX = Parm#2 = bManualReset
xor r8d,r8d          ; R8  = Parm#3 = bInitialState
xor r9d,r9d          ; R9  = Parm#4 = Name, 0=None
call [CreateEvent]   ; Call WinAPI function
test rax,rax         ; Status=0 means error
jz .Error            ; Go if error creating event
stosq                ; Store handle in the thread parameters, EventDone 
; Create thread, store event handle
xor ecx,ecx                 ; RCX = Parm#1 = Security
xor edx,edx                 ; RDX = Parm#2 = Stack size, 0=default
lea r8,[DrawSessionThread]  ; R8  = Parm#3 = Execute routine pointer
xor r9d,r9d                 ; R9  = Parm#4 = Subroutine parameter, not used here
push rcx                    ; Parm#6 = Thread ID return , 0=None
push rcx                    ; Parm#5 = Creating Flags = Not suspended, run immediately
sub rsp,32                  ; Make parameters shadow
call [CreateThread]         ; Call WinAPI function
add rsp,32+16               ; Remove parameters shadow + 2 parameters
test rax,rax                ; Status=0 means error
jz .Error                   ; Go if error creating thread
stosq                       ; Store handle in the thread parameters, ThreadHandle 
; Clear measurement counter and array of measurement results
mov ecx,MEASURE_POINTS_COUNT * 2 + 15 - 3  ; OLD: + 4 dwords: 2 counters + break flag + align
xor eax,eax                                ; -3 because 3 qwords already stored
rep stosq

; Done, no errors, parallel thread with  DrawSessionThread  subroutine RUNNING
clc
.L90:
mov rsp,rbp
.Exit:
ret

; Exit point for errors detected
.Error:
stc
jmp .L90


