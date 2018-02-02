;------------------------------------------------------------------------;
; Read and analysing ACPI MADT table                                     ;
; MADT = Multiple APIC Description Table (multiprocessing info)          ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save ACPI MADT info             ;
;	         RSI = Pointer to 48KB transit buffer for load ACPI table      ;
;                                                                        ;
; OUTPUT:  RDI = Input RDI + 26 , buffer size fixed = 26 bytes           ;
;          Destination memory at [input RDI] = Results, 0=Not detected	 ;
;          6 bytes [00-05] = OEM ID (ACPI MADT relative)                 ;
;          8 bytes [06-13] = Manufacturer Model ID (ACPI MADT relative)  ;
;          Dword   [14-17] = OEM revision (ACPI MADT relative)           ;
;          Dword   [18-21] = Number of Local APICs                       ;
;          Dword   [22-25] = Number of I/O APICs                         ;
;------------------------------------------------------------------------;
GetAcpiMadt:
push rbx rcx rdx                  ; Operations, different for MADT and SRAT
mov edx,'APIC'                    ; RDX = ACPI table signature
mov ecx,26                        ; RCX = Output buffer size
;--- This entry used for ACPI SRAT analysing ---
EntryGetAcpi:
cld                               ; Clear direction, because STOSB used
push rax rsi rbp r8 r9 r10 r11 rax   ; R8-R11 because API
mov rbp,rsp                      ; RBP used for restore RSP and addr. variables
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                       ; Make parameters shadow
;--- Blank results ---
; RCX loaded at entry
mov [rbp+4],ecx                  ; Store size for adjust if not available
mov al,0
rep stosb
;--- Save signature at temporary storage, for branch MADT/SRAT ---
mov [rbp],edx
;--- Get ACPI table ---
; Parm#1 = RCX = Table provider signature, ACPI is one of variants  
; Parm#2 = RDX = Firmware table ID, signature
; Parm#3 = R8  = Pointer to a Buffer
; Parm#4 = R9  = Available size of Buffer, bytes
; Output = RAX = Used Buffer size, this < Available size, 
;		       insufficient memory error. 0 means other type error 
;---
add rsi,0Fh
and rsi,0FFFFFFFFFFFFFFF0h
mov rbx,49152-16
mov rcx,'IPCA'
; RDX loaded at entry
mov r8,rsi
mov r9,rbx
mov rax,[_GetSystemFirmwareTable]
test rax,rax
jz AcpiNotAvailable   ; Go skip if function not declared by OS
call rax 
test rax,rax
jz AcpiNotAvailable   ; Go skip operation if error returned
cmp rax,rbx
jae AcpiNotAvailable  ; Go skip operation if insufficient buffer size
mov ecx,[rbp+4]       ; RCX = Stored size
sub rdi,rcx           ; This sub. must not execute if exit by ACPI table n/a
;--- Interpreting ACPI table, get text strings ---
xchg rbx,rax          ; RBX = Save MADT size
mov rcx,14            ; RCX = Region size
mov rdx,rsi           ; Save RSI
add rsi,10
AcpiScanText:
lodsb
cmp al,' '
jb AsciiNotChar
cmp al,'}'
jna AsciiChar
AsciiNotChar:
mov al,'.'
AsciiChar:
stosb
loop AcpiScanText
;--- Copy revision dword ---
movsd
;--- Interpreting ACPI MADT table, get number of APICs ---
lea rsi,[rdx+44]      ; RSI = MADT/SRAT start address
add rbx,rdx           ; RBX = MADT/SRAT end address
xor ecx,ecx           ; ECX = Counter of Local APICs (MADT) / CPUs (SRAT)
xor edx,edx           ; EDX = Counter of I/O APICs (MADT) / RAMs (SRAT)
;--- Select branches: MADT or SRAT ---
; Compare first byte only because only two tables (MADT, SRAT) supported yet
;---
cmp byte [rbp],'S'
je BranchSrat
;--- This branch for MADT, signature = "APIC" ---
MadtScanApics:
mov al,[rsi]
cmp al,0                         ; 0 = Local APIC
jne @f                           ; Go skip if not a Local APIC
test byte [rsi+4],00000001b
jz @f                            ; Go skip if Local APIC disabled
inc ecx                          ; +1 if found
@@:
cmp al,1			 ; 1 = I/O APIC
jne @f				 ; Go skip if not a I/O APIC
inc edx 			 ; +1 if found
@@:
movzx rax,byte [rsi+1]  ; RAX = Size of structure, for skip it and access next
add rsi,rax
cmp rsi,rbx
jb MadtScanApics        ; Cycle for scan structures
;--- Store numbers of APICs ---
EntryStoreAcpi:
xchg eax,ecx
stosd                   ; Store number of Local APICs found , or CPUs for SRAT
xchg eax,edx			    
stosd                   ; Store number of I/O APICs found , or RAMs for SRAT
;--- Exit, RBP mus be valid as input value ---
AcpiNotAvailable:
jmp ExitCpuClk

;------------------------------------------------------------------------;
; Read and analysing ACPI SRAT table                                     ;
; SRAT = System (Static) Resource Affinity Table (NUMA info)             ;
; NUMA = Non Uniform Memory Access                                       ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save ACPI SRAT info             ;
;	         RSI = Pointer to 48KB transit buffer for load ACPI table      ;
;                                                                        ;
; OUTPUT:  RDI = Input RDI + 30 , buffer size fixed = 30 bytes           ;
;          Destination memory at [input RDI] = Results, 0=Not detected	 ;
;          6 bytes [00-05] = OEM ID (ACPI MADT relative)                 ;
;          8 bytes [06-13] = Manufacturer Model ID (ACPI MADT relative)  ;
;          Dword   [14-17] = OEM revision (ACPI MADT relative)           ;
;          Dword   [18-21] = Number of NUMA Domains                      ;
;          Dword   [22-25] = Number of CPUs (as Local APICs)             ;
;          Dword   [26-29] = Number of RAMs (as Regions)                 ;
;------------------------------------------------------------------------;
GetAcpiSrat:
push rbx rcx rdx                 ; Operations, different for MADT and SRAT
mov edx,'SRAT'                   ; RDX = ACPI table signature
mov ecx,30                       ; RCX = Output buffer size
jmp EntryGetAcpi

;--- Interpreting ACPI SRAT table, get number of Domains, CPUs, RAMs ---
; R8, R9 = Domains bitmap and count,
; RCX=CPUs, RDX=RAMs, RSI=Start, RBX=Limit, RDI=Destination
; RCX,RDX already 0
;---
BranchSrat:
add rsi,4                        ; RSI = Adjust unformatted area start address
xor r8,r8                        ; R8 = Domains bitmap
xor r9,r9                        ; R9 = Domains counter
SratScanNodes:
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
jb SratScanNodes         ; Cycle for scan structures
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
;--- Continue in the common branch ---
jmp EntryStoreAcpi

