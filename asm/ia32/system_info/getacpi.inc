;--- Read and analysing ACPI MADT table ---------------------------------;
; MADT = Multiple APIC Description Table (multiprocessing info)          ;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save ACPI MADT info             ;
;	         ESI = Pointer to 48KB transit buffer                          ;
;                      for load ACPI table                               ;
;                                                                        ;
; OUTPUT:  EDI = Input EDI + 28 , buffer size fixed = 28 bytes           ;
;          Destination memory at [input EDI] = Results, 0=Not detected	 ;
;          6 bytes [00-05] = OEM ID (ACPI MADT relative)                 ;
;          8 bytes [06-13] = Manufacturer Model ID (ACPI MADT relative)  ;
;          2 bytes [14-15] = Reserved for alignment                      ;
;          Dword   [16-19] = OEM revision (ACPI MADT relative)           ;
;          Dword   [20-23] = Number of Local APICs                       ;
;          Dword   [24-27] = Number of I/O APICs                         ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

GetAcpiMadt:
push ebx esi ebp
; Set ACPI table parameters
mov edx,'APIC'                   ; RDX = ACPI table signature
mov ecx,28                       ; RCX = Output buffer size, for MADT mode
; Parse ACPI table header
call HelperAcpiHeader
jc AcpiNotAvailable              ; Go if error or this ACPI table not found 
; Interpreting ACPI MADT table, get number of APICs,
; pre-blank registers used as counters
xor ecx,ecx                      ; ECX = Counter of Local APICs (MADT)
xor edx,edx                      ; EDX = Counter of I/O APICs (MADT)
; This code for parse ACPI MADT, signature = "APIC"
.MadtScanApics:
mov al,[esi]
cmp al,0                         ; 0 = Local APIC
jne @f                           ; Go skip if not a Local APIC
test byte [esi + 4],00000001b
jz @f                            ; Go skip if Local APIC disabled
inc ecx                          ; counter+1 if local APIC found
@@:
cmp al,1			 ; 1 = I/O APIC
jne @f				 ; Go skip if not a I/O APIC
inc edx 			 ; +1 if found
@@:
movzx eax,byte [esi + 1]  ; EAX = Size of structure, for skip it and access next
add esi,eax
cmp esi,ebx
jb .MadtScanApics       ; Cycle for scan structures
; Store numbers of APICs
EntryStoreAcpi:
xchg eax,ecx
stosd                   ; Store number of Local APICs found , or CPUs for SRAT
xchg eax,edx			    
stosd                   ; Store number of I/O APICs found , or RAMs for SRAT
; Restore registers and exit 
AcpiNotAvailable:
AcpiDone:
pop ebp esi ebx
ret

;------------------------------------------------------------------------;
; Read and analysing ACPI SRAT table                                     ;
; SRAT = System (Static) Resource Affinity Table (NUMA info)             ;
; NUMA = Non Uniform Memory Access                                       ;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save ACPI SRAT info             ;
;	         ESI = Pointer to 48KB transit buffer for load ACPI table      ;
;                                                                        ;
; OUTPUT:  EDI = Input EDI + 32 , buffer size fixed = 32 bytes           ;
;          Destination memory at [input EDI] = Results, 0=Not detected	 ;
;          6 bytes [00-05] = OEM ID (ACPI SRAT relative)                 ;
;          8 bytes [06-13] = Manufacturer Model ID (ACPI SRAT relative)  ;
;          2 bytes [14-15] = Reserved for alignment                      ;
;          Dword   [16-19] = OEM revision (ACPI SRAT relative)           ;
;          Dword   [20-23] = Number of NUMA Domains                      ;
;          Dword   [24-27] = Number of CPUs (as Local APICs)             ;
;          Dword   [28-31] = Number of RAMs (as Regions)                 ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

GetAcpiSrat:
push ebx esi ebp

; Set ACPI table parameters
mov edx,'SRAT'                ; EDX = ACPI table signature
mov ecx,32                    ; ECX = Output buffer size, for SRAT mode

; Parse ACPI table header
call HelperAcpiHeader
jc AcpiNotAvailable           ; Go if error or this ACPI table not found 

; Push local variables, qword [esp] = port R8, dword [esp+8] = port R9D
; port R8 = domains bitmap, port R9D = wrong domain number flag
push 0 0 0

; Pre-blank registers used as counters
xor ecx,ecx                   ; ECX = Counter of CPUs (SRAT)
xor edx,edx                   ; EDX = Counter of RAMs (SRAT)

; Interpreting ACPI SRAT table, get number of Domains, CPUs, RAMs
; ECX = CPUs, EDX = RAMs, ESI = Start, EBX = Limit, EDI = Destination
; ECX, EDX already 0
BranchSrat:
add esi,4                     ; ESI = Adjust unformatted area start address

; Cycle for scan nodes
.SratScanNodes:
mov al,[esi]

; Detect processor affinity structure
cmp al,0                         ; 0 = Processor affinity structure
jne @f                           ; Go skip if not a CPU
test byte [esi + 04],00000001b   ; Check Flags.Enable
jz @f                            ; Go skip if entry disabled
inc ecx                          ; CPUs +1
mov ebp,[esi + 09]
and ebp,00FFFFFFh
shl ebp,24                       ; This component, bits[31-08] only for CPUs
@@:

; Detect memory affinity structure 
cmp al,1                         ; 1 = Memory affinity structure
jne @f
test byte [esi + 28],00000001b   ; Check Flags.Enable
jz @f                            ; Go skip if entry disabled
inc edx                          ; RAMs +1
xor ebp,ebp                      ; This component, bits[31-08]=0 for RAMs
@@:

mov al,[esi + 02]                ; AL = NUMA Proximity Domain number
and eax,00011111b                ; Select bits for 32-bit half-mask

push ecx
xor ecx,ecx
bts ecx,eax
test byte [esi + 02],00100000b 
jnz .LHigh
or [esp + 00 + 04 + 00],ecx
jmp .LDone
.LHigh:
or [esp + 00 + 04 + 04],ecx
.LDone:
pop ecx

cmp byte [esi + 02],63
jbe @f
mov dword [esp + 08], -1         ; Mark output as invalid if Domain Number > 63
@@:

movzx eax,byte [esi + 01]  ; EAX = Size of structure, for skip it and access next
add esi,eax
cmp esi,ebx
jb .SratScanNodes          ; Cycle for scan structures

; End of cycle for scan nodes
xor eax,eax              ; Number of NUMA nodes
mov bl,64
mov esi,[esp + 00]       ; EBP:ESI = domains bitmap
mov ebp,[esp + 04]
.ScanOnes:               ; This cycle for count "1" bits = Number of NUMA nodes
shrd esi,ebp,1           ; CF=1(C) if current bit =1
jnc @f
inc eax                  ; Counter +1 if "1" detected
@@:
shr ebp,1
dec bl
jnz .ScanOnes

or eax,[esp + 08]        ; This is checks for invalid number
stosd                    ; Store number of NUMA Domains

; Continue in the common branch
add esp,12
jmp EntryStoreAcpi


;------------------------------------------------------------------------;
; Read and analysing ACPI SLIT table                                     ;
; SLIT = System Locality Distance Information Table (NUMA info)          ;
; NUMA = Non Uniform Memory Access                                       ;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save ACPI SLIT info             ;
;	         ESI = Pointer to 48KB transit buffer for load ACPI table      ;
;                                                                        ;
; OUTPUT:  EDI = Input EDI + 20 , buffer size fixed = 20 bytes           ;
;          Destination memory at [input EDI] = Results, 0=Not detected	 ;
;          6 bytes [00-05] = OEM ID (ACPI SLIT relative)                 ;
;          8 bytes [06-13] = Manufacturer Model ID (ACPI SLIT relative)  ;
;          2 bytes [14-15] = Reserved for alignment                      ;
;          Dword   [16-19] = OEM revision (ACPI SLIT relative)           ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;
GetAcpiSlit:
push ebx esi ebp
; Set ACPI table parameters
mov edx,'SLIT'                   ; RDX = ACPI table signature
mov ecx,20                       ; RCX = Output buffer size, for SRAT mode
; Parse ACPI table header
call HelperAcpiHeader
jmp AcpiDone                     ; Go done, yet without table-specific decode 


;------------------------------------------------------------------------;
; Helper for parse ACPI table header                                     ;
; INPUT:   ESI = Pointer to transit buffer for load ACPI table           ;
;          EDI = Pointer to destination buffer, for save parse results   ;  
;          ECX = Output data block size, bytes, exclude 2 bytes reserved ;
;          EDX = Required ACPI table signature                           ;
;                                                                        ;
; OUTPUT:  CF = Error flag, CF=0 means no errors, CF=1 means error,      ;
;          for example, required ACPI table not found                    ;
;          ESI = Updated for skip ACPI table header,                     ;
;          EBX = Table end address, used as scan limit                   ;
;          output ESI, EBX valid only if no errors, when output CF=0     ;
;          EDI = Updated destination pointer,                            ;
;          skip header only if OK, for additional writes table-specific  ;
;          skip all space if ERROR, for continue fill system parameters  ;
;------------------------------------------------------------------------;

HelperAcpiHeader:
cld
push ebp eax eax             ; Note push eax eax for variable space: 2 dwords
cld                          ; Clear direction, because STOSB used
; Blank results
; RCX loaded at entry
mov [esp + 04],ecx           ; Store size for adjust if not available
mov al,0
rep stosb                    ; Pre-blank output and advance EDI
; Save signature at temporary storage, for branch MADT/SRAT/SLIT
mov [esp + 0],edx
; Buffer alignment
add esi,0Fh
and esi,0FFFFFFF0h
mov ebx,TEMP_BUFFER_SIZE-16
mov eax,[_GetSystemFirmwareTable]
test eax,eax
jz .Error              ; Go skip if function not declared by OS
; Call WinAPI, get ACPI table
push ebx       ; Parm#4 = Available size of Buffer, bytes
push esi       ; Parm#3 = Pointer to a Buffer
push edx       ; Parm#2 = Firmware table ID, signature
push 'IPCA'    ; Parm#1 = Table provider signature, ACPI is one of variants
call eax 
; Output = EAX = Used Buffer size, this < Available size, 
;		       insufficient memory error. 0 means other type error 
test eax,eax
jz .Error              ; Go skip operation if error returned
cmp eax,ebx
jae .Error             ; Go skip operation if insufficient buffer size
mov ecx,[esp + 04]     ; ECX = Stored size
sub edi,ecx            ; This sub. must not execute if exit by ACPI table n/a
; Interpreting ACPI table, get text strings
xchg ebx,eax           ; EBX = Save MADT size
mov ecx,14             ; ECX = Region size
mov edx,esi            ; Save ESI
add esi,10
.AcpiScanText:
lodsb
cmp al,' '
jb .AsciiNotChar
cmp al,'}'
jna .AsciiChar
.AsciiNotChar:
mov al,'.'
.AsciiChar:
stosb
loop .AcpiScanText
; Store reserved word
xor eax,eax
stosw
; Copy revision dword
movsd
; RSI = MADT/SRAT/SLIT start address, skip header
lea esi,[edx + 44]
; RBX = Table end address, used as scan limit
add ebx,edx
; Return points
clc                 ; CF=0 means no errors, header parsed OK, ESI valid
.Exit:
pop eax eax ebp
ret
.Error:
stc
jmp .Exit
