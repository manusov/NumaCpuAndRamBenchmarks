;--- Callback handler for drawings-child window -------------------------------;
; This subroutine called by OS,                                                ; 
; when application call DispatchMessage subroutine                             ;
; WM = Window Message                                                          ; 
;                                                                              ;
; Callback procedure input parameters:                                         ;
;  Parm#1 = [esp+04] = hwnd = Handle to the window                             ;
;  Parm#2 = [esp+08] = uMsg = System defined message                           ;
;  Parm#3 = [esp+12] = wParam = Additional message inform. (1), uMsg-dependent ;
;  Parm#4 = [esp+16] = lParam = Additional message inform. (2), uMsg-dependent ;
;                                                                              ;
; Output = EAX = Status, encoding = f(message type)                            ; 
;------------------------------------------------------------------------------;

; Result of 1000000000 / 1000000 = 1000 , used for convert 
; nanoseconds per instruction (nsPI) to Megabytes per Second (MBPS)
; Decimal megabyte = 1000000 bytes (not binary 1048576)
; Second = 1000000000 nanoseconds
NSPI_TO_MBPS       EQU  1000

; 10000 milliseconds = 10 seconds, events wait time for
; communication with child thread
THREAD_TIME_LIMIT  EQU  10000  

; Entry point
WindowProc_Win1:
pushad
mov ebp,esp                   ; EBP = Pointer to stack frame
mov ebx,[ebp + 36 + 00]       ; EBX = hwnd
mov eax,[ebp + 36 + 04]       ; EAX = uMsg
mov ebp,[ebp + 36 + 08]       ; EBP = wParam 
; Select handler
cmp eax,WM_CREATE
je .WinCreate         ; Go if WM (Window Message) = OPEN WINDOW
cmp eax,WM_DESTROY
je .WinDestroy        ; Go if WM = CLOSE WINDOW
cmp eax,WM_COMMAND
je .WinCommand        ; Go if WM = USER INPUT
cmp eax,WM_TIMER
je .WinTimer          ; Go if WM = TIMER TICK
cmp eax,WM_PAINT
je .WinPaint          ; Go if WM = PAINT GRAPHICS
; Otherwise Default OS routine
; Here must be valid input parameters [1-4] in the stack
; +36 because 32 bytes PUSHAD + 4 bytes EIP
.WinDefaultProc:
mov ebp,esp
push dword [ebp + 36 + 12]
push dword [ebp + 36 + 08]
push dword [ebp + 36 + 04]
push dword [ebp + 36 + 00]
call [DefWindowProc]          ; DEFAULT WINDOW PROCEDURE
; Exit points
.WinFinish:   ; No EAX destroy here, otherwise window not closed at WinXP, Win7
mov [esp+28],eax
popad
ret
.WinStatus:
mov eax,1
jmp .WinFinish


;------------------ Handler for system event: window create -------------------;

.WinCreate:
; Initializing drawing context
mov [DrawParms + DRPM.DrawPreviousY],0
; Build buttons
lea esi,[DialogueContent_Win1]     ; ESI = Pointer to elements descriptors
mov edi,START_HANDLE_WIN1          ; EDI = Start handle
mov ebp,[Dialogue_Win1.hInstance]  ; EBP = Parent module handle = [wc.hInstance]
call CreateDialogueElements
; Initialize graphics context
push 0                           ; Parm#1 = Handle to exist DC, 0 = Application current screen
call [CreateCompatibleDC]        ; Create compatible Device Context for buffered video output 
mov [HandleMemDC],eax            ; Store handle
mov esi,eax                      ; ESI = Store handle for later use
xor eax,eax                      ; EAX = 0 for compact push 0
push eax                         ; Parm#6 = Offset in the file-mapping object, not used here
push eax                         ; Parm#5 = Handle to file-mapping object, not used here
push BitmapPointer               ; Parm#4 = Pointer to Bitmap Pointer variable
push DIB_RGB_COLORS              ; Parm#3 = Type of color data
push BitmapInfo                  ; Parm#2 = Pointer to Bitmap Descriptor
push esi                         ; Parm#1 = Handle to a Device Context
call [CreateDIBSection]
mov [HandleBitmap],eax              
push eax                         ; Parm#2 = Handle to a Bitmap
push esi                         ; Parm#1 = Handle to a Device Context
call [SelectObject]
; Create brushes
cld
lea esi,[BrushesList]            ; ESI = Pointer to color data
lea edi,[HandlesBrushes]         ; EDI = Pointer for store brushes handles
mov ebp,4                        ; EBP = Number of brushes
@@:
lodsd                            ; EAX = Color, RSI+4, select next color 
push eax                         ; Parm#1 = Color
call [CreateSolidBrush]
stosd                            ; Store brush handle, EDI+4
dec ebp
jnz @b                           ; Cycle for all brushes
; Create font for texts in the drawings window
xor eax,eax                      ; EAX = 0 for compact push 0
push eax                         ; Parm#14 = Pointer to font typename string, here not used
push VARIABLE_PITCH              ; Parm#13 = Font pitch and family
push CLEARTYPE_QUALITY           ; Parm#12 = Output quality
push CLIP_DEFAULT_PRECIS         ; Parm#11 = Clip precision
push OUT_OUTLINE_PRECIS          ; Parm#10 = Output precision
push DEFAULT_CHARSET             ; Parm#9  = Charset
push eax                         ; Parm#8  = Strike, here=0=none
push eax                         ; Parm#7  = Underline, here=0=none
push eax                         ; Parm#6  = Italic, here=0=none
push FW_DONTCARE                 ; Parm#5  = Weight of the font
push eax                         ; Parm#4 = Orientation
push eax                         ; Parm#3 = Escapment
push eax                         ; Parm#2 = Width
push 16                          ; Parm#1 = Height
call [CreateFont]
mov [HandleFont],eax 
; Create and set timer, for benchmarks progress (but not for measure)
push 0                      ; Parm#4 = Notify function pointer, not used here
push TIMER_TICK             ; Parm#3 = Timer ticks period, ms
push 0                      ; Parm#2 = Event ID
push ebx                    ; Parm#1 = Parent window handle
call [SetTimer]
; Wait thread started (1 of 2 waits)
; Wait for measurement thread run, this event at thread entry point
push THREAD_TIME_LIMIT                     ; Parm#2 = Time limit
push [DrawThreadParms + DTHP.EventStart]   ; Parm#1 = Handle for start event
call [WaitForSingleObject]
; Note status after [WaitForSingleObject] ignored, better check it
; End of initialization, enable draw counter
mov [Win1_Running],1
jmp .WinStatus


;------------------ Handler for system event: window destroy ------------------;
; include "Cancel" button handler

.WinDestroy:
; Stop measurement thread
mov [DrawThreadParms + DTHP.MeasureBreak],1
; Wait thread terminated (2 of 2 waits)
; Wait for measurement thread done, this event at thread exit point
push THREAD_TIME_LIMIT                    ; Parm#2 = Time limit
push [DrawThreadParms + DTHP.EventDone]   ; Parm#1 = Handle for done event
call [WaitForSingleObject]
; Note status after [WaitForSingleObject] ignored, better check it
; De-Initialize graphics context
push [HandleMemDC]
call [DeleteDC]
push [HandleBitmap]
call [DeleteObject]
; Delete brushes
cld
lea esi,[HandlesBrushes]   ; ESI = Pointer for store brushes handles
mov ebp,4                  ; EBP = Number of brushes
@@:
lodsd                      ; EAX = Brush handle
push eax                   ; Parm#1 = Handle
call [DeleteObject]
dec ebp
jnz @b
; Delete font
push [HandleFont]
call [DeleteObject]
;--- Delete timer ---
push 0                     ; Parm#2 = Set same as for SetTimer function
push ebx                   ; Parm#1 = Parent window handle
call [KillTimer]
; Destroy window, disable draw counter
mov byte [Win1_Running],0
; Close window
; This absent in the Window 0 handler, but it work because automatically
; closed when exit application.
push 0                     ; Parm#4 = Not used = 0
push 0                     ; Parm#3 = Not used = 0
push WM_CLOSE              ; Parm#2 = Message
push ebx                   ; Parm#1 = Handle
call [SendMessage]
; Terminate, destroy window
push 0                     ; Parm#1 = Exit code
call [PostQuitMessage]
xor eax,eax
jmp .WinFinish



;----------------- Handler for user event: buttons input ----------------------;

.WinCommand:
; Check phase of execution
cmp [Win1_Init],1
jne .WinDefaultProc   ; Run default OS handler if Window 1 not initialized yet
;--- Detecting buttons ---
cmp ebp,HANDLE_BUTTON_CANCEL1
je .WinDestroy                        ; Go if "Cancel" button
cmp ebp,HANDLE_BUTTON_RUN1
jne .WinDefaultProc                   ; Go if not "Resize" button

;------------------------ "Resize" button handler -----------------------------; 

; Set thread break flag
mov [DrawThreadParms + DTHP.MeasureBreak],1
; Wait for measurement thread done, this event at thread exit point
push THREAD_TIME_LIMIT                    ; Parm#2 = Time limit
push [DrawThreadParms + DTHP.EventDone]   ; Parm#1 = Handle for done event
call [WaitForSingleObject]
; Note status after [WaitForSingleObject] ignored, better check it
; Remove thread and associated handles, note can corrupt pointers
call DrawSessionStop
; Clear drawings variables 
mov [DrawParms + DRPM.DrawPreviousY],0
mov [DrawParms + DRPM.TimerCount],0
; Rescaling
lea edi,[DrawThreadParms]
finit
cmp [InputParms + IPB.UpdatedAsm],LATENCY_MODE
jae .L40
fld [edi + DTHP.StatMbpsMax]
push 90 10
jmp .L41
.L40:
fld [edi + DTHP.StatNspiMax]
push 9 2
.L41:
fidiv dword [esp+4]
fimul dword [esp]
fistp dword [esp+4]
; Restart
call DrawSessionStart   ; note can corrupt pointers
pop eax eax
test eax,eax
jnz @f
inc eax
@@:
mov [DrawParms + DRPM.ValueGridY],eax
; Rescale, same operation at drawsessionstart.inc
push eax -GRIDSTEPY
fild dword [esp]      ; ST0 = - grid step, pixels
fidiv dword [esp+4]   ; ST0 = ( - grid step, pixels ) / ( Y units per grid )      
fstp [DrawParms + DRPM.Ymultiplier]
pop eax eax
; Wait for measurement thread run, this event at thread entry point
push THREAD_TIME_LIMIT                    ; Parm#2 = Time limit
push [DrawThreadParms + DTHP.EventStart]  ; Parm#1 = Handle for start event
call [WaitForSingleObject]
; Note status after [WaitForSingleObject] ignored, better check it
mov [Win1_Running],1
jmp .WinStatus


;--------------- Handler for timer event: timer tick callback -----------------;

.WinTimer:
lea ecx,[Win1_Running]
lea edx,[DrawParms + DRPM.TimerCount]    ; For two variables: COUNT, PREVIOUS Y
xor eax,eax
; Check session running
cmp [ecx],al               ; 0=Stop, 1=First, 2=Progress
je .SkipTimer              ; Skip if halted
inc dword [edx]            ; Counter +1
; Check session abort
cmp [DrawThreadParms + DTHP.MeasureBreak],0
je .YetNotHalt 
mov [ecx],al               ; Running=0 means HALTED
; Set revisualization request
.YetNotHalt:
push 0         ; Parm#3 = Background erase flag, 0=Background unchanged
push 0         ; Parm#2 = Pointer to invalidated region, 0=All window
push ebx       ; Parm#1 = Window handle
call [InvalidateRect]      ; This cause callback WM_PAINT operation
.SkipTimer:
jmp .WinStatus


;-------------- Handler for system event: window paint callback ---------------;

.WinPaint:
push ebx
; Make paint context
push PaintStruc       ; Parm#2 = Pointer to PAINTSTRUCT structure
push ebx              ; Parm#1 = Parent window handle
call [BeginPaint]
mov [HandleDC],eax
; Prepare non-volatile parms, select handler = F (execution phase) 
mov al,[Win1_Running]
cmp al,0                   ; 0 = Halted
je .EndPaint
cmp al,1                   ; 1 = First drawings (grid)
je .FirstPaint             ; Go if this is first pass, draw axes
; continue draw y=f(x), not a first pass
; Setup drawings
lea ecx,[DrawThreadParms]              ; ECX = Pointer to draw parameters block
lea edi,[ecx + DTHP.MeasureArray]      ; EDI = Pointer to measure data array
mov ebp,[ecx + DTHP.MeasureCounter]    ; EBP = Measure counter
mov ebx,[ecx + DTHP.VisualCounter]     ; EBX = Visual counter
lea esi,[ebx+1]                        ; ESI = Divisor for averaging 
mov eax,ebx
shl eax,3         ; 2^3 = 8 bytes per entry (QWORD)
add edi,eax       ; EDI = Positioning pointer to current entry
mov [ecx + DTHP.VisualCounter],ebp
sub ebp,ebx       ; EBP = Required number of pixels for visual now
jbe .EndLine      ; Go if now present data already visualized
add ebx,GRIDBLANKX + 1 

; This cycle for EBP points draw per one timer tick
.DrawLine: 
; Calculate EAX = F(measurement result)
; Remember about can use integer compare for floation point operands,
; because shifted order. Instead FCOM / FSTSW AX / SAHF.
; Optimize registers usage, offsets relative R8 many times is not optimal.
; Better use 2 indexes: base and offset [ecx+edx] ?
; Better use helpers subroutines for comparision and conditional update ?
; Use median ordering for min-max also.
; Remove finit from body of cycle ? 
; Get current measurement, must be before ordering
; at this point FPU x87 initialized

finit
fld qword [edi]          ; ST0 = Current measured Clocks per Instruction (CPI)

; Calculate average CPI, nsPI, MBPS
push esi                          ; ESI = divisor for averaging
fld st0                           ; ST1 = ST0 = current measured value
fadd [DrawThreadParms + DTHP.StatCpiSum]
fst [DrawThreadParms + DTHP.StatCpiSum]
fidiv dword [esp]                 ; this access to R13 in the stack
fst [DrawThreadParms + DTHP.StatCpiAverage]   ; store average CPI
call HelperCPItoNSPI
fst [DrawThreadParms + DTHP.StatNspiAverage]  ; store average nsPI = f (average CPI)
call HelperNSPItoMBPS
fadd [DrawThreadParms + DTHP.StatMbpsSum]
fst [DrawThreadParms + DTHP.StatMbpsSum]      ; store average MBPS, averaging separately
fidiv dword [esp]                 ; this access to ESI in the stack
fstp [DrawThreadParms + DTHP.StatMbpsAverage]
pop esi

; Median ordering, also used for min/max, ordering by "bubble" method
; Note floating point values can be ordered by integer compare,
; because shifted exponent used at FP format
push ebx edi ebp

.L23:
lea ecx,[esi - 1]
lea edx,[DrawThreadParms + DTHP.MeasureArray]
mov ebx,edx                ; EBX  = Pointer to array first element, used later
lea edi,[edx + ecx*8]      ; EDI = Pointer to array last element, used later
jecxz .L21                 
xor eax,eax

.L20:
mov ebp,[edx + 0 + 4]
cmp ebp,[edx + 8 + 4]
jb .L22
ja .L24

mov ebp,[edx + 0 + 0]
cmp ebp,[edx + 8 + 0]
jbe .L22

.L24:
mov ebp,[edx + 0 + 0]
xchg ebp,[edx + 8 + 0]
mov [edx + 0 + 0],ebp

mov ebp,[edx + 0 + 4]
xchg ebp,[edx + 8 + 4]
mov [edx + 0 + 4],ebp

mov al,1
.L22:

add edx,8
dec ecx
jnz .L20        ; DEC/JNZ faster then LOOP
test eax,eax
jnz .L23 
.L21:

; Get minimum and maximum after median ordering
; minimum time - maximum bandwidth
fld qword [ebx]                    ; ST0 = first element, minimum CPI
fst [DrawThreadParms + DTHP.StatCpiMin]
call HelperCPItoNSPI
fst [DrawThreadParms + DTHP.StatNspiMin]
call HelperNSPItoMBPS
fstp [DrawThreadParms + DTHP.StatMbpsMax]
; maximum time - minimum bandwidth
fld qword [edi]                   ; ST0 = last element, maximum CPI
fst [DrawThreadParms + DTHP.StatCpiMax]
call HelperCPItoNSPI
fst [DrawThreadParms + DTHP.StatNspiMax]
call HelperNSPItoMBPS
fstp [DrawThreadParms + DTHP.StatMbpsMin]

; Get median CPI, nsPI after median ordering, note FPU already initialized 
lea ecx,[esi - 1]
shr ecx,1                     ; This branch for median ODD-sized array
fld qword [ebx + ecx*8]       ; Get median
jnc .L30                      ; Go if median for ODD size
fadd qword [ebx + ecx*8 + 8]
fld1
fadd st0,st0
fdivp st1,st0
.L30:
fst [DrawThreadParms + DTHP.StatCpiMedian]
call HelperCPItoNSPI
fstp [DrawThreadParms + DTHP.StatNspiMedian]

;; Get median MBPS, recalculate because reciprocal restrictions
lea ecx,[esi - 1]
shr ecx,1                      ; This branch for median ODD-sized array
fld qword [ebx + ecx*8]        ; Get median
pushf
call HelperCPItoNSPI
call HelperNSPItoMBPS
popf
jnc .L31                       ; Go if median for ODD size
fld qword [ebx + ecx*8 + 8]
call HelperCPItoNSPI
call HelperNSPItoMBPS
faddp st1,st0
fld1
fadd st0,st0
fdivp st1,st0
.L31:
fstp [DrawThreadParms + DTHP.StatMbpsMedian]

pop ebp edi ebx

; Start calculations for drawings Y = f( CPI ),
; ST0 = current value, loaded before re-ordering
call HelperCPItoNSPI
cmp [InputParms + IPB.UpdatedAsm],LATENCY_MODE
jae .L50
call HelperNSPItoMBPS
.L50:
fmul [DrawParms + DRPM.Ymultiplier]  ; ST0 = Pixel offset from BaseY=0, negative offset
push eax
fistp dword [esp]
pop eax

;- XOR EAX,EAX  ; DEBUG FOR HORIZONTAL LINE

; Setup coordinates for GUI,
; note at this point can set EAX=0 for draw horizontal line DEBUG.
add eax,SUBWINY-GRIDBLANKY - 1
lea edx,[ebx + 1]
lea ecx,[eax + 1]
; Work with Y-coordinate, adjust Y-coordinates for make solid line by Y
push esi
mov esi,[DrawParms + DRPM.DrawPreviousY]   ; ESI = DrawPreviousY
test esi,esi
jz @f
mov ecx,esi
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
mov esi,ecx
sub esi,eax
cmp esi,1
jbe @f
inc ecx           ; Make down extra-pixel for vertical line  
@@:
pop esi

; Store coordinates for rectangle as part of line Y=F(X)
; Add support approximation modes
push ebp edi
mov ebp,[InputParms.UpdatedApprox]
test ebp,ebp
jz .NoApproxMode
mov edi,00001111b
cmp ebp,APPROX_X16
je .CheckApproxMode
mov edi,00011111b
.CheckApproxMode:
test esi,edi
jnz .NoApproxMode
dec ebx
dec eax
add edx,2  
add ecx,2
.NoApproxMode:
pop edi ebp

; Build RECT structure with rectangle points coordinates
mov [GraphRect + RECT.left],ebx
mov [GraphRect + RECT.top],eax
mov [GraphRect + RECT.right],edx
mov [GraphRect + RECT.bottom],ecx

; Draw current point Y=F(X)
push [HandlesBrushes + 04]   ; Parm#3 = Handle to a color brush
push GraphRect               ; Parm#2 = Pointer to RECT strucnure
push [HandleMemDC]           ; Parm#1 = Handle to a Device Context
call [FillRect]

; Cycle with update parameters
add edi,8      ; EDI = Pointer to measurement results array
inc esi        ; ESI = Divisor for averaging
inc ebx        ; EBX = X-coordinate offset by visualization position
dec ebp        ; EBP = Number of pixels per timer tick
jnz .DrawLine  ; This cycle for EBP points draw per one timer tick
.EndLine:
jmp .RedrawTexts

; This branch for first paint, draw statical elements, not changed when draw
.FirstPaint:
mov [Win1_Running],2
mov [DrawParms + DRPM.TimerCount],0
; Draw
push [HandleFont]         ; Parm#2 = Handle to Selected Object
push [HandleMemDC]        ; Parm#1 = Handle to a Device Context
call [SelectObject]
push COLOR_TEXT_VALUES    ; Parm#2 = RDX = Text front color value = 00bbggrrh
push [HandleMemDC]        ; Parm#1 = RCX = Handle to a Device Context
call [SetTextColor]
push COLOR_TEXT_BACK      ; Parm#2 = Text background color value = 00bbggrrh
push [HandleMemDC]        ; Parm#1 = Handle to a Device Context
call [SetBkColor]
; Blank work field
xor eax,eax
lea ebx,[GraphRect]
mov [ebx + RECT.left],eax
mov [ebx + RECT.top],eax
mov [ebx + RECT.right],SUBWINX
mov [ebx + RECT.bottom],SUBWINY
push [HandlesBrushes + 8]        ; Parm#3 = Handle to a color brush
push ebx                         ; Parm#2 = Pointer to RECT strucnure
push [HandleMemDC]               ; Parm#1 = Handle to a Device Context
call [FillRect]

; Coordinate X-grid, vertical lines
mov esi,GRIDBLANKX          ; ESI = Base point, X
mov edi,GRIDX               ; EDI = Number of lines
.XVgrid:
lea eax,[esi + 1]            ; Line width = 1 pixel
mov [ebx + RECT.left],esi
mov [ebx + RECT.top],0
mov [ebx + RECT.right],eax
mov [ebx + RECT.bottom],SUBWINY-GRIDBLANKY
push [HandlesBrushes + 00]  ; Parm#3 = Handle to a color brush
push ebx                    ; Parm#2 = Pointer to RECT strucnure
push [HandleMemDC]          ; Parm#1 = Handle to a Device Context
call [FillRect]             ; Fill rectangle function
add esi,GRIDSTEPX           ; Add X grid step
dec edi                     ; Cycle for X-grid vertical lines
jnz .XVgrid

;; Coordinate Y-grid, horizontal lines
mov esi,SUBWINY-GRIDBLANKY  ; ESI = Base point, Y
mov edi,GRIDY               ; EDI = Number of lines
.YHgrid:
lea eax,[esi - 1]
mov [ebx + RECT.left],GRIDBLANKX
mov [ebx + RECT.top],eax
mov [ebx + RECT.right],SUBWINX
mov [ebx + RECT.bottom],esi
push [HandlesBrushes + 00]  ; Parm#3 = Handle to a color brush
push ebx                    ; Parm#2 = Pointer to RECT strucnure
push [HandleMemDC]          ; Parm#1 = Handle to a Device Context
call [FillRect]             ; Fill rectangle function
sub esi,GRIDSTEPY           ; Subtract Y grid step
dec edi                     ; Cycle for Y-grid vertical lines
jnz .YHgrid

; Initializing values for X-grid vertical lines and Y-grid horizontal lines texts
mov bx,200h + GRIDX     ; BH = Counter, Pass#1 = XGrid (BH=2), Pass#2 = YGrid (BH=1)
                        ; BL = Number of lines, X
mov esi,GRIDBLANKX      ; ESI = Base point, X
mov edi,SUBWINY-20      ; EDI = Base point Y, 20 pixels up
xor ebp,ebp             ; EBP = Units value, X starts from 0

; This cycle both for X and Y grids texts, X/Y logic dynamically selected 
.BothGrids:
; Build ASCII string
push ebx edi
lea edi,[BigTexts]   ; EDI = Pointer for print decimal string as ASCII
mov edx,edi
mov bl,0             ; BL = Template control for print number, 0=No template
mov eax,ebp          ; EAX = Value for print, Bytes/KB/MB
call DecimalPrint32  ; Print number
mov ecx,edi
sub ecx,edx
mov al,0
stosb                ; Store terminator byte = 0
pop edi ebx
; Calculate correction value for Y axis grid values print
mov edx,9
sub edx,ecx         ; R10 = chars positions count
jbe .RejectAddend   ; Go skip if correction not required 
imul edx,edx,7
; Reject corrections if horizontal axis or overflow
cmp bh,1            ; BH=1 means Y-mode , BH=2 means X-mode
je .DoneAddend      ; Go with EDX = correction if branch for vertical axis text 
.RejectAddend:
xor edx,edx
.DoneAddend:
; Set region for text string write
lea eax,[edi + 18]    ; EAX = Y coordinate down/right corner  
push eax              ; Parm#5 = Y coordinate down/right corner
lea eax,[esi + edx + 65]
push eax              ; Parm#4 = X coordinate down/right corner
push edi              ; Parm#3 = Y coordinate up/left corner
lea eax,[esi + edx]   
push eax              ; Parm#2 = X coordinate up/left corner
push GraphRect        ; Parm#1 = Pointer to RECT structure to store parameters
call [SetRect]        ; Set region rectangle function 
; Draw text
push DT_LEFT          ; Parm#5 = Method of formatting/alignment of text string
push GraphRect        ; Parm#4 = Pointer to RECT structure, initialized by [SetRect] function
push -1               ; Parm#3 = String length, -1 means NULL-terminated string
push BigTexts         ; Parm#2 = Pointer to text string
push [HandleMemDC]    ; Parm#1 = Handle to a Device Context
call [DrawText]        ; Draw text function 

; Cycle for lines of selected grid
lea ecx,[DrawParms]
cmp bh,1                          ; BH=1 means Y-mode , BH=2 means X-mode
je .YGrid
; X-mode entry point
add ebp,[ecx + DRPM.ValueGridX]   ; Addend per X grid
add esi,GRIDSTEPX
jmp .CycleGrids
; Y-mode entry point
.YGrid:
add ebp,[ecx + DRPM.ValueGridY]   ; Addend per Y grid
sub edi,GRIDSTEPY
.CycleGrids:
dec bl
jnz .BothGrids

; Cycle for horizontal and vertical grids
mov bl,GRIDY - 1                  ; R14D = Number of lines, Y
mov esi,GRIDBLANKX - 68                       ; ESI = X coordinate
mov edi,SUBWINY-GRIDBLANKY - GRIDSTEPY - 15   ; EDI = Y coordinate
mov ebp,[ecx + DRPM.ValueGridY]  ; EBP = Units value, Y starts with offset
dec bh             ; BH = Counter, Pass#1 = XGrid (BH=2), Pass#2 = YGrid (BH=1)
jnz .BothGrids

; Prepare for X-units, here change color
push COLOR_TEXT_UNITS     ; Parm#2 = Text front color value = 00bbggrrh
push [HandleMemDC]        ; Parm#1 = Handle to a Device Context
call [SetTextColor]
mov eax,[DrawParms + DRPM.SelectUnits]
lea esi,[U_B]             ; ESI = Pointer to text string, size units select
cmp al,0
je @f
lea esi,[U_KB]
cmp al,1
je @f
lea esi,[U_MB]
@@:
mov edx,SUBWINX-49        ; EDX = X coordinate up/left corner
mov ecx,SUBWINY-43        ; ECX = Y coordinate up/left corner
mov ebx,SUBWINX-13        ; EBX = X coordinate down/right corner
mov eax,SUBWINY-25        ; EAX = Y coordinate down/right corner
call HelperTextConstDraw

; Prepare for Y-units, use color from previous step
lea esi,[U_MBPS]             ; ESI = Pointer to text string, performance units 
cmp [DrawParms + DRPM.SelectMode],0
je @f
lea esi,[U_NS]
@@:
mov edx,7                    ; EDX = X coordinate up/left corner
mov ecx,5                    ; ECX = Y coordinate up/left corner
mov ebx,40+7+20 + 24         ; EBX = X coordinate down/right corner
mov eax,18+5                 ; EAX = Y coordinate down/right corner
call HelperTextConstDraw

; Print statistics table, lines and texts
; horizontal sequence of strings:  CPI, nsPI, MBPS
lea edi,[DrawTableUp]       ; EDI = Pointer to text strings short pointers table
mov edx,522                 ; RDX = X coordinate up/left corner
mov ecx,27                  ; ECX = Y coordinate up/left corner
mov ebx,522 + 40            ; EBX = X coordinate down/right corner
mov eax,27 + 17             ; EAX = Y coordinate down/right corner
@@:
movzx ebp,word [edi]          ; Get 16-bit compact-encoded offset
add edi,2                     ; Update pointer to next offset
test ebp,ebp
jz @f                         ; Go exit if terminator (0) detected
lea esi,[ebp + BasePointGui]  ; Convert compact offs. to 64-bit absolute address
call HelperTextConstDraw
add edx,64                    ; Shift 64 points right, X+64
add ebx,64                    ; Shift 64 points right, X+64
jmp @b                        ; cycle for: CPI, nsPI, MBPS 
@@:

; vertical sequence of strings:  maximum, minimum, average, median
lea edi,[DrawTableLeft]       ; EDI = Pointer to text strings short pointers table
mov edx,458                   ; RDX = X coordinate up/left corner
mov ecx,27 + 19               ; ECX = Y coordinate up/left corner
mov ebx,458 + 60              ; EBX = X coordinate down/right corner
mov eax,27 + 19 + 17          ; EAX = Y coordinate down/right corner
@@:
movzx ebp,word [edi]          ; Get 16-bit compact-encoded offset
add edi,2                     ; Update pointer to next offset
test ebp,ebp
jz @f                         ; Go exit if terminator (0) detected
lea esi,[ebp + BasePointGui]  ; Convert compact offs. to 64-bit absolute address
call HelperTextConstDraw
add ecx,15                    ; Shift 15 points down, Y+15
add eax,15                    ; Shift 15 points down, Y+15
jmp @b                        ; cycle for: maximum, minimum, average, median
@@:

; lines of table, up horizontal line and left vertical line
lea ebx,[GraphRect]
mov [ebx + RECT.left],519
mov [ebx + RECT.top],27 + 17
mov [ebx + RECT.right],519 + 240
mov [ebx + RECT.bottom],27 + 17 + 2
push [HandlesBrushes + 12]    ; Parm#3 = Handle to a color brush
push ebx                      ; Parm#2 = Pointer to RECT strucnure
push [HandleMemDC]            ; Parm#1 = Handle to a Device Context
call [FillRect]               ; Fill rectangle function, up horizontal line
mov [ebx + RECT.left],518
mov [ebx + RECT.top],27 + 19
mov [ebx + RECT.right],518 + 2
mov [ebx + RECT.bottom],27 + 19 + 17*4
push [HandlesBrushes + 12]    ; Parm#3 = Handle to a color brush
push ebx                      ; Parm#2 = Pointer to RECT strucnure
push [HandleMemDC]            ; Parm#1 = Handle to a Device Context
call [FillRect]               ; Fill rectangle function, left vertical line

; Target method CPU instructions dump
lea esi,[InputParms]
mov ebp,[esi + IPB.DumpMethodLength]
mov esi,[esi + IPB.DumpMethodStart]
cmp ebp,128
jbe @f
mov ebp,128
@@:
push COLOR_TEXT_DUMP      ; Parm#2 = Text front color value = 00bbggrrh
push [HandleMemDC]        ; Parm#1 = Handle to a Device Context
call [SetTextColor]

xor eax,eax
.AsmDump:
; Write string for one hex byte
push eax
push eax
lea edi,[BigTexts]
lodsb
call HexPrint8
mov al,0
stosb
pop eax
; Set coordinates
push esi
mov esi,eax                 ; EAX = counter, ESI = counter temporary copy #1
mov edx,SUBWINX - 250 + 64  ; EDX = X coordinate, up left corner
mov ecx,95 + 48             ; ECX = Y coordinate up/left corner
mov ebx,SUBWINX - 005 + 64  ; EBX = X coordinate down/right corner
mov eax,95 + 18 + 48        ; EAX = Y coordinate down/right corner 
; Adjust coordinates
push esi
and esi,07h                 ; Bits 0-2 used for X-positioning
imul esi,esi,21             ; X scale = 21 pixels per char
add edx,esi                 ; Add offset to X-start
add ebx,esi                 ; Add offset to X-end
pop esi
shr esi,3
and esi,0Fh                 ; Bits 3-6 used for Y-positioning
imul esi,esi,18             ; Y scale = 18 pixels per char 
add ecx,esi                 ; Add offset to Y-start
add eax,esi                 ; Add offset to Y-end
; Draw one hex byte
lea esi,[BigTexts]
call HelperTextConstDraw
pop esi eax
inc eax
dec ebp
jnz .AsmDump 

; TSC clock frequency
lea edi,[DrawValueTSC]
mov word [edi],0000h + '?'      ; This char '?' used if error
mov eax,dword [OutputParms + OPB.TSCfrequencyMHz + 0]  ; EAX = TSC Frequency, MHz
mov edx,dword [OutputParms + OPB.TSCfrequencyMHz + 4]
mov ecx,eax
or ecx,edx                  ; Frequency = 0 means error
jz @f                       ; Go skip value write if error
mov bx,0100h                ; BX=Template X.Y , 1 digit after '.'
call DoublePrint
mov al,0
stosb
@@:
push COLOR_TEXT_INFO        ; Parm#2 = Text front color value = 00bbggrrh
push [HandleMemDC]          ; Parm#1 = Handle to a Device Context
call [SetTextColor]
mov edx,SUBWINX - 187       ; EDX = X coordinate up/left corner
mov ecx,6                   ; ECX = Y coordinate up/left corner
mov ebx,SUBWINX - 5         ; EBX = X coordinate down/right corner
mov eax,6+18                ; EAX = Y coordinate down/right corner
lea esi,[DrawTextTSC]       ; ESI = Pointer to text string
call HelperTextConstDraw

; This text strings must be updated at each visualization tick
.RedrawTexts:

; Initializing parameters for benchmark statistics, bandwidth and timings
mov ebp,0300h

; Print timings statistics, CPI minimum, maximum, average, median
; note CPI = Clocks Per Instruction
lea esi,[DrawThreadParms + DTHP.StatCpiMin]  ; ESI = variables group base
mov edx,SUBWINX - 250
mov ebx,SUBWINX - 190 
call HelperWriteStatisticsYconst

; Print timings statistics, nsPI minimum, maximum, average, median
; note nsPI = nanoseconds per instruction
lea esi,[DrawThreadParms + DTHP.StatNspiMin]  ; ESI = variables group base
mov edx,SUBWINX - 250 + 64*1
mov ebx,SUBWINX - 190 + 64*1 
call HelperWriteStatisticsYconst

; This number template for Megabytes per Second
mov ebp,0200h

; Print timings statistics, MBPS minimum, maximum, average, median
; note MBPS = megabytes per second
; Note Min/Max here swapped, because min. time means max. bandwidth
lea esi,[DrawThreadParms + DTHP.StatMbpsMax]  ; ESI = variables group base
mov edx,SUBWINX - 250 + 64*2
mov ebx,SUBWINX - 150 + 64*2 
call HelperWriteStatisticsYconst

; End of prepare, visual prepared objects
.DrawVisual:
xor eax,eax          ; EAX = 0 for compact push 0
push SRCCOPY         ; Parm#9 = Raster operation code
push eax             ; Parm#8 = Y source
push eax             ; Parm#7 = X source
push [HandleMemDC]   ; Parm#6 = Handle of source Device Context
push SUBWINY         ; Parm#5 = Height
push SUBWINX         ; Parm#4 = Width
push SHIFTY          ; Parm#3 = Y destination
push SHIFTX          ; Parm#2 = X destination
push [HandleDC]      ; Parm#1 = Handle of destination Device Context
call [BitBlt]                       
; Delete paint context
.EndPaint:
push PaintStruc         ; Parm#2 = Pointer to PAINTSTRUCT structure
push dword [esp+4]      ; Parm#1 = Parent window handle
call [EndPaint]         ; This WinAPI returns BOOL status, but ignored
; Paint callback handling done
pop ebx
jmp .WinStatus


;------------------------ Helpers for calculations ----------------------------;

;--- Convert CPI (Clocks per Instruction) to nsPI (nanosec. per instr.) ----;
; INPUT:  ST0 = CPI as floating point double                                ;
; OUTPUT: ST0 = nsPI as floating point double                               ;
;---------------------------------------------------------------------------;
HelperCPItoNSPI:
fmul [OutputParms.TSCperiodNs]
ret

;--- Convert nsPI (nanosec. per instr.) to MBPS (Megabytes per Second) -----;
; INPUT:  ST0 = nsPI as floating point double                               ;
; OUTPUT: ST0 = MBPS  as floating point double                              ;
;---------------------------------------------------------------------------;
HelperNSPItoMBPS:
push NSPI_TO_MBPS                  ; This constant for decimal megabytes and ns.
mov eax,[InputParms.OperandWidth]  ; RAX = bits per instruction
shr eax,3                          ; RAX = bytes per instruction 
push eax
fild dword [esp+4]   ; ST0 = 1000, f(decimal megabytes, ns), ST1 = nsPI 
fxch st1             ; ST0 = nsPI , ST1 = 1000 
fdivp st1,st0        ; ST0 = 1000 / nsPI = decimal megainstructions per second
fimul dword [esp+0]  ; ST0 = decimal megabytes per second
fimul [InputParms + IPB.UpdatedThreads]  ; Multiply by number of threads
pop eax eax
ret
 

;-------------------------- Helpers for drawings ------------------------------;

; Note about stack re-alignment when subroutines call

;--- Print text string ----------------------------------------------;
; INPUT:   ESI = Pointer to text string                              ;   
;          EDX = xLeft                                               ; 
;          ECX = yTop                                                ;
;          EBX = xRight                                              ;
;          EAX = yBottom                                             ;
;                                                                    ;
; OUTPUT:  None in the CPU registers                                 ;
;          All registers (EAX, EBX, ECX, EDX, ESI, EDI, EBP)         ; 
;          is  non volatile for this subroutine                      ;    
;--------------------------------------------------------------------;
HelperTextConstDraw:
push eax ecx edx
; Set region rectangle 
push eax            ; Parm#5 = yBottom 
push ebx            ; Parm#4 = xRight
push ecx            ; Parm#3 = yTop   
push edx            ; Parm#2 = xLeft                      
push GraphRect      ; Parm#1 = Pointer to RECT structure to store parameters
call [SetRect]      ; Set region rectangle function 
; Draw text
push DT_LEFT        ; Parm#5 = Method of formatting/alignment of text string
push GraphRect      ; Parm#4 = Pointer to RECT structure, initialized by [SetRect] function
push -1             ; Parm#3 = String length, -1 means NULL-terminated string
push esi            ; Parm#2 = Pointer to text string
push [HandleMemDC]  ; Parm#1 = Handle of a Device Context
call [DrawText]     ; Draw text function 
; Done
pop edx ecx eax
ret

;--- Print floating point value, double -----------------------------;
; INPUT:   EDX = X up left position                                  ;
;          ECX = Y up left position                                  ; 
;          EBX = X down right position                               ;
;          EAX = Y down right position                               ;
;          EBP = numeric template for floating point print           ;
;          EDI:ESI = floating point value, double, 64-bit            ;
;                    EDI = High32 , ESI = Low32                      ;
;                                                                    ;
; OUTPUT:  None in the CPU registers                                 ;
;          All registers (EAX, EBX, ECX, EDX, ESI, EDI, EBP)         ; 
;          is  non volatile for this subroutine                      ;    
;--------------------------------------------------------------------;      
HelperTextVarDouble:
push eax ecx edx
; Build text string
push eax ebx ecx edx edi
lea edi,[BigTexts]
mov ecx,11-3         ; 11 (not 9) for blank if variable X-size font 
mov ax,0000h + '?'   ; Blank field with "?" means yet not determined
stosw
mov al,' '
rep stosb            ; Blank entire field, if next value shorter then previous
mov al,0
stosb
pop edi
mov ebx,ebp          ; BH, BL = template
mov eax,esi          ; EDX:EAX = value, double
mov edx,edi
test eax,eax
jnz .L1
test edx,edx
jz .L0
.L1:
push edi
lea edi,[BigTexts]
call DoublePrint
pop edi
.L0:
pop edx ecx ebx eax
; Draw text string
push esi
lea esi,[BigTexts]
call HelperTextConstDraw
pop esi 
; Done
pop edx ecx eax
ret


;--- Print cycle for Min, Max, Average, Median ----------------------;
; INPUT:   EDX = X up left position                                  ;
;          ECX = Y up left position, for common entry point only     ; 
;          EBX = X down right position                               ;
;          EAX = Y down right position, for common entry point only  ;
;          EBP = numeric template for floating point print           ;
;          ESI = pointer to variables group                          ;
;                                                                    ;
; OUTPUT:  None in the CPU registers                                 ;
;          RSI, EBP is non volatile for this subroutine              ;
;--------------------------------------------------------------------;      

; Entry point for Y fixed (R8D, EAX pre-defined)
HelperWriteStatisticsYconst:
mov ecx,24 + 23
mov eax,42 + 23

; Common entry point
HelperWriteStatistics:
mov edi,4
@@:
push esi edi
mov edi,[esi+4]
mov esi,[esi+0]
call HelperTextVarDouble
add ecx,15
add eax,15
pop edi esi
add esi,8                  ; Sequental access to Min, Max, Average, Median
dec edi
jnz @b
ret
 
