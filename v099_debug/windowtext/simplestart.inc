;-------------------------------------------------------------------;
; Benchmarks mode settings = F(Options)                             ;
; Used for run simple or drawings benchmarks after dialogue         ;
; Procedure 1 of 3 = SimpleStart, SimpleProgress, SimpleStop        ;
;                                                                   ;
; INPUT:  Dialogue control block, parameters from OS API            ;
; OUTPUT: Options variables fields, used for benchmarks control     ;
;                                                                   ;
;-------------------------------------------------------------------;

SimpleStart:

; Extract ASM-METHOD
lea rdi,[InputParms]
mov esi,HANDLE_X86_READ         ; Compare pattern = FIRST handle
@@:
call SendMessageGetCheck
jz @f                           ; Go if BST_CHECKED
inc esi
cmp esi,HANDLE_LAST_METHOD      ; Compare with LAST handle
jbe @b                          ; Cycle for radio buttons group
@@:
sub esi,HANDLE_X86_READ         ; 0=First button, ..., 16=Last, 17-255 = Invalid
mov [rdi+IPB.SelectMethod],sil  ; Write method number, SIL register = bits SI.[7-0]

; Save ASM method code
push rsi

; Extract ENABLE PARALLEL THREADS
mov esi,HANDLE_PARALLEL_THREADS
call SendMessageGetCheck
mov eax,[SystemInfo.dwNumberOfProcessors]
jz @f
mov eax,1
@@:
mov [rdi+IPB.NumThreads],eax    ; Write number of threads

; Extract ENABLE HYPER THREADING
xor eax,eax
test dword [PARALLEL_NUMA_LARGE_PAGES_BASE+E_HYPER_THREADING],WS_DISABLED
jnz @f                      ; Go skip if HT not supported by platform, AL=0
mov esi,HANDLE_HYPER_THREADING
call SendMessageGetCheck
inc eax
@@:
mov [rdi+IPB.OptionHT],al       ; Write hyper-threading option

; Extract LARGE PAGES CONTROL
xor eax,eax
test dword [PARALLEL_NUMA_LARGE_PAGES_BASE+E_LARGE_PAGES],WS_DISABLED
jnz @f                      ; Go skip if HT not supported by platform, AL=0
mov esi,HANDLE_LARGE_PAGES
call SendMessageGetCheck
inc eax
@@:
mov [rdi+IPB.OptionLP],al       ; Write large pages option

; Extract NUMA CONTROL
xor esi,esi
test dword [PARALLEL_NUMA_LARGE_PAGES_BASE+NUMA_ABSENT],WS_DISABLED
jnz .NumaOff                       ; Go skip if NUMA not sup. by platform, SI=0 
mov esi,HANDLE_NUMA_ABSENT
@@:
call SendMessageGetCheck
jz @f                              ; Go if BST_CHECKED
inc esi
cmp esi,HANDLE_NUMA_REMOTE         ; Compare with LAST handle
jbe @b                             ; Cycle for radio buttons group
@@:
sub esi,HANDLE_NUMA_ABSENT - 1     ; 0=First button, ..., 2=Last, 3-255 = Invalid
.NumaOff:
mov [rdi+IPB.OptionNUMA],sil       ; Note SIL means bits SI.[7-0]

; Extract TARGET OBJECT (CACHE/RAM/CUSTOM) SELECT
mov esi,HANDLE_L1_CACHE
@@:
call SendMessageGetCheck
jz @f                              ; Go if BST_CHECKED
inc esi
cmp esi,HANDLE_CUSTOM_BLOCK        ; Compare with LAST handle
jbe @b                             ; Cycle for radio buttons group
@@:
sub esi,HANDLE_L1_CACHE            ; 0=First button, ..., 4=Last, 5-255 = Invalid
mov [rdi+IPB.OptionTarget],sil

; Restore ASM method code
pop rax

; Set instruction width, bits = F (selected instruction set)
; Because also F(temporal, non-temporal), set after it
; Support non-temporal Reads
lea rdx,[MethodsTemporal]
cmp esi,3                          ; ESI=3 means Target Object = RAM
jne @f                             ; Small block mode if not a RAM
mov cl,[InputParms.NonTemporalRead]
lea rdx,[MethodsNonTemporal_SSE41]
cmp cl,1
je @f
lea rdx,[MethodsNonTemporal_AVX2]
cmp cl,2
je @f
lea rdx,[MethodsNonTemporal]
@@:
imul eax,eax,6                     ; 6 bytes per Methods List entry (BMETHOD)
mov al,[rdx+rax+5]                 ; EAX = Data bytes per instruction
shl eax,3                          ; EAX = Data bits per instruction
mov [rdi+IPB.OperandWidth],eax     ; Write operand width, bits

; Memory allocation moved to threads initializer
; Set Block #1 base
; mov rcx,[MemoryAlignedBase]      ; RCX = Block #1 base used later
; mov [rdi+IPB.BlockBase1],rcx     ; Write Block #1 base

; Set Block size
mov rax,[BlockSizeEdit]
cmp esi,4
je .SetBlk                         ; Go if custom block mode
; Memory block size for benchmark mode = RAM
mov eax,CONST_RAM_BLOCK            ; Defined from v0.94
cmp esi,3
je .SetBlk
imul eax,esi,12                    ; 12 bytes per entry
mov rax,[CacheL1Data+rax]
mov rdx,rax
shr rax,1                          ; size/2
sub rax,512                        ; size-512
.SetBlk:
mov [rdi+IPB.BlockSize],rax        ; Write One block size 

; Memory allocation moved to threads initializer
; Set Block #2 base = Block #1 base + Block size
; add rax,rcx
; mov [rdi+IPB.BlockBase2],rax       ; Write Block #2 base

; Set SLOW MODE
push rsi
mov esi,HANDLE_SLOW_MODE
call SendMessageGetCheck
mov [rdi+IPB.OptionMeasure],al     ; 0=Disabled, 1=Enabled, 2-255 = Invalid
pop rsi

; Set number of measurement repeats
and eax,1
imul eax,eax,20                        ; RAX = Scaled index for fast/slow mode
mov eax,[BenchmarksRepeats+rax+rsi*4]  ; RSI = Scaled index for L1/L2/L3/RAM/Custom
mov [rdi+IPB.NumRepeats],eax           ; Write number of repeats from EAX

; Select methods table, optimized for small or big blocks 
; Support non-temporal Reads
cmp esi,3                       ; If DL=3 , means Analysed Object=RAM
lea rsi,[MethodsTemporal]
jne @f
mov cl,[InputParms.NonTemporalRead]
lea rsi,[MethodsNonTemporal_SSE41]
cmp cl,1
je @f
lea rsi,[MethodsNonTemporal_AVX2]
cmp cl,2
je @f
lea rsi,[MethodsNonTemporal]
@@:

; Select ASM method: for target subroutine and code for dump
movzx eax,byte [rdi+IPB.SelectMethod]  ; RAX = Method
imul rax,rax,6                         ; 6 bytes per entry
add rsi,rax                            ; RSI = Pointer to entry
xor eax,eax                            ; This clear entire RAX
lea rdx,[BasePointCode]                ; RDX = Base point for compact offsets
lea rdi,[PatternRoutine]               ; RDI = Destination variables group base

; Set target subroutine address (benchmark pattern selector )
lodsw
lea rcx,[rdx+rax]
mov [rdi+000],rcx               ; QWORD [PatternSelector] = RCX

; Set target subroutine dump fragment start and size
lodsw
lea rcx,[rdx+rax]
mov [rdi+008],rcx               ; QWORD [DumpMethodStart] = RCX
lodsb
mov ah,0                                  
mov [rdi+016],eax               ; DWORD [DumpMethodLength] = EAX

; Exit
ret
