;--- Detect CPUID support and execute CPUID function #0. ----------------;
; Note CPUID can be supported by CPU but locked by Virtual Monitor.      ;
; Note check bit EFLAGS.21 toggleable, it is CPUID support indicator.    ;
; Note probably wrong result if debug trace this subroutine code.        ;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save CPU Vendor String          ;
;                                                                        ;
; OUTPUT:  CF flag = Status: 0(NC)=Support OK, 1(C)=Not supported        ;
;          Output EAX, EDI, Destination memory valid only if CF=0(NC)    ;
;          EAX = Largest standard CPUID function supported               ;
;          EDI = Input EDI + 12 + 4 , string size fixed = 12 bytes       ;
;          Destination memory at [input RDI] =                           ;
;           bytes [00-11] = CPU Vendor String                            ;
;           byte  [12-12] = 00h, terminator for copy by StringWrite      ;
;           bytes [13-15] = Reserved                                     ;           
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

CheckCpuId:
cld                       ; Clear direction, because STOSD used
push ebx
; Check for ID bit writeable for "1"
mov ebx,21
pushf                     ; In the 32-bit mode, push EFLAGS
pop eax
bts eax,ebx               ; Set EAX.21=1
push eax
popf                      ; Load EFLAGS with EFLAGS.21=1
pushf                     ; Store EFLAGS
pop eax                   ; Load EFLAGS to RAX
btr eax,ebx               ; Check EAX.21=1, Set EAX.21=0
jnc .NoCpuId              ; Go error branch if cannot set EFLAGS.21=1
; Check for ID bit writeable for "0"
push eax
popf                      ; Load EFLAGS with EFLAGS.21=0
pushf                     ; Store EFLAGS
pop eax                   ; Load EFLAGS to RAX
btr eax,ebx               ; Check EAX.21=0
jc .NoCpuId               ; Go if cannot set EFLAGS.21=0
; Execute CPUID function 0, store results
xor eax,eax               ; EAX = Function number for CPUID instruction
cpuid                     ; Execute CPUID function 0
xchg eax,ebx              ; XCHG instead MOV, short code, EBX valid for next
stosd                     ; Store Vendor String [00-03]
xchg eax,edx	  
stosd                     ; Store Vendor String [04-07]
xchg eax,ecx
stosd                     ; Store Vendor String [08-11]
xor eax,eax               ; EAX = 0
stosd                     ; Zero dword, terminator byte and 3 reserved bytes
xchg eax,ebx              ; Restore EAX = Largest standard function supported
; Exit points
.ExitCpuId:
pop ebx
ret                       ; Return, at this point CF=0(NC) after XOR EAX,EAX
.NoCpuId:
stc                       ; CF=1(C) means error
jmp .ExitCpuId 
