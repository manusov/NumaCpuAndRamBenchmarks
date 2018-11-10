;------------------------------------------------------------------------;
; Get CPU name string, execute CPUID functions 80000002h-80000004h.      ;
; Call this subroutine only if CPUID supported.                          ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save CPU Vendor String          ;
;                                                                        ;
; OUTPUT:  RDI = Input RDI + 48 , string size fixed = 48 bytes           ;
;          Destination memory at [input RDI] = CPU Vendor String         ;           
;          String formatted by subroutine, left spaces deleted           ;
;          If feature not supported, string contain "None"               ; 
;------------------------------------------------------------------------;

GetCpuName:
cld                       ; Clear direction, because STOSB/STOSD used
push rbx rcx rdx rax rsi
; Blank and prepare for not support mode
mov ecx,48                ; RCX = String length, also clear bits RCX[63-32]
mov al,' '                ; AL = Space
rep stosb                 ; Blank string
mov dword [rdi-48],'None'
; Check functions supported ---
mov esi,80000000h         ; ESI = First extended function = 80000000h
mov eax,esi               ; EAX = Function for verify support = 80000000h
cpuid                     ; Execute CPUID function 80000000h
lea ebx,[esi+4]           ; EBX = Maximum function 80000004h
cmp eax,ebx
jb ExitCpuId2             ; Go exit if required functions not supported
sub rdi,48                ; Must be +48 if go exit when not supported
; Get and store CPU name string
push rdi
CycleCpuName:
lea eax,[esi+2]           ; EAX = Function number, start from 80000002h
cpuid
stosd
xchg eax,ebx
stosd
xchg eax,ecx
stosd
xchg eax,edx
stosd
inc esi
cmp si,4-2                ; This give function number EAX=80000004h
jbe CycleCpuName
pop rdi
; Formatting CPU name string, delete left spaces and byte 00h
mov rsi,rdi
mov ecx,48                ; RCX = String size limit
mov ebx,ecx               ; EBX = Used space count = 48 - Unused space count
ScanCpuName:              ; This cycle for delete left spaces
lodsb
dec ebx                   ; Count used space
cmp al,0
je EndCpuName             ; Go blank if end of string without name, here RCX=48
cmp al,' '
loope ScanCpuName         ; Cycle for skip left spaces
mov cl,48
je EndCpuName             ; Go blank if only spaces in the string, here RCX=48
inc ebx
dec esi
; Copy used portion of name
CopyCpuName:              ; This cycle for copy name before terminator = 00h
lodsb
cmp al,0
je EndCpuName
stosb
dec ecx
dec ebx
jnz CopyCpuName
; Blank unused portion of name
EndCpuName:               ; Start blank tail with space = 20h
mov al,' '
rep stosb
; Exit points
ExitCpuId2:
pop rsi rax
jmp ExitCpuId1

