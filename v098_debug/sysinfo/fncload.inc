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
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
; Get handle for KERNEL32.DLL
lea rcx,[NameKernel32]       ; RCX = Parm#1 = Pointer to module name string
call [GetModuleHandle]       ; RAX = Return module handle
xchg rbx,rax                 ; RBX = Module handle, XCHG is compact
; Get functions pointers
cld
lea rsi,[FunctionsNames]
lea rdi,[FunctionsPointers]
xor r12d,r12d
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
; Exit with CF=status
shr r12,1                    ; Move bit R12.0 to CF
mov rsp,rbp
pop r12 rdi rsi rbp rbx
ret



  
