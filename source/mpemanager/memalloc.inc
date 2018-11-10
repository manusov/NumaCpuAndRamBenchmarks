;------------------------------------------------------------------------------;
;    Memory allocation library, for non-NUMA platforms and test scenarios.     ;
;------------------------------------------------------------------------------;

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
sub rsp,32                     ; Build parameters shadow, 4 qwords

; Normal and Large Pages branches divergention
test [rbx+THCTRL.LargePages],0001h  ; Check Large Pages flag
jnz .LargePages                     ; Go if Large Pages branch 

; Branch for Normal Pages
.NormalPages:
xor ecx,ecx                        ; Parm#1 = RCX = Virtual address, 0=Auto select
mov rdx,[rbx+THCTRL.SizeBytes]     ; RDX = Memory block size 
add rdx,4096                       ; Parm#2 = RDX = Size + alignment reserved
mov r8d,MEM_RESERVE + MEM_COMMIT   ; Parm#3 = Allocation type
mov r9d,PAGE_READWRITE             ; Parm#4 = Page Attribute
call [VirtualAlloc]                ; Call memory allocation WinAPI, return RAX = Pointer
jmp .StatusPhase                   ; Go to status phase

; Branch for Large Pages
.LargePages:
call [GetCurrentProcess]              ; RAX = Get Process Handle
test rax,rax                          ; Check process handle
jz .StatusPhase                       ; Go error if process handle = 0
mov rcx,rax                           ; Parm#1 = RCX = Process Handle
; Calculate block size with required alignment and reserve
mov rax,[rbx+THCTRL.SizeBytes]        ; RAX = Required block size
mov r8,[LargePageSize]                ; R8 = Large Page size, bytes
test r8,r8                            ; Check Large Page size detection results
jz .StatusPhase                       ; Go error to prevent divide by 0
xor edx,edx                           ; Dividend bits [127-64] = 0
div r8                                ; Divide RDX:RAX / R8.  RAX = Quotient, RDX = Remainder
mul r8                                ; Multiply back RAX * R8. This for reject remainder
lea r8,[rax+r8]                       ; Parm#3 = R8 = Block size + alignment reserved
; Prepare other parameters and call function 
xor edx,edx                           ; Parm#2 = RDX = Virtual address, 0=Auto select
mov r9d,MEM_RESERVE + MEM_COMMIT + MEM_LARGE_PAGES  ; Parm#4 = R9 = Allocation type
push 0                                ; Alignment
push PAGE_READWRITE                   ; Parm#5 = [RSP+40] = Page Attribute
sub rsp,32                            ; Create stack frame
call [VirtualAllocEx]                 ; Call memory allocation WinAPI, return RAX = Pointer
add rsp,32+16                         ; Remove stack frame

; Normal and Large Pages branches convergention 
.StatusPhase:
test rax,rax                   ; Check pointer = memory allocation result
jz ExitMemalloc                ; Go error if memory allocation failed
mov [rbx+THCTRL.TrueBase],rax  ; Store base for release function
test eax,0FFFh                 ; Check 4K-alignment
jz @f                          ; Go if already aligned or if RAX=0 when error
and rax,0FFFFFFFFFFFFF000h
add rax,00000000000001000h
@@:                            ; Set required memory blocks bases, use input RBX
add [rbx+THCTRL.Base1],rax     ; Source base, or destination for single block
add [rbx+THCTRL.Base2],rax     ; Destination base, if both source and destination used

; This exit point also used by some other subroutines
ExitMemalloc:                  ; Src. and Dst. later updated by Read/Write/Copy scenario
test rax,rax                   ; Check pointer = memory allocation and alignment result 
jnz @f                         ; Go if no errors, with CF=0 after TEST instruction
stc                            ; Set CF = Error flag
@@:
mov rsp,rbp                    ; Restore RSP after stack alignment at subroutine entry 
ret

;---------- Memory release ----------------------------------------------------;
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
mov rcx,[rbx+THCTRL.TrueBase]  ; RCX = Parm#1 = Base address, before alignment
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
mov rdx,[rbx+THCTRL.TrueBase]  ; RDX = Parm#2 = Base address
xor r8d,r8d                    ; R8 = Parm#2 = Size, 0=Auto by allocated size
mov r9d,MEM_RELEASE            ; R9 = Parm#3 = Type of memory free operation
call [VirtualFreeEx]           ; Call memory release WinAPI, return RAX = Status 

; Normal and Large Pages branches convergention 
.StatusPhase:
jmp ExitMemalloc
