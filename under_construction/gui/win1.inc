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

; TODO.
; Create and set timer, for benchmarks progress (but not for measure)
;mov rcx,rbx                            ; Parm#1 = RCX = Parent window handle
;xor edx,edx                            ; Parm#2 = RDX = Event ID
;mov r8d,TIMER_TICK                     ; Parm#3 = R8 = Timer ticks period, ms
;xor r9d,r9d                            ; Parm#4 = R9 = Notify function pointer, not used here
;call [SetTimer]

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
; ...
; ...
; ...
jmp .WinStatus


; Handler for timer event: tick
.WinTimer:
; ...
; ...
; ...
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

call DrawSessionProgress

; ...
jmp .EndPaint


.FirstPaint:
mov byte [rcx],2          ; [rcx] = [Win1_Running]
mov [DrawParms + DRPM.DrawCount],ebp
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
mov r12d,GRIDBLANKX          ; R12D = Base point, X
mov r13d,SUBWINY-GRIDBLANKY  ; R13D = Base point, Y
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
add r12d,GRIDSTEPX           ; Add X grid step
dec r14d                     ; Cycle for X-grid vertical lines
jnz .XVgrid
; Coordinate Y-grid, horizontal lines
mov r12d,SUBWINY-GRIDBLANKY  ; R12D = Base point, Y
mov r13d,GRIDY               ; R13D = Number of lines
.YHgrid:
lea eax,[r12d-1]
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
; Numbers for X-grid vertical lines and Y-grid horizontal lines
push rbx                     ; Save RBX=Handle, this also important for stack alignment: 5 parms. functions
mov bh,2                     ; BH = Counter, Pass#1 = XGrid, Pass#2 = YGrid
mov r14d,GRIDX               ; R14D = Number of lines, X
mov r12d,GRIDBLANKX          ; R12D = Base point, X
mov r13d,SUBWINY-20          ; R13D = Base point Y

xor ebp,ebp                  ; EBP = Units value

.BothGrids:
; xor ebp,ebp                  ; EBP = Units value
.XVYHtext:

;; Set region for text string write
;; Here don't align stack because PUSH RBX
;mov rcx,rdi                  ; Parm#1 = RCX = Pointer to RECT structure to store parameters
;mov edx,r12d                 ; Parm#2 = RDX = X coordinate up/left corner
;mov r8d,r13d                 ; Parm#3 = R8 = Y coordinate up/left corner  ,  TODO MAKE EQU INSTEAD NUMBER.
;lea r9d,[r12d + 65]          ; Parm#4 = R9 = X coordinate down/right corner  ,  TODO MAKE EQU INSTEAD NUMBER. ; old = 65
;lea eax,[r13d + 18]                                                         ;  TODO MAKE EQU INSTEAD NUMBER. ; old = +18 
;push rax                     ; Parm#5 = Y coordinate down/right corner
;sub rsp,32                   ; Create parameters shadow
;call [SetRect]               ; Set region rectangle function 
;add rsp,32+8                 ; Remove parameters shadow and one parameter
;; Build ASCII string
;push rdi
;lea rdi,[BigTexts]           ; RDI = Pointer for print decimal string as ASCII
;mov rdx,rdi
;mov bl,0                     ; BL = Template control for print number, 0=No template
;mov eax,ebp                  ; EAX = Value for print, Bytes/KB/MB
;call DecimalPrint32          ; Print number
;mov al,0
;stosb                        ; Store terminator byte = 0
;pop rdi


; Build ASCII string
push rdi
lea rdi,[BigTexts]           ; RDI = Pointer for print decimal string as ASCII
mov rdx,rdi
mov bl,0                     ; BL = Template control for print number, 0=No template
mov eax,ebp                  ; EAX = Value for print, Bytes/KB/MB
;mov r10,rdi
call DecimalPrint32          ; Print number
;mov r11,rdi
;sub r11,r10
;imul r11,r11,8
mov al,0
stosb                        ; Store terminator byte = 0
pop rdi

;cmp bh,1
;je @f
;xor r11d,r11d
;@@:

; Set region for text string write
; Here don't align stack because PUSH RBX
mov rcx,rdi                  ; Parm#1 = RCX = Pointer to RECT structure to store parameters
mov edx,r12d                 ; Parm#2 = RDX = X coordinate up/left corner
; lea edx,[r12d + r11d]

mov r8d,r13d                 ; Parm#3 = R8 = Y coordinate up/left corner  ,  TODO MAKE EQU INSTEAD NUMBER.
;lea r9d,[r12d + r11d + 65]  ; Parm#4 = R9 = X coordinate down/right corner  ,  TODO MAKE EQU INSTEAD NUMBER. ; old = +65
lea r9d,[r12d + 65]

lea eax,[r13d + 18]                                                          ;  TODO MAKE EQU INSTEAD NUMBER. ; old = +18 
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
mov r12d,GRIDBLANKX - 15 - 35                  ; R12D = X coordinate , TODO MAKE EQU INSTEAD NUMBER. OR ADAPTIVE RIGHT ALIGNED
mov r13d,SUBWINY-GRIDBLANKY - GRIDSTEPY - 15   ; R13D = Y coordinate
mov ebp,[DrawParms + DRPM.ValueGridY]
dec bh
jnz .BothGrids
pop rbx





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

