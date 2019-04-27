
DrawSessionStart:

call GuiToUserParms
call SessionStart

cld
lea rbx,[InputParms]
lea r14,[DialogueContent_Win1]
lea r15,[BasePointGui]

; Down comments strings, assembler method
lea rsi,[AM_Strings_Prefix]
mov ecx,[rbx + IPB.UpdatedAsm]
cmp ecx,ASM_LIMIT
jae .L10 
lea rsi,[AM_Strings_Values]
jrcxz .L10
.L11:
lodsb
cmp al,0
jne .L11
loop .L11
.L10:
sub rsi,r15
mov [r14 + HANDLE_TEXT_METHOD1*16 - 256 + 12],si

; Down comments strings, target object (Cache, DRAM, Custom block)
lea rsi,[Unknown_Object]
mov ecx,[rbx + IPB.UpdatedTarget]
cmp ecx,TARGET_LIMIT
jae .L12 
lea rsi,[Target_00]
jrcxz .L12
.L13:
lodsb
cmp al,0
jne .L13
loop .L13
.L12:
sub rsi,r15
mov [r14 + HANDLE_TEXT_OBJECT1*16 - 256 + 12],si

; Down comments strings, threads number
lea rdi,[Threads_Number]
mov word [rdi],0000h + '?'
mov eax,[rbx + IPB.UpdatedThreads]
cmp eax,MAX_THREADS
ja .L14
push rbx    ; TODO> RBX can be non-optimal because this PUSH/POP requirement
mov bl,0
call DecimalPrint32
pop rbx
.L14:

; Down comments strings, number of measurement repeats
lea rdi,[Repeats_Number]
push rdi 
mov ecx,12
mov al,' '
rep stosb
pop rdi 
mov eax,[rbx + IPB.MeasureRepeats]
push rbx    ; TODO> RBX can be non-optimal because this PUSH/POP requirement
mov bl,0
call DecimalPrint32
pop rbx

; Down comments strings, normal or large pages
lea rax,[NPages_String]
cmp [rbx + IPB.UpdatedLP],LP_USED
jne .L15 
lea rax,[LPages_String]
.L15:
sub rax,r15
mov [r14 + HANDLE_LARGE_PAGES1*16 - 256 + 12],ax

; Down comments strings, NUMA optimization mode
lea rsi,[NumaOff_String]
mov ecx,[rbx + IPB.UpdatedNUMA]
cmp ecx,NUMA_LIMIT
jae .L16 
; lea rsi,[NumaU_String]
jrcxz .L16
.L17:
lodsb
cmp al,0
jne .L17
loop .L17
.L16:
sub rsi,r15
mov [r14 + HANDLE_NUMA_MODE1*16 - 256 + 12],si

; TODO. Optimize RSI/RBX usage.

lea rsi,[InputParms]
lea rdi,[DrawParms]

; Calculate X grid step values, must be integer power of 2 for next step correct 
mov rax,[rsi + IPB.StartBlockSize]
bsr rcx,rax
mov eax,1
shl eax,cl

; Special support for DRAM and Custom modes
; TODO. Make this at block setup, for detect mode single time ?
mov ecx,[rsi + IPB.UpdatedTarget]
cmp ecx,TARGET_DRAM
je .L1
cmp ecx,TARGET_CUSTOM
jne .L2
.L1:
shr eax,3
.L2:

; Update and store X grid step values
; Select and store X grid units: Bytes, Kilobytes, Megabytes
xor edx,edx                ; EDX = SelectUnits, 0=Bytes / 1=KB / 2=MB
cmp eax,1024
jb @f                      ; Go with Units=Bytes if grid step < 1 KB  
inc edx
shr eax,10
cmp eax,1024
jb @f                      ; Go with Units=KB if grid step < 1 MB
inc edx
shr eax,10
@@:                        ; Otherwise Units=MB
mov [rdi + DRPM.ValueGridX],eax
mov [rdi + DRPM.SelectUnits],edx

; Set first approximation constant for Y grid step values
mov eax,DEFAULT_Y_MBPS_PER_GRID
xor edx,edx
cmp [rsi + IPB.UpdatedAsm],LATENCY_MODE
jb @f
mov eax,DEFAULT_Y_NS_PER_GRID
inc edx
@@:
mov [rdi + DRPM.ValueGridY],eax
mov [rdi + DRPM.SelectMode],edx
ret