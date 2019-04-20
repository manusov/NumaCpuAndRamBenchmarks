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
sub rsp,32+8            ; Parameters shadow for call API + alignment
mov rbx,rcx             ; RBX = Storage for hwnd
; Select handler
cmp rdx,WM_CREATE
je .WinCreate           ; Go if WM (Window Message) = OPEN WINDOW
cmp rdx,WM_DESTROY
je .WinDestroy          ; Go if WM = CLOSE WINDOW
cmp rdx,WM_COMMAND
je .WinCommand          ; Go if WM = USER INPUT
; Otherwise Default OS routine
; Here must be valid input RCX, RDX, R8, R9
.WinDefaultProc:
call [DefWindowProc]    ; DEFAULT WINDOW PROCEDURE
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
call SetDialogueDefaults           ; Set defaults
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

; ...

jmp .WinDefaultProc



