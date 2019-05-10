;--- Create window. ------------------------------------------------;
; Called from HeadBlock and from Window 0.                          ;
; Terminology notes: WS = Window Style                              ;
;                                                                   ;
; INPUT:   RSI = Window name string pointer                         ;
;          RDI = Dialogue descriptor pointer                        ;
;          RBX = Parent handle                                      ;
;          AL  = Repeat flag, 0=Initializing, first call, 1=Second  ;
;          AH  = Select parent handle, 0=Application, 1=RBX         ;
;          R10 = Window Style                                       ;
;          R11 = Window X-base                                      ;
;          R12 = Window Y-base                                      ;
;          R13 = Window X-size                                      ;
;          R14 = Window Y-size                                      ;
;                                                                   ;
; OUTPUT:  ZF flag = Status: 0(NZ)=OK, 1(Z)=Error                   ;
;          Variables state                                          ;
;-------------------------------------------------------------------;

CreateApplicationWindow:
cld                                             ; Clear direction, because STOSQ used
push rbx rcx rdx rax rsi rbp r8 r9 r10 r11 rax  ; R8-R11 because API, volatile
mov rbp,rsp                                     ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h                      ; Align stack
sub rsp,32                                      ; Make parameters shadow
; Start initializing window elements
test al,al
jnz .SkipInitWin         ; Go if repeat call, already initialized
xor ecx,ecx              ; Parm#1 = 0, means current module
call [GetModuleHandle]	 ; Get handle for this application
mov [rdi+WNDCLASSEX.hInstance],rax
test rax,rax
jz @f                    ; Go if error returned
; Load icon
.ContinueWin:		
xor ecx,ecx              ; Parm#1 = 0 , means standard icon
mov edx,IDI_APPLICATION  ; Parm#2 = Pre-defined type
call [LoadIcon]          ; Load icon for window
mov [rdi+WNDCLASSEX.hIcon],rax
mov [rdi+WNDCLASSEX.hIconSm],rax
test rax,rax
jz @f                   ; Go if error returned
; Load cursor
xor ecx,ecx             ; Parm#1 = 0 , means current
mov edx,IDC_ARROW       ; Parm#2 = Pre-defined type
call [LoadCursor]       ; Load cursor for window
mov [rdi+WNDCLASSEX.hCursor],rax
test rax,rax
jz @f                   ; Go if error returned
; Register class
mov rcx,rdi             ; Parm#1 = Pointer to descriptor 
call [RegisterClassEx]  ; Register window class
test rax,rax
@@:
jz ErrorCreateWindow     ; Go if error returned
; Start Create Window, create and show window
; Parm#1  = Extended window style = 0 , means none
; Parm#2  = Pointer to Window class name = ClassName
; Parm#3  = Pointer to Window name = WindowTitle
; Parm#4  = Window style, Example = WS_VISIBLE+WS_DLGFRAME+WS_SYSMENU
; Parm#5  = X base 
; Parm#6  = Y base
; Parm#7  = X size
; Parm#8  = Y size
; Parm#9  = Parent handle = 0 , means default
; Parm#10 = Handle to menu, if used for this window type = 0, means none
; Parm#11 = Parent module handle = [wc.hInstance]
; Parm#12 = Pointer to passed parameters = 0, means none
.SkipInitWin:
xor eax,eax                               ; This for compact push zeroes
push rax                                  ; Parm#12 = 0
push [rdi+WNDCLASSEX.hInstance]           ; Parm#11 = Handle
push rax	                                ; Parm#10 = 0
push rax  ;  rbx                          ; Parm#9  = 0 or Parent
push r14                                  ; Parm#8  = Ysize
push r13                                  ; Parm#7  = Xsize 
push r12                                  ; Parm#6  = Ybase
push qword [rbp+08]                       ; Parm#5  = Xbase, R11 is volatile
mov r9,[rbp+16]                           ; Parm#4  = Style, R10 is volatile
mov r8,rsi                                ; Parm#3  = Name
mov rdx,[rdi+WNDCLASSEX.lpszClassName]    ; Parm#2  = Class
xor ecx,ecx                               ; Parm#1  = Style
sub rsp,32	                              ; Make parameters shadow
call [CreateWindowEx]	                    ; Call function
add rsp,32+8*8	                          ; Remove shadow and parameters
test rax,rax                              ; RAX = Status, 0 if error
; Done
ErrorCreateWindow:    ; This branch must not corrupt flags, ZF used
; TODO. Same return #1
mov rsp,rbp
pop rax r11 r10 r9 r8 rbp rsi rax rdx rcx rbx    ; R8-R11 because API, volatile
ret

