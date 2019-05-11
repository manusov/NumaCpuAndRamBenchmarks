;--- Detection CPU Caches -----------------------------------------------;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save cache info                 ;
;                                                                        ;
; OUTPUT:  RDI = Input RDI + 60 , buffer size fixed = 60 bytes           ;
;          Destination memory at [input RDI] = Results, 0=Not detected   ;
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
; Can destroy registers, volatile by Microsoft x64 calling convention.   ; 
;------------------------------------------------------------------------;

GetCpuCache:								  
cld                            ; Clear direction, because STOSB used
push rbx rsi
; Blank results
mov ecx,60
xor eax,eax                    ; This entire EAX used for CPUID later
rep stosb
; Check support CPUID#4 = Deterministic cache info, execute CPUID#0
; Here EAX=0
cpuid
cmp eax,4
jb NoMoreCaches
; Execute CPUID#4, subfunctions sequence 
xor esi,esi                    ; ESI = Subfunctions count
CycleCache:
mov eax,4                      ; EAX = Function
mov ecx,esi                    ; ECX = Subfunction
cpuid
; Select handler = F(Type,Level)
test al,01Fh                   ; AL.[7-5]=Level, AL.[4-0]=Type
jz NoMoreCaches                ; Go exit if no more caches
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
; Exit
NoMoreCaches:
pop rsi rbx
ret

; Start handling detected cache type/level
StoreCache:
movsx rdx,dl                   ; Save address offset
push rdx
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
pop rdx
mov [rdi+rdx],rcx
; Extract threads share cache
mov ebx,eax
shr eax,14                     ; Logical CPUs: 12 bit [26-14] shift to [11-00]
shr ebx,26                     ; CPU Cores: 6 bit [31-26] shift to [05-00]
and ax,0FFFh                   ; Extract Logical CPUs = Threads
and bx,003Fh                   ; Extrach Cores		  
inc eax                        ; This because "-1" encoding
inc ebx                        ; This because "-1" encoding
; Store threads share cache
mov [rdi+rdx+08],ax
mov [rdi+rdx+10],bx
; Return to cycle
jmp NextCache

