;--- Extract data for visualization (text strings) from detected system info --;
;                                                                              ;
; INPUT:   None at registers                                                   ;
;          System Parameters Block = results of system information             ;
;                                                                              ;
; OUTPUT:  None at registers                                                   ;
;          Temporary multifunctional buffer, data for visualization            ;  
;                                                                              ;
; All CPU registers can be corrupted                                           ;
;------------------------------------------------------------------------------;

; TODO.
; Required input-output parameters detalization.

SysParmsToBuffer:

; Initializing pointers
lea r8,[SystemParameters]
lea rsi,[TextTfms]
lea rbp,[TEMP_BUFFER + 000]
mov rdi,rbp

; Build Window 0 user interface elements = F (System Information)
; SysInfo Line 1, Processor
call StringWrite
mov eax,[r8 + SYSPARMS.CpuSignature]
call HexPrint32
mov ax,'h'
stosw

; SysInfo Line 1 continue, CPU Frequency
lea rdi,[rbp + 080]
call StringWrite
mov rax,[r8 + SYSPARMS.TscMHz]
mov bx,0100h
call DoublePrint
call StringWrite
mov al,0
stosb

; SysInfo Line 2, Cache
lea rdi,[rbp + 160]
mov ecx,5                  ; RCX = Number of types/levels
lea r9,[r8 + SYSPARMS.CpuCache.CpuidTraceCache]
call StringWrite
mov ax,'  '                ; First interval
stosw
.CacheLevels:              ; Cycle for all cache types/levels
mov r10,[r9]               ; Get R10 = Cache size, bytes
test r10,r10
jnz .MakeLevel             ; Go write if this type/level exist
@@:
lodsb                      ; Skip this type/level name string if not exist 
cmp al,0
jne @b
jmp .NextLevel
.MakeLevel:
mov ax,'  '
stosw
stosw
stosw
call StringWrite           ; Print Cache Level name
xchg rax,r10
xor edx,edx
mov ebx,1024               ; Note this operation also clear bits RBX[63-32]
div rbx
mov byte [rdi],'?'
shld rdx,rax,32
inc rdi
test edx,edx
jnz .NextLevel
dec rdi
mov bl,0
call DecimalPrint32
cmp ecx,5
jne .NextLevel             ; Skip write "KuOps" if no trace cache
push rsi
lea rsi,[TextKuOps]       
call StringWrite           ; Print Cache Level name
pop rsi
.NextLevel:
add r9,12
loop .CacheLevels          ; Cycle for all cache types/levels
mov al,0
stosb                      ; String terminator 00h

; SysInfo Line 3, ACPI MADT, Local APICs, I/O APICs
lea rbx,[r8 + SYSPARMS.MadtText]    ; RBX = Pointer to System Information
lea rsi,[TextMadt]                  ; RSI = Pointer to Name
lea rdi,[TEMP_BUFFER + 320]         ; RDI = Build position 3
cmp dword [rbx],0 ; old rbx+18/20
jne .PrintMadt             ; Go print if Local APICs count > 0
lea rsi,[TextNoMadt]
call StringWrite
jmp .EndMadt
.PrintMadt:
call AcpiHeaderPrint
lea rbx,[TextLocalApics]
call AcpiNumberPrint       ; Print Number of Local APICs
mov ecx,5
mov al,' '
rep stosb
lea rbx,[TextIoApics]
call AcpiNumberPrint       ; Print Number of I/O APICs
.EndMadt:
mov al,0
stosb                      ; String terminator 00h

; SysInfo Line 4, ACPI SRAT, NUMA domains, CPUs, RAMs
lea rbx,[r8 + SYSPARMS.SratText]    ; RBX = Pointer to System Information
lea rsi,[TextSrat]                  ; RSI = Pointer to Name
lea rdi,[TEMP_BUFFER + 480]         ; RDI = Build position 4
cmp dword [rbx],0 ; old rbx+18/20
jne .PrintSrat
lea rsi,[TextNoSrat]
call StringWrite
jmp .EndSrat
.PrintSrat:
call AcpiHeaderPrint
lea rbx,[TextDomains]
call AcpiNumberPrint       ; Print Number of NUMA Domains
mov ecx,5
mov al,' '
rep stosb
lea rbx,[TextCPUs]
call AcpiNumberPrint       ; Print Number of CPUs, NUMA-aware
mov ecx,5
mov al,' '
rep stosb
lea rbx,[TextRAMs]
call AcpiNumberPrint       ; Print Number of RAMs, NUMA-aware
.EndSrat:
mov al,0
stosb                      ; String terminator 00h

; SysInfo Line 5, ACPI SLIT, yet without table-specific extended info
lea rbx,[r8 + SYSPARMS.SlitText]    ; RBX = Pointer to System Information
lea rsi,[TextSlit]                  ; RSI = Pointer to Name
lea rdi,[TEMP_BUFFER + 640]         ; RDI = Build position 4
cmp dword [rbx],0 ; old rbx+18/20
jne .PrintSlit
lea rsi,[TextNoSlit]
call StringWrite
jmp .EndSlit
.PrintSlit:
call AcpiHeaderPrint
.EndSlit:
mov al,0
stosb                      ; String terminator 00h

; SysInfo Line 6, NUMA Nodes
lea rsi,[TextNumaNodes]      ; RSI = Pointer to Name
lea rdi,[TEMP_BUFFER + 800]  ; RDI = Build position 5
call StringWrite
lea rsi,[r8 + SYSPARMS.NumaNodes]
lodsd                        ; EAX = Number of NUMA Nodes
mov bl,0
call DecimalPrint32
cmp eax,4
jbe @f                       ; Print for first 4 NUMA nodes only
mov eax,4                    ; Set number of visualized nodes = 4
@@:
xchg ecx,eax                 ; ECX = Number of NUMA Nodes
jecxz .EndNuma
mov ax,'  '
stosw
stosw
@@:                          ; Cycle for print first 4 NUMA nodes bitmap
lodsq
call HexPrint64
mov al,'h'
stosb
dec ecx
jz .EndNuma
mov ax,', '
stosw
jmp @b                       ; Cycle for print first 4 NUMA nodes bitmap
.EndNuma:
mov al,0
stosb					               ; String terminator 00h

; SysInfo Line 7, Processors by OS
lea rsi,[TextOsCpuCount]	   ; RSI = Pointer to String name
lea rdi,[TEMP_BUFFER + 960]  ; RDI = Build position 6
call StringWrite             ; Print "OS processors count="
mov eax,[r8 + SYSPARMS.SystemInfo.dwNumberOfProcessors]
mov bl,0                     ; BL = 0, mode for decimal print
mov byte [rdi],'?'
inc rdi
cmp eax,10000h               ; Validity limit = 65536
jae @f
dec rdi
call DecimalPrint32
@@:
mov ax,'  '
stosw
stosw
call StringWrite             ; Print "mask="
mov rax,[r8 + SYSPARMS.SystemInfo.dwActiveProcessorMask]
call HexPrint64
mov al,'h'
stosb
mov ax,'  '
stosw
stosw
mov eax,[r8 + SYSPARMS.ProcessorGroups]
push rax
lea rsi,[TextProcGroups]
call StringWrite
pop rax
mov bl,0
call DecimalPrint32
mov ax,'  '
stosw
stosw
call StringWrite
mov eax,[r8 + SYSPARMS.ProcessorsTotal]
call DecimalPrint32
mov al,0
stosb

; SysInfo Line 8, Physical memory by OS
lea rsi,[TextPhysMemory]
lea rdi,[TEMP_BUFFER + 1120]  ; RDI = Build position 7
lea r9,[r8 + SYSPARMS.MemoryInfo.ullTotalPhys]
mov ecx,2                     ; 2 strings: Physical, Available
.PrintMemory:                 ; Print "OS physical memory (MB)="
call StringWrite              ; at second pass "Available (MB)="
mov rax,[r9]                  ; RAX = Physical memory by OS, Bytes
xor edx,edx                   ; RDX = 0, Dividend high bits
mov ebp,1024*1024             ; RBP = Divisor for Bytes to Megabytes, 32 clear 64
div rbp                       ; RAX = Physical memory by OS, Megabytes
mov byte [rdi],'?'
inc rdi
shld rdx,rax,32
test edx,edx                  ; Check bits [63-32] of Size, Megabytes
jnz @f                        ; Go skip print if overflow
dec rdi
mov bl,0
call DecimalPrint32
@@:
mov ax,'  '
stosw
stosw
add r9,8
loop .PrintMemory

; SysInfo Line 8 continue, Large pages
mov rax,[r8 + SYSPARMS.LargePageSize]
test rax,rax
jz @f                  ; Skip if large page size = 0
call StringWrite
mov byte [rdi],'?'
inc rdi
test eax,01FFFFFh
jnz @f                 ; Skip if large page size unaligned by 2MB
cmp  rax,10000000h
ja @f                  ; Skip if large page size above 1GB
dec rdi
shr rax,20             ; Convert bytes to megabytes
call DecimalPrint32    ; Print large page size
@@:
mov al,0
stosb

; Done
ret

; Local helpers

;---------- Write ACPI Header ---------------------------------;
; INPUT:   RSI = Source pointer to Header Name string          ;
;          RBX = Source pointer to ACPI tags and rev. dword    ;
;          RDI = Destination pointer                           ;
;                                                              ;
; OUTPUT:  RSI = Updated RBX after Read data, used for next    ;
;          RDI = Updated RDI after Write data, used for next   ;
;--------------------------------------------------------------;
AcpiHeaderPrint:
call StringWrite   ; Print ACPI table name, examples = "MADT tags", "SRAT tags"
mov ecx,7
mov al,' '
rep stosb
mov rsi,rbx          ; Load second source pointer
mov cl,6             ; Load entire RCX not required, it 0  
rep movsb            ; Part 1 of tags
stosb                ; Interval = 2 spaces
stosb
mov cl,8
rep movsb            ; Part 2 of tags
mov cl,4
rep stosb
push rsi
lea rsi,[TextOemRev]
call StringWrite     ; Print "OEM Rev="
pop rsi
lodsw                ; This for RSI+2 only
lodsd
call HexPrint32      ; OEM Rev value, hex
mov al,'h'
stosb
mov cl,5
mov al,' '
rep stosb
ret

;---------- Write ACPI Numeric Info (Name=Value) --------------;
; INPUT:   RSI = Source pointer to System Info buffer          ;
;          RBX = Source pointer to Name string                 ;
;          RDI = Destination pointer                           ;
;                                                              ;
; OUTPUT:  RSI = Updated RSI after Read data, used for next    ;
;          RDI = Updated RDI after Write data, used for next   ;
;--------------------------------------------------------------;
AcpiNumberPrint:
push rsi
mov rsi,rbx
call StringWrite    ; Print parameter name
pop rsi
lodsd               ; EAX = Number of Local APICs / IOAPICs / NUMA Domains /...
mov bl,0            ; BL = 0, mode for decimal print
mov byte [rdi],'?'  ; Parameter value show "?" if invalid
inc rdi
cmp eax,10000h       ; This limit used for this parameters set
jae @f               ; Skip if value >= 10000h, means invalid and show "?"
dec rdi
call DecimalPrint32  ; Print value, overwrite "?" if valid
@@:
ret


