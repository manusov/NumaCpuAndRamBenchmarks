;------------------------------------------------------------------------------;
; Callback handler for drawings-child window                                   ;
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

WindowProc_Win1:

push rbx rsi rdi rbp r12 r13 r14 r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
push rbp
sub rsp,32+8          ; Parameters shadow for call API + alignment
mov rbx,rcx           ; RBX = Storage for hwnd
; Select handler
cmp rdx,WM_CREATE
je .WinCreate         ; Go if WM (Window Message) = OPEN WINDOW
cmp rdx,WM_DESTROY
je .WinDestroy        ; Go if WM = CLOSE WINDOW
cmp rdx,WM_COMMAND
je .WinCommand        ; Go if WM = USER INPUT
cmp rdx,WM_TIMER
je .WinTimer          ; Go if WM = TIMER TICK
cmp rdx,WM_PAINT
je .WinPaint          ; Go if WM = PAINT GRAPHICS
; Otherwise Default OS routine
; Here must be valid input RCX, RDX, R8, R9
.WinDefaultProc:
call [DefWindowProc]  ; DEFAULT WINDOW PROCEDURE
; Exit points
.WinFinish:   ; No EAX destroy here, otherwise window not closed at WinXP, Win7
add rsp,32+8  ; Remove parameters shadow + alignment
pop rbp       ; Can directly use POP RSP ?
mov rsp,rbp
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret
.WinStatus:
mov eax,1
jmp .WinFinish


; Handler for system event: window create
.WinCreate:
; TODO.
; Initializing statistics
; call InitStatistics
mov [DrawParms + DRPM.DrawPreviousY],0
; TODO. Make this at InitStatistics subroutine 
; Build buttons
lea rsi,[DialogueContent_Win1]      ; RSI = Pointer to elements descriptors
mov edi,START_HANDLE_WIN1           ; RDI = Start handle
mov r14,[Dialogue_Win1.hInstance]   ; R14 = Parent module handle = [wc.hInstance]
call CreateDialogueElements
; Initialize graphics context
xor ecx,ecx                            ; Parm#1 = RCX = Handle to exist DC, 0=Application current screen
call [CreateCompatibleDC]              ; Create compatible Device Context for buffered video output 
mov [HandleMemDC],rax                  ; Store handle
mov rsi,rax                            ; RSI = Store handle for later use
xchg rcx,rax                           ; Parm#1 = RCX = Handle to a Device Context
lea rdx,[BitmapInfo]                   ; Parm#2 = RDX = Pointer to Bitmap Descriptor
mov r8d,DIB_RGB_COLORS                 ; Parm#3 = R8  = Type of color data
lea r9,[BitmapPointer]                 ; Parm#4 = R9  = Pointer to Bitmap Pointer variable
xor eax,eax                            ; RAX = 0 for compact push 0
push rax                               ; Parm#6 = Offset in the file-mapping object, not used here
push rax                               ; Parm#5 = Handle to file-mapping object, not used here
sub rsp,32                             ; Create parameters shadow
call [CreateDIBSection]
add rsp,32+16                          ; Remove parameters shadow and 2 parameters
mov [HandleBitmap],rax
mov rcx,rsi                            ; Parm#1 = RCX = Handle to a Device Context
xchg rdx,rax                           ; Parm#2 = RDX = Handle to a Bitmap
call [SelectObject]
; Create brushes
cld
lea rsi,[BrushesList]                  ; RSI = Pointer to color data
lea rdi,[HandlesBrushes]               ; RDI = Pointer for store brushes handles
mov ebp,3                              ; EBP = Number of brushes
@@:
lodsd                                  ; EAX = Color, RSI+4, select next color 
xchg ecx,eax                           ; Parm#1 = RCX = Color
call [CreateSolidBrush]
stosq                                  ; Store brush handle, RDI+8
dec ebp
jnz @b                                 ; Cycle for all brushes
; Create font for texts in the drawings window
mov ecx,16                             ; Parm#1 = RCX = Height
xor edx,edx                            ; Parm#2 = RDX = Width
xor r8d,r8d                            ; Parm#3 = R8  = Escapment
xor r9d,r9d                            ; Parm#4 = R9  = Orientation
xor eax,eax                            ; RAX = 0 for compact push 0
push rax                               ; Parm#14 = Pointer to font typename string, here not used
push VARIABLE_PITCH                    ; Parm#13 = Font pitch and family
push CLEARTYPE_QUALITY                 ; Parm#12 = Output quality
push CLIP_DEFAULT_PRECIS               ; Parm#11 = Clip precision
push OUT_OUTLINE_PRECIS                ; Parm#10 = Output precision
push DEFAULT_CHARSET                   ; Parm#9  = Charset
push rax                               ; Parm#8  = Strike, here=0=none
push rax                               ; Parm#7  = Underline, here=0=none
push rax                               ; Parm#6  = Italic, here=0=none
push FW_DONTCARE                       ; Parm#5  = Weight of the font
sub rsp,32                             ; Create parameters shadow
call [CreateFont]
add rsp,32+80                          ; Remove parameters shadow and 10 parameters
mov [HandleFont],rax 
; Create and set timer, for benchmarks progress (but not for measure)
mov rcx,rbx                            ; Parm#1 = RCX = Parent window handle
xor edx,edx                            ; Parm#2 = RDX = Event ID
mov r8d,TIMER_TICK                     ; Parm#3 = R8 = Timer ticks period, ms
xor r9d,r9d                            ; Parm#4 = R9 = Notify function pointer, not used here
call [SetTimer]
; Wait thread started (1 of 2 waits)
; Wait for measurement thread run, this event at thread entry point
mov rcx,[DrawThreadParms + DTHP.EventStart]  ; RCX = Parm#1 = Handle for start event
mov edx,10000                    ; RDX = Parm#2 = 10 seconds, time limit
call [WaitForSingleObject]
;
; TODO. Check status after [WaitForSingleObject]
;
; End of initialization, enable draw counter
mov [Win1_Running],1
jmp .WinStatus


; Handler for system event: window destroy
.WinDestroy:
push rcx rdx r8 r9
sub rsp,32
; Stop measurement thread
mov [DrawThreadParms + DTHP.MeasureBreak],1
; Wait thread terminated (2 of 2 waits)
; Wait for measurement thread done, this event at thread exit point
mov rcx,[DrawThreadParms + DTHP.EventDone]   ; RCX = Parm#1 = Handle for done event
mov edx,10000                    ; RDX = Parm#2 = 10 seconds
call [WaitForSingleObject]
;
; TODO. Use RSI-based addressing
; TODO. Check status after [WaitForSingleObject]
;
; De-Initialize graphics context
mov rcx,[HandleMemDC]
call [DeleteDC]
mov rcx,[HandleBitmap]
call [DeleteObject]
; Delete brushes
cld
lea rsi,[HandlesBrushes]               ; RSI = Pointer for store brushes handles
mov ebp,3                              ; EBP = Number of brushes
@@:
lodsq                                  ; RAX = Brush handle
xchg rcx,rax                           ; Parm#1 = RCX = Handle
call [DeleteObject]
dec ebp
jnz @b
; Delete font
mov rcx,[HandleFont]
call [DeleteObject]
;--- Delete timer ---
mov rcx,rbx                            ; Parm#1 = RCX = Parent window handle
xor rdx,rdx                            ; Set same as for SetTimer function
call [KillTimer]
; Destroy window, disable draw counter
mov byte [Win1_Running],0
; Close window
; This absent in the Window 0 handler, but it work because automatically
; closed when exit application. Verify it and registers validity
; (RCX, RDX, R8, R9) when run Default Window Procedure.
mov rcx,rbx                ; Parm#1 = RCX = Handle
mov edx,WM_CLOSE           ; Parm#2 = RDX = Message
xor r8d,r8d                ; Parm#3 = R8 = Not used
xor r9d,r9d                ; Parm#4 = R9 = Not used
call [SendMessage]
; Terminate
add rsp,32
pop r9 r8 rdx rcx
; Destroy window
xor ecx,ecx                ; Parm#1 = RCX = Exit code
call [PostQuitMessage]
xor eax,eax
jmp .WinFinish


; Handler for user event: buttons input
.WinCommand:
; Check phase of execution
cmp [Win1_Init],1
jne .WinDefaultProc   ; Run default OS handler if Window 1 not initialized yet
;--- Detecting buttons ---
cmp r8,HANDLE_BUTTON_CANCEL1
je .WinDestroy                        ; Go if "Cancel" button
cmp r8,HANDLE_BUTTON_RUN1
jne .WinDefaultProc                   ; Go if not "Resize" button
; ...
; ...

; DEBUG. REDUNDANT WITH INITSTATISTICS
mov [Win1_Running],1
mov [DrawThreadParms + DTHP.MeasureCounter],0
mov [DrawThreadParms + DTHP.VisualCounter],0
mov [DrawParms + DRPM.DrawPreviousY],0
; DEBUG. REDUNDANT WITH INITSTATISTICS
; ...
; ...
jmp .WinStatus


; Handler for timer event: tick
.WinTimer:
lea rcx,[Win1_Running]
lea rdx,[DrawParms + DRPM.TimerCount]     ; For two variables: COUNT, PREVIOUS Y
xor eax,eax                ; This clear entire RAX
cmp [rcx],al               ; 0=Stop, 1=First, 2=Progress
je .SkipTimer              ; Skip if halted
inc dword [rdx]            ; Counter +1

; cmp dword [rdx],SUBWINX - 134 ; TODO. Make EQU.
; jbe .YetNotHalt            ; Skip if limit not reached yet
; mov qword [rdx],rax        ; Count=0 means READY TO NEXT START, clear BOTH DWORDS: COUNT and PREVIOUS Y 

cmp [DrawThreadParms + DTHP.MeasureBreak],0
je .YetNotHalt 
mov [rcx],al               ; Running=0 means HALTED
.YetNotHalt:
mov rcx,rbx                ; Parm#1 = RCX = Window handle 
xor edx,edx                ; Parm#2 = RDX = Pointer to invalidated region, 0=All window
xor r8d,r8d                ; Parm#3 = R8  = Background erase flag, 0=Background unchanged
call [InvalidateRect]      ; This cause callback WM_PAINT operation
.SkipTimer:
jmp .WinStatus


; Handler for system event: window paint callback
.WinPaint:
; Make paint context
mov rcx,rbx                ; Parm#1 = RCX = Parent window handle
lea rdx,[PaintStruc]       ; Parm#2 = RDX = Pointer to PAINTSTRUCT structure
call [BeginPaint]
mov [HandleDC],rax
; Prepare non-volatile parms, select handler = F (execution phase) 
mov rsi,[HandleMemDC]      ; Get to non-volatile register
lea rdi,[GraphRect]
xor ebp,ebp                ; RBP = 0, used for compact encoding
lea rcx,[Win1_Running]
mov al,[rcx]
cmp al,0             ; 0 = Halted
je .EndPaint
cmp al,1             ; 1 = First drawings (grid)
je .FirstPaint       ; Go if this is first pass, draw axes
; ...
; ... draw y=f(x), not a first pass

; TODO. Replace to DrawSessionThread
; call DrawSessionProgress



; Setup drawings

;;- DEBUG
;;- mov eax,[DrawParms + DRPM.DrawCount]
;mov eax,[DrawThreadParms + DTHP.MeasureCounter]
;add eax,GRIDBLANKX
;lea edx,[eax+1]
;mov ecx,SUBWINY-GRIDBLANKY - 2
;mov r8d,SUBWINY-GRIDBLANKY - 1
;;- DEBUG
;mov [rdi+RECT.left],eax
;mov [rdi+RECT.top],ecx
;mov [rdi+RECT.right],edx
;mov [rdi+RECT.bottom],r8d
;;- Draw current point Y=F(X)
;mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
;mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT strucnure
;mov r8,[HandlesBrushes+08]   ; Parm#3 = R8 = Handle to a color brush
;call [FillRect]

lea rcx,[DrawThreadParms]
lea r14,[rcx + DTHP.MeasureArray]
mov ebp,[rcx + DTHP.MeasureCounter]
mov r12d,[rcx + DTHP.VisualCounter]

mov rax,r12
shl rax,4
add r14,rax

mov [rcx + DTHP.VisualCounter],ebp
sub ebp,r12d
jbe .EndLine
add r12,GRIDBLANKX + 1 

; This cycle for EBP points draw per one timer tick
.DrawLine: 
; Calculate EAX = F(measurement result)

;- DEBUG -
; mov eax,ebp
; and eax,0Fh
; mov edx,eax
; mul edx
; neg eax
;- DEBUG -
; mov rax,[r14 + 0]  ; bytes count ; OLD = instructions count
; mov edx,[r14 + 8]  ; delta TSC
; neg eax
;- DEBUG -


;- DEBUG Y-POSITION CALC -

; TODO. Replace X/1000000 with X * 1/1000000, pre-defined constant,
;       because multiply faster then divide.
; TODO. Minimize calculations by pre-define TWO constants: 
;       1000000 and Ymultiplier as ONE constant.
; TODO. Make addressing with base register for OutputParms, DrawParms ?
;

;- DEBUG Y-POSITION CALC -
;
; finit
; cmp [InputParms + IPB.UpdatedAsm],LATENCY_MODE
; jae .LatencyMode 
;
;. BandwidthMode:
; push 1000000                         ; Decimal megabytes divisor, TODO. Make EQU
; fild qword [r14 + 8]                 ; ST0 = Total bytes
; fidiv dword [rsp]                    ; ST0 = Total decimal megabytes          
; fild qword [r14 + 0]                 ; ST0 = Delta TSC, clocks
; fmul [OutputParms + OPB.TSCperiodS]  ; ST0 = Delta Time, seconds , ST1 = Total decimal megabytes   
; fdivp st1,st0                        ; ST0 = Megabytes / Second = MBPS
; fmul [DrawParms + DRPM.Ymultiplier]  ; ST0 = Pixel offset from BaseY=0, negative offset
; fistp qword [rsp]                    ; Store ST0 as integer
; pop rax                              ; RAX = Integer pixel offset
; jmp .DoneLatencyOrBandwidth
;
; .LatencyMode:
; mov eax,[InputParms + IPB.OperandWidth]
; shr eax,3
; push rax
; fild qword [r14 + 0]                       ; ST0 = Delta TSC
; fmul qword [OutputParms + OPB.TSCperiodNs] ; ST0 = Delta time, nanoseconds
; fild qword [r14 + 8]                       ; ST0 = Total bytes, ST1 = Delta time
; fidiv dword [rsp]                          ; ST0 = Total qwords, units for latency test
; fdivp st1,st0
; fmul [DrawParms + DRPM.Ymultiplier]  ; ST0 = Pixel offset from BaseY=0, negative offset
; fistp qword [rsp]                    ; Store ST0 as integer
; pop rax                              ; RAX = Integer pixel offset
;
; .DoneLatencyOrBandwidth:
; 
;- DEBUG Y-POSITION CALC -

; TODO. Optimize this mathematics.

finit
fld1
mov rax,[r14 + 8]     ; RAX = Executed block size, bytes, per one thread
xor edx,edx
mov ecx,[InputParms + IPB.OperandWidth]
shr ecx,3
div rcx               ; RAX = Number of instructions executed at one thread
push rcx rcx rax
fild qword [r14 + 0]  ; ST0 = Number of TSC clocks
fild qword [rsp]      ; ST0 = Number of instructions executed at one thread, ST1 = TSC clocks 
fdivp st1,st0         ; ST0 = ST1/ST0 = Clocks Per Instruction (CPI) at one thread
; required store CPI statistics
fmul [OutputParms + OPB.TSCperiodNs]  ; ST0 = Nanoseconds Per instruction
fld st0
fmul [DrawParms + DRPM.Ymultiplier]  ; ST0 = Pixel offset from BaseY=0, negative offset
fistp qword [rsp+16]   ; Store pixel offset for LATENCY
fdivp st1,st0          ; ST0 = Instructions Per nanosecond
fimul dword [rsp + 8]  ; ST0 = Bytes Per nanosecond
mov dword [rsp],1000
fimul dword [rsp]      ; ST0 = Decimal Megabytes Per Second (MBPS)
; required store MBPS statistics
fmul [DrawParms + DRPM.Ymultiplier]  ; ST0 = Pixel offset from BaseY=0, negative offset
fistp qword [rsp]                    ; Store ST0 as integer
pop rax rcx rcx        ; Store pixel offset for MBPS
; Select MBPS or Latency
cmp [InputParms + IPB.UpdatedAsm],LATENCY_MODE
jb @f
xchg rax,rcx
@@:

;- DEBUG Y-POSITION CALC -
;
; xor eax,eax ; 0 = BaseY axis
;
;- DEBUG Y-POSITION CALC -


; Setup coordinates for GUI
add eax,SUBWINY-GRIDBLANKY - 1
lea rdx,[r12+1]
lea ecx,[eax+1]
; Work with Y-coordinate, adjust Y-coordinates for make solid line by Y
mov r8d,[DrawParms + DRPM.DrawPreviousY]   ; R8D = DrawPreviousY
test r8d,r8d
jz @f
mov ecx,r8d
@@:
mov [DrawParms + DRPM.DrawPreviousY],eax   ; Set DrawPreviousY
jnz @f
inc ecx
inc eax
@@:
cmp eax,ecx
jbe @f
xchg eax,ecx      ; Must be Down > Top
@@:
jne @f
inc ecx           ; Make Down+1 if Top=Down
@@:
mov r8d,ecx
sub r8d,eax
cmp r8d,1
jbe @f
inc ecx            ; Make down extra-pixel for vertical line  
@@:
; Store coordinates for rectangle as part of line Y=F(X)
mov [rdi+RECT.left],r12d
mov [rdi+RECT.top],eax
mov [rdi+RECT.right],edx
mov [rdi+RECT.bottom],ecx
; Draw current point Y=F(X)
mov rcx,rsi                    ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                    ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[HandlesBrushes + 08]   ; Parm#3 = R8 = Handle to a color brush
call [FillRect]
add r14,16     ; R14 = Pointer to measurement results array
inc r12d       ; R12D = X-coordinate offset by visualization position
dec ebp        ; EBP = Number of pixels per timer tick
jnz .DrawLine  ; This cycle for EBP points draw per one timer tick
.EndLine:

; ...
; jmp .EndPaint
jmp .DrawVisual


.FirstPaint:
mov byte [rcx],2          ; [rcx] = [Win1_Running]
mov [DrawParms + DRPM.TimerCount],ebp
; Draw
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,[HandleFont]         ; Parm#2 = RDX = Handle to Selected Object
call [SelectObject]
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_VALUES    ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_BACK      ; Parm#2 = RDX = Text background color value = 00bbggrrh
call [SetBkColor]
; Blank work field
mov [rdi+RECT.left],ebp
mov [rdi+RECT.top],ebp
mov [rdi+RECT.right],SUBWINX
mov [rdi+RECT.bottom],SUBWINY
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[HandlesBrushes+16]   ; Parm#3 = R8 = Handle to a color brush
call [FillRect]
; Coordinate X-grid, vertical lines
mov r12d,GRIDBLANKX          ; R12D = Base point, X
mov r13d,SUBWINY-GRIDBLANKY  ; R13D = Base point, Y
mov r14d,GRIDX               ; R14D = Number of lines
.XVgrid:
lea rax,[r12 + 1]            ; Line width = 1 pixel
mov [rdi+RECT.left],r12d
mov [rdi+RECT.top],ebp
mov [rdi+RECT.right],eax
mov [rdi+RECT.bottom],r13d
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[HandlesBrushes+00]   ; Parm#3 = R8 = Handle to a color brush
call [FillRect]              ; Fill rectangle function
add r12d,GRIDSTEPX           ; Add X grid step
dec r14d                     ; Cycle for X-grid vertical lines
jnz .XVgrid
; Coordinate Y-grid, horizontal lines
mov r12d,SUBWINY-GRIDBLANKY  ; R12D = Base point, Y
mov r13d,GRIDY               ; R13D = Number of lines
.YHgrid:
lea rax,[r12 - 1]
mov [rdi+RECT.left],GRIDBLANKX
mov [rdi+RECT.top],eax
mov [rdi+RECT.right],SUBWINX
mov [rdi+RECT.bottom],r12d
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[HandlesBrushes+00]   ; Parm#3 = R8 = Handle to a color brush
call [FillRect]              ; Fill rectangle function
sub r12d,GRIDSTEPY           ; Subtract Y grid step
dec r13d                     ; Cycle for Y-grid vertical lines
jnz .YHgrid

; TODO.
; Probably better separate fragments with subroutine,
; instead single cycle for X and Y grids.

; Numbers for X-grid vertical lines and Y-grid horizontal lines
push rbx                     ; Save RBX=Handle, this also important for stack alignment: 5 parms. functions
mov bh,2                     ; BH = Counter, Pass#1 = XGrid, Pass#2 = YGrid
mov r14d,GRIDX               ; R14D = Number of lines, X
mov r12d,GRIDBLANKX          ; R12D = Base point, X
mov r13d,SUBWINY-20          ; R13D = Base point Y
xor ebp,ebp                  ; EBP = Units value
.BothGrids:
.XVYHtext:
; Build ASCII string
push rdi
lea rdi,[BigTexts]           ; RDI = Pointer for print decimal string as ASCII
mov rdx,rdi
mov bl,0                     ; BL = Template control for print number, 0=No template
mov eax,ebp                  ; EAX = Value for print, Bytes/KB/MB
call DecimalPrint32          ; Print number
mov rcx,rdi
sub rcx,rdx
mov al,0
stosb                        ; Store terminator byte = 0
pop rdi
; Calculate correction value for Y axis grid values print
mov r10d,9
sub r10,rcx
jbe .RejectAddend 
imul r10,r10,7
; Reject corrections if horizontal axis or overflow
cmp bh,1
je .DoneAddend
.RejectAddend:
xor r10,r10
.DoneAddend:
; Set region for text string write
; Here don't align stack because PUSH RBX
mov rcx,rdi                  ; Parm#1 = RCX = Pointer to RECT structure to store parameters
lea rdx,[r12 + r10]          ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,r13d                 ; Parm#3 = R8  = Y coordinate up/left corner  ,  TODO MAKE EQU INSTEAD NUMBER.
lea r9,[r12 + r10 + 65]      ; Parm#4 = R9  = X coordinate down/right corner  ,  TODO MAKE EQU INSTEAD NUMBER. ; old = +65
lea rax,[r13 + 18]                                                            ;  TODO MAKE EQU INSTEAD NUMBER. ; old = +18 
push rax                     ; Parm#5 = Y coordinate down/right corner
sub rsp,32                   ; Create parameters shadow
call [SetRect]               ; Set region rectangle function 
add rsp,32+8                 ; Remove parameters shadow and one parameter
lea rdx,[BigTexts]
; Draw text
; Here don't align stack because PUSH RBX , Parm#2 = RDX = Pointer to text string
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context, Parm#2 = RDX set by previous step
mov r8,-1                    ; Parm#3 = R8 = String length, -1 means NULL-terminated string
mov r9,rdi                   ; Parm#4 = R9 = Pointer to RECT structure, initialized by [SetRect] function
push DT_LEFT                 ; Parm#5 = Method of formatting/alignment of text string
sub rsp,32                   ; Create parameters shadow
call [DrawText]              ; Draw text function 
add rsp,32+8                 ; Remove parameters shadow and 1 parameter
; Cycle for lines of selected grid
cmp bh,1
je .YGrid
; TODO.
; Make compact offsets with base register = offset DrawParms
add ebp,[DrawParms + DRPM.ValueGridX]   ; Addend per X grid
add r12d,GRIDSTEPX
jmp .CycleGrids
.YGrid:
add ebp,[DrawParms + DRPM.ValueGridY]   ; Addend per Y grid
sub r13d,GRIDSTEPY
.CycleGrids:
dec r14d
jnz .XVYHtext 
; Cycle for horizontal and vertical grids
mov r14d,GRIDY - 1                             ; R14D = Number of lines, Y
mov r12d,GRIDBLANKX - 68                       ; R12D = X coordinate , TODO MAKE EQU INSTEAD NUMBER. OR ADAPTIVE RIGHT ALIGNED
mov r13d,SUBWINY-GRIDBLANKY - GRIDSTEPY - 15   ; R13D = Y coordinate
mov ebp,[DrawParms + DRPM.ValueGridY]
dec bh
jnz .BothGrids
pop rbx

; Prepare for X-units, here change color
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_UNITS     ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
; TODO.
; Make compact offsets with base register = offset DrawParms
mov eax,[DrawParms + DRPM.SelectUnits]
lea r12,[U_B]                ; R12 = Pointer to text string, Cache&RAM mode
cmp al,0
je @f
lea r12,[U_KB]
cmp al,1
je @f
lea r12,[U_MB]
@@:
; TODO. Make EQU instead in-line constants
mov edx,SUBWINX-40 - 9       ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,SUBWINY-40 - 3       ; Parm#3 = R8  = Y coordinate up/left corner
mov r9d,SUBWINX-4 - 9        ; Parm#4 = R9  = X coordinate down/right corner
mov eax,SUBWINY-22 - 3       ; RAX = Transit for Parm#5 = Y coordinate down/right corner
call TextConstDraw

; Prepare for Y-units
lea r12,[U_MBPS]             ; R12 = Pointer to text string , Mode = Cache&RAM
cmp [DrawParms + DRPM.SelectMode],0
je @f
lea r12,[U_NS]
@@:
mov edx,7                    ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,5                    ; Parm#3 = R8 = Y coordinate up/left corner
mov r9d,40+7+20 + 24         ; Parm#4 = R9 = X coordinate down/right corner
mov eax,18+5                 ; RAX = Transit for Parm#5 = Y coordinate down/right corner
call TextConstDraw

; Target method CPU instructions dump
lea r12,[InputParms]
xor r15,r15
mov r14,[r12 + IPB.DumpMethodStart]
mov r13d,[r12 + IPB.DumpMethodLength]
cmp r13d,128
jbe @f
mov r13d,128
@@:
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_DUMP      ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
.AsmDump:
lea r12,[BigTexts]
push rdi
mov rdi,r12
mov al,[r14 + r15]
call HexPrint8
mov al,0
stosb
pop rdi
; Set coordinates. TODO. Make EQU.
mov edx,SUBWINX-250 + 64
mov r8d,95 + 48
mov r9d,SUBWINX-005 + 64
mov eax,95+18 + 48
; Adjust coordinates
mov ecx,r15d                 ; R15 = counter, ECX = counter temporary copy #1
mov r10d,r15d                ; R10 = counter temporary copy #2  
and ecx,07h                  ; Bits 0-2 used for X-positioning
imul ecx,ecx,21              ; X scale = 21 pixels per char
add edx,ecx                  ; Add offset to X-start
add r9d,ecx                  ; Add offset to X-end
shr r10d,3
and r10d,0Fh                 ; Bits 3-6 used for Y-positioning
imul r10d,r10d,18            ; Y scale = 18 pixels per char 
add r8d,r10d                 ; Add offset to Y-start
add eax,r10d                 ; Add offset to Y-end
call TextConstDraw
inc r15
dec r13d
jnz .AsmDump 

; TSC clock frequency
push rbx rdi
lea rdi,[DrawValueTSC]
mov word [rdi],0000h + '?'
mov rax,[OutputParms + OPB.TSCfrequencyMHz]     ; RAX = TSC Frequency, MHz
test rax,rax
jz @f
mov bx,0100h
call DoublePrint
mov al,0
stosb
@@:
pop rdi rbx
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_INFO      ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
mov edx,SUBWINX - 187        ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,6                    ; Parm#3 = R8 = Y coordinate up/left corner
mov r9d,SUBWINX - 5          ; Parm#4 = R9 = X coordinate down/right corner
mov eax,6+18                 ; RAX = Transit for Parm#5 = Y coordinate down/right corner
lea r12,[DrawTextTSC]        ; R12 = Pointer to text string
call TextConstDraw

; Initializing parameters for benchmark statistics, bandwidth and timings
lea r14,[DrawParms]
; Timings, CPI (Clocks Per Instructions)
mov edx,SUBWINX - 300
mov r8d,24 + 23
mov r9d,SUBWINX - 188 
mov eax,42 + 23
mov r11,[r14 + DRPM.StatClksMinimum]
lea r12,[DrawTextMinimum]
lea r15,[DrawValueMinimum]
call TextConstCpiMbps
; Statistics, MBPS maximum
mov r8d,24 + 23 + 18*1
mov eax,42 + 23 + 18*2
mov r11,[r14 + DRPM.StatClksMaximum]
lea r12,[DrawTextMaximum]
lea r15,[DrawValueMaximum]
call TextConstCpiMbps
; Statistics, MBPS average
mov r8d,24 + 23 + 18*2
mov eax,42 + 23 + 18*2
mov r11,[r14 + DRPM.StatClksAverage]
lea r12,[DrawTextAverage]
lea r15,[DrawValueAverage]
call TextConstCpiMbps
; Statistics, MBPS median
mov r8d,24 + 23 + 18*3
mov eax,42 + 23 + 18*3
mov r11,[r14 + DRPM.StatClksMedian]
lea r12,[DrawTextMedian]
lea r15,[DrawValueMedian]
call TextConstCpiMbps

; Bandwidth, MBPS (Megabytes Per Second)
lea r12,[DrawTextMBPS]       ; This must be non-volatile register
lea r15,[DrawValueMBPS]
; Statistics, MBPS minimum
mov edx,SUBWINX - 132
mov r8d,24 + 23
mov r9d,SUBWINX - 5
mov eax,42 + 23
mov r11,[r14 + DRPM.StatClksMinimum]
call TextConstCpiMbps        ; MBPS (Megabytes per Second) visual
; Statistics, MBPS maximum
mov r8d,24 + 23 + 18*1
mov eax,42 + 23 + 18*1
mov r11,[r14 + DRPM.StatClksMaximum]
call TextConstCpiMbps        ; MBPS (Megabytes per Second) visual
; Statistics, MBPS average, this for statistics, MBPS average = F (clocks average)
mov r8d,24 + 23 + 18*2
mov eax,42 + 23 + 18*2
mov r11,[r14 + DRPM.StatClksAverage]
call TextConstCpiMbps        ; MBPS (Megabytes per Second) visual
; TODO. Statistics MBPS, median. Note. doubles can be ordered as integer because shifted order.
mov r8d,24 + 23 + 18*3
mov eax,42 + 23 + 18*3
mov r11,[r14 + DRPM.StatClksMedian]
call TextConstCpiMbps        ; MBPS (Megabytes per Second) visual

; End of prepare, visual prepared objects
.DrawVisual:
mov rcx,[HandleDC]           ; Parm#1 = RCX = Handle of destination Device Context
mov edx,SHIFTX               ; Parm#2 = RDX = X destination
mov r8d,SHIFTY               ; Parm#3 = R8  = Y destination
mov r9d,SUBWINX              ; Parm#4 = R9  = Width
xor eax,eax                  ; RAX = 0 for compact push 0
push rax                     ; This push for align stack, not a parameter
push SRCCOPY                 ; Parm#9 = Raster operation code
push rax                     ; Parm#8 = Y source
push rax                     ; Parm#7 = X source
push [HandleMemDC]           ; Parm#6 = Handle of source Device Context
push SUBWINY                 ; Parm#5 = Height
sub rsp,32                   ; Create parameters shadow
call [BitBlt]                       
add rsp,32+40+8              ; Remove parameters shadow, 5 parameters, 1 qword for align
; Delete paint context
.EndPaint:
mov rcx,rbx                  ; Parm#1 = RCX = Parent window handle
; TODO.
; mov rdx,[PaintStruc]       ; Parm#2 = RDX = Pointer to PAINTSTRUCT structure
lea rdx,[PaintStruc]
; BUG at v1.01.xx ?
call [EndPaint]
jmp .WinStatus


; Helpers
; Note about stack re-alignment when subroutines call
; TODO. Comment and remove to Helper_<old name>.


TextConstDraw:
mov rcx,rdi                  ; Parm#1 = RCX = Pointer to RECT structure to store parameters
; push rax                   ; This for stack align
push rax                     ; Parm#5 = Y coordinate down/right corner
sub rsp,32                   ; Create parameters shadow
call [SetRect]               ; Set region rectangle function 
; add rsp,32+8+8             ; Remove parameters shadow, 1 parameter, align
add rsp,32+8
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,r12                  ; Parm#2 = RDX = Pointer to text string
mov r8,-1                    ; Parm#3 = R8 = String length, -1 means NULL-terminated string
mov r9,rdi                   ; Parm#4 = R9 = Pointer to RECT structure, initialized by [SetRect] function
; push rax                   ; This for stack align
push DT_LEFT                 ; Parm#5 = Method of formatting/alignment of text string
sub rsp,32                   ; Create parameters shadow
call [DrawText]              ; Set region rectangle function 
; add rsp,32+8+8             ; Remove parameters shadow, 1 parameter, align
add rsp,32+8
ret


TextConstCpiMbps:
push rbx rdx rdi r9 rax  ; RAX pushed for stack re-alignment when subroutine call

push rax rdi
; lea rdi,[DrawValueMBPS]    ; RDI = Destination string
mov rdi,r15
mov ecx,12-2                 ; 12 (not 9) for blank if variable X-size font 
mov ax,0000h + '?'
stosw
mov al,' '
rep stosb                    ; Blank entire field, if next value shorter then previous
pop rdi rax

; TODO. At other (?) subroutine, calculate statistics.

call TextConstDraw 

pop rax r9 rdi rdx rbx
ret


