;------------------------------------------------------------------------------;
; Initializing and start measurement and drawings session,                     ;
; used for draw performance = f( block size ) function.                        ;
; Include run  DrawSessionThread  subroutine as parallel thread                ;
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
call SessionStart

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

; Down comments strings, threads number
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
mov eax,[rbx + IPB.MeasureRepeats]
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
; lea rsi,[NumaU_String]
jrcxz .L16
.L17:
lodsb
cmp al,0
jne .L17
loop .L17
.L16:
sub rsi,r15
mov [r14 + HANDLE_NUMA_MODE1*16 - 256 + 12],si

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
mov ecx,MEASURE_POINTS_COUNT * 2 + 2  ; + 4 dwords: 2 counters + break flag + align
xor eax,eax
rep stosq

; Wait for measurement thread run, this event at thread entry point
; mov rcx,[rsi + DTHP.EventStart]  ; RCX = Parm#1 = Handle for start event
; mov edx,10000                    ; RDX = Parm#2 = 10 seconds, time limit
; call [WaitForSingleObject]
; test rax,rax                     ; special encoding, 0 means event detected OK 
; jnz .Error
; WAIT EVENT MOVED TO WIN1.INC
; OTHERWISE KILL PARENT THREAD (GUI) CAUSE KILL CHILD THREAD (MEASUREMENT),
; AND CHILD THREAD DON'T SIGNALING DONE-EVENT.

; Done, no errors, parallel thread with  DrawSessionThread  subroutine RUNNING
clc
.L90:
mov rsp,rbp
ret

; Exit point for errors detected
.Error:
stc
jmp .L90


