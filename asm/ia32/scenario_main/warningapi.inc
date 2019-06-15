;--- Output warning messages box if some optional OS API functions not found --;
;                                                                              ;
; INPUT:  CF flag = status: 0(NC) = all functions found, skip subroutine       ;
;                           1(C)  = one or some functions not found,           ;
;                                   need generating warning box                ;
;         Statical table used: FunctionsNames as text strings                  ;
;         Builded table used: FunctionsPointers, x[i]=0 = means not found      ;
;                                                                              ;
; OUTPUT: None                                                                 ;
;         Registers save/destroy by Microsoft ia32 calling convention          ;
;------------------------------------------------------------------------------;

ShowWarningAPI:       ; CF flag is input parameter, CF=1 means errors detected
jnc .L9               ; Go skip if no errors
push ebx esi edi
cld
; Parameters for text build
lea esi,[MsgWarnAPI]
lea edi,[TEMP_BUFFER]
call StringWrite
; Build list of not-loaded functions at KERNEL32.DLL
lea esi,[FunctionsNamesKernel32]
lea ebx,[FunctionsPointersAll]
call HelperCheckLibraries
; Build list of not-loaded functions at ADVAPI32.DLL
lea esi,[FunctionsNamesAdvapi32]
call HelperCheckLibraries
; Text termination
mov al,0
stosb 
; Warning box and wait user input
push MB_ICONWARNING    ; Parm#4 = Message box icon type
push PRODUCT_ID        ; Parm#3 = Pointer to caption
push TEMP_BUFFER       ; Parm#2 = Pointer to string
push 0                 ; Parm#1 = Parent window handle or 0
call [MessageBoxA]
; Exit
pop edi esi ebx
.L9:
ret

; Local helper subroutine for check load functions,
; subroutine required because 2 DLLs
; INPUT:   ESI = Pointer to DLL functions names constants list
;          EBX = Pointer to DLL functions entry points variables list
; OUTPUT:  EBX = Advance by list check
;          Other registers can be corrupted

HelperCheckLibraries:
.L0:
mov eax,[ebx]        ; Get current function pointer
add ebx,4
test eax,eax         ; 0 means invalid pointer
jnz .L1              ; Go if current pointer valid
call StringWrite     ; This branch - write name of not found function
mov ax,0A0Dh
stosw
jmp .L2
.L1:                 ; This branch - skip name of found function
lodsb
cmp al,0
jne .L1
.L2:
cmp byte [esi],0
jne .L0              ; Cycle for functions list
ret

