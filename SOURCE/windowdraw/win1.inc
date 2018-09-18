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
; Initializing statistics
call InitStatistics
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
; End of initialization, enable draw counter
mov [Win1_Running],1
jmp .WinStatus


; Handler for system event: window destroy
.WinDestroy:
push rcx rdx r8 r9
sub rsp,32
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
jne .WinDefaultProc                   ; Go if not "Run" button
;--- Handling Run (Restart or Resize) button ---
call InitStatistics
;--- Get Maximum MBPS, check validity ---
mov eax,[MaximumMBPS]               ; EAX = Maximum MBPS
mov ecx,eax                         ; ECX = Maximum MBPS copy
jecxz .InvalidMax                   ; Go if this is first pass after open Window 1

; Temporary fix for Y units per pixel bad approximation
; cmp eax,8000
; jae @f
; mov eax,8000
; @@:

; Calculate Y Range
shr eax,2                           ; EAX = Maximum MBPS / 4
lea eax,[eax+ecx]                   ; EAX = 5/4 * Maximum MBPS
mov ecx,10000
cmp eax,100000
jae @f
mov ecx,1000
cmp eax,10000
jae @f
mov ecx,100
cmp eax,1000
jae @f
mov ecx,10
cmp eax,100
jae @f
mov ecx,1
; Make Range is multiplicity of 10^X
@@:
xor edx,edx
div ecx
mul ecx
; Calculate MBPS per Grid
push rax
mov ecx,10
xor edx,edx
div ecx
mov [ValueGridY],eax          ; DWORD [ValueGridY] = Range / 10 = MBPS per Grid
pop rax
; Calculate MBPS per Pixel
; mov ecx,SUBWINY
; xor edx,edx
; div ecx
; mov [YdivisorFloat],eax     ; DWORD [Ydivisor] = Range / SUBWINY = MBPS per Pixel
finit                         ; Initialize x87 FPU
push rax
push SUBWINY                  ; x87 can get data from memory only
fild qword [rsp+8]            ; st0 = MBPS
fidiv dword [rsp+0]           ; st0 = MBPS / SUBWINY
fst [YdivisorFloat] 
pop rax rax                   ; Release stack
; Exit
.InvalidMax:
jmp .WinStatus


; Handler for timer event: tick
.WinTimer:
lea rcx,[Win1_Running]
lea rdx,[DrawCount]        ; For two variables: COUNT, PREVIOUS Y
xor eax,eax                ; This clear entire RAX
cmp [rcx],al               ; 0=Stop, 1=First, 2=Progress
je .SkipTimer              ; Skip if halted
inc dword [rdx]            ; Counter +1
cmp dword [rdx],SUBWINX-3
jbe .YetNotHalt            ; Skip if limit not reached yet
mov qword [rdx],rax        ; Count=0 means READY TO NEXT START, clear BOTH DWORDS: COUNT and PREVIOUS Y 
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
je .FirstPaint
; Here draw progress support
lea rcx,[DrawCount]
mov eax,[rcx]        ; EAX = Draw ticks counter
cmp eax,SUBWINX-3
ja .GoDraw
; Set Xleft, Xright
mov [rdi+RECT.left],eax
inc eax
mov [rdi+RECT.right],eax
; Save registers, skip if draw ticks counter = 0 
push rbx rcx rsi rdi
test eax,eax
jz .SkipZero         ; Go skip if draw ticks counter = 0 

; BEGIN TARGET BENCHMARKS MEASUREMENTS
; Prepare parameters
lea r14,[BenchRepeats]
mov ebp,[InputParms.NumRepeats]
mov [r14+00],rbp             ; BenchRepeats
mov rcx,rax                  ; RCX = Draw tick number
mov eax,[r14+24]             ; Xmultiplier
mul rcx
mov [r14+08],rax             ; BenchInstructions 
lea rbx,[ThreadsControl]
mov ecx,[InputParms.NumThreads]
push rbx rcx
@@:
mov [rbx+THCTRL.SizeInst],rax
add rbx,THCTRL_SIZE
loop @b
pop rcx rbx
; Select Single-Thread or Multi-Thread
cmp ecx,1
ja .MultiThread             ; Go if Parallel Threads enabled
call SingleThreadBenchmark  ; Call target routine with measure Delta TSC, for single-thread
jmp .AfterThread
.MultiThread:
call MultiThreadRestart     ; Call target routine with measure Delta TSC, for multi-thread
.AfterThread:
mov rsi,rax                 ; Copy result RSI = RAX = Delta TSC 
; END TARGET BENCHMARKS MEASUREMENTS

lea rdi,[StatClksMinimum]  ; RDI = Base address for variables group
; Calculate clocks per instruction
; Number of target instructions calculated by tables
xchg rax,rbx               ; RBX = Delta TSC, clocks, XCHG for compact encoding
mov rax,[rdi+40]           ; RAX = BenchRepeats (measurement repeats)
mul qword [rdi+48]         ; Multiply by BenchInstructions (total instructions)   
; Calculate instructions per clock
test rdx,rdx               ; RAX = Total instructions per this clocks, RDX = High 64 bits
jnz .OverflowIPC           ; Go if error = instructions count overflow
mov ecx,1000
div rcx                    ; This because must be Clocks*1000
xchg rax,rbx               ; RAX = Delta TSC , RBX = Instructions/1000 
xor edx,edx                                 
test rbx,rbx
jz .OverflowIPC            ; Go if error = clocks count underflow
div rbx                    ; RAX = Delta TSC / (Instructions/1000)
jmp .NoOverflowIPC         ; RAX = Clocks per Instruction * 1000
.OverflowIPC:
xor eax,eax                ; Clear value if error
.NoOverflowIPC:
; Store statistics, RAX = Clocks per Instruction * 1000
cmp rax,[rdi+00]           ; Compare Minimum Clocks Per Instruction
jae @f
mov [rdi+00],rax           ; Store Minimum Clocks Per Instruction
@@:
cmp rax,[rdi+08]           ; Compare Maximum Clocks Per Instruction
jbe @f
mov [rdi+08],rax           ; Store Maximum Clocks Per Instruction
@@:
add [rdi+24],rax           ; Add to Sum
mov rax,[rdi+24]           ; Get Sum
mov ecx,[rdi+32]           ; RCX = DrawCount 
xor edx,edx
div rcx                    ; Divide Sum by Number of iterations passed 
mov [rdi+16],rax           ; Store Average Clocks Per Instruction
; Calculate Y-coordinate, here RSI = Delta TSC
mov rax,[TscClockHz]       ; RAX = Frequency Hz
; Multiply by number of threads 
mov edx,[InputParms.NumThreads]
mul rdx
; Convert to MHz
; mov ebx,1000000            ; Constant for decimal megabytes
; xor edx,edx                ; RDX = 0 , dividend high qword
; div rbx                    ; RAX = Frequency/1000000
; mul qword [rdi+40]         ; BenchRepeats
; test rdx,rdx
; jnz .OverflowY
; mul qword [rdi+48]         ; BenchInstructions
; test rdx,rdx
; jnz .OverflowY
; mul qword [rdi+56]         ; BenchBPI (bytes per instruction) 
; test rdx,rdx               ; RAX = Frequency * Decimal Megabytes transferred
; jnz .OverflowY
; test rsi,rsi
; jz .OverflowY                     
; div rsi                    ; RAX = ( Frequency * Decimal Megabytes transferred ) / clocks
; mov ebx,[rdi+68]           ; Ydivisor
; xor edx,edx
; div rbx                    ; RAX = Values for Y-coordinates
; jmp .NoOverflowY
; .OverflowY:
; xor eax,eax
; .NoOverflowY:
finit                        ; Initialize x87 FPU
push rax                     ; RAX = Frequency * Threads
push rsi                     ; RSI = Delta TSC  
push 1000000                 ; Constant for decimal megabytes
fild qword [rdi+40]          ; BenchRepeats
fild qword [rdi+48]          ; BenchInstructions
fild qword [rdi+56]          ; BenchBPI (bytes per instruction)  
fild qword [rsp+08]          ; Delta-TSC
fild qword [rsp+16]          ; st0 = Frequency * Threads 
fidiv dword [rsp+00]         ; st0 = st0 / 1000000
fmul st0,st4                 ; st0 = st0 * BenchRepeats
fmul st0,st3                 ; st0 = st0 * BenchInstructions
fmul st0,st2                 ; st0 = st0 * BenchBPI  
fdiv st0,st1                 ; st0 = st0 / DeltaTSC
fdiv dword [rdi+68]          ; YdivisorFloat
fist dword [rsp+16]
pop rax rax rax              ; Release stack, get result = EAX = pixel offset
; Prevent overflows
; test eax,eax
; js .NeedCorrect            ; Negative value is invalid
; cmp eax,SUBWINY-3        
; jbe .SkipCorrect           ; Not invalid if below limit 
; .NeedCorrect:
; xor eax,eax
; .SkipCorrect:

test eax,eax
jns .SkipZero                ; Negative value is invalid
xor eax,eax

; Restore registers
.SkipZero:
pop rdi rsi rcx rbx
; Generating Ytop, Ybottom, positioning at drawings sub-screen
mov edx,SUBWINY-2
mov ebp,SUBWINY-3
sub edx,eax
sub ebp,eax
; Work with Y-coordinate
mov eax,[rcx+04]   ; EAX = DrawPreviousY
test eax,eax
jz @f
mov ebp,eax
@@:
mov [rcx+04],edx   ; Set DrawPreviousY
jnz @f
inc ebp
inc edx
@@:
cmp edx,ebp
jbe @f
xchg edx,ebp      ; Must be Down > Top
@@:
jne @f
inc ebp           ; Make Down+1 if Top=Down
@@:
mov eax,ebp
sub eax,edx
cmp eax,1
jbe @f
inc ebp            ; Make down extra-pixel for vertical line  
@@:
; Set Ytop, Ybottom
mov [rdi+RECT.top],edx
mov [rdi+RECT.bottom],ebp
; Draw current point Y=F(X)
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[HandlesBrushes+08]   ; Parm#3 = R8 = Handle to a color brush
call [FillRect]
; Go draw entry
.GoDraw: 
mov ebp,6   ; 9+128 if full text
jmp .EntryDrawText
; Common settings for text font
.FirstPaint:
; Initializing
mov byte [rcx],2                    ; [rcx] = [Win1_Running]
mov [DrawCount],ebp
; Draw
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,[HandleFont]         ; Parm#2 = RDX = Handle to Selected Object
call [SelectObject]
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_UNITS     ; Parm#2 = RDX = Text front color value = 00bbggrrh
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
mov r12d,1                   ; R12D = Base point, X
mov r13d,SUBWINY-1           ; R13D = Base point, Y
mov r14d,GRIDX               ; R14D = Number of lines
.XVgrid:
lea eax,[r12d+1]             ; Line width = 1 pixel
mov [rdi+RECT.left],r12d
mov [rdi+RECT.top],ebp
mov [rdi+RECT.right],eax
mov [rdi+RECT.bottom],r13d
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[HandlesBrushes+00]   ; Parm#3 = R8 = Handle to a color brush
call [FillRect]              ; Fill rectangle function
add r12d,GRIDSTEPX
dec r14d
jnz .XVgrid
; Coordinate Y-grid, horizontal lines
mov r12d,SUBWINY-1           ; R12D = Base point, Y
mov r13d,GRIDY               ; R13D = Number of lines
.YHgrid:
lea eax,[r12d-1]
mov [rdi+RECT.left],1
mov [rdi+RECT.top],eax
mov [rdi+RECT.right],SUBWINX
mov [rdi+RECT.bottom],r12d
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[HandlesBrushes+00]   ; Parm#3 = R8 = Handle to a color brush
call [FillRect]              ; Fill rectangle function
sub r12d,GRIDSTEPY
dec r13d
jnz .YHgrid
; Numbers for X-grid vertical lines and Y-grid horizontal lines
push rbx                     ; Save RBX=Handle, this also important for stack alignment: 5 parms. functions
mov bh,2                     ; BH = Counter, Pass#1 = XGrid, Pass#2 = YGrid
mov r14d,GRIDX               ; R14D = Number of lines, X
.BothGrids:
mov r12d,7                   ; R12D = Base point, X
mov r13d,SUBWINY-20          ; R13D = Base point Y
xor ebp,ebp                  ; EBP = Units value
.XVYHtext:
; Set region for text string write
; Here don't align stack because PUSH RBX
mov rcx,rdi                  ; Parm#1 = RCX = Pointer to RECT structure to store parameters
mov edx,r12d                 ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,r13d                 ; Parm#3 = R8 = Y coordinate up/left corner
lea r9d,[r12d+65]            ; Parm#4 = R9 = X coordinate down/right corner
lea eax,[r13d+18]
push rax                     ; Parm#5 = Y coordinate down/right corner
sub rsp,32                   ; Create parameters shadow
call [SetRect]               ; Set region rectangle function 
add rsp,32+8                 ; Remove parameters shadow and one parameter
; Built ASCII string
push rdi
lea rdi,[BigTexts]           ; RDI = Pointer for print decimal string as ASCII
mov rdx,rdi
mov bl,0                     ; BL = Template control for print number, 0=No template
mov eax,ebp                  ; EAX = Value for print, Bytes/KB/MB
call DecimalPrint32          ; Print number
mov al,0
stosb                        ; Store terminator byte = 0
pop rdi
; Draw text
; Here don't align stack because PUSH RBX
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
add ebp,[ValueGridX]
add r12d,GRIDSTEPX
jmp .CycleGrids
.YGrid:
add ebp,[ValueGridY]
sub r13d,GRIDSTEPY
.CycleGrids:
dec r14d
jnz .XVYHtext 
; Cycle for horizontal and vertical grids
mov r14d,GRIDY               ; R14D = Number of lines, Y
dec bh
jnz .BothGrids
pop rbx
; Text strings
; Can run this cycle with smaller EBP for entry when drawing line Y=F(X) 
mov ebp,9+128
.EntryDrawText:
mov r14,[DumpMethodStart]
mov r13d,[DumpMethodLength]
.TextConst:
; Code dump, iterations with EBP = 9+16 ... 9+1 
cmp ebp,9
jbe .NotDump                 ; Go skip if this pass not for dump
dec r13d
js .SkipPass                 ; Go skip if dump size already done
cmp ebp,9+128
jne @f                       ; Go skip color if not a first dump pass
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_DUMP      ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
@@:
lea r12,[BigTexts]
push rdi
mov rdi,r12
mov al,[r14]
inc r14
call HexPrint8
mov al,0
stosb
pop rdi
; Set coordinates
mov edx,SUBWINX-172
mov r8d,6+150
mov r9d,SUBWINX-005
mov eax,6+150+18
; Adjust coordinates
mov ecx,9+128
sub ecx,ebp
mov r10d,ecx
and ecx,07h                  ; Bits 0-2 used for X-positioning
imul ecx,ecx,21
add edx,ecx                  ; Add offset to X-start
add r9d,ecx                  ; Add offset to X-end
shr r10d,3
and r10d,0Fh                 ; Bits 3-6 used for Y-positioning
imul r10d,r10d,18
add r8d,r10d                 ; Add offset to Y-start
add eax,r10d                 ; Add offset to Y-end
; Go draw
jmp .TextConstDraw
.NotDump:
; Prepare for X-units, here change color
cmp ebp,9
jne @f
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_UNITS     ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
@@:
mov al,[SelectUnits]
lea r12,[U_B]                ; R12 = Pointer to text string, Cache&RAM mode
cmp al,0
je @f
lea r12,[U_KB]
cmp al,1
je @f
lea r12,[U_MB]
@@:
mov edx,SUBWINX-40           ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,SUBWINY-40           ; Parm#3 = R8 = Y coordinate up/left corner
mov r9d,SUBWINX-4            ; Parm#4 = R9 = X coordinate down/right corner
mov eax,SUBWINY-22           ; RAX = Transit for Parm#5 = Y coordinate down/right corner
cmp ebp,9                    ; Go if this pass for X-units
je .TextConstDraw
; Prepare for Y-units
mov edx,7                    ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,5                    ; Parm#3 = R8 = Y coordinate up/left corner
mov r9d,40+7+20              ; Parm#4 = R9 = X coordinate down/right corner
mov eax,18+5                 ; RAX = Transit for Parm#5 = Y coordinate down/right corner
lea r12,[U_MBPS]             ; R12 = Pointer to text string , Mode = Cache&RAM
mov r9d,40+7                 ; Parm#4 = R9 = X coordinate down/right corner
@@:
cmp ebp,8
je .TextConstDraw            ; Go if this pass for Y-units
; TSC Frequency, here change color
cmp ebp,7
jne .TextConstNoTSC
push rbx rdi
lea rdi,[TextTSC1]
mov word [rdi],0000h + '?'
mov rax,[TscClockHz]         ; RAX = TSC Frequency, Hz
mov ebx,1000000
xor edx,edx
div ebx                     ; EAX = TSC Frequency, MHz 
shld rdx,rax,32
test edx,edx
jnz @f                      ; Go skip if value > 2^32
mov bl,0
call DecimalPrint32
mov al,0
stosb
@@:
pop rdi rbx
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_INFO      ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
mov edx,SUBWINX-342          ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,6                    ; Parm#3 = R8 = Y coordinate up/left corner
mov r9d,SUBWINX-5            ; Parm#4 = R9 = X coordinate down/right corner
mov eax,6+18                 ; RAX = Transit for Parm#5 = Y coordinate down/right corner
lea r12,[TextTSC]            ; R12 = Pointer to text string
jmp .TextConstDraw           ; Go if this pass for TSC frequency
.TextConstNoTSC:
; Statistics, clocks minimum
mov edx,SUBWINX-342
mov r8d,6+18*1
mov r9d,SUBWINX-204 + 015 ; -143 ; +015 at v0.90
mov eax,6+18+18*1
lea r12,[TextMinimum]        ; This must be non-volatile register
lea r10,[TextMinimum1]       ; This can be volatile register
mov r11,[StatClksMinimum]
cmp ebp,6
je .TextConstClks            ; Go if this pass for statistics, clocks minimum
; Statistics, clocks maximum
mov r8d,6+18*2
mov eax,6+18+18*2
lea r12,[TextMaximum]
lea r10,[TextMaximum1]
mov r11,[StatClksMaximum]
cmp ebp,5
je .TextConstClks            ; Go if this pass for statistics, clocks maximum
; Statistics, clocks average
mov r8d,6+18*3
mov eax,6+18+18*3
lea r12,[TextAverage]
lea r10,[TextAverage1]
mov r11,[StatClksAverage]
cmp ebp,4
jne .TextConstNoClks         ; Go if this pass not for statistics, clocks maximum
; Clocks extraction
.TextConstClks:
push rax rbx rdx rdi
mov rdi,r10                  ; RDI = Destination string
mov ecx,12-1                 ; 12 (not 9) for blank if variable X-size font 
push rdi
mov al,'?'
stosb
mov al,' '
rep stosb                    ; Blank entire field, if next value shorter then previous
pop rdi
mov ebx,1000                 ; RBX = Divisor, used twice
; Divisions moved to handling results, here R11=CLOCKS*10
mov rax,r11
cmp rax,1000000
jae .InvalidClocks
xor edx,edx                  ; This clear entire RDX = Dividend high 64 bits
div rbx                      ; RAX = ( Clocks per Quant ) * 1000
mov bl,0
call DecimalPrint32          ; Print X , integer part of X.YYY
mov al,'.'
stosb                        ; Print "."
xchg eax,edx
mov bl,3                     ; Template = 3 chars
call DecimalPrint32          ; Print Y , float part of X.YZ
jmp .InvalidClocks           
.TextConstNoClks:
; Statistics, MBPS minimum
mov edx,SUBWINX-141
mov r8d,6+18*1
mov r9d,SUBWINX-025 ; -005
mov eax,6+18+18*1
lea r12,[TextMBPS]           ; This must be non-volatile register
@@:
mov r11,[StatClksMinimum]
cmp ebp,3
je .TextConstMbps            ; Go if this pass not for statistics, MBPS maximum = F (clocks minimum)
; Statistics, MBPS maximum
mov r8d,6+18*2
mov eax,6+18+18*2
mov r11,[StatClksMaximum]
cmp ebp,2
je .TextConstMbps            ; Go if this pass not for statistics, MBPS minimum = F (clocks maximum)
; Statistics, MBPS average
mov r8d,6+18*3               ; This for statistics, MBPS average = F (clocks average)
mov eax,6+18+18*3
mov r11,[StatClksAverage]
; MBPS (Megabytes per Second) extraction
.TextConstMbps:
push rax rbx rdx rdi
lea rdi,[TextMBPS1]          ; RDI = Destination string
@@:
mov ecx,12-1                 ; 12 (not 9) for blank if variable X-size font 
push rdi
mov al,'?'
stosb
mov al,' '
rep stosb                    ; Blank entire field, if next value shorter then previous
pop rdi
; Divisions moved to handling results, here R11=CLOCKS*10
test r11,r11
jz .InvalidClocks
cmp r11,1000000
jae .InvalidClocks
mov rax,[TscClockHz]
xor edx,edx
mov ebx,1000000/1000       ; Divisor/1000 , Frequency*1000 , because Clocks*1000 
div rbx
shld rdx,rax,32
test edx,edx
jnz .InvalidClocks         ; Frequency / ClocksPerInstruction
mul [BenchBPI]             ; Take Bytes per Instruction
shld rdx,rax,32
test edx,edx
jnz .InvalidClocks                   
; Multiply by number of threads 
mov edx,[InputParms.NumThreads]
mul rdx
; Divide to CPI
xor edx,edx                          ; Frequency / ClocksPerInstruction
div r11
; Store maximum MBPS for auto Y-scale
; Note detecting MAX branch not required, because
; this code automatically detect maximum
lea rbx,[MaximumMBPS]
cmp [rbx],eax
jae .AfterMax
mov [rbx],eax
.AfterMax:
; Print
mov bl,0
call DecimalPrint32          ; Note terminator byte 00h exist in the template
; Skip point
.InvalidClocks:
pop rdi rdx rbx rax          ; String terminated by template, don't write terminator = 0
; Draw after prepare
.TextConstDraw:
mov rcx,rdi                  ; Parm#1 = RCX = Pointer to RECT structure to store parameters
push rax                     ; This for stack align
push rax                     ; Parm#5 = Y coordinate down/right corner
sub rsp,32                   ; Create parameters shadow
call [SetRect]               ; Set region rectangle function 
add rsp,32+8+8               ; Remove parameters shadow, 1 parameter, align
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,r12                  ; Parm#2 = RDX = Pointer to text string
mov r8,-1                    ; Parm#3 = R8 = String length, -1 means NULL-terminated string
mov r9,rdi                   ; Parm#4 = R9 = Pointer to RECT structure, initialized by [SetRect] function
push rax                     ; This for stack align
push DT_LEFT                 ; Parm#5 = Method of formatting/alignment of text string
sub rsp,32                   ; Create parameters shadow
call [DrawText]              ; Set region rectangle function 
add rsp,32+8+8               ; Remove parameters shadow, 1 parameter, align
; Cycle for text strings
.SkipPass:
dec ebp
jnz .TextConst
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
mov rdx,[PaintStruc]         ; Parm#2 = RDX = Pointer to PAINTSTRUCT structure
call [EndPaint]
jmp .WinStatus
