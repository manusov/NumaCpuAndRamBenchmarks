;--- Callback handler for main-parent window ----------------------------------;
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

WindowProc_Win0:
pushad
mov ebp,esp                   ; EBP = Pointer to stack frame
mov ebx,[ebp + 36 + 00]       ; EBX = hwnd
mov eax,[ebp + 36 + 04]       ; EAX = uMsg
mov ebp,[ebp + 36 + 08]       ; EBP = wParam 
; Select handler
cmp eax,WM_CREATE
je .WinCreate           ; Go if WM (Window Message) = OPEN WINDOW
cmp eax,WM_DESTROY
je .WinDestroy          ; Go if WM = CLOSE WINDOW
cmp eax,WM_COMMAND
je .WinCommand          ; Go if WM = USER INPUT
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

;------ Handler for create window (open), build user interface objects --------;
 
.WinCreate:
lea esi,[DialogueContent_Win0]     ; ESI = Pointer to elements descriptors
mov edi,START_HANDLE_WIN0          ; EDI = Start handle
mov ebp,[Dialogue_Win0.hInstance]  ; EBP = Parent module handle = [wc.hInstance]
call CreateDialogueElements
call SetDialogueDefaults           ; Set defaults
jmp .WinFinish


;------------------- Handler for destroy window (close) -----------------------;

.WinDestroy:
push 0                              ; Parm#1 = Exit code
call [PostQuitMessage]
xor eax,eax
jmp .WinFinish


;----------------------- Handler for user commands ----------------------------;

; Buttons: Run Simple, Run Drawings, Defaults, Close buttons
; Also used for make active/inactive block size edit string.
; Also used for extract and store variables = F(options state)
.WinCommand:

; Check phase of execution, now EBP = event code
cmp byte [Win0_Init],1
jne .WinDefaultProc    ; Run default OS handler if Window 0 not initialized yet
; Handling this event if Window 0 already initialized
; Activate custom block radio button if edit string active
call [GetFocus]
cmp eax,[SubHandles - 64 + HANDLE_CUSTOM_EDIT*4]
jne  @f
push HANDLE_CUSTOM_BLOCK       ; Parm#4 = radio button ID
push HANDLE_CUSTOM_BLOCK       ; Parm#3 = last button in the group
push HANDLE_L1_CACHE           ; Parm#2 = first button in the group
push ebx                       ; Parm#1 = dialog box handle
call [CheckRadioButton]
@@:
; Detect state of "Custom Size" radio button
; Use fact: BST_UNCHECKED=0, BST_CHECKED=1
mov esi,HANDLE_CUSTOM_BLOCK  ; ESI = Pointer to Handle
call SendMessageGetCheck
mov edi,0                 ; Can.t XOR EDI,EDI because must save ZF flag
jne @f                    ; Skip set EDI=1 if not a BST_CHECKED
inc edi                   ; BST_CHECKED=1, if CHECKED set EDI=1, otherwise EDI=0
@@:
; Set state for block size edit string
push 0                    ; Parm#4 = Not used = 0
push edi                  ; Parm#3 = Select: 0=Disable, EDI from previous step
push WM_ENABLE            ; Parm#2 = Message
push dword [SubHandles - 64 + HANDLE_CUSTOM_EDIT*4]  ; Parm#1 = Handle
call [SendMessage]                  
; Set or clear READ ONLY state for edit string
push 0                    ; Parm#4 = Not used = 0
xor edi,0001b             ; Invert bit D0
push edi                  ; Parm#3 = Select: 0=Clear READ ONLY, 1=Set READ ONLY
push EM_SETREADONLY       ; Parm#2 = Message
push dword [SubHandles - 64 + HANDLE_CUSTOM_EDIT*4]  ; Parm#1 = Handle
call [SendMessage]
; Check event source
cmp ebp,START_HANDLE_WIN0
jb .WinDefaultProc     ; Skip if other handles, otherwise WM_SETTEXT cause error
cmp ebp,HANDLE_BUTTON_CANCEL
ja .WinDefaultProc
; Extract CUSTOM BLOCK SIZE as text string
; Get text
lea esi,[BigTexts]  ; ESI = Buffer base address
push esi            ; Parm#4 = Pointer to buffer
push 8+1            ; Parm#3 = Max. number of chars, include 0=terminator
push WM_GETTEXT     ; Parm#2 = Command
push dword [SubHandles - 64 + HANDLE_CUSTOM_EDIT*4]  ; Parm#1 = Handle
call [SendMessage]
; Extract editable block size for "Run" button
; Here ESI valid string base, Calculate CUSTOM BLOCK SIZE as number
push esi
mov ecx,8           ; ECX = String Length
xor edx,edx         ; EDX = Editable block size value
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
mov [BlockSizeEdit],edx   ; Store value = custom block size
pop edi
; Build ASCII hex number
push edi            ; Parm#4 = Source buffer
xchg eax,edx
call HexPrint32
mov al,0
stosb
; Send back to edit field, for correct wrong input
push 0              ; Parm#3 = Don't used
push WM_SETTEXT     ; Parm#2 = Command, "WM" means Window Message
push dword [SubHandles - 64 + HANDLE_CUSTOM_EDIT*4]  ; Parm#1 = Dest. element handle
call [SendMessage]

;---------------------- Buttons handlers selector -----------------------------;

; Detect buttons press
cmp ebp,HANDLE_BUTTON_ABOUT
je .ButtonHandlerAbout                ; Go if "About" button pressed
cmp ebp,HANDLE_BUTTON_BRIEF
je .ButtonHandlerVectorBrief          ; Go if "Vector Brief" button pressed
cmp ebp,HANDLE_BUTTON_RUN_SIMPLE
je .ButtonHandlerRunSimple            ; Go if "Run Simple" button pressed
cmp ebp,HANDLE_BUTTON_RUN_DRAWINGS
je .ButtonHandlerRunDrawings          ; Go if "Run Drawings" button pressed
cmp ebp,HANDLE_BUTTON_DEFAULTS
je .ButtonHandlerDefaults             ; Go if "Defaults" button pressed
cmp ebp,HANDLE_BUTTON_CANCEL
je .ButtonHandlerCancel               ; Go if "Cancel" button pressed
jmp .WinDefaultProc                   ; Otherwise default OS handler

;------------------------ Button handler: About -------------------------------;

.ButtonHandlerAbout:
; Disable parent window when daughter window work
call HelperButtonsDisable
; Message box and wait user input
push MB_ICONINFORMATION    ; Parm#4 = Message box icon type = Info
push ABOUT_CAP             ; Parm#3 = Pointer to caption
push ABOUT_ID              ; Parm#2 = Pointer to "About" string
push 0                     ; Parm#1 = Parent window handle or 0
call [MessageBoxA]
; Enable and activate parent window
.ReturnParentWindow:
call HelperButtonsEnable
; Exit after drawings works
jmp .WinFinish


;-------------------- Button handler: Vector Brief ----------------------------;

.ButtonHandlerVectorBrief:
; Disable parent window when daughter window work
call HelperButtonsDisable
push ebx               ; save handle for restore parent window activity
call RunVectorBrief
jc .VectorBriefError
call ResultVectorBrief
push 0                 ; Parm#4 = Message box icon type
push PRODUCT_ID        ; Parm#3 = Pointer to caption
push TEMP_BUFFER + VECTOR_BRIEF_TEMP_TRANSIT  ; Parm#2 = Pointer st.
push 0                 ; Parm#1 = Parent window handle or 0
call [MessageBoxA]
jmp .VectorBriefDone
; Error handling with messages
.VectorBriefError:
call RuntimeError
; Done 
.VectorBriefDone:
pop ebx
jmp .ReturnParentWindow


;----------------------- Button handler: Run Simple ---------------------------;

.ButtonHandlerRunSimple:
; Disable parent window when daughter window work
call HelperButtonsDisable
push ebx               ; save handle for restore parent window activity
; Extract settings from GUI widgets
mov al,0  ; Run context = simple, without memory allocation multiplication
call GuiToUserParms
; Required modification.
; Resources (memory) must be relesed even if error detected
call SessionStart
jc .RunSimpleError     ; Go if error at start
call SessionProgress
jc .RunSimpleError     ; Go if error at execution
call SessionStop
jc .RunSimpleError     ; Go if error at termination
; call routines from "Run simple" scenario
call ResultSimple 
; input parameters for build message box
push 0                 ; Parm#4 = Message box icon type
push PRODUCT_ID        ; Parm#3 = Pointer to caption
push TEMP_BUFFER       ; Parm#2 = Dialogue window Parm#2 = Pointer to string
push 0                 ; Parm#1 = Parent window handle or 0
call [MessageBoxA]
jmp .RunSimpleDone
; Error handling with messages
.RunSimpleError: 
call RuntimeError
; Done
.RunSimpleDone:
pop ebx
jmp .ReturnParentWindow


;----------------------- Button handler: Run Drawings -------------------------;

.ButtonHandlerRunDrawings:
; Disable parent window when daughter window work
call HelperButtonsDisable
push ebx               ; save handle for restore parent window activity
; At this point, parallel thread with DrawSessionThread subroutine STARTED.
call DrawSessionStart 
jc .RunDrawingsError
; Create (Open) daughter window = Window 1
; Build child window for drawings Speed=F(Size)
; Terminology notes: WS = Window Style
xchg eax,ebx
lea ebx,[Create_Win1]
mov [ebx + 12],eax
mov al,01h
xchg al,[Win1_Init]           ; AL = 1 , if window 1 already initialized
mov [ebx + 16],al
call CreateApplicationWindow  ; For this run option (0101h) EBX = Parent Handle
mov [Handle_Win1],eax         ; Store EAX = Window handle, drawings window
; Wait events for Window 1
lea edi,[DialogueMsg_Win1]	  ; RDI = MSG structure
call WaitEvent                ; Window 1 work as callbacks inside this
; At this point, 
; parallel thread with DrawSessionThread subroutine HALTED and REMOVED.
call DrawSessionStop
jnc .RunDrawingsDone 
; Error handling with messages
.RunDrawingsError: 
call RuntimeError
; Done
.RunDrawingsDone:
pop ebx
jmp .ReturnParentWindow


;-------------------------- Button handler: Defaults --------------------------;

.ButtonHandlerDefaults:
call SetDialogueDefaults
jmp .WinFinish


;--------------------------- Button handler: Cancel ---------------------------;

.ButtonHandlerCancel:
jmp .WinDestroy

;------------------- Helpers for buttons disable and enable -------------------;

; Note about stack re-alignment when subroutines call

;--- Disable all buttons, used before open child window box ---;
; INPUT:   None                                                ;
; OUTPUT:  None                                                ;
; Registers volatile/non volatile by ia32 calling convention   ;
;--------------------------------------------------------------;
HelperButtonsDisable:
mov al,0
jmp HelperButtonsState 

;--- Enable all buttons, used after close child window box ----;
; INPUT:   None                                                ;
; OUTPUT:  None                                                ;
; Registers volatile/non volatile by ia32 calling convention   ;
;--------------------------------------------------------------;
HelperButtonsEnable:
mov al,1
; No ret, continue at next subroutine

;--- Set state: Disable or Enable for all buttons -------------;
; INPUT:   AL = State, 0 = Disabled, 1 = Enabled               ;
; OUTPUT:  None                                                ;
; Registers volatile/non volatile by ia32 calling convention   ;
;--------------------------------------------------------------;
HelperButtonsState:
push ebx esi edi
movzx ebx,al
mov esi,HANDLE_ALL_BUTTONS_BASE
mov edi,HANDLE_ALL_BUTTONS_COUNT
@@:
push ebx                              ; Parm#2 = Enable flag, 0/1
push dword [SubHandles - 64 + esi*4]  ; Parm#1 = Button handle
call [EnableWindow]
inc esi
dec edi
jnz @b                                ; Cycle for all buttons
pop edi esi ebx
ret

