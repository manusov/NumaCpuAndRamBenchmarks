;--- Create GUI window. --------------------------------------------;
; Called from HeadBlock (for build Window 0 = main)                 ;
; and from Window 0 (for build Window 1 = drawings).                ;
; Terminology notes: WS = Window Style                              ;
;                                                                   ;
; INPUT:   EBX = Pointer to window description structure,           ;
;                see WINDESC macro at gui\connect_equ.inc           ;
;                                                                   ;
; OUTPUT:  ZF flag = Status: 0(NZ)=OK, 1(Z)=Error                   ;
;          Variables state                                          ;
;-------------------------------------------------------------------;

CreateApplicationWindow:
pushad
; Load registers
mov edi,[ebx + 04]                   ; EDI = Dialogue window descriptor pointer
; Start initializing window elements
cmp byte [ebx + 16],0
jnz .SkipInitWin                     ; Go if repeat call, already initialized
; Get current module handle
push 0                               ; Parm#1 = 0, means current module
call [GetModuleHandle]	             ; Get handle for this application
mov [edi + WNDCLASSEX.hInstance],eax
test eax,eax
jz @f                                ; Go if error returned
; Load icon
.ContinueWin:		
push IDI_APPLICATION                 ; Parm#2 = Pre-defined type
push 0                               ; Parm#1 = 0 , means standard icon
call [LoadIcon]                      ; Load icon for window
mov [edi + WNDCLASSEX.hIcon],eax
mov [edi + WNDCLASSEX.hIconSm],eax
test eax,eax
jz @f                                ; Go if error returned
; Load cursor
push IDC_ARROW                       ; Parm#2 = Pre-defined type
push 0                               ; Parm#1 = 0 , means current
call [LoadCursor]                    ; Load cursor for window
mov [edi + WNDCLASSEX.hCursor],eax
test eax,eax
jz @f                                ; Go if error returned
; Register class
push edi                             ; Parm#1 = Pointer to descriptor 
call [RegisterClassEx]               ; Register window class
test eax,eax
; Point for errors handling
@@:
jz .DoneCreateWindow                 ; Go if error returned
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
xor eax,eax                            ; This for compact push zeroes
push eax                               ; Parm#12 = 0
push [edi + WNDCLASSEX.hInstance]      ; Parm#11 = Handle
push eax	                             ; Parm#10 = 0
push eax                               ; Parm#9  = 0 or Parent
movzx ecx,word [ebx + 24]
push ecx                               ; Parm#8  = Ysize
mov cx,word [ebx + 22]
push ecx                               ; Parm#7  = Xsize 
mov cx,word [ebx + 20]
push ecx                               ; Parm#6  = Ybase
mov cx,word [ebx + 18]
push ecx                               ; Parm#5  = Xbase
push dword [ebx + 08]                  ; Parm#4  = Style
push dword [ebx + 00]                  ; Parm#3  = Name
push [edi + WNDCLASSEX.lpszClassName]  ; Parm#2  = Class
push eax                               ; Parm#1  = Style
call [CreateWindowEx]                  ; Call Win API function
test eax,eax                           ; EAX = Status, 0 if error
; Done
.DoneCreateWindow:
popad
ret

