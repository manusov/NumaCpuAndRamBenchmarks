;------------------------------------------------------------------------------;
; Measurement result message box, brief variant without drawings.              ;
; Can be used for debug, also for improve final product useability on          ;
; platforms with limited functionality                                         ;
; Procedure 3 of 3 =                                                           ;
;           VectorBriefStart, VectorBriefProgress, VectorBriefStop             ;
;                                                                              ;
; INPUT:  None at registers                                                    ;
;         Statical text strings used                                           ;
;         MPE control and status block used                                    ;
;                                                                              ;
; OUTPUT: None                                                                 ;
;         Registers save/destroy by Microsoft x64 calling convention           ;
;------------------------------------------------------------------------------;

VectorBriefStop:

push rbx rsi rdi
cld

; Parameters for text built, write first strings
lea rdi,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_TRANSIT]   ; RDI = Destination build
lea rbx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]   ; RBX = Test results
lea rcx,[VectorBriefControl]                        ; RCX = Control table

; CPU name
mov rsi,rbx
call StringWrite

; TSC clock
lea rsi,[MsgResultBrief]                ; RSI = String constants
call StringWrite
finit
fild qword [rbx+VECTORBRIEF.TscClock]   ; TSC frequency at Hz, long integer
fld [DividendNs]
fdiv st0,st1
fstp [rbx+VECTORBRIEF.TscClockNs]       ; TSC period at nanoseconds, double 
fdiv [DivisorMHz]
fstp qword [rdi]
mov rax,[rdi]
mov bx,0200h
call DoublePrint 

; CPUID features
lea rbx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]   ; RBX = Test results
call StringWrite
.L0:
call StringWrite
push rsi
mov al,[rcx]
mov ah,al
and al,00111111b
shr ah,6
mov rdx,[rbx+VECTORBRIEF.StdFeatures]
cmp ah,1
je .L02
mov rdx,[rbx+VECTORBRIEF.ExtFeatures]
cmp ah,2
je .L02
mov rdx,[rbx+VECTORBRIEF.AddFeatures] 
.L02:
movzx rax,al
bt rdx,rax
setc al
mov ah,1
lea rsi,[Nsup]
call StringWriteSelected
inc rcx
pop rsi
cmp byte [rsi],0  ; Check strings sequence terminator byte
jne .L0           ; Cycle for CPUID features strings

; OS context features
inc rsi
call StringWrite
.L1:
call StringWrite
push rsi
mov al,[rcx]
and eax,00111111b
mov rdx,[rbx+VECTORBRIEF.OsContext]
bt rdx,rax
setc al
mov ah,1
lea rsi,[Nsup]
call StringWriteSelected
inc rcx
pop rsi
cmp byte [rsi],0  ; Check strings sequence terminator byte
jne .L1           ; Cycle for OS context features strings

; Instructions timings
inc rsi
call StringWrite
xor r8d,r8d

.L2:
lea rbx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]
mov al,[rcx+16]  ; AL = CPUID validator
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
lea rax,[NsupCPU]
jnc .L16         ; Go skip if routine not supported by CPU

.L14:
mov al,[rcx+17]  ; AL = OS context validator
test al,0C0h
jz .L15          ; Go execute if checks not required
mov rdx,[rbx+VECTORBRIEF.OsContext]
and eax,03Fh
bt rdx,rax
jc .L15          ; Go execute if routine supported by OS context
lea rax,[NsupOS]

.L16:
push rax
call StringWrite
pop rax
push rsi
xchg rsi,rax
call StringWrite
pop rsi
jmp .L12

.L15:

; This point if result valid, pattern supported and executed
lea rbx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_RESULTS]   ; RBX = Test results
call StringWrite
mov eax,[rcx+08]       ; Instructions count, 32-bit
mov edx,[rcx+12]       ; Measurement count, 32-bit
mul rdx                ; RAX = 64-bit product
push rax
push qword [rbx+VECTORBRIEF.dtSse128read+r8]
fild qword [rsp+08]    ; st0 = instructions count
fild qword [rsp]       ; st0 = clocks count , st1 = instructions count
fdiv st0,st1
fst qword [rsp]
fmul [rbx+VECTORBRIEF.TscClockNs]
fstp qword [rsp+08]
ffree st0
fincstp
pop rax
mov bx,0300h
call DoublePrint
mov ax,' ('
stosw
pop rax
call DoublePrint
mov eax,' ns)'
stosd

.L12:
add rcx,18
add r8,8
cmp byte [rsi],0  ; Check strings sequence terminator byte
jne .L2           ; Cycle for instructions timings strings

; Text termination
mov al,0
stosb 

; Other parameters
xor ecx,ecx           ; RCX = Parm#1 = Parent window handle or 0
lea rdx,[TEMP_BUFFER + VECTOR_BRIEF_TEMP_TRANSIT]  ; RDX = Parm#2 = Pointer st.
lea r8,[PRODUCT_ID]   ; R8  = Parm#3 = Pointer to caption
xor r9d,r9d           ; R9  = Parm#4 = Message box icon type

; Message box and wait user input
mov rbx,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
call [MessageBoxA]
mov rsp,rbx

; Exit
pop rdi rsi rbx
ret
