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

; Load function for check run NCRB ia32 version under Win64, 
; this required for show warning about non-optimal mode
lea ecx,[NameKernel32]            ; ECX = Pointer to module name string
lea esi,[FunctionNameWoW64]       ; ESI = Pointer to functions names list
lea edi,[FunctionPointerWoW64]    ; EDI = Pointer to fncs. pointers list build 
call HelperLoadLibraries

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


;--- Check WoW64 mode -------------------------------------------------------;
;    this required for show warning about non-optimal mode                   ;
;                                                                            ;
; INPUT: None                                                                ;
;                                                                            ; 
; OUTPUT: CF flag = status: 0(NC) = all functions detected                   ;
;                           1(C)  = warning message required,                ;
;                                   because WoW64 mode detected              ;
;                                                                            ;
;         Registers save/destroy by Microsoft ia32 calling convention        ;
;----------------------------------------------------------------------------;
CheckWoW64:
push ebx
; Check pointer to WoW64 detect function  
mov ebx,[_IsWow64Process]
test ebx,ebx
jz .Done
; Get process handle
call [GetCurrentProcess]   ; Get handle of current process
test eax,eax
jz .Done
; Reserve stack space for variable
push 0
; Call WoW64 detect function
mov ecx,esp
push ecx                   ; Parm#2 = Pointer to BOOL WoW64 Process
push eax                   ; Parm#1 = hProcess handle 
call ebx                   ; Call function: IsWow64Process 
test eax,eax
jz .Done
; Check output variable
pop eax
test eax,eax
jz .Done
stc
; Done
.Done:
pop ebx
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

