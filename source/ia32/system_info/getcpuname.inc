;--- Get CPU name string, execute CPUID functions 80000002h-80000004h. --;
; Call this subroutine only if CPUID supported.                          ;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save CPU Vendor String          ;
;                                                                        ;
; OUTPUT:  EDI = Input EDI + 48 + 4 , string size fixed = 48 bytes       ;
;          Destination memory at [input EDI] =                           ; 
;           bytes [00-47] = CPU Vendor String                            ;
;           byte  [48-48] = 00h, terminator for copy by StringWrite      ;
;           bytes [49-51] = Reserved                                     ;           
;          String formatted by subroutine, left spaces deleted           ;
;          If feature not supported, string contain "None",              ;
;          but errors not reported, NCRB can work without this feature   ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

GetCpuName:
cld                        ; Clear direction, because STOSB/STOSD used
push ebx esi
; Blank and prepare for not support mode
mov ecx,48                 ; RCX = String length
mov al,' '                 ; AL = Space
rep stosb                  ; Blank string
mov dword [edi-48],'None'  ; Default string, if feature not supported
mov dword [edi],0          ; Zero terminator byte and 3 reserved bytes 
; Check functions supported ---
mov esi,80000000h          ; ESI = First extended function = 80000000h
mov eax,esi                ; EAX = Function for verify support = 80000000h
cpuid                      ; Execute CPUID function 80000000h
lea ebx,[esi+4]            ; EBX = Maximum function 80000004h
cmp eax,ebx
jb .Exit                   ; Go exit if required functions not supported
sub edi,48                 ; Must be +48 if go exit when not supported
; Get and store CPU name string
push edi
.StoreCpuName:
lea eax,[esi+2]            ; EAX = Function number, start from 80000002h
cpuid
stosd
xchg eax,ebx
stosd
xchg eax,ecx
stosd
xchg eax,edx
stosd
inc esi
cmp si,4-2                 ; This give function number EAX=80000004h
jbe .StoreCpuName
pop edi
; Formatting CPU name string, delete left spaces and byte 00h
mov esi,edi
mov ecx,48                ; ECX = String size limit
mov ebx,ecx               ; EBX = Used space count = 48 - Unused space count
.ScanCpuName:             ; This cycle for delete left spaces
lodsb                      
dec ebx                   ; Count used space
cmp al,0
je .EndCpuName            ; Go blank if end of string without name, here RCX=48
cmp al,' '
loope .ScanCpuName        ; Cycle for skip left spaces
mov cl,48
je .EndCpuName            ; Go blank if only spaces in the string, here RCX=48
inc ebx
dec esi
; Copy used portion of name
.CopyCpuName:             ; This cycle for copy name before terminator = 00h
lodsb
cmp al,0
je .EndCpuName
stosb
dec ecx
dec ebx
jnz .CopyCpuName
; Blank unused portion of name
.EndCpuName:              ; Start blank tail with space = 20h
mov al,' '
rep stosb
; Exit point
.Exit:
add edi,4                 ; Skip terminator byte and 3 reserved bytes, all 0
pop esi ebx
ret
