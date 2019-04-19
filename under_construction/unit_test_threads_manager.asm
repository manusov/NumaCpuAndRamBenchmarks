; Template for unit tests.
; Unit test for threads and memory manager. 

include 'win64a.inc'
include 'global\connect_equ.inc'
include 'threads_manager\connect_equ.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

sub rsp,8*5

call SystemFunctionsLoad
jc ErrorSkip

lea rcx,[NumaNodesList]
xor edx,edx
call BuildNumaNodesList
jc ErrorSkip
test eax,eax
jz ErrorSkip

cld
push rax
lea rsi,[MessageText]
lea rdi,[TEMP_BUFFER]
call StringWrite
pop rcx

lea rsi,[NumaNodesList]
@@:
mov ax,0A0Dh
stosw
mov rax,[rsi + NUMACTRL.NodeAffinity]
call HexPrint64
add rsi,NUMACTRL_SIZE
mov al,'h'
stosb
loop @b

lea rdx,[TEMP_BUFFER]  ; RDX = Parm #2 = Message
lea r8,[WinCaption]    ; R8  = Parm #3 = Caption (upper message)
xor r9d,r9d            ; R9  = Parm #4 = Message flags
xor ecx,ecx            ; RCX = Parm #1 = Parent window
call [MessageBoxA]

ErrorSkip:

xor ecx,ecx
call [ExitProcess]

; only for build unit test, dummy routines
GetBandwidthPattern:  
GetBandwidthDump:
GetLatencyPattern:
GetLatencyDump:
MeasureTsc:
ret

;------------------------------------------------------------------------------;
; Load some functions from KERNEL32.DLL system library.                        ;  
; This functions not declared in the import section and required manually load ;
; for Windows XP x64 compatibility reasons. Otherwise abort when interpreting  ;
; import section under Windows XP x64.                                         ;
;                                                                              ;
; INPUT:  None at registers                                                    ;
;         Statical table used: FunctionsNames as text strings                  ;
;                                                                              ;
; OUTPUT: CF flag = status: 0(NC) = all functions detected                     ;
;                           1(C)  = warning message required,                  ;
;                                   some or all functions not detected         ;
;         Functions address table updated                                      ;
;         Registers save/destroy by Microsoft x64 calling convention           ;
;------------------------------------------------------------------------------;

SystemFunctionsLoad:
push rbx rbp rsi rdi r12
cld

; Pre-load library ADVAPI32.DLL, next step uses GetModuleHandle, GetProcAddress
; for detect functions entry points. Returned handle and status ignored at this
; step, because availability of library and functions detected at next step.
; Note pre-load KERNEL32.DLL not required because static import used.
; Load this library, because it not loaded by statical import
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea rcx,[NameAdvapi32]
call [LoadLibrary]
mov [HandleAdvapi32],rax
mov rsp,rbp


; Load functions of KERNEL32.DLL
lea rcx,[NameKernel32]            ; RCX = Parm#1 = Pointer to module name string
lea rsi,[FunctionsNamesKernel32]  ; RSI = Pointer to functions names list
lea rdi,[FunctionsPointersAll]    ; RDI = Pointer to fncs. pointers list build 
xor r12d,r12d                     ; R12 = Error flag, pre-blank
call HelperLoadLibraries 
; Load functions of ADVAPI32.DLL
lea rcx,[NameAdvapi32]
lea rsi,[FunctionsNamesAdvapi32]
call HelperLoadLibraries
; Exit with CF=status
shr r12,1                    ; Move bit R12.0 to CF
pop r12 rdi rsi rbp rbx
ret

; Local helper subroutine for load functions,
; subroutine required because 2 DLLs
; INPUT:   RCX = Pointer to DLL file name
;          RSI = Pointer to DLL functions names constants list
;          RDI = Pointer to DLL functions entry points variables list
;          R12 = Error flag value for OR operation, bit R12.0 = error flag
; OUTPUT:  RDI = Advance by output list build
;          R12 = Error flag updated
;          Functions entries pointers updated
;          Other registers can be corrupted
   
HelperLoadLibraries:
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
; Get handle for selected DLL
call [GetModuleHandle]       ; RAX = Return module handle
xchg rbx,rax                 ; RBX = Module handle, XCHG is compact
; Get functions pointers
.L0:
xor eax,eax
test rbx,rbx
jz .L2
mov rcx,rbx                  ; RCX = Parm#1 = Pointer to module handle
mov rdx,rsi                  ; RDX = Parm#2 = Pointer to function name
call [GetProcAddress]        ; RAX = Return function address
.L2:
stosq
test rax,rax
jnz .L1
or r12d,1
.L1:
lodsb
cmp al,0
jne .L1                      ; This cycle for find next function name
cmp byte [rsi],0
jne .L0                      ; This cycle for sequence of functions names
mov rsp,rbp
ret

include 'global\connect_code.inc'
include 'threads_manager\connect_code.inc'

section '.data' data readable writeable

include 'global\connect_const.inc'
include 'threads_manager\connect_const.inc'

;------------------------------------------------------------------------------;
;            Data for load some system functions - constants pool.             ;
; This functions not declared in the import section and required manually load ;
; for Windows XP x64 compatibility reasons. Otherwise abort when interpreting  ;
; import section under Windows XP x64, because some API functions not found.   ;
;------------------------------------------------------------------------------;

; Library name KERNEL32.DLL
NameKernel32:                         ; Libraries names
DB  'KERNEL32.DLL',0

; Library name ADVAPI32.DLL
NameAdvapi32:
DB  'ADVAPI32.DLL',0

; Functions names at KERNEL32.DLL
FunctionsNamesKernel32:               ; Functions names
DB  'GlobalMemoryStatusEx',0
DB  'GetNumaHighestNodeNumber',0
DB  'GetNumaNodeProcessorMask',0
DB  'VirtualAllocExNuma',0
DB  'SetThreadAffinityMask',0                
DB  'GetSystemFirmwareTable',0
DB  'GetLogicalProcessorInformation',0
DB  'GetLargePageMinimum',0
; DB  'VirtualAllocEx',0
DB  'GetActiveProcessorCount',0             ; added for support > 64 processors 
DB  'GetActiveProcessorGroupCount',0        ; added for support > 64 processors
DB  'GetNumaNodeProcessorMaskEx', 0         ; added for support > 64 processors
DB  'SetThreadGroupAffinity',0              ; added for support > 64 processors
DB  0                           ; List must be terminared by additional 00 byte

; Functions names at ADVAPI32.DLL
FunctionsNamesAdvapi32:
DB  'OpenProcessToken',0
DB  'AdjustTokenPrivileges',0
DB  0                           ; List must be terminared by additional 00 byte

WinCaption   DB ' Unit test for threads and memory manager' , 0
MessageText  DB 'Test for NUMA nodes list.' , 0

include 'global\connect_var.inc'
include 'threads_manager\connect_var.inc'

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

