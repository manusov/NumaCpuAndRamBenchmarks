;--- Get Large Pages information by Windows API -------------------------;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save destination info           ;
;                                                                        ;
; OUTPUT:  RDI = Input RDI + 9 , buffer size fixed = 9 bytes             ;
;          Destination memory at [input RDI] = Results, 0=Not detected	 ;
;          Qword [000-007] = Large page size (minimum if some variants)  ;
;          Dword [008-011] = Availability flag:                          ; 
;                            0=n/a , 1=large pages mappings useable      ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft x64 calling convention.   ; 
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
virtual at r8 
tp TOKEN_PRIVILEGES 
end virtual

;--- Procedure entry point ---

GetLargePagesInfo:
push rbx rsi rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack
sub rsp,128+32               ; Reserve stack space for token, variables, parms.

; Get large page size
mov rax,[_GetLargePageMinimum]
test rax,rax
jz .NoLargePages                     ; Go skip if function not available
call rax
test rax,rax
jz .NoLargePages                     ; Go skip if large page size = 0
stosq                                ; Store large page size  (qword)

; Check privileges control availability
xor eax,eax
cmp [_OpenProcessToken],rax        ; check fnc. pointer after dynamical import
je .NoLargeRights                  ; Go skip if function not available
cmp [_AdjustTokenPrivileges],rax   ; check fnc. pointer after dynamical import 
je .NoLargeRights                  ; Go skip if function not available

; Get privileges
call [GetCurrentProcess]
test rax,rax
jz .LocalExit              ; Go if error get current process
mov rcx,rax                ; RCX = Parm#1 = hCurrentProcess
mov edx,MAXIMUM_ALLOWED    ; RDX = Parm#2 = Desired access 
lea r8,[rsp+120-32]        ; R8  = Parm#3 = Pointer to token handle
call [_OpenProcessToken] 
test rax,rax 
jz .LocalExit              ; Go if error get token handle   
; Code use this template
mov rcx,[rsp+120-32]       ; RCX = Parm#1 = Token Handle  
xor edx,edx                ; RDX = Parm#2 = DisableAllPrivileges flag, none
lea r8,[rsp+48]            ; R8  = Parm#3 = Pointer to TOKEN PRIVILEGES struct.
xor r9d,r9d                ; R9  = Parm#4 = Buffer Length = Not used = 0
mov [rsp+32],r9            ; Parm#6 = Previous state = not used = 0
mov [rsp+40],r9            ; Parm#5 = Return length = not used = 0
mov [tp.PrivilegeCount],1  ; command entries count = 1 
mov [tp.Privileges.Luid.usedpart],SE_LOCK_MEMORY_PRIVILEGE  ; command parameter 
and [tp.Privileges.Luid.ignorehigh32bitpart],0              ; blank field  
mov [tp.Privileges.Attributes],SE_PRIVILEGE_ENABLED         ; command
call [_AdjustTokenPrivileges] 
mov rbx,rax                ; Store status for latent analyse 
mov rcx,[rsp+120-32]       ; RCX = Parm#1 = Handle 
call [CloseHandle] 
.LocalExit:

; Check errors
test rax,rax                ; GetCurrentProcess and OpenProcessToken and CloseHandle success ?
jz .NoLargeRights           ; Go skip memory allocation try if error
xor eax,eax
test rbx,rbx                ; AdjustTokenPrivileges success ?
jz .NoLargeRights           ; Go skip memory allocation try if error 

; Get Process Handle
call [GetCurrentProcess]
test rax,rax
jz .NoLargeRights              ; Go if error
mov rbx,rax                    ; RBX = Handle

; Probe operation: try memory allocation with large pages
mov rcx,rax                           ; Parm#1 = RCX = Process Handle 
xor edx,edx                           ; Parm#2 = RDX = Virtual address, 0=Auto select
mov r8,[rdi-8]                        ; Parm#3 = R8 = Size for probe
mov r9d,MEM_COMMIT + MEM_LARGE_PAGES  ; Parm#4 = R9 = Allocation type
push 0                                ; Alignment
push PAGE_READWRITE                   ; Parm#5 = [RSP+40] = Page Attribute
sub rsp,32                            ; Create stack frame
call [VirtualAllocEx]
add rsp,32+16                         ; Remove stack frame

; Check allocation status
test rax,rax
jz @f                          ; Skip if not allocated

; Memory release after probe allocation
mov rcx,rbx                    ; RCX = Parm#1 = Process handle
mov rdx,rax                    ; RDX = Parm#2 = Base address
xor r8d,r8d                    ; R8 = Parm#2 = Size, 0=Auto by allocated size
mov r9d,MEM_RELEASE            ; R9 = Parm#3 = Type of memory free operation
call [VirtualFreeEx]

; Check allocation-release status
@@:
test rax,rax
setnz al
movzx eax,al
stosd                 ; Store Large pages availability status  (dword) 

; Exit points, note stack restored by MOV RSP,RBP at this branch 
.Exit:
mov rsp,rbp
pop rbp rsi rbx
ret

.NoLargePages:
stosq                 ; Large page size = 0  (qword)
.NoLargeRights:
stosd                 ; Large pages availability status = 0  (dword)
jmp .Exit
