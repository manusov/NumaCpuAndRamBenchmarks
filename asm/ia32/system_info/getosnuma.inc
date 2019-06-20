;--- Get NUMA information by Windows API --------------------------------;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save OS NUMA info               ;
;                                                                        ;
; OUTPUT:  EDI = Input EDI + 516 , buffer size fixed = 516 bytes         ;
;          Destination memory at [input EDI] = Results, 0=Not detected	 ;
;          Dword [000-003] = Number of NUMA Domains, 0=Not detected      ;
;          Qword [004-011] = Domain (Node) #0 Affinity Mask              ;
;          ...                                                           ;
;          Qword [012-515] = Domain (Node) #63 Affinity Mask             ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

GetOsNumaInfo:
cld                     ; Clear direction, because STOS used
push ebx esi ebp 0      ; Note push 0 for create local variable
mov ebp,esp             ; EBP = Pointer to local variable

; Blank results
mov ebx,edi             ; RBX = Backup output region address
mov ecx,516
mov al,0
rep stosb

; Get maximum node number
mov eax,[_GetNumaHighestNodeNumber]
test eax,eax
jz .Exit                ; Go skip if function not declared by OS
push ebp                ; Parm #1 = Pointer to variable
call eax                ; Return EAX=0 if error, EAX>0 if no errors
test eax,eax
jz .Exit                ; Go skip if error returned
cmp dword [ebp],65535   ; [EBP] = Number of nodes
ja .Exit                ; Go skip if invalid number of nodes

; Scan nodes, store affinity masks
xor esi,esi             ; ESI = Start node number
lea edi,[ebx + 4]       ; EDI = Pointer to nodes list, qwords sequence

.NumaScan:              ; EBX = Pointer to nodes counter, dword
mov eax,[_GetNumaNodeProcessorMask]
test eax,eax
jz .Exit                ; Go skip if function not declared by OS , Note Buggy output EDI 
push edi                ; Parm #2 = Pointer to variable = Processor Mask
push esi                ; Parm #1 = Node number
call eax                ; Write qword to [edi]
test  eax,eax
jnz @f                  ; Go skip blank if this node valid
mov [edi + 0],eax       ; At this point EAX = 0
mov [edi + 4],eax       ; At this point EAX = 0
@@:
jz @f                   ; Go skip pointer advance if this node not valid
inc dword [ebx]         ; Memory at [EBX] = Valid nodes counter +1
add edi,8               ; Nodes list pointer +8
@@:
mov eax,edi
sub eax,ebx
cmp eax,516
jae .Exit               ; Exit if 64 valid nodes stored
inc esi                 ; Scanned node number +1
dec dword [ebp]         ; Nodes counter -1
jns .NumaScan           ; Stop cycle when make [ebp] = -1

; Exit points
lea edi,[ebx + 260]     ; Output EDI must be valid, even if no NUMA, note +516 for x64
.Exit:
pop eax ebp esi ebx
ret
