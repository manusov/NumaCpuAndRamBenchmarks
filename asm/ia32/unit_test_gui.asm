;------------------------------------------------------------------------------;
;                       Template for unit tests.                               ;
;               Unit test for GUI, show some GUI widgets,                      ; 
;           without platform features detection (can be inconsistent).         ; 
;------------------------------------------------------------------------------;

include 'win32a.inc'
include 'global\connect_equ.inc'
include 'gui\connect_equ.inc'

; This for build unit test
VECTOR_BRIEF_TEMP_TRANSIT  EQU  0

format PE GUI 4.0
entry start
section '.text' code readable executable
start:

; Build structure for unit test
lea edi,[TEMP_BUFFER]
mov dword [edi + 000],'#1'
mov dword [edi + 080],'#2'
mov dword [edi + 160],'#3'
mov dword [edi + 320],'#4'
mov dword [edi + 480],'#5'
mov dword [edi + 640],'#6'
mov dword [edi + 800],'#7'
mov dword [edi + 960],'#8'

; Set pattern for dump
lea esi,[InputParms]
mov [esi + IPB.DumpMethodStart],CpuVendorString
mov [esi + IPB.DumpMethodLength],31

; Set patterns for statistics
; xor eax,eax
; mov dword [DrawThreadParms + DTHP.StatCpiMin + 0],eax
; mov dword [DrawThreadParms + DTHP.StatCpiMin + 4],eax

;- Create (Open) parent window = Window 0
;- Terminology notes: WS = Window Style
;- lea rsi,[PRODUCT_ID]       ; RSI = Window name string pointer
;- lea rdi,[Dialogue_Win0]    ; RDI = Dialogue descriptor pointer
;- mov r10d,WS_VISIBLE+WS_DLGFRAME+WS_SYSMENU ; R10 = Window Style
;- mov r11d,WIN0_XBASE        ; R11 = Window X-base
;- mov r12d,WIN0_YBASE        ; R12 = Window Y-base
;- mov r13d,WIN0_XSIZE        ; R13 = Window X-size
;- mov r14d,WIN0_YSIZE        ; R14 = Window Y-size
;- xor ebx,ebx                ; RBX = 0, means no parent handle
;- xor eax,eax                ; AH  = 0 , means parent = Application

; Create (Open) parent window = Window 0
; Terminology notes: WS = Window Style
lea ebx,[Create_Win0] 
call CreateApplicationWindow    ; AL  = 0 , means first call
lea eax,[MessageBadWindow]      ; EAX = Pointer to error string, used if abort
jz ErrorProgram                 ; Go if error returned

; Get messages with wait user events for window from OS
; GetMessage (internal) subroutine parameters
; Parm#1 = Pointer to structure receive data = DialMsg
; Parm#2 = Window handle whose message retrieved, 0=any
; Parm#3 = Filter Min , both Min=Max=0 means no filtering
; Parm#4 = Filter Max , both Min=Max=0 means no filtering
; Output RAX = Message status, used values (but this is not a message code):
; 0, means returned WM_QUIT, returned when window subroutine
;    call PostQuitMessage subroutine, means quit
; 1, means process event
mov byte [Win0_Init],1         ; Enable commands handling by callback routine
lea edi,[DialogueMsg_Win0]     ; EDI = MSG structure
call WaitEvent                 ; Window 1 work as callbacks inside this

; Exit point
ExitProgram:

; Unit test purpose dummy point
ErrorProgram:

; Done
push 0
call [ExitProcess]

; This for build unit test
RunVectorBrief:
ResultVectorBrief:
SessionStart:
SessionProgress:
SessionStop:
ResultSimple:
ThreadsReStart:
ThreadsUpdatePattern:
ThreadsUpdateBlockSize:
ThreadsUpdateAdaptive:
ret

include 'global\connect_code.inc'
include 'gui\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'gui\connect_const.inc'

WinCaption   DB ' Unit test for GUI' , 0
MessageText  DB 'Template text...' , 0

; this for unit test
; TODO. Make this as part of temporary buffer, for regular access method
CpuVendorString  DB  'UNIT TEST',0
CpuNameString    DB  'UNIT TEST',0
CacheL1Data      DB  12*3 DUP (1)  

; Test purpose
AdaptiveRepeats:
DQ  10 ; ADAPTIVE_FAST
DQ  20 ; ADAPTIVE_SLOW
ThreadsList DQ 0

; variables area start here
include 'global\connect_var.inc'
include 'gui\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphics 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

