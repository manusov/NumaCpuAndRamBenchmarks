;------------------------------------------------------------------------;
; Get CPU signature and verify features, execute CPUID functions:        ;
; 00000001h, 80000001h. Call this subroutine only if CPUID supported.    ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save CPU Signature and          ;
;                Features Info                                           ;
;                                                                        ;
; OUTPUT:  CF flag = Status: 0(NC)=Support OK, 1(C)=Too old CPU          ;
;          Output AL valid only if CF=1(C)                               ;
;          Output RDI and destination memory valid only if CF=0(NC)      ;
;          AL  = Error code: 0=CPUID#1 not supported                     ;
;                            1=TSC not supported                         ;
;                            other codes not used                        ;
;         RDI = Input RDI + 5 , buffer size fixed = 5 bytes              ;
;         Destination memory at [input RDI] = Results                    ;
;         Dword [0-3] = CPU TFMS (Type,Family,Model,Stepping)            ;
;         Byte  [4-4] = Special options flags:                           ;
;                       D0=SSE, D1=AVX, D2=FMA, D3=AVX2,                 ;
;                       D4=AVX512F (foundation part of AVX512),          ;
;                       D[5-6]=Reserved, D7=HT                           ;
;------------------------------------------------------------------------;
GetCpuFeatures:
cld                       ; Clear direction, because STOSD used
push rbx rcx rdx rsi
;--- Check CPUID#1 support, execute CPUID#0 ---
mov esi,1                 ; Function code = 1 , used twice
xor eax,eax
cpuid                     ; Execute CPUID#0
cmp eax,esi
mov al,0                  ; AL = Error code, 0 means CPUID#1 not sup.
jb ExitCpuId3             ; Go with CF=1, AL=0 if CPUID#1 not supported
;--- Execute CPUID#1 ---
xchg eax,esi
cpuid                     ; Execute CPUID#1
stosd                     ; Store TFMS
;--- Check TSC support ---
bt edx,4                  ; EDX.4 = TSC feature bit
cmc                       ; Invert CF, make CF=1 if TSC not supported
mov al,1                  ; AL = Error code, 0 means TSC not supported
jb ExitCpuId3             ; Go with CF=1, AL=1 if TSC not supported
;--- Save FMA and HTT bits for future use ---
mov esi,ecx               ; Save ECX for ECX.12=FMA future use
and si,1000h              ; Mask FMA bits
shl esi,16                ; ESI.28 = FMA , other ESI bits = 0
;--- Check Hyper-Threading ---
bt edx,28
jnc .CheckSSE             ; Skip if HTT feature bit = 0
shr ebx,16                ; Shift logical CPU count to BL
cmp bl,2
jb .CheckSSE              ; Skip if Logical Processors < 2
mov si,0080h              ; ESI.7 = HTT
;--- Start check optional features, check SSE ---
; SI = Bitmap, here SI=0, ESI.[31-16]=Bitmap
.CheckSSE:
bt edx,25                 ; EDX.25 = SSE feature bit
jnc .StoreCpuId3          ; Go skip SSE, AVX, FMA if SSE not supported
inc esi                   ; Set ESI.0 = SSE
;--- Check SSE4.1, used from v0.93 for 128-bit NT Read (VMOVNTDQA xmm) ---
bt ecx,19                 ; ECX.19 = SSE 4.1 feature bit
jz .CheckSSE41            ; Go skip if SSE4.1 not supported
or esi,00100000b          ; If supported, set mask bit 5
.CheckSSE41:
;--- Check AVX support by CPU and OS (OS must save-restore AVX context) ---
mov eax,018000000h        ; This mask for bits 27, 28
and ecx,eax               ; ECX = Part of features
cmp ecx,eax
jne .StoreCpuId3          ; Go if OSXSAVE(ECX.27) or AVX(ECX.28) not sup.
xor ecx,ecx               ; ECX = XCR register index
xgetbv                    ; Read XCR0 to EDX:EAX
and al,00000110b          ; This mask for bits 1, 2
cmp al,00000110b
jne .StoreCpuId3          ; Go skip AVX, FMA if AVX not supported
or esi,00000010b          ; Set ESI.1 = AVX
;--- Check FMA support ---
bt esi,28                 ; Check previously saved FMA feature bit
jnc .CheckAVX2
or esi,00000100b          ; Set ESI.2 = FMA
;--- Check AVX2 support ---
.CheckAVX2:
xor eax,eax
cpuid                     ; Execute CPUID#0
cmp eax,7
jb .StoreCpuId3           ; Go skip AVX2 if CPUID#7 not supported
mov eax,7                 ; EAX = CPUID function
xor ecx,ecx               ; ECX = CPUID sub-function
cpuid                     ; Execute CPUID#7 subfunction 0
test bl,00100000b         ; EBX.5 = AVX2 feature bit
jz .StoreCpuId3           ; Go skip AVX2 if feature bit = 0
or esi,00001000b          ; Set ESI.3 = AVX2
;--- Check AVX512F (foundation) support, by CPU and OS ---
; Note if this point reached, XCR0 supported
;---
bt ebx,16                 ; EBX.16 = AVX512F feature bit
jnc .StoreCpuId3          ; Go skip AVX512F if feature bit = 0
xor ecx,ecx               ; ECX = XCR register index
xgetbv                    ; Read XCR0 to EDX:EAX
and al,11100000b          ; Check bits, associated with AVX512
cmp al,11100000b
jne .StoreCpuId3          ; Go skip if AVX512 context not supported by OS
or esi,00010000b          ; Set ESI.4 = AVX512F
;--- Store result ---
.StoreCpuId3:
xchg eax,esi
stosb 
;--- Exit ---
clc                       ; CF=0 (NC) means CPU features check OK
ExitCpuId3:
pop rsi rdx rcx rbx
ret
