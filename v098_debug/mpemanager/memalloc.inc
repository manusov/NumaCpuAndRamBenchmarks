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
xor ecx,ecx                    ; Parm#1 = RCX = Virtual address, 0=Auto select
mov rdx,[rbx+THCTRL.SizeBytes] 
add rdx,4096                   ; Parm#2 = RDX = Size + alignment reserved
mov r8d,MEM_COMMIT             ; Parm#3 = Allocation type
mov r9d,PAGE_READWRITE         ; Parm#4 = Page Attribute
call [VirtualAlloc]
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
mov eax,1                      ; This for status "no error" when input size = 0
mov rcx,[rbx+THCTRL.TrueBase]  ; RCX = Parm#1 = Base address, before alignment
jrcxz @f                       ; Skip if not allocated
cdq                            ; RDX = Parm#2 = Size, 0=Auto by allocated size
mov r8d,MEM_RELEASE            ; R8  = Parm#3 = Type of memory free operation
call [VirtualFree]
@@:
jmp ExitMemalloc
