;--- Initializing and start measurement and drawings session, ---------------;
;    used for draw performance = f( block size ) function.                   ;
;    Include run  DrawSessionThread  subroutine as parallel thread           ;
;                                                                            ;
; INPUT:   None at registers                                                 ;
;          DialogueContent with user options settings data                   ;
;                                                                            ;
; OUTPUT:  CF = status flag, 0(NC)=No errors, 1(C)=Error                     ;
;          AL = error status, valid only if error detected (when CF=1)       ; 
;               see file "thread manager\sessionstart.inc" for decode        ;
;          UPB = Update user parameters block with GUI widgets settings      ;
;          IPB = Update input parameters block with benchmark scenario data  ;   
;                                                                            ;
; UPB = f (GUI) , IPB = f (UPB)                                              ;
;                                                                            ;
; All CPU registers can be corrupted, this rule for complex modules:         ;
; directory threads_manager\:                                                ;
;   sessionstart.inc, sessionstop.inc, sessionprogress.inc.                  ;
; directory gui\:                                                            ;
;   drawsessionstart.inc, drawsessionstop.inc                                ;
;   but not drawsessionthread.inc because OS callback                        ;       
; Remember about PUSH-POP in the callers.                                    ;
;----------------------------------------------------------------------------;

DrawSessionStart:

mov al,1  ; Run context = drawings, with memory allocation multiplication
call GuiToUserParms   ; Update UPB
call SessionStart     ; Initializing, allocate events, threads, memory, IPB
jc .Exit              ; Go if error returned, AL = Error code

; Initializing pointers
cld
lea ebx,[InputParms]

; Down comments strings, assembler method
lea esi,[AM_Strings_Prefix]
mov ecx,[ebx + IPB.UpdatedAsm]
cmp ecx,ASM_LIMIT
jae .L10 
lea esi,[AM_Strings_Values]
jecxz .L10
.L11:
lodsb
cmp al,0
jne .L11
loop .L11                ; find string indexed by RCX
.L10:
sub esi,BasePointGui     ; calculate ESI.[15-0] = compact 16-bit offset
mov [DialogueContent_Win1 + HANDLE_TEXT_METHOD1*16 - 256 + 12],si

; Down comments strings, target object (Cache, DRAM, Custom block)
lea esi,[Unknown_Object]
mov ecx,[ebx + IPB.UpdatedTarget]
cmp ecx,TARGET_LIMIT
jae .L12 
lea esi,[Target_00]
jecxz .L12
.L13:
lodsb
cmp al,0
jne .L13
loop .L13                ; find string indexed by RCX
.L12:
sub esi,BasePointGui     ; calculate RSI.[15-0] = compact 16-bit offset
mov [DialogueContent_Win1 + HANDLE_TEXT_OBJECT1*16 - 256 + 12],si

; Down comments strings, number of threads
lea edi,[Threads_Number]
mov word [edi],0000h + '?'
mov eax,[ebx + IPB.UpdatedThreads]
cmp eax,MAX_THREADS
ja .L14
push ebx    ; Store EBX = pointer to IPB
mov bl,0    ; BL = print template mode
call DecimalPrint32
pop ebx
.L14:

; Down comments strings, number of measurement repeats
lea edi,[Repeats_Number]
push edi 
mov ecx,12
mov al,' '
rep stosb   ; Pre-blank text field, 
pop edi     ; note can be previous long text at buffer or screen 
mov eax,dword [ebx + IPB.MeasureRepeats + 0]
; Required support overflows Repeats Count > 4GB, actual for adaptive modes
cmp dword [ebx + IPB.MeasureRepeats + 4],0
je .L18        ; Go if no overflow
mov eax,' >4G'
stosd
jmp .L19
.L18:
push ebx
mov bl,0    ; BL = print template mode
call DecimalPrint32
pop ebx
.L19:

; Special strings for adaptive repeats modes
lea eax,[Repeats_String]   ; Text string 'Repeats=' + numeric field
cmp [ebx + IPB.UpdatedMeasure],MEASURE_CAREFUL
jbe .L30                   ; Go if brief or careful, text string + numeric field
lea eax,[Measure_D_02]
cmp [ebx + IPB.UpdatedMeasure],MEASURE_B_ADAPTIVE
je .L30                    ; Go if brief adaptive, text string only
lea eax,[Measure_D_03]     ; otherwise careful adaptive, text string only
.L30:
sub eax,BasePointGui        ; calculate ESI.[15-0] = compact 16-bit offset
mov [DialogueContent_Win1 + HANDLE_TEXT_REPEATS1*16 - 256 + 12],ax

; Down comments strings, normal or large pages
lea eax,[NPages_String]
cmp [ebx + IPB.UpdatedLP],LP_USED
jne .L15 
lea eax,[LPages_String]
.L15:
sub eax,BasePointGui        ; calculate RSI.[15-0] = compact 16-bit offset
mov [DialogueContent_Win1 + HANDLE_LARGE_PAGES1*16 - 256 + 12],ax

; Down comments strings, NUMA optimization mode
lea esi,[NumaOff_String]
mov ecx,[ebx + IPB.UpdatedNUMA]
cmp ecx,NUMA_LIMIT
ja .L16             ; old bug = jae .L16  
jecxz .L16
.L17:
lodsb
cmp al,0
jne .L17
loop .L17
.L16:
sub esi,BasePointGui   ; calculate RSI.[15-0] = compact 16-bit offset
mov [DialogueContent_Win1 + HANDLE_NUMA_MODE1*16 - 256 + 12],si

; Support option, actual for drawings only: adaptive measurement modes
lea esi,[AdaptiveRepeats]
mov eax,[ebx + IPB.UpdatedMeasure]
sub eax,MEASURE_B_ADAPTIVE
jc .L20
cmp eax,1
ja .L20
mov ecx,[esi + eax*8 + 0]
mov dword [ebx + IPB.AdaptiveSeconds + 0],ecx
mov ecx,[esi + eax*8 + 4]
mov dword [ebx + IPB.AdaptiveSeconds + 4],ecx

; Select Bandwidth or Latency measurement mode
lea esi,[ThreadsList]  ; ESI = storage for RCX reloads
mov ecx,esi
cmp [ebx + IPB.UpdatedAsm],LATENCY_MODE
jae .L22

; Run with default recommended repeats count: Bandwidth mode
mov edx,[ebx + IPB.UpdatedThreads]
call ThreadsReStart
jc .Error02                  ; If error, go to error handling branch
jmp .L21

; Run with default recommended repeats count: Latency mode
; Setup pattern for prepare walk list
.L22:
push ebx
push [ebx + IPB.UpdatedThreads]
mov eax,[ebx + IPB.PatternRoutine]    ; for first call, prepare list for walk
mov ebx,ecx
pop ecx
call ThreadsUpdatePattern
pop ebx
; Run service pattern, prepare walk list, delta TSC ignored here
mov ecx,esi
mov edx,[ebx + IPB.UpdatedThreads]
call ThreadsReStart
jc .Error02                  ; If error, go to error handling branch
; Setup pattern for walk
push ebx
mov ecx,[ebx + IPB.UpdatedThreads]
mov eax,[ebx + IPB.WalkRoutine]       ; for second call, walk at prepared list
mov ebx,esi
call ThreadsUpdatePattern
pop ebx

; Run measurement pattern, walk by prepared walk list, get delta TSC in the RAX
mov ecx,esi
mov edx,[ebx + IPB.UpdatedThreads]
call ThreadsReStart
jc .Error02                  ; If error, go to error handling branch

; Done point for adaptive modes ( bandwidth and latency branches ),
; pre-measurements executed, 
; next, test control = f ( results ) must be calculated
.L21:

; Calculate seconds and ratio after run with default recommended repeats count
; for measurements calibration 
push edx eax                        ; EDX:EAX = Delta TSC after ThreadsReStart
finit
fld [ebx + IPB.AdaptiveSeconds]     ; ST0 = Target seconds
fild qword [esp]                    ; ST0 = Measured delta TSC , ST1 = Target seconds
fmul [OutputParms.TSCperiodS]       ; ST0 = Measured seconds
fdivp st1,st0                       ; ST0 = Target / Measured 
fild [ebx + IPB.MeasureRepeats]     ; ST0 = Product repeats
fmulp st1,st0                       ; ST0 = Corrected product
fild [ebx + IPB.StartBlockSize]
fmulp st1,st0                       ; ST0 = Corrected product for divide per bytes count 
fistp [ebx + IPB.AdaptiveProduct]
pop eax edx                         ; This for release stack only

; Skip point for non-adaptive modes
.L20:

; Note option: X-approximation modes, 
; interpreted and supported at drawsessionthread.inc module. 

; Load pointer to drawings parameters
lea edi,[DrawParms]

; Calculate X grid step values, must be integer power of 2 for next step correct
; Note this operation increase memory size requirements, because increase RAX
mov eax,[ebx + IPB.StartBlockSize]
bsr ecx,eax
mov eax,1
shl eax,cl   ; RAX = Target block size, note example here EAX=8K for L1=32K

; Special support for DRAM and Custom modes
mov ecx,[ebx + IPB.UpdatedTarget]
cmp ecx,TARGET_DRAM
je .L1                    ; Go for size / 8 if target = DRAM
cmp ecx,TARGET_CUSTOM
jne .L2                   ; Go skip without size / 8 if target = not a Custom
.L1:
shr eax,3                 ; size / 8 , for DRAM and Custom modes
.L2:

; Set Xstart, Xdelta for drawings
push eax                            ; EAX = Block size per X-grid
xor edx,edx                         ; RDX = Dividend high 64 bits, =0
mov ecx,GRIDSTEPX                   ; RCX = Pixels per X-grid
div ecx                             ; (Block size per grid) / (pixels per grid)
mov [ebx + IPB.StartBlockSize],eax  ; X start value
mov [ebx + IPB.DeltaBlockSize],eax  ; X addend per draw iteration 
pop eax

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
mov [edi + DRPM.ValueGridX],eax
mov [edi + DRPM.SelectUnits],edx

; Set first approximation constant for Y-grid step values
mov eax,DEFAULT_Y_MBPS_PER_GRID
xor edx,edx
cmp [ebx + IPB.UpdatedAsm],LATENCY_MODE
jb @f
mov eax,DEFAULT_Y_NS_PER_GRID
inc edx
@@:
mov [edi + DRPM.ValueGridY],eax
mov [edi + DRPM.SelectMode],edx

; Set Ymultiplier scale constant for conversion 
; measurement results to Y-coordinate
; Note same operation at win1.inc for re-scaling, 
finit
push eax -GRIDSTEPY
fild dword [esp]        ; ST0 = - grid step, pixels
fidiv dword [esp + 4]   ; ST0 = ( - grid step, pixels ) / ( Y units per grid )      
fstp [edi + DRPM.Ymultiplier]
pop eax eax

; Initializing handles and run thread with DrawSessionThread  subroutine 
lea esi,[DrawThreadParms]
mov edi,esi
cld
; Create event for thread start, store event handle
push 0               ; Parm#4 = Name, 0=None
push 0               ; Parm#3 = bInitialState
push 1               ; Parm#2 = bManualReset
push 0               ; Parm#1 = lpEventAttributes
call [CreateEvent]   ; Call WinAPI function
test eax,eax         ; Status=0 means error
jz .Error            ; Go if error creating event
stosd                ; Store handle in the thread parameters, EventStart 
; Create event for thread done, store event handle
push 0               ; Parm#4 = Name, 0=None
push 0               ; Parm#3 = bInitialState
push 1               ; Parm#2 = bManualReset
push 0               ; Parm#1 = lpEventAttributes
call [CreateEvent]   ; Call WinAPI function
test eax,eax         ; Status=0 means error
jz .Error            ; Go if error creating event
stosd                ; Store handle in the thread parameters, EventDone 
; Create thread, store event handle
xor eax,eax          ; EAX = 0 for compact PUSH 0
push eax                    ; Parm#6 = Thread ID return , 0=None
push eax                    ; Parm#5 = Creating Flags = Not suspended, run immediately
push eax                    ; Parm#4 = Subroutine parameter, not used here
push DrawSessionThread      ; Parm#3 = Execute routine pointer
push eax                    ; Parm#2 = Stack size, 0=default
push eax                    ; Parm#1 = Security
call [CreateThread]         ; Call WinAPI function
test eax,eax                ; Status=0 means error
jz .Error                   ; Go if error creating thread
stosd                       ; Store handle in the thread parameters, ThreadHandle 
; Clear measurement counter and array of measurement results
;- mov ecx,( DRAW_POINTS_COUNT + 20 - 3 ) * 2  ; OLD: + 4 dwords: 2 counters + break flag + align
;-                                             ; -3 because 3 qwords already stored
mov ecx,( DRAW_POINTS_COUNT + 15 ) * 2 + 3
xor eax,eax
rep stosd

; Done, no errors, parallel thread with  DrawSessionThread  subroutine RUNNING
clc
.L90:
.Exit:                 ; Note AL = error status must be valid at this point
ret

; Exit point for errors detected
.Error:
stc
jmp .L90

; CF=1 with AL=2  means timings measurement error, 
; note this branch without valid RBP
.Error02:
mov al,2
stc
ret

