;--- Extract data for visualization (text strings) from detected system info --;
;                                                                              ;
; INPUT:   None at registers                                                   ;
;          System Parameters Block = results of system information             ;
;          see global\connect_equ.inc for layout                               ;
;                                                                              ;
; OUTPUT:  None at registers                                                   ;
;          Temporary multifunctional buffer, data for visualization            ;  
;                                                                              ;
; All CPU registers can be corrupted                                           ;
;------------------------------------------------------------------------------;

SysParmsToBuffer:

; Initializing pointers
lea esi,[TextTfms]
lea ebp,[TEMP_BUFFER + 000]
mov edi,ebp

; Build Window 0 user interface elements = F (System Information)
; SysInfo Line 1, Processor
call StringWrite
mov eax,[SystemParameters.CpuSignature]
call HexPrint32
mov ax,'h'
stosw

; SysInfo Line 1 continue, CPU Frequency
lea edi,[ebp + 080]
call StringWrite
mov eax,dword [SystemParameters.TscMHz + 0]
mov edx,dword [SystemParameters.TscMHz + 4]
mov bx,0100h
call DoublePrint
call StringWrite
mov al,0
stosb

; SysInfo Line 2, Cache
push ebp
lea edi,[ebp + 160]
mov ecx,5                  ; ECX = Number of types/levels
lea ebp,[SystemParameters.CpuCache.CpuidTraceCache]
call StringWrite
mov ax,'  '                ; First interval
stosw
.CacheLevels:              ; Cycle for all cache types/levels
mov ebx,ebp
mov ebx,[ebx]              ; Get EBX = Cache size, bytes
test ebx,ebx
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
mov eax,ebx
xor edx,edx
push ecx
mov ecx,1024
div ecx
pop ecx
mov byte [edi],'?'
inc edi
test edx,edx
jnz .NextLevel
dec edi
mov bl,0
call DecimalPrint32        ; Print value in Kilobytes or KuOps
cmp ecx,5
jne .NextLevel             ; Skip write "KuOps" if no trace cache
push esi
lea esi,[TextKuOps]       
call StringWrite           ; Print units for KuOps 
pop esi
.NextLevel:
add ebp,12
loop .CacheLevels          ; Cycle for all cache types/levels
mov al,0
stosb                      ; String terminator 00h
pop ebp

; SysInfo Line 3, ACPI MADT, Local APICs, I/O APICs
lea ebx,[SystemParameters.MadtText]  ; EBX = Pointer to System Information
lea esi,[TextMadt]                   ; ESI = Pointer to Name
lea edi,[TEMP_BUFFER + 320]          ; EDI = Build position 3
cmp dword [ebx],0          ; old rbx+18/20
jne .PrintMadt             ; Go print if Local APICs count > 0
lea esi,[TextNoMadt]
call StringWrite
jmp .EndMadt
.PrintMadt:
call AcpiHeaderPrint
lea ebx,[TextLocalApics]
call AcpiNumberPrint       ; Print Number of Local APICs
mov ecx,5
mov al,' '
rep stosb
lea ebx,[TextIoApics]
call AcpiNumberPrint       ; Print Number of I/O APICs
.EndMadt:
mov al,0
stosb                      ; String terminator 00h

; SysInfo Line 4, ACPI SRAT, NUMA domains, CPUs, RAMs
lea ebx,[SystemParameters.SratText]  ; EBX = Pointer to System Information
lea esi,[TextSrat]                   ; ESI = Pointer to Name
lea edi,[TEMP_BUFFER + 480]          ; EDI = Build position 4
cmp dword [ebx],0                    ; old rbx+18/20
jne .PrintSrat
lea esi,[TextNoSrat]
call StringWrite
jmp .EndSrat
.PrintSrat:
call AcpiHeaderPrint
lea ebx,[TextDomains]
call AcpiNumberPrint       ; Print Number of NUMA Domains
mov ecx,5
mov al,' '
rep stosb
lea ebx,[TextCPUs]
call AcpiNumberPrint       ; Print Number of CPUs, NUMA-aware
mov ecx,5
mov al,' '
rep stosb
lea ebx,[TextRAMs]
call AcpiNumberPrint       ; Print Number of RAMs, NUMA-aware
.EndSrat:
mov al,0
stosb                      ; String terminator 00h

; SysInfo Line 5, ACPI SLIT, yet without table-specific extended info
lea ebx,[SystemParameters.SlitText]  ; EBX = Pointer to System Information
lea esi,[TextSlit]                   ; ESI = Pointer to Name
lea edi,[TEMP_BUFFER + 640]          ; EDI = Build position 4
cmp dword [ebx],0                    ; old rbx+18/20
jne .PrintSlit
lea esi,[TextNoSlit]
call StringWrite
jmp .EndSlit
.PrintSlit:
call AcpiHeaderPrint
.EndSlit:
mov al,0
stosb                      ; String terminator 00h

; SysInfo Line 6, NUMA Nodes
lea esi,[TextNumaNodes]      ; ESI = Pointer to Name
lea edi,[TEMP_BUFFER + 800]  ; EDI = Build position 5
call StringWrite
lea esi,[SystemParameters.NumaNodes]
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
lodsd
xchg edx,eax
lodsd
xchg edx,eax
call HexPrint64        ; NUMA nodes affinity masks visualized as 64-bit values
                       ; even for ia32 version
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
lea esi,[TextOsCpuCount]	   ; ESI = Pointer to String name
lea edi,[TEMP_BUFFER + 960]  ; EDI = Build position 6
call StringWrite             ; Print "OS processors count="
mov eax,[SystemParameters.SystemInfo.dwNumberOfProcessors]
mov bl,0                     ; BL = 0, mode for decimal print
mov byte [edi],'?'
inc edi
cmp eax,10000h               ; Validity limit = 65536
jae @f
dec edi
call DecimalPrint32
@@:
mov ax,'  '
stosw
stosw
call StringWrite             ; Print "mask="
mov eax,[SystemParameters.SystemInfo.dwActiveProcessorMask]
call HexPrint32
mov al,'h'
stosb
mov ax,'  '
stosw
stosw
mov eax,[SystemParameters.ProcessorGroups]
push eax
lea esi,[TextProcGroups]
call StringWrite
pop eax
mov bl,0
call DecimalPrint32
mov ax,'  '
stosw
stosw
call StringWrite
mov eax,[SystemParameters.ProcessorsTotal]
; Support platforms without processor groups
test eax,eax
jnz @f
mov eax,[SystemParameters.SystemInfo.dwNumberOfProcessors]
@@: 
; Write processors total
call DecimalPrint32
mov al,0
stosb

; SysInfo Line 8, Physical memory by OS
lea esi,[TextPhysMemory]
lea edi,[TEMP_BUFFER + 1120]  ; EDI = Build position 7
lea ebx,[SystemParameters.MemoryInfo.ullTotalPhys]
mov ecx,2                     ; 2 strings: Physical, Available
.PrintMemory:                 ; Print "OS physical memory (MB)="
call StringWrite              ; at second pass "Available (MB)="
mov eax,[ebx + 0]             ; EDX:EAX = Physical memory by OS, Bytes
mov edx,[ebx + 4]
mov ebp,1024*1024             ; RBP = Divisor for Bytes to Megabytes, 32 clear 64
div ebp                       ; RAX = Physical memory by OS, Megabytes
mov byte [edi],'?'
push ebx
mov bl,0                         ; BL = Print template mode = 0 = no template
call DecimalPrint32
pop ebx
@@:
mov ax,'  '
stosw
stosw
add ebx,8
loop .PrintMemory

; SysInfo Line 8 continue, Large pages
mov eax,[SystemParameters.LargePageSize]
test eax,eax
jz @f                  ; Skip if large page size = 0
call StringWrite
mov byte [edi],'?'
inc edi
test eax,01FFFFFh
jnz @f                 ; Skip if large page size unaligned by 2MB
cmp  eax,10000000h
ja @f                  ; Skip if large page size above 1GB
dec edi
shr eax,20             ; Convert bytes to megabytes
mov bl,0               ; BL = Print template mode = 0 = no template
call DecimalPrint32    ; Print large page size
@@:
mov al,0
stosb
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
mov esi,ebx          ; Load second source pointer
mov cl,6             ; Load entire ECX not required, it 0  
rep movsb            ; Part 1 of tags
stosb                ; Interval = 2 spaces
stosb
mov cl,8
rep movsb            ; Part 2 of tags
mov cl,4
rep stosb
push esi
lea esi,[TextOemRev]
call StringWrite     ; Print "OEM Rev="
pop esi
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
; INPUT:   ESI = Source pointer to System Info buffer          ;
;          EBX = Source pointer to Name string                 ;
;          EDI = Destination pointer                           ;
;                                                              ;
; OUTPUT:  ESI = Updated RSI after Read data, used for next    ;
;          EDI = Updated RDI after Write data, used for next   ;
;--------------------------------------------------------------;
AcpiNumberPrint:
push esi
mov esi,ebx
call StringWrite    ; Print parameter name
pop esi
lodsd               ; EAX = Number of Local APICs / IOAPICs / NUMA Domains /...
mov bl,0            ; BL = 0, mode for decimal print
mov byte [edi],'?'  ; Parameter value show "?" if invalid
inc edi
cmp eax,10000h       ; This limit used for this parameters set
jae @f               ; Skip if value >= 10000h, means invalid and show "?"
dec edi
call DecimalPrint32  ; Print value, overwrite "?" if valid
@@:
ret
