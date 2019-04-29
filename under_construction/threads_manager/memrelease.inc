;---------- Memory release ----------------------------------------------------;
; Memory allocation mode for non-NUMA platforms and test scenarios.            ;
;                                                                              ;
; INPUT:   RBX = Pointer to Control Entry                                      ;   
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          Registers corrupted: by convention plus RBP                         ;
;------------------------------------------------------------------------------;

MemoryRelease:
mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
sub rsp,32                     ; Build parameters shadow, 4 qwords

; Normal and Large Pages branches divergention
test [rbx+THCTRL.LargePages],0001h    ; Check Large Pages flag
jnz .LargePages                       ; Go if Large Pages branch 

; Branch for Normal Pages
.NormalPages:
mov eax,1                      ; This for status "no error" when input size = 0
mov rcx,[rbx+THCTRL.OrigBase]  ; RCX = Parm#1 = Base address, before alignment
jrcxz @f                       ; Skip if RCX=0 means block not allocated
cdq                            ; RDX = Parm#2 = Size, 0=Auto by allocated size
mov r8d,MEM_RELEASE            ; R8  = Parm#3 = Type of memory free operation
call [VirtualFree]             ; Call memory release WinAPI, return RAX = Status
@@:
jmp .StatusPhase               ; Go to status phase

; Branch for Large Pages
.LargePages:
call [GetCurrentProcess]       ; RAX = Get Process Handle
test rax,rax
jz .StatusPhase                ; Go error if process handle = 0
xchg rcx,rax                   ; RCX = Parm#1 = Process handle, note XCHG RAX is compact code
mov rdx,[rbx+THCTRL.OrigBase]  ; RDX = Parm#2 = Base address
xor r8d,r8d                    ; R8 = Parm#2 = Size, 0=Auto by allocated size
mov r9d,MEM_RELEASE            ; R9 = Parm#3 = Type of memory free operation
call [VirtualFreeEx]           ; Call memory release WinAPI, return RAX = Status 

; Normal and Large Pages branches convergention 
.StatusPhase:
jmp ExitMemalloc
