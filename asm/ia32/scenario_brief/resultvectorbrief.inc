;--- Build text data for measurement result message box, ----------------------; 
; brief variant without drawings.                                              ;
; Can be used for debug, also for improve final product useability on          ;
; platforms with limited functionality                                         ;
;                                                                              ;
; INPUT:  None at registers                                                    ;
;         Statical text strings used                                           ;
;         MPE control and status block used                                    ;
;                                                                              ;
; OUTPUT: None                                                                 ;
;         Registers save/destroy by Microsoft ia32 calling convention          ;
;------------------------------------------------------------------------------;

ResultVectorBrief:

push ebx esi edi ebp
cld

; Parameters for text build, write first strings
lea edi,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_TRANSIT]   ; EDI = Destination build
lea ebx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]   ; EBX = Test results
lea ecx,[VectorBriefControl]                        ; ECX = Control table

; CPU name, already build by subroutine RunVectorBrief
mov esi,ebx
call StringWrite

; TSC clock, already build by subroutine RunVectorBrief
lea esi,[MsgResultBrief]                            ; ESI = String constants
call StringWrite_CRLF_Equal
mov eax,dword [ebx + VECTORBRIEF.TscClockMHzF + 0]
mov edx,dword [ebx + VECTORBRIEF.TscClockMHzF + 4]
mov bx,0200h
call DoublePrint 

; CPUID features, already build by subroutine RunVectorBrief
lea ebx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]   ; EBX = Test results
call StringWrite_CRLF_Colon
.L0:
call StringWrite_CRLF_Colon
push esi
mov al,[ecx]
mov ah,al
and al,00111111b
shr ah,6
lea esi,[ebx + VECTORBRIEF.StdFeatures]
cmp ah,1
je .L02
lea esi,[ebx + VECTORBRIEF.ExtFeatures]
cmp ah,2
je .L02
lea esi,[ebx + VECTORBRIEF.AddFeatures] 
.L02:
mov edx,[esi]
test al,00100000b
jz .L01
mov edx,[esi+4]
.L01:
and eax,00011111b
bt edx,eax
setc al
mov ah,1
lea esi,[Nsup]
call StringWriteSelected
inc ecx
pop esi
cmp byte [esi],0  ; Check strings sequence terminator byte
jne .L0           ; Cycle for CPUID features strings

; OS context features
inc esi
call StringWrite_CRLF_Colon
.L1:
call StringWrite_CRLF_Colon
push esi
mov al,[ecx]
lea esi,[ebx + VECTORBRIEF.OsContext]
mov edx,[esi]
test al,00100000b
jz .L11
mov edx,[esi+4]
.L11:
and eax,00011111b
bt edx,eax
setc al
mov ah,1
lea esi,[Nsup]
call StringWriteSelected
inc ecx
pop esi
cmp byte [esi],0  ; Check strings sequence terminator byte
jne .L1           ; Cycle for OS context features strings

; Instructions timings
inc esi
call StringWrite_CRLF_Colon
xor ebp,ebp

.L2:
lea ebx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]
mov al,[ecx+3]   ; AL = CPUID validator
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
jnz .L17
mov edx,[edx]
jmp .L18
.L17:
mov edx,[edx+4]
.L18:
and eax,00011111b
bt edx,eax
lea eax,[NsupCPU]
jnc .L16           ; Go skip if routine not supported by CPU

.L14:
mov al,[ecx+4]     ; AL = OS context validator
test al,11000000b
jz .L15            ; Go execute if checks not required
lea edx,[ebx + VECTORBRIEF.OsContext]
test al,00100000b
jnz .L27
mov edx,[edx]
jmp .L28
.L27:
mov edx,[edx+4]
.L28:
and eax,00011111b
bt edx,eax
jc .L15          ; Go execute if routine supported by OS context
lea eax,[NsupOS]

; This point if result NOT valid, pattern NOT supported and NOT executed
.L16:
push eax
call StringWrite_CRLF_Colon
pop eax
push esi
xchg esi,eax
call StringWrite
pop esi
jmp .L12

; This point if result valid, pattern supported and executed
.L15:
lea ebx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]   ; RBX = Test results
call StringWrite_CRLF_Colon
push ecx
mov cl,[ecx+01]               ; Instructions count, 32-bit
mov eax,1
shl eax,cl
pop ecx
movzx edx,byte [ecx + 02]     ; Measurement count, 32-bit
imul edx,edx,REPEATS_SCALE
mul edx                       ; EDX:EAX = 64-bit product
push edx eax
mov eax,ebp
push dword [ebx + VECTORBRIEF.dtSse128read + eax + 4]
push dword [ebx + VECTORBRIEF.dtSse128read + eax + 0]
fild qword [esp + 08]  ; st0 = instructions count
fild qword [esp]       ; st0 = clocks count , st1 = instructions count
fdiv st0,st1           ; st0 = clocks per instruction (CPI)
fst qword [esp + 00]                   ; store CPI
fmul [ebx + VECTORBRIEF.TscClockNsF]   ; st0 = nanoseconds per instruction (nsPI)
fstp qword [esp + 08]                  ; store nsPI
ffree st0
fincstp
pop eax edx        ; pop CPI
mov bx,0300h
call DoublePrint   ; print CPI
mov ax,' ('
stosw
pop eax edx        ; pop nsPI
call DoublePrint   ; print nsPI, " ( x ns ) "
mov eax,' ns)'
stosd

.L12:
add ecx,5
add ebp,8
cmp byte [esi],0  ; Check strings sequence terminator byte
jne .L2           ; Cycle for instructions timings strings

; Text termination
mov al,0
stosb 

; Exit
pop ebp edi esi ebx
ret
