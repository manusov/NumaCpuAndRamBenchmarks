;--- Detection CPU Caches -----------------------------------------------;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save cache info                 ;
;                                                                        ;
; OUTPUT:  EDI = Input EDI + 60 , buffer size fixed = 60 bytes           ;
;          Destination memory at [input EDI] = Results, 0=Not detected   ;
;          Qword [00-07] = Trace instruction cache size, micro ops       ;
;          Word  [08-09] = Maximum threads at this level                 ;
;          Word  [10-11] = Maximum APIC IDs per package                  ;
;          Qword [12-19] = L1 instruction cache size, bytes              ;
;          Word  [20-21] = Maximum threads at this level                 ;
;          Word  [22-23] = Maximum APIC IDs per package                  ;
;          Qword [24-31] = L1 data cache size, bytes                     ;
;          Word  [32-33] = Maximum threads at this level                 ;
;          Word  [34-35] = Maximum APIC IDs per package                  ;
;          Qword [36-43] = L2 unified cache size, bytes                  ;
;          Word  [44-45] = Maximum threads at this level                 ;
;          Word  [46-47] = Maximum APIC IDs per package                  ;
;          Qword [48-55] = L3 unified cache size, bytes                  ;
;          Word  [56-57] = Maximum threads at this level                 ;
;          Word  [58-59] = Maximum APIC IDs per package                  ; 
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

GetCpuCache:								  
cld                            ; Clear direction, because STOSB used
push ebx esi ebp
; Blank results
mov ecx,60
xor eax,eax                    ; This entire EAX used for CPUID later
rep stosb
; Check support CPUID#4 = Deterministic cache info, execute CPUID#0
; Here EAX=0
cpuid
cmp eax,4
jb NoMoreDeterministicCaches
; Execute CPUID#4, subfunctions sequence 
xor esi,esi                    ; ESI = Subfunctions count
CycleCache:
mov eax,4                      ; EAX = Function
mov ecx,esi                    ; ECX = Subfunction
cpuid
; Select handler = F(Type,Level)
test al,01Fh                   ; AL.[7-5]=Level, AL.[4-0]=Type
jz NoMoreDeterministicCaches   ; Go exit if no more caches
mov dl,12-60                   ; DL = Offset for addressing output data
cmp al,022h                    ; AL = Selector
je StoreCache                  ; Go analyse if L1 instruction cache
mov dl,24-60
cmp al,021h
je StoreCache                  ; Go analyse if L1 data cache
mov dl,36-60
cmp al,043h
je StoreCache                  ; Go analyse if L2 Unified cache
; This patch for Oracle VMBox
cmp al,41h                     ; Oracle VMBox can map physical L3 Unified cache as virtual L2 Data Cache
je StoreCache                  ; Go analyse if L2 Data cache (really L3 Unified cache)
; End of patch for Oracle VMBox
mov dl,48-60
cmp al,063h                
je StoreCache                  ; Go analyse if L3 Unified cache
NextCache:
inc esi
cmp esi,256                    ; This for hang prevent if failure	
jb CycleCache

; Special support for Intel Trace Cache
NoMoreDeterministicCaches:
xor eax,eax
cpuid
cmp eax,2
jb DoneCaches
mov eax,2       ; EAX = Function = 2 = Get Cache Descriptors
xor ecx,ecx     ; ECX = Subfunction = 0
cpuid
cmp al,1        ; Leaf count must be 1, otherwise skip
jne DoneCaches
test eax,eax    ; If bit EAX.31 = 1, means EAX contain invalid data   
jns @f          ; Go skip clear, if EAX data valid
xor eax,eax
@@:
test ebx,ebx    ; If bit EBX.31 = 1, means EBX contain invalid data
jns @f          ; Go skip clear, if EBX data valid
xor ebx,ebx
@@:
test ecx,ecx    ; If bit ECX.31 = 1, means ECX contain invalid data
jns @f          ; Go skip clear, if ECX data valid
xor ecx,ecx
@@:
test edx,edx    ; If bit EDX.31 = 1, means EDX contain invalid data
jns @f          ; Go skip clear, if EDX data valid
xor edx,edx
@@:
mov ebp,15      ; 4 registers * 4 bytes - 1 unused (AL)
.CacheDescriptors:
shrd eax,ebx,8
shrd ebx,ecx,8
shrd ecx,edx,8
shr edx,8
lea esi,[IntelTraceCaches]
.CacheValues:
cmp al,[esi + 0]
jne @f
push eax
movzx eax,byte [esi + 1]        ; EAX = Trace Cache size, KuOps
shl eax,10                      ; EAX = Trace Cache size, uOps
mov dword [edi - 60 + 0],eax    ; Store Trace Cache size, uOps
mov dword [edi - 60 + 4],0 
pop eax
@@:
add esi,2
cmp byte [esi],0
jne .CacheValues        ; Cycle for possible Cache types IDs  
dec ebp
jnz .CacheDescriptors   ; Cycle for Cache Descriptors array 

; Exit
DoneCaches:
pop ebp esi ebx
ret

; Start handling detected cache type/level
StoreCache:
movsx edx,dl                   ; Save address offset
push edx
; Calculate cache size
; Cache size (bytes) = (ways+1)*(partitions+1)*(linesize+1)*(sets+1) =
; = (EBX[31-22]+1) * (EBX[21-12]+1) * (EBX[11-00]+1) * (ECX+1)
mov edx,ebx
shr edx,22                     ; Shift EBX[31-22] = Ways
and edx,03FFh 
inc edx
inc ecx
imul ecx,edx                   ; Multiply to ECX = Sets
mov edx,ebx                    
shr edx,12                     ; Shift EBX[21-12] = Partitions
and edx,03FFh		     
inc edx
imul ecx,edx                   ; Mask EBX[11-00] = Line Size
and ebx,0FFFh
inc ebx
imul ecx,ebx                   ; ECX = Cache size, bytes
; Store cache size
pop edx
mov dword [edi + edx + 0],ecx
mov dword [edi + edx + 4],0
; Extract threads share cache
mov ebx,eax
shr eax,14                     ; Logical CPUs: 12 bit [26-14] shift to [11-00]
shr ebx,26                     ; CPU Cores: 6 bit [31-26] shift to [05-00]
and ax,0FFFh                   ; Extract Logical CPUs = Threads
and bx,003Fh                   ; Extrach Cores		  
inc eax                        ; This because "-1" encoding
inc ebx                        ; This because "-1" encoding
; Store threads share cache
mov [edi + edx + 08],ax
mov [edi + edx + 10],bx
; Return to cycle
jmp NextCache

