
; INT3
; cld
; lea rbx,[TEMP_BUFFER]
; mov rdi,rbx
; mov ecx,TEMP_BUFFER_SIZE
; mov al,11h
; rep stosb
; xor ecx,ecx  ; RCX = node 0
; mov rdx,rbx  ; RDX = pointer to buffer
; call [_GetNumaNodeProcessorMaskEx]
; INT3
; call [GetCurrentThread]
; mov rcx,rax
; mov rdx,rbx
; lea r8,[rbx+32]
; call [_SetThreadGroupAffinity]
; nop
; nop
; nop

; INT3
; mov eax,NUMACTRL_SIZE
; mov ebx,THCTRL_SIZE

; INT3
; mov [ProcessorGroups],0

; INT3
; cld
; lea rbx,[TEMP_BUFFER]
; mov rdi,rbx
; mov ecx,TEMP_BUFFER_SIZE
; mov al,11h
; rep stosb
; call BuildNumaList

