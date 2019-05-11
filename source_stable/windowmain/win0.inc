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

;========== DEBUG FRAGMENT ====================================================;
; Connect point for debug method 3 of 3 = Window debug.
; ( Use also 1 = Template debug, 2 = Application debug )

; BUG: if keyboard focus at daughter elements of window, key press not
; works, because keu event associated with daughter element, not main window? 
; Check by press "About" button before CTRL-ALT-Fx key, when
; CTRL-ALT-Fx key not works. Note "About" button returns focus to main window.

; include '_debug_\window_key_trap.inc'

;========== END OF DEBUG FRAGMENT =============================================;

; Select handler
cmp rdx,WM_CREATE
je .WinCreate         ; Go if WM (Window Message) = OPEN WINDOW
cmp rdx,WM_DESTROY
je .WinDestroy        ; Go if WM = CLOSE WINDOW
cmp rdx,WM_COMMAND
je .WinCommand        ; Go if WM = USER INPUT
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


; Handler for create window (open), build user interface objects 
.WinCreate:
lea rsi,[DialogueContent_Win0]     ; RSI = Pointer to elements descriptors
mov edi,START_HANDLE_WIN0          ; RDI = Start handle
mov r14,[Dialogue_Win0.hInstance]  ; R14 = Parent module handle = [wc.hInstance]
call CreateDialogueElements
call SetDefaults_Win0              ; Set defaults
jmp .WinFinish


; Handler for destroy window (close)
.WinDestroy:
xor ecx,ecx                        ; Parm#1 = RCX = Exit code
call [PostQuitMessage]
xor eax,eax
jmp .WinFinish


; Handler for user commands
; Buttons: Run Simple, Run Drawings, Defaults, Close buttons
; Also used for make active/inactive block size edit string.
; Also used for extract and store variables = F(options state)
.WinCommand:
; Check phase of execution
cmp byte [Win0_Init],1
jne .WinDefaultProc    ; Run default OS handler if Window 0 not initialized yet
; Handling if Window 0 already initialized
mov r14,r8              ; R14 = Store event code
; Activate custom block radio button if edit string active
call [GetFocus]
cmp rax,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]
jne  @f
mov rcx,rbx
mov edx,HANDLE_L1_CACHE
mov r8d,HANDLE_CUSTOM_BLOCK
mov r9d,r8d
call [CheckRadioButton]
@@:
; Detect state of "Custom Size" radio button
; Use fact: BST_UNCHECKED=0, BST_CHECKED=1
mov esi,HANDLE_CUSTOM_BLOCK  ; RSI = Pointer to Handle
call SendMessageGetCheck
mov edi,0                 ; Can.t XOR EDI,EDI because must save ZF flag
jne @f                    ; Skip set EDI=1 if not a BST_CHECKED
inc edi                   ; BST_CHECKED=1, if CHECKED set EDI=1, otherwise EDI=0
@@:
; Set state for block size edit string
mov rcx,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]  ; RCX = Parm#1 = Handle
mov edx,WM_ENABLE         ; RDX = Parm#2 = Message
mov r8d,edi               ; R8  = Parm#3 = Select: 0=Disable, EDI from previous step
xor r9d,r9d               ; R9  = Parm#4 = Not used
call [SendMessage]                  
; Set or clear READ ONLY state for edit string
mov rcx,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]  ; RCX = Parm#1 = Handle
mov edx,EM_SETREADONLY    ; RDX = Parm#2 = Message
xor edi,0001b             ; Invert bit D0
mov r8d,edi               ; R8 = Parm#3 = Select: 0=Clear READ ONLY, 1=Set READ ONLY
xor r9d,r9d               ; R9 = Parm#4 = Not used = 0
call [SendMessage]
; Check event source
cmp r14,START_HANDLE_WIN0
jb .WinDefaultProc     ; Skip if other handles, otherwise WM_SETTEXT cause error
cmp r14,HANDLE_BUTTON_ABOUT
ja .WinDefaultProc
; Extract CUSTOM BLOCK SIZE as text string
; Get text
lea rsi,[BigTexts]  ; RSI = Buffer base address
mov rcx,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]  ; RCX = Parm#1 = Handle
mov edx,WM_GETTEXT  ; RDX = Parm#2 = Command
mov r8d,8+1         ; R8  = Parm#3 = Max. number of chars, include 0=terminator
mov r9,rsi          ; R9  = Parm#4 = Pointer to buffer
call [SendMessage]
; Extract editable block size for "Run" button
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
; Build ASCII hex number
mov r9,rdi          ; R9 = Parm#4 = Source buffer
xchg eax,edx
call HexPrint32
mov al,0
stosb
; Send back to edit field, for correct wrong input
mov rcx,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]  ; RCX=Parm#1=Dest. element handle
mov edx,WM_SETTEXT        ; RDX = Parm#2 = Command, "WM" means Window Message
xor r8d,r8d               ; R8  = Parm#3 = Don't used
call [SendMessage]
; Detect "About", "Run Simple", "Run Drawings", "Defaults" and "Cancel" buttons
cmp r14,HANDLE_BUTTON_RUN_SIMPLE
je .RunSimple                             ; Go if "Run Simple" button pressed
cmp r14,HANDLE_BUTTON_RUN_DRAWINGS
je .RunDrawings                           ; Go if "Run Drawings" button pressed
cmp r14,HANDLE_BUTTON_BRIEF
je .VectorBrief                           ; Go if "Vector Brief" button pressed
cmp r14,HANDLE_BUTTON_DEFAULTS
je .Defaults                              ; Go if "Defaults" button pressed
cmp r14,HANDLE_BUTTON_CANCEL
je .Cancel                                ; Go if "Cancel" button pressed
cmp r14,HANDLE_BUTTON_ABOUT
je .About                                 ; Go if "About" button pressed
jmp .WinDefaultProc                       ; Otherwise default OS handler


; Button handler: About
.About:
; Disable parent window when daughter window work
mov rcx,rbx                   ; RCX = Parm#1 = Handle
xor edx,edx                   ; RDX = Parm#2 = 0, means DISABLE
call [EnableWindow]
; Message box and wait user input
xor ecx,ecx	                  ; RCX = Parm#1 = Parent window handle or 0
lea rdx,[ABOUT_ID]            ; RDX = Parm#2 = Pointer to "About" string
lea r8,[ABOUT_CAP]            ; R8  = Parm#3 = Pointer to caption
mov r9d,0040h                 ; R9  = Parm#4 = Message box icon type = Info
call [MessageBoxA]
; Enable and activate parent window
.ReturnParentWindow:
mov rcx,rbx                   ; RCX = Parm#1 = Handle
mov edx,1                     ; RDX = Parm#2 = 1, means ENABLE
call [EnableWindow]
mov rcx,rbx                   ; RCX = Parm#1 = Handle
mov edx,WM_ACTIVATE           ; RDX = Parm#2 = Message
mov r8d,WA_CLICKACTIVE        ; R8  = Parm#3 = Activation method
xor r9d,r9d                   ; R9  = Parm#4 = 0 , not used
call [SendMessage]
; Exit after drawings works
jmp .WinFinish

; Buttons handlers
; Button handler: Run Simple
.RunSimple:
; Disable parent window when daughter window work
mov rcx,rbx            ; RCX = Parm#1 = Handle
xor edx,edx            ; RDX = Parm#2 = 0, means DISABLE
call [EnableWindow]
push rbx               ; save handle for restore parent window activity
call ExecuteSimple
pop rbx
jmp .ReturnParentWindow

; Button handler: Run Drawings
.RunDrawings:
; Disable parent window when daughter window work
mov rcx,rbx            ; RCX = Parm#1 = Handle
xor edx,edx            ; RDX = Parm#2 = 0, means DISABLE
call [EnableWindow]
push rbx               ; save handle for restore parent window activity
call ExecuteDraw
pop rbx
jmp .ReturnParentWindow

; Button handler: Vector Brief
.VectorBrief:
; Disable parent window when daughter window work
mov rcx,rbx            ; RCX = Parm#1 = Handle
xor edx,edx            ; RDX = Parm#2 = 0, means DISABLE
call [EnableWindow]
push rbx               ; save handle for restore parent window activity
call ExecuteBrief
pop rbx
jmp .ReturnParentWindow

; Button handler: Defaults
.Defaults:
call SetDefaults_Win0
jmp .WinFinish

; Button handler: Cancel
.Cancel:
jmp .WinDestroy




