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

  
