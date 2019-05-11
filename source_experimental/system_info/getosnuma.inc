;--- Get NUMA information by Windows API --------------------------------;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save ACPI SRAT info             ;
;                                                                        ;
; OUTPUT:  RDI = Input RDI + 516 , buffer size fixed = 516 bytes         ;
;          Destination memory at [input RDI] = Results, 0=Not detected	 ;
;          Dword [000-003] = Number of NUMA Domains, 0=Not detected      ;
;          Qword [004-011] = Domain (Node) #0 Affinity Mask              ;
;          ...                                                           ;
;          Qword [012-515] = Domain (Node) #63 Affinity Mask             ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft x64 calling convention.   ; 
;------------------------------------------------------------------------;

GetOsNumaInfo:
cld                          ; Clear direction, because STOSQ used
push rbx rsi rbp 0           ; Note push 0 for create local variable
mov rbp,rsp                  ; RBP used for restore RSP and addressing var.
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack because WinAPI used
sub rsp,32                   ; Make parameters shadow
; Blank results
mov rbx,rdi                  ; RBX = Backup output region address
mov ecx,516
mov al,0
rep stosb
; Get maximum node number
mov rcx,rbp                  ; RCX = Parm #1 = Pointer to variable
mov rax,[_GetNumaHighestNodeNumber]
test rax,rax
jz .Exit                     ; Go skip if function not declared by OS
call rax                     ; Return EAX=0 if error, EAX>0 if no errors
test rax,rax
jz .Exit                     ; Go skip if error returned
cmp dword [rbp],65535        ; [RBP] = Number of nodes
ja .Exit                     ; Go skip if invalid number of nodes
; Scan nodes
xor esi,esi                  ; RSI = Start node number
lea rdi,[rbx+4]              ; RDI = Pointer to nodes list, qwords sequence
.NumaScan:                   ; RBX = Pointer to nodes counter, dword
mov ecx,esi                  ; RCX = Parm #1 = Node number
mov rdx,rdi                  ; RDX = Parm #2 = Pointer to var. = Processor Mask
mov rax,[_GetNumaNodeProcessorMask]
test rax,rax
jz .Exit                     ; Go skip if function not declared by OS 
call rax                     ; Write qword to [rdx]
test  rax,rax
jnz @f                       ; Go skip blank if this node valid
mov qword [rdi],0
@@:
jz @f                        ; Go skip pointer advance if this node not valid
inc dword [rbx]              ; Memory at [RBX] = Valid nodes counter +1
add rdi,8                    ; Nodes list pointer +8
@@:
mov rax,rdi
sub rax,rbx
cmp eax,516
jae .Exit                    ; Exit if 64 valid nodes stored
inc esi                      ; Scanned node number +1
dec dword [rbp]              ; Nodes counter -1
jns .NumaScan                ; Stop cycle when make [rbp]=-1
; Exit points
lea rdi,[rbx+516]            ; Output RDI must be valid, even if no NUMA
.Exit:
mov rsp,rbp
pop rax rbp rsi rbx
ret
