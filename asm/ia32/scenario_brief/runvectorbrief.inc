;--- Vector brief features detection and benchmarks mode run -------;
;                                                                   ;
; INPUT:   Threads management entries list.                         ;
; OUTPUT:  CF = Error flag: 0(NC)=OK, 1(C)=Execution error          ;
;          AL = Error code, valid if CF=1(C)                        ;
;               00 = Memory allocation error                        ;
;               01 = Memory release error                           ;
;               02 = Time measurement error                         ;
;               04 = Address arithmetics error                      ;
;               04-0FFh = Reserved                                  ;
;          Threads management entries list.                         ;
;-------------------------------------------------------------------;

RunVectorBrief:

; Clear control structures, size/8 because REP STOSQ (QWORDS)
cld
lea edi,[TEMP_BUFFER]
mov ebx,edi
xor eax,eax
mov ecx,TEMP_BUFFER_SIZE / 4
rep stosd

lea edi,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]
call GetCpuName         ; Update CPU NAME at vector brief data

call MeasureTsc         ; Update TSC clock variables at vector brief data
mov al,2
jc .ExitRestoreStack 

; Check CPUID functions 00000000h, 00000001h = CPU Standard Features
; OS context check by XCR0 also at this step 
xor eax,eax
cpuid
cmp eax,1               ; Check standard CPU features limit
jb .L1
mov eax,1
cpuid
mov [edi+00+00],edx
mov [edi+00+04],ecx

; Check XCR0 (AVX support) by CPU and OS (OS must save-restore AVX context)
mov eax,018000000h      ; This mask for bits 27, 28
and ecx,eax             ; ECX = Part of features
cmp ecx,eax
jne .L1                 ; Go if OSXSAVE(ECX.27) or AVX(ECX.28) not sup.
xor ecx,ecx             ; ECX = XCR register index
xgetbv                  ; Read XCR0 to EDX:EAX
mov [edi+24+00],eax
mov [edi+24+04],ecx
.L1:

; Check CPUID functions 80000000h, 80000001h = CPU Extended Features
mov eax,80000000h
cpuid
cmp eax,80000001h       ; Check extended CPU features limit
jb .L2
mov eax,80000001h
cpuid
mov [edi+08+00],edx
mov [edi+08+04],ecx
.L2:

; Check CPUID functions 00000000h, 00000007h = CPU Additional Features
xor eax,eax
cpuid
cmp eax,7               ; Check standard CPU features limit, for additional
jb .L3
mov eax,7
xor ecx,ecx
cpuid
mov [edi+16+00],ebx
mov [edi+16+04],ecx
.L3:

; Prepare for timings measurement
lea esi,[InstructionTimingControl]    ; ESI = Pointer to Control sequence
add edi,4*8                           ; EDI = Pointer to dTSC values storage

; Cycle for timings measurement patterns

.L10:
cmp dword [esi],0       ; compare entry bytes [0-3]
jnz .L16
cmp byte [esi+4],0      ; compare entry byte [4]
jz .L11                 ; Go if all 5 bytes = 0 means end of list
.L16:

lea ebx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]

mov al,[esi+3]   ; AL = CPUID validator
mov ah,al
shr ah,6
jz .L14          ; Go execute if checks not required
lea edx,[ebx + VECTORBRIEF.StdFeatures]
dec ah
jz .L13
lea edx,[ebx + VECTORBRIEF.ExtFeatures]
dec ah
jz .L13
lea edx,[ebx + VECTORBRIEF.AddFeatures]
.L13:
test al,00100000b
jnz .L40
mov edx,[edx]
jmp .L41
.L40:
mov edx,[edx+4]
.L41:
and eax,00011111b
bt edx,eax
jnc .L12           ; Go skip if routine not supported by CPU
.L14:
mov al,[esi+4]     ; AL = OS context validator
test al,11000000b
jz .L15            ; Go execute if checks not required
lea edx,[ebx + VECTORBRIEF.OsContext]
test al,00100000b
jnz .L50
mov edx,[edx]
jmp .L51
.L50:
mov edx,[edx+4]
.L51:
and eax,00011111b
bt edx,eax
jnc .L12         ; Go skip if routine not supported by OS context

.L15:
push esi edi     ; store pointers, corrupted by performance patterns run
lea edi,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_TRANSIT]

mov al,[esi+0]                            ; AL = Method ID
test al,al
js .L17
and al,07Fh
call GetBandwidthPattern
jmp .L18
.L17:
and al,07Fh
call GetMathPattern
.L18:

mov [edi + THCTRL.EntryPoint],ebx    ; EBX = Subroutine address
mov cl,[esi+1]                       ; CL = Power of 2
mov eax,1
shl eax,cl
mov [edi + THCTRL.SizeInst],eax      ; EAX = Instructions count per 4K
movzx eax,byte [esi+2]               ; EAX = Repeats count
mov edx,REPEATS_SCALE
mul edx
mov dword [edi + THCTRL.Repeats + 0],eax  ; EDX:EAX = Measurement repeats count
mov dword [edi + THCTRL.Repeats + 4],edx
lea eax,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_DATA]
mov [edi + THCTRL.Base1],eax
add eax,VECTOR_BRIEF_DATA_SIZE / 2
mov [edi + THCTRL.Base2],eax

;- lea esi,[MathData]  ; FIX THIS, USE Base3
; If this run for math. pattern, Base1 used for load numeric constants vector,
; not for block 1 base.
test byte [esi+0],80h
jz @f
mov [edi + THCTRL.Base1],MathData
@@:
;-

mov ebx,edi         ; Note can remove this if Get...Pattern return in EAX 
call RunTarget      ; Return RAX = dTSC per pattern
pop edi esi         ; restore pointers, corrupted by performance patterns run

.L12:
add esi,5
stosd                                     ; Store dTSC per pattern
xchg eax,edx
stosd
jmp .L10
.L11:

; Exit point if no errors, single-thread branch
clc
.ExitRestoreStack:
ret


