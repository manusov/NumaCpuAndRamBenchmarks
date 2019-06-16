;------------------------------------------------------------------------------;
;                        Template for unit tests.                              ;
;                Unit test for threads and memory manager:                     ;
;    build with run single routine: get and show NUMA nodes list = bitmap.     ; 
;------------------------------------------------------------------------------;

include 'win32a.inc'
include 'global\connect_equ.inc'
include 'threads_manager\connect_equ.inc'

format PE GUI 4.0
entry start
section '.text' code readable executable
start:

; Note ADVAPI32 library load in the SystemFunctionsLoad subroutine at this test 
; this action moved to main module at BUILD_NCRB.ASM

call SystemFunctionsLoad
jc ErrorSkip


;--- Additional debug skip ---

;- INT3
;- lea ebx,[NumaNodesList]     ; EBX = pointer to nodes list
;- mov ecx,4096                ; ECX = memory allocation size per node
;- mov edx,1                   ; EDX = nodes count
;- mov eax,LP_NOT_SUPPORTED    ; EAX = large pages option 
;- call MemAllocationNuma
;
;- INT3
;- lea ebx,[NumaNodesList]     ; EBX = pointer to nodes list
;- mov edx,1                   ; EDX = nodes count  
;- call MemReleaseNuma 
; 
;- jmp DebugSkip 


;- INT3
;- lea ecx,[ThreadsList]
;- lea edx,[InputParms]
;- mov [edx + IPB.UpdatedThreads],2
;- mov [edx + IPB.DomainsCount],1
;- mov [edx + IPB.UpdatedPG],PG_NOT_SUPPORTED
;- mov [edx + IPB.OperandWidth],8
;- call BuildThreadsList

;- INT3
;- lea ecx,[ThreadsList]
;- mov edx,2 
;- call ReleaseThreadsList
; 
;- jmp DebugSkip


;- lea ebx,[ThreadsList]
;- mov [ebx + THCTRL.EntryPoint],L0
;- call RunTarget
;- jmp DebugSkip
;-
;- L0:
;- nop
;- ret


;--- First debug fragment ---

lea ecx,[NumaNodesList]
xor edx,edx
mov eax,NUMA_OPTIMAL
call BuildNumaNodesList  ; Build list of NUMA nodes
jc ErrorSkip
test eax,eax
jz ErrorSkip

cld
push eax                 ; Store nodes count
lea esi,[MessageText]
lea edi,[TEMP_BUFFER]
call StringWrite         ; Write name of test
pop ecx                  ; ECX = restore nodes count

lea esi,[NumaNodesList]
@@:
mov ax,0A0Dh
stosw
mov eax,[esi + NUMACTRL.NodeAffinity]
call HexPrint32          ; Write 32-bit hex number: node affinity mask
add esi,NUMACTRL_SIZE    ; RSI = Pointer for nodes entries in the list
mov al,'h'
stosb
loop @b                  ; cycle for all nodes

mov al,0
stosb

DebugSkip:

push 0              ; Parm #4 = Message flags
push WinCaption     ; Parm #3 = Caption (upper message)
push TEMP_BUFFER    ; Parm #2 = Message
push 0              ; Parm #1 = Parent window
call [MessageBoxA]

; Note ADVAPI32 library unload absent in this test, it exist at BUILD_NCRB.ASM

ErrorSkip:

push 0
call [ExitProcess]

; only for build unit test, dummy routines
GetBandwidthPattern:  
GetBandwidthDump:
GetLatencyPattern:
GetLatencyDump:
MeasureTsc:
ret

;--- Load some functions from KERNEL32.DLL system library. --------------------;  
; This functions not declared in the import section and required manually load ;
; for Windows XP compatibility reasons. Otherwise abort when interpreting      ;
; import section under Windows XP.                                             ;
;                                                                              ;
; INPUT:  None at registers                                                    ;
;         Statical table used: FunctionsNames as text strings                  ;
;                                                                              ;
; OUTPUT: CF flag = status: 0(NC) = all functions detected                     ;
;                           1(C)  = warning message required,                  ;
;                                   some or all functions not detected         ;
;         Functions address table updated                                      ;
;         Registers save/destroy by Microsoft ia32 calling convention          ;
;------------------------------------------------------------------------------;

SystemFunctionsLoad:
push ebx ebp esi edi
cld
; Load functions of KERNEL32.DLL
lea ecx,[NameKernel32]            ; ECX = Pointer to module name string
lea esi,[FunctionsNamesKernel32]  ; ESI = Pointer to functions names list
lea edi,[FunctionsPointersAll]    ; EDI = Pointer to fncs. pointers list build 
xor ebp,ebp                       ; EBP = Error flag, pre-blank
call HelperLoadLibraries 
; Load functions of ADVAPI32.DLL, 
; note this library must be pre-loaded when application starts
lea ecx,[NameAdvapi32]
lea esi,[FunctionsNamesAdvapi32]
call HelperLoadLibraries
; Exit with CF=status
shr ebp,1                    ; Move bit EBP.0 to CF
pop edi esi ebp ebx
ret

;--- Local helper subroutine for load functions, ----------------------------;
;    subroutine required because 2 DLLs                                      ;
;                                                                            ;
; INPUT:   ECX = Pointer to DLL file name                                    ;
;          ESI = Pointer to DLL functions names constants list               ;
;          EDI = Pointer to DLL functions entry points variables list        ;
;          EBP = Error flag value for OR operation, bit EBP.0 = error flag   ;
;                                                                            ;
; OUTPUT:  EDI = Advance by output list build                                ;
;          EBP = Error flag updated                                          ;
;          Functions entries pointers updated                                ;
;          Other registers can be corrupted                                  ;
;----------------------------------------------------------------------------;
   
HelperLoadLibraries:
; Get handle for selected DLL
push ecx
call [GetModuleHandle]       ; EAX = Return module handle
xchg ebx,eax                 ; EBX = Module handle, XCHG is compact
; Get functions pointers
.L0:
xor eax,eax
test ebx,ebx
jz .L2
push esi                     ; Parm#2 = Pointer to function name
push ebx                     ; Parm#1 = Pointer to module handle
call [GetProcAddress]        ; EAX = Return function address
.L2:
stosd
test eax,eax
jnz .L1
or ebp,1
.L1:
lodsb
cmp al,0
jne .L1                  ; This cycle for find next function name, skip string
cmp byte [esi],0
jne .L0                  ; This cycle for sequence of functions names
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

