;--- Memory allocation library, for non-NUMA platform ---

;---------- Memory allocation with alignment = 4096 bytes ---------------------;
; INPUT:   RBX = Pointer to Control Entry                                      ;
;          First entry of Threads Control List contains operational data       ;   
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;  
;          Registers corrupted: by convention plus RBP                         ;
;          First entry of Threads Control List updated                         ;
;------------------------------------------------------------------------------;
MemoryAllocation:
mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
sub rsp,32                     ; Built parameters shadow, 4 qwords

; Normal and Large Pages branches divergention
test [rbx+THCTRL.LargePages],0001h
jnz .LargePages 

; Branch for Normal Pages
.NormalPages:
xor ecx,ecx                        ; Parm#1 = RCX = Virtual address, 0=Auto select
mov rdx,[rbx+THCTRL.SizeBytes] 
add rdx,4096                       ; Parm#2 = RDX = Size + alignment reserved
mov r8d,MEM_RESERVE + MEM_COMMIT   ; Parm#3 = Allocation type
mov r9d,PAGE_READWRITE             ; Parm#4 = Page Attribute
call [VirtualAlloc]
jmp .StatusPhase

; Branch for Large Pages
.LargePages:
call [GetCurrentProcess]              ; Get Process Handle
test rax,rax
jz .StatusPhase
mov rcx,rax                           ; Parm#1 = RCX = Process Handle
; Calculate block size with required alignment and reserve
mov rax,[rbx+THCTRL.SizeBytes]        ; RAX = Required block size
mov r8,[LargePageSize]
test r8,r8
jz .StatusPhase                       ; Go error to prevent divide by 0
xor edx,edx
div r8
mul r8
lea r8,[rax+r8]                       ; Parm#3 = R8 = Block size + alignment reserved
; Prepare other parameters and call function 
xor edx,edx                           ; Parm#2 = RDX = Virtual address, 0=Auto select
mov r9d,MEM_RESERVE + MEM_COMMIT + MEM_LARGE_PAGES  ; Parm#4 = R9 = Allocation type
push 0                                ; Alignment
push PAGE_READWRITE                   ; Parm#5 = [RSP+40] = Page Attribute
sub rsp,32                            ; Create stack frame
call [VirtualAllocEx]
add rsp,32+16                         ; Remove stack frame

; Normal and Large Pages branches convergention 
.StatusPhase:
test rax,rax
jz ExitMemalloc
mov [rbx+THCTRL.TrueBase],rax  ; Store base for release function
test eax,0FFFh
jz @f                          ; Go if already aligned or if RAX=0 when error
and rax,0FFFFFFFFFFFFF000h
add rax,00000000000001000h
@@:
add [rbx+THCTRL.Base1],rax     ; Set required memory blocks bases, use input
add [rbx+THCTRL.Base2],rax
ExitMemalloc:
test rax,rax
jnz @f
stc
@@:
mov rsp,rbp
ret

;---------- Memory release ----------------------------------------------------;
; INPUT:   RBX = Pointer to Control Entry                                      ;   
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          Registers corrupted: by convention plus RBP                         ;
;------------------------------------------------------------------------------;
MemoryRelease:
mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
sub rsp,32                     ; Built parameters shadow, 4 qwords

; Normal and Large Pages branches divergention
test [rbx+THCTRL.LargePages],0001h
jnz .LargePages 

; Branch for Normal Pages
.NormalPages:
mov eax,1                      ; This for status "no error" when input size = 0
mov rcx,[rbx+THCTRL.TrueBase]  ; RCX = Parm#1 = Base address, before alignment
jrcxz @f                       ; Skip if not allocated
cdq                            ; RDX = Parm#2 = Size, 0=Auto by allocated size
mov r8d,MEM_RELEASE            ; R8  = Parm#3 = Type of memory free operation
call [VirtualFree]
@@:
jmp .StatusPhase

; Branch for Large Pages
.LargePages:
call [GetCurrentProcess]              ; Get Process Handle
test rax,rax
jz .StatusPhase
mov rcx,rax                    ; RCX = Parm#1 = Process handle
mov rdx,[rbx+THCTRL.TrueBase]  ; RDX = Parm#2 = Base address
xor r8d,r8d                    ; R8 = Parm#2 = Size, 0=Auto by allocated size
mov r9d,MEM_RELEASE            ; R9 = Parm#3 = Type of memory free operation
call [VirtualFreeEx]

; Normal and Large Pages branches convergention 
.StatusPhase:
jmp ExitMemalloc
