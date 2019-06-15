;--- Get Large Pages information by Windows API -------------------------;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save destination info           ;
;                                                                        ;
; OUTPUT:  EDI = Input EDI + 9 , buffer size fixed = 9 bytes             ;
;          Destination memory at [input EDI] = Results, 0=Not detected	 ;
;          Qword [000-007] = Large page size (minimum if some variants)  ;
;          Dword [008-011] = Availability flag:                          ; 
;                            0=n/a , 1=large pages mappings useable      ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

;--- Macro definitions ---

; Constants for privileges control 
SE_PRIVILEGE_ENABLED     = 2         ; command for privilege control 
SE_LOCK_MEMORY_PRIVILEGE = 4         ; parameter is privilege type

; Templates for privileges control
; Locally unique identifier
struct LUID 
usedpart             dd  ?    ; used for parameter: SE_LOCK_MEMORY_PRIVILEGE   
ignorehigh32bitpart  dd  ?    ; usually blanked to 0 
ends 
; Command with parameter definition for priviligies control
struct LUID_AND_ATTRIBUTES 
Luid        LUID               ; includes command parameter 
Attributes  dd  ?              ; used for command: SE_PRIVILEGE_ENABLED  
ends 
; Count of entries, entries (command + parameter)
struct TOKEN_PRIVILEGES 
PrivilegeCount  dd  ?                ; number of entries, set to 1 in the code 
Privileges      LUID_AND_ATTRIBUTES  ; one entry: command + parameter 
ends 
; Structure addressing and layout definitions 
virtual at ebx 
tp TOKEN_PRIVILEGES 
end virtual

;--- Procedure entry point ---

GetLargePagesInfo:
push ebx esi ebp
sub esp,128           ; Reserve stack space for token and variables

; Get large page size
mov eax,[_GetLargePageMinimum]
test eax,eax
jz .NoLargePages      ; Go skip if function not available
call eax
test eax,eax
jz .NoLargePages      ; Go skip if large page size = 0
stosd                 ; Store large page size, low 32 bit
xor eax,eax           ; EAX = 0 , note this used at next step
stosd                 ; Store large page size, high 32 bit = 0 

; Check privileges control availability, check Win API functions pointers
cmp [_OpenProcessToken],eax        ; check fnc. pointer after dynamical import
je .NoLargeRights                  ; Go skip if function not available
cmp [_AdjustTokenPrivileges],eax   ; check fnc. pointer after dynamical import 
je .NoLargeRights                  ; Go skip if function not available

; Get privileges
call [GetCurrentProcess]
test eax,eax
jz .LocalExit              ; Go if error get current process
lea ebp,[esp + 120]        ; EBP = Pointer to token handle
push ebp                   ; Parm#3 = Pointer to token handle 
push MAXIMUM_ALLOWED       ; Parm#2 = Desired access
push eax                   ; Parm#1 = hCurrentProcess
call [_OpenProcessToken] 
test eax,eax 
jz .LocalExit              ; Go if error get token handle   

; Code use this template
xor eax,eax                ; EAX = 0 for compact PUSH 0 next  
mov ebx,esp                ; EBX = Pointer to TOKEN PRIVILEGES structure
push eax                   ; Parm#6 = Previous state = not used = 0
push eax                   ; Parm#5 = Return length = not used = 0
push eax                   ; Parm#4 = Buffer Length = Not used = 0
push ebx                   ; Parm#3 = Pointer to TOKEN PRIVILEGES structure 
push eax                   ; RDX = Parm#2 = DisableAllPrivileges flag, none
push dword [ebp]           ; Parm#1 = Token Handle
mov [tp.PrivilegeCount],1  ; command entries count = 1 
mov [tp.Privileges.Luid.usedpart],SE_LOCK_MEMORY_PRIVILEGE  ; command parameter 
and [tp.Privileges.Luid.ignorehigh32bitpart],0              ; blank field  
mov [tp.Privileges.Attributes],SE_PRIVILEGE_ENABLED         ; command
call [_AdjustTokenPrivileges] 
mov ebx,eax              ; Store status for latent analyse 
push dword [ebp]         ; Parm#1 = Token Handle 
call [CloseHandle] 
.LocalExit:

; Check errors
test eax,eax             ; GetCurrentProcess and OpenProcessToken and CloseHandle success ?
jz .NoLargeRights        ; Go skip memory allocation try if error
xor eax,eax
test ebx,ebx             ; AdjustTokenPrivileges success ?
jz .NoLargeRights        ; Go skip memory allocation try if error 

; Get Process Handle
call [GetCurrentProcess]
test eax,eax
jz .NoLargeRights        ; Go if error
mov ebx,eax              ; EBX = Handle

; Probe operation: try memory allocation with large pages
push PAGE_READWRITE                 ; Parm#5 = Page Attribute
push MEM_COMMIT + MEM_LARGE_PAGES   ; Parm#4 = Allocation type
push dword [edi-8]                  ; Parm#3 = Size for probe
push 0                              ; Parm#2 = Virtual address, 0=Auto select
push ebx                            ; Parm#1 = Process Handle
call [VirtualAllocEx]

; Check allocation status
test eax,eax
jz @f                     ; Skip if not allocated

; Memory release after probe allocation
push MEM_RELEASE          ; Parm#3 = Type of memory free operation
push 0                    ; Parm#2 = Size, 0=Auto by allocated size
push eax                  ; Parm#2 = Base address
push ebx                  ; Parm#1 = Process handle
call [VirtualFreeEx]

; Check allocation-release status
@@:
test eax,eax
setnz al
movzx eax,al
stosd                 ; Store Large pages availability status  (dword) 

; Exit points, note stack restored by MOV RSP,RBP at this branch 
.Exit:
add esp,128
pop ebp esi ebx
ret

.NoLargePages:
stosd                 ; Large page size = 0  (qword)
stosd
.NoLargeRights:
stosd                 ; Large pages availability status = 0  (dword)
jmp .Exit
