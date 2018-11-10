;-------------------------------------------------------------------;
; Vector brief features detection and benchmarks mode run.          ;
; Procedure 2 of 3 =                                                ;
;           VectorBriefStart, VectorBriefProgress, VectorBriefStop  ;
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

VectorBriefProgress:

mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
push rbp rbp

; Clear control structures, size/8 because REP STOSQ (QWORDS)
cld
lea rdi,[TEMP_BUFFER]
mov rbx,rdi
xor eax,eax
mov ecx,TEMP_BUFFER_SIZE / 8
rep stosq

lea rdi,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]
call GetCpuName
xor eax,eax
stosq

call MeasureCpuClk
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
mov [rdi+00+00],edx
mov [rdi+00+04],ecx

; Check XCR0 (AVX support) by CPU and OS (OS must save-restore AVX context)
mov eax,018000000h      ; This mask for bits 27, 28
and ecx,eax             ; ECX = Part of features
cmp ecx,eax
jne .L1                 ; Go if OSXSAVE(ECX.27) or AVX(ECX.28) not sup.
xor ecx,ecx             ; ECX = XCR register index
xgetbv                  ; Read XCR0 to EDX:EAX
mov [rdi+24+00],eax
mov [rdi+24+04],ecx
.L1:

; Check CPUID functions 80000000h, 80000001h = CPU Extended Features
mov eax,80000000h
cpuid
cmp eax,80000001h       ; Check extended CPU features limit
jb .L2
mov eax,80000001h
cpuid
mov [rdi+08+00],edx
mov [rdi+08+04],ecx
.L2:

; Check CPUID functions 00000000h, 00000007h = CPU Additional Features
xor eax,eax
cpuid
cmp eax,7               ; Check standard CPU features limit, for additional
jb .L3
mov eax,7
xor ecx,ecx
cpuid
mov [rdi+16+00],ebx
mov [rdi+16+04],ecx
.L3:

; Prepare for timings measurement
lea rsi,[InstructionTimingControl]    ; RSI = Pointer to Control sequence
add rdi,4*8                           ; RDI = Pointer to dTSC values storage

; Cycle for timings measurement patterns
.L10:
cmp qword [rsi],0
jz .L11                 ; Go if address=0 means end of list

lea rbx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]

mov al,[rsi+16]  ; AL = CPUID validator
mov ah,al
shr ah,6
jz .L14          ; Go execute if checks not required
mov rdx,[rbx+VECTORBRIEF.StdFeatures]
dec ah
jz .L13
mov rdx,[rbx+VECTORBRIEF.ExtFeatures]
dec ah
jz .L13
mov rdx,[rbx+VECTORBRIEF.AddFeatures]
.L13:
and eax,03Fh
bt rdx,rax
jnc .L12         ; Go skip if routine not supported by CPU

.L14:
mov al,[rsi+17]  ; AL = OS context validator
test al,0C0h
jz .L15          ; Go execute if checks not required
mov rdx,[rbx+VECTORBRIEF.OsContext]
and eax,03Fh
bt rdx,rax
jnc .L12         ; Go skip if routine not supported by OS context

.L15:
push rsi rdi
lea rbx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_TRANSIT]
lodsq                   ; RAX = Subroutine address
mov [rbx+THCTRL.EntryPoint],rax
lodsd
mov [rbx+THCTRL.SizeInst],rax
lodsd
mov [rbx+THCTRL.Repeats],rax
lea rax,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_DATA]
mov [rbx+THCTRL.Base1],rax
add rax,VECTOR_BRIEF_DATA_SIZE / 2
mov [rbx+THCTRL.Base2],rax
lea r8,[VectorLoad]                       ; FIX THIS, USE Base3
call SingleThreadBenchmark                ; Return RAX = dTSC per pattern
pop rdi rsi

.L12:
add rsi,18
stosq                                     ; Store dTSC per pattern
jmp .L10
.L11:

; Exit point if no errors, single-thread branch
clc
.ExitRestoreStack:
pop rbp rbp
mov rsp,rbp
ret


