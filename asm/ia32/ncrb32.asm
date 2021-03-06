;==============================================================================;
;                                                                              ;
;               NCRB (NUMA CPU&RAM Benchmarks). Win32 Edition.                 ;
;                           (C)2021 IC Book Labs.                              ;
;                                                                              ;
;  This file is main module: translation object, interconnecting all modules.  ;
;                                                                              ;
;        Translation by Flat Assembler version 1.73.27 ( Jan 27, 2021 )        ;
;                         http://flatassembler.net/                            ;
;                                                                              ;
;       Edit by FASM Editor 2.0, use this editor for correct tabulations.      ;
;               http://asmworld.ru/instrumenty/fasm-editor-2-0/                ;
;                                                                              ;
;==============================================================================;


;------------------------------ DEFINITIONS -----------------------------------;
; FASM definitions for Win32
include 'win32a.inc'
; NCRB definitions
include 'global\connect_equ.inc'             ; Global library and data
include 'gui\connect_equ.inc'                ; GUI library and data
include 'scenario_brief\connect_equ.inc'     ; Scenario "Vector brief"
include 'scenario_main\connect_equ.inc'      ; System information for Main win. 
include 'scenario_simple\connect_equ.inc'    ; Scenario "Run simple"
include 'system_info\connect_equ.inc'        ; System information library
include 'targets_bandwidth\connect_equ.inc'  ; Memory bandwidth patterns
include 'targets_latency\connect_equ.inc'    ; Memory latency patterns
include 'targets_math\connect_equ.inc'       ; Math calculations patterns
include 'threads_manager\connect_equ.inc'    ; Threads and memory management


;------------------------------- CODE secion ----------------------------------;
format PE GUI 4.0
entry start
section '.text' code readable executable
start:

; Pre-load library ADVAPI32.DLL, next step uses GetModuleHandle, GetProcAddress
; for detect functions entry points. Returned handle and status stored but 
; ignored at this step, because availability of library and functions
; detected at next step: at subroutine SystemFunctionsLoad
; Note pre-load KERNEL32.DLL not required because static import used.
; Load this library (ADVAPI32.DLL), because it not loaded by static import
push NameAdvapi32
call [LoadLibrary]
mov [HandleAdvapi32],eax       ; Store EAX = Library handle

; Load optional system functions, dynamical import used
; For this functions cannot use static import,
; because required Win XP (x64) compatibility
call SystemFunctionsLoad       ; This subroutine output CF=1 if error
call ShowWarningAPI            ; This subroutine input CF as warning flag

; Show warning if NCRB ia32 version runs under Win64, note this is possible
; but non-optimal, can use NCRB x64 version for this platform.
call CheckWoW64                ; This subroutine output CF=1 if WoW64 detected
call ShowWarningWoW64          ; This subroutine input CF as warning flag

; Get system information
call GetSystemParameters       ; Get system information into fixed buffer
jc .Error                      ; Go if platform detection error 
call SysParmsToBuffer          ; Extract sys. info text strings
call SysParmsToGui             ; Update GUI widgets by sys. info results

; Create (Open) parent window = Window 0, terminology notes: WS = Window Style
lea ebx,[Create_Win0] 
call CreateApplicationWindow    ; AL  = 0 , means first call
mov [Handle_Win0],eax           ; Store EAX = Window handle, main window
lea eax,[MessageBadWindow]      ; EAX = Pointer to error string, used if abort
jz .Exit                        ; Go if error returned

; Get messages with wait user events for window from OS
; GetMessage subroutine parameters
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

; Exit application with unload dynamical import
.Exit:

; Unload dynamical import
mov ecx,[HandleAdvapi32]         ; ECX = Library ADVAPI32.DLL handle
jecxz @f                         ; Go skip unload if handle = null
push ecx                         ; Parm#1 = Library ADVAPI32.DLL handle
call [FreeLibrary]               ; Unload ADVAPI32.DLL
@@:

; Exit application
push 0                           ; Parm#1 = Exit code for this application
call [ExitProcess]               ; Exit application, NO RETURN from this call

; Exit point for error exit, terminology note: MB = Message Box
; Error handling: display OS message box, return button ID (not used)
; Parm#1 = Parent window handle
; Parm#2 = Pointer to message string must be valid at this point,
;	         value get from transit storage: EBP
; Parm#3 = Caption=0 means error message, otherwise pointer to caption
; Parm#4 = Message Box Icon Error = MB_ICNERROR
; Output = EAX = Pressed button ID, not used at this call
; Note INVOKE replaced to instructions for code size optimization!
; invoke MessageBoxA,0,r14,0,MB_ICONERROR
.Error:
push MB_ICONERROR   ; Parm#4
push 0              ; Parm#3
push ebp            ; Parm#2
push 0              ; Parm#1
call [MessageBoxA]
jmp .Exit             ; Go exit with dynamical imported library unload

; NCRB code: set of subroutines
include 'global\connect_code.inc'              ; Global library
include 'gui\connect_code.inc'                 ; GUI subroutines
include 'scenario_brief\connect_code.inc'      ; "Vector brief" subroutines
include 'scenario_main\connect_code.inc'       ; Main window sys info subrout.
include 'scenario_simple\connect_code.inc'     ; "Run simple" subroutines
include 'system_info\connect_code.inc'         ; System information library
include 'targets_bandwidth\connect_code.inc'   ; Bandwidth measurement patterns
include 'targets_latency\connect_code.inc'     ; Latency measurement patterns
include 'targets_math\connect_code.inc'        ; Math performance patterns
include 'threads_manager\connect_code.inc'     ; Threads and memory ctrl. subr.


;----------------------------- DATA section -----------------------------------;
section '.data' data readable writeable

; NCRB data constants, located before variables for EXE file space minimization
; include constants and variables with pre-defined state
include 'global\connect_const.inc'             ; Constants and pre-def. vars.
include 'gui\connect_const.inc'                ; GUI constants and pre-def. v.
include 'scenario_brief\connect_const.inc'     ; "Vector brief" constants
include 'scenario_main\connect_const.inc'      ; Main window constants
include 'scenario_simple\connect_const.inc'    ; "Run simple" constants
include 'system_info\connect_const.inc'        ; System information constants
include 'targets_bandwidth\connect_const.inc'  ; Bandwidth rout. list and data
include 'targets_latency\connect_const.inc'    ; Latency rout. list and data
include 'targets_math\connect_const.inc'       ; Math rout. list and data
include 'threads_manager\connect_const.inc'    ; Threads management constants

; NCRB data variables, located at top of file for EXE file space minimization
; include variables without pre-defined state 
include 'global\connect_var.inc'               ; Global visible variables
include 'gui\connect_var.inc'                  ; GUI variables
include 'scenario_brief\connect_var.inc'       ; "Vector brief" variables
include 'scenario_main\connect_var.inc'        ; Main win. system info. vars.
include 'scenario_simple\connect_var.inc'      ; "Run simple" variables
include 'system_info\connect_var.inc'          ; System nformation variables
include 'targets_bandwidth\connect_var.inc'    ; Bandwidth patterns variables
include 'targets_latency\connect_var.inc'      ; Latency patterns variables
include 'targets_math\connect_var.inc'         ; Math patterns variables
include 'threads_manager\connect_var.inc'      ; Threads management variables


;---------------------------- IMPORT section ----------------------------------;
section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphics 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

