;------------------------------------------------------------------------------;
; Trap module for connect debug windows, accepts from caller:                  ;
; Parm#1 = RCX = hwnd = Handle to the window                                   ;
; Parm#2 = RDX = uMsg = System defined message                                 ;
; Parm#3 = R8  = wParam = Additional message information (1), uMsg-dependent   ;
; Parm#4 = R9  = lParam = Additional message information (2), uMsg-dependent   ;
; RBX = Storage for hwnd                                                       ;
;------------------------------------------------------------------------------;

cmp rdx,WM_KEYDOWN
jne .SkipKeyTrap      ; Go skip if event not WM_KEYDOWN
cmp r8,VK_F1
jb .SkipKeyTrap       ; Go skip if key code < F1
cmp r8,VK_F12
ja .SkipKeyTrap       ; Go skip if key code > F12  

; Save parameters for integrating debug fragment into NCRB context
push rax rbx rcx rdx r8 r9 r10 r11 r12 r13 r14 r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h  ; Stack alignment for WinAPI calls
sub rsp,32                  ; Parameters shadow for WinAPI calls

; Check CTRL key pressed
lea r12,[r8-VK_F1]
mov ecx,VK_CONTROL
call [GetKeyState]
test ax,ax
jz .SkipKeyDetect 

; Check ALT key pressed
mov ecx,VK_MENU
call [GetKeyState]
test ax,ax
jz .SkipKeyDetect 

; Disable parent window when daughter window work
mov rcx,rbx                   ; RCX = Parm#1 = Handle
xor edx,edx                   ; RDX = Parm#2 = 0, means DISABLE
call [EnableWindow]

; Select and call handler
push rbx rbp
shl r12,3
call qword [.window_debug_list+r12]
pop rbp rbx
jmp .AfterKeyDetected

; Offsets for debug handlers call
.window_debug_list:
DQ  .window_debug_01
DQ  .window_debug_02
DQ  .window_debug_03
DQ  .window_debug_04
DQ  .window_debug_05
DQ  .window_debug_06
DQ  .window_debug_07
DQ  .window_debug_08
DQ  .window_debug_09
DQ  .window_debug_10
DQ  .window_debug_11
DQ  .window_debug_12

; Debug handlers modules connect
include '_debug_\window_debug_01.inc'
include '_debug_\window_debug_02.inc'
include '_debug_\window_debug_03.inc'
include '_debug_\window_debug_04.inc'
include '_debug_\window_debug_05.inc'
include '_debug_\window_debug_06.inc'
include '_debug_\window_debug_07.inc'
include '_debug_\window_debug_08.inc'
include '_debug_\window_debug_09.inc'
include '_debug_\window_debug_10.inc'
include '_debug_\window_debug_11.inc'
include '_debug_\window_debug_12.inc'

; Enable and activate parent window
.AfterKeyDetected:
mov rcx,rbx                   ; RCX = Parm#1 = Handle
mov edx,1                     ; RDX = Parm#2 = 1, means ENABLE
call [EnableWindow]
mov rcx,rbx                   ; RCX = Parm#1 = Handle
mov edx,WM_ACTIVATE           ; RDX = Parm#2 = Message
mov r8d,WA_CLICKACTIVE        ; R8  = Parm#3 = Activation method
xor r9d,r9d                   ; R9  = Parm#4 = 0 , not used
call [SendMessage]

; Return points
.SkipKeyDetect:
mov rsp,rbp
pop r15 r14 r13 r12 r11 r10 r9 r8 rdx rcx rbx rax
.SkipKeyTrap:







