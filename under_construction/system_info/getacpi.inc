;--- Read and analysing ACPI MADT table ---------------------------------;
; MADT = Multiple APIC Description Table (multiprocessing info)          ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save ACPI MADT info             ;
;	         RSI = Pointer to 48KB transit buffer                          ;
;                      for load ACPI table                               ;
;                                                                        ;
; OUTPUT:  RDI = Input RDI + 28 , buffer size fixed = 28 bytes           ;
;          Destination memory at [input RDI] = Results, 0=Not detected	 ;
;          6 bytes [00-05] = OEM ID (ACPI MADT relative)                 ;
;          8 bytes [06-13] = Manufacturer Model ID (ACPI MADT relative)  ;
;          2 bytes [14-15] = Reserved for alignment                      ;
;          Dword   [16-19] = OEM revision (ACPI MADT relative)           ;
;          Dword   [20-23] = Number of Local APICs                       ;
;          Dword   [24-27] = Number of I/O APICs                         ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft x64 calling convention.   ; 
;------------------------------------------------------------------------;

GetAcpiMadt:
push rbx rsi rbp rax             ; Note push rax for make local variable
mov rbp,rsp                      ; RBP used for restore RSP and addr. variables
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                       ; Make parameters shadow
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
mov al,[rsi]
cmp al,0                         ; 0 = Local APIC
jne @f                           ; Go skip if not a Local APIC
test byte [rsi+4],00000001b
jz @f                            ; Go skip if Local APIC disabled
inc ecx                          ; counter+1 if local APIC found
@@:
cmp al,1			 ; 1 = I/O APIC
jne @f				 ; Go skip if not a I/O APIC
inc edx 			 ; +1 if found
@@:
movzx rax,byte [rsi+1]   ; RAX = Size of structure, for skip it and access next
add rsi,rax
cmp rsi,rbx
jb .MadtScanApics        ; Cycle for scan structures
; Store numbers of APICs
EntryStoreAcpi:
xchg eax,ecx
stosd                   ; Store number of Local APICs found , or CPUs for SRAT
xchg eax,edx			    
stosd                   ; Store number of I/O APICs found , or RAMs for SRAT
; Exit, RBP mus be valid as input value
AcpiNotAvailable:
AcpiDone:
mov rsp,rbp
pop rax rbp rsi rbx
ret

;------------------------------------------------------------------------;
; Read and analysing ACPI SRAT table                                     ;
; SRAT = System (Static) Resource Affinity Table (NUMA info)             ;
; NUMA = Non Uniform Memory Access                                       ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save ACPI SRAT info             ;
;	         RSI = Pointer to 48KB transit buffer for load ACPI table      ;
;                                                                        ;
; OUTPUT:  RDI = Input RDI + 32 , buffer size fixed = 32 bytes           ;
;          Destination memory at [input RDI] = Results, 0=Not detected	 ;
;          6 bytes [00-05] = OEM ID (ACPI SRAT relative)                 ;
;          8 bytes [06-13] = Manufacturer Model ID (ACPI SRAT relative)  ;
;          2 bytes [14-15] = Reserved for alignment                      ;
;          Dword   [16-19] = OEM revision (ACPI SRAT relative)           ;
;          Dword   [20-23] = Number of NUMA Domains                      ;
;          Dword   [24-27] = Number of CPUs (as Local APICs)             ;
;          Dword   [28-31] = Number of RAMs (as Regions)                 ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft x64 calling convention.   ; 
;------------------------------------------------------------------------;
GetAcpiSrat:
push rbx rsi rbp rax             ; Note push rax for make local variable
mov rbp,rsp                      ; RBP used for restore RSP and addr. variables
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                       ; Make parameters shadow
mov edx,'SRAT'                   ; RDX = ACPI table signature
mov ecx,32                       ; RCX = Output buffer size, for SRAT mode
; Parse ACPI table header
call HelperAcpiHeader
jc AcpiNotAvailable              ; Go if error or this ACPI table not found 
; Pre-blank registers used as counters
xor ecx,ecx                      ; ECX = Counter of CPUs (SRAT)
xor edx,edx                      ; EDX = Counter of RAMs (SRAT)
; Interpreting ACPI SRAT table, get number of Domains, CPUs, RAMs
; R8, R9 = Domains bitmap and count,
; RCX=CPUs, RDX=RAMs, RSI=Start, RBX=Limit, RDI=Destination
; RCX,RDX already 0
BranchSrat:
add rsi,4                        ; RSI = Adjust unformatted area start address
xor r8,r8                        ; R8 = Domains bitmap (SRAT)
xor r9,r9                        ; R9 = Domains counter (SRAT)
; Cycle for scan nodes
.SratScanNodes:
mov al,[rsi]
cmp al,0                         ; 0 = Processor affinity structure
jne @f                           ; Go skip if not a CPU
test byte [rsi+04],00000001b     ; Check Flags.Enable
jz @f                            ; Go skip if entry disabled
inc ecx                          ; CPUs +1
mov r10d,[rsi+09]
and r10d,00FFFFFFh
shl r10d,24                      ; This component, bits[31-08] only for CPUs
@@:
cmp al,1                         ; 1 = Memory affinity structure
jne @f
test byte [rsi+28],00000001b     ; Check Flags.Enable
jz @f                            ; Go skip if entry disabled
inc edx                          ; RAMs +1
xor r10d,r10d                    ; This component, bits[31-08]=0 for RAMs
@@:
mov r10l,[rsi+02]                ; R10D = NUMA Proximity Domain number
bts r8,r10
cmp r10d,63
jbe @f
mov r9d,-1               ; R9D = Mark output as invalid if Domain Number > 63
@@:
movzx eax,byte [rsi+1]   ; RAX = Size of structure, for skip it and access next
add rsi,rax
cmp rsi,rbx
jb .SratScanNodes         ; Cycle for scan structures
; End of cycle for scan nodes
xor eax,eax              ; Number of NUMA nodes
mov bl,64
.ScanOnes:               ; This cycle for count "1" bits = Number of NUMA nodes
shr r8,1                 ; CF=1(C) if current bit =1
jnc @f
inc eax                  ; Counter +1 if "1" detected
@@:
dec bl
jnz .ScanOnes
or eax,r9d               ; This is checks for invalid number
stosd                    ; Store number of NUMA Domains
; Continue in the common branch
jmp EntryStoreAcpi


;------------------------------------------------------------------------;
; Read and analysing ACPI SLIT table                                     ;
; SLIT = System Locality Distance Information Table (NUMA info)          ;
; NUMA = Non Uniform Memory Access                                       ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save ACPI SLIT info             ;
;	         RSI = Pointer to 48KB transit buffer for load ACPI table      ;
;                                                                        ;
; OUTPUT:  RDI = Input RDI + 20 , buffer size fixed = 20 bytes           ;
;          Destination memory at [input RDI] = Results, 0=Not detected	 ;
;          6 bytes [00-05] = OEM ID (ACPI SLIT relative)                 ;
;          8 bytes [06-13] = Manufacturer Model ID (ACPI SLIT relative)  ;
;          2 bytes [14-15] = Reserved for alignment                      ;
;          Dword   [16-19] = OEM revision (ACPI SLIT relative)           ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft x64 calling convention.   ; 
;------------------------------------------------------------------------;
GetAcpiSlit:
push rbx rsi rbp rax             ; Note push rax for make local variable
mov rbp,rsp                      ; RBP used for restore RSP and addr. variables
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                       ; Make parameters shadow
mov edx,'SLIT'                   ; RDX = ACPI table signature
mov ecx,20                       ; RCX = Output buffer size, for SRAT mode
; Parse ACPI table header
call HelperAcpiHeader
jmp AcpiDone                     ; Go done, yet without table-specific decode 


;------------------------------------------------------------------------;
; Helper for parse ACPI table header                                     ;
; INPUT:   RSI = Pointer to transit buffer for load ACPI table           ;
;          RDI = Pointer to destination buffer, for save parse results   ;  
;          ECX = Output data block size, bytes, exclude 2 bytes reserved ;
;          EDX = Required ACPI table signature                           ;
;                                                                        ;
; OUTPUT:  CF = Error flag, CF=0 means no errors, CF=1 means error,      ;
;          for example, required ACPI table not found                    ;
;          RSI = Updated for skip ACPI table header,                     ;
;          RBX = Table end address, used as scan limit                   ;
;          output RSI, RBX valid only if no errors, when output CF=0     ;
;          RDI = Updated destination pointer,                            ;
;          skip header only if OK, for additional writes table-specific  ;
;          skip all space if ERROR, for continue fill system parameters  ;
;------------------------------------------------------------------------;

HelperAcpiHeader:
cld
push rbp rax                     ; Note push rax for variable space: 2 dwords
mov rbp,rsp                      ; RBP used for restore RSP and addr. variables
and rsp,0FFFFFFFFFFFFFFF0h       ; Align stack
sub rsp,32                       ; Make parameters shadow
cld                              ; Clear direction, because STOSB used
; Blank results
; RCX loaded at entry
mov [rbp+4],ecx                  ; Store size for adjust if not available
mov al,0
rep stosb                        ; Pre-blank output and advance RDI
; Save signature at temporary storage, for branch MADT/SRAT/SLIT
mov [rbp],edx
; Get ACPI table
; Parm#1 = RCX = Table provider signature, ACPI is one of variants  
; Parm#2 = RDX = Firmware table ID, signature
; Parm#3 = R8  = Pointer to a Buffer
; Parm#4 = R9  = Available size of Buffer, bytes
; Output = RAX = Used Buffer size, this < Available size, 
;		       insufficient memory error. 0 means other type error 
add rsi,0Fh
and rsi,0FFFFFFFFFFFFFFF0h
mov ebx,TEMP_BUFFER_SIZE-16  ; Note bits RBX[63-32] cleared by 32-bit operation
mov ecx,'IPCA'               ; Note bits RCX[63-32] cleared by 32-bit operation
; RDX loaded at entry
mov r8,rsi
mov r9,rbx
mov rax,[_GetSystemFirmwareTable]
test rax,rax
jz .Error              ; Go skip if function not declared by OS
call rax 
test rax,rax
jz .Error              ; Go skip operation if error returned
cmp rax,rbx
jae .Error             ; Go skip operation if insufficient buffer size
mov ecx,[rbp+4]        ; RCX = Stored size
sub rdi,rcx            ; This sub. must not execute if exit by ACPI table n/a
; Interpreting ACPI table, get text strings
xchg rbx,rax           ; RBX = Save MADT size
mov rcx,14             ; RCX = Region size
mov rdx,rsi            ; Save RSI
add rsi,10
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
lea rsi,[rdx+44]
; RBX = Table end address, used as scan limit
add rbx,rdx
; Return points
clc                   ; CF=0 means no errors, header parsed OK, RSI valid
.Exit:
mov rsp,rbp
pop rax rbp
ret
.Error:
stc
jmp .Exit
