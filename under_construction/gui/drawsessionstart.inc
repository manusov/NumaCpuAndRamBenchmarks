
DrawSessionStart:
call GuiToUserParms
call SessionStart

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
cmp [rsi + IPB.UpdatedAsm],LATENCY_MODE
jb @f
mov eax,DEFAULT_Y_NS_PER_GRID
@@:
mov [rdi + DRPM.ValueGridY],eax


ret