;---------- Memory performance measurement (Single thread) --------------------;

;---------- Open single-thread session, include allocate memory ---------------;
; INPUT:   RBX = Pointer to Control Entry                                      ;
;          First entry of Threads Control List contains operational data       ;   
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          Registers corrupted, except R15 = saved                             ;
;          First entry of Threads Control List updated                         ;    
;------------------------------------------------------------------------------;
SingleThreadOpen:

;- yet locked support NUMA for single-thread -
call MemoryAllocation

;- yet locked support NUMA for single-thread - 
;- (!) note registers corruption when unlock: RBX, RSI plus -
; mov [rbx+THCTRL.AffinityMode],0
; mov al,[InputParms.OptionNUMA]
; cmp al,2
; jb .L0
; cmp al,3
; ja .L0
; This branch for NUMA mode
; mov ecx,1                      ; Threads count = 1
; push rbx rsi
; call MultiThreadOpen
; pop rsi rbx  
; jmp .L1
; .L0:
; This branch for non-NUMA mode
; call MemoryAllocation 
; .L1:
;- end of locked -

ret

;---------- Close single-thread session, include release memory ---------------;
; INPUT:   RBX = Pointer to Control Entry                                      ;   
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          Registers corrupted, except R15 = saved                             ;
;------------------------------------------------------------------------------;
SingleThreadClose:

;- yet locked support NUMA for single-thread -
call MemoryRelease

;- yet locked support NUMA for single-thread - 
;- (!) note registers corruption when unlock: RBX, RSI plus -
; mov al,[InputParms.OptionNUMA]
; cmp al,2
; jb .L0
; cmp al,3
; ja .L0
; This branch for NUMA mode
; mov ecx,1                      ; Threads count = 1
; push rbx rsi
; call MultiThreadClose
; pop rsi rbx  
; jmp .L1
; .L0:
; This branch for non-NUMA mode
; call MemoryRelease 
; .L1:
;- end of locked -

ret

;---------- Call benchmark target subroutine ----------------------------------;
; INPUT:   RBX = Pointer to Control Entry                                      ; 
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          RAX = Delta TSC                                                     ;  
;          Registers corrupted, except R15 = saved                             ;
;------------------------------------------------------------------------------;
SingleThreadBenchmark:   ; This entry point for single-thread benchmark mode

;- yet locked support NUMA for single-thread -
; mov rax,[rbx+THCTRL.AffinityMode]
; cmp eax,1
; je SingleThreadAffinity        ; Go single thread affinitization for <=64 CPUs
; cmp eax,2
; je SingleThreadAffinityGroups  ; Go single thread affinitization for >64 CPUs
;- end of locked -  

; Load input parameters, part 1
mov rsi,[rbx+THCTRL.Base1]       ; RSI = Block #1 pointer (64-bit flat)
mov rdi,[rbx+THCTRL.Base2]       ; RDI = Block #1 pointer (64-bit flat)
mov rbp,[rbx+THCTRL.Repeats]     ; RBP = Number of measurement repeats
mov r14,rbx
; Serialization, note EAX,EBX,ECX,EDX changed
xor eax,eax        ; EAX=0 means function 0 for CPUID
cpuid              ; This CPUID for serialization only, results ignored
; Load input parameters, part 2
mov rcx,[r14+THCTRL.SizeInst]    ; RCX = Block length, units = instructions 
mov rbx,[r14+THCTRL.EntryPoint]  ; RBX = Target subroutine address
; Get start time
rdtsc              ; EDX:EAX = TSC value at start: EDX=High, EAX=Low
shl rdx,32         ; Positioning TSC high 32 bits to RDX.[63-32]
lea r14,[rax+rdx]  ; R14 = Interval start TSC value, integral 64 bits
; Call target routine (required not destroy R14, R15)
call rbx           ; Run target operation
; Get end time
rdtsc              ; EDX:EAX = TSC value at end: EDX=High, EAX=Low
shl rdx,32         ; Positioning TSC high 32 bits to RDX.[63-32]
or rax,rdx         ; RAX = Interval end TSC value, integral 64 bits
sub rax,r14        ; RAX = Delta TSC value
push rax
xor eax,eax        ; EAX=0 means function 0 for CPUID
cpuid              ; This CPUID for events serialization only, results ignored
pop rax
; Return, RAX = delta TSC
ret

;- yet locked support NUMA for single-thread -
; SingleThreadAffinity:
; Set affinity mask for this thread
; mov rcx,[rbx+THCTRL.ThreadHandle]     ; Parm#1 = RCX = Thread handle
; mov rdx,[rbx+THCTRL.ThreadAffinity]   ; Parm#2 = RDX = Thread Affinity mask
; mov rax,rdx
; test rdx,rdx
; jz @f                ; Skip set affinity mask if memory allocated without NUMA
; call [_SetThreadAffinityMask]
; @@:
; Store current affinity mask
; mov [rbx+THCTRL.OrigAffinity],rax
; Target memory operation, save RAX = delta TSC
; push rbx
; call SingleThreadBenchmark
; pop rbx
; push rax
; Restore original affinity mask
; mov rcx,[rbx+THCTRL.ThreadHandle]     ; Parm#1 = RCX = Thread handle
; mov rdx,[rbx+THCTRL.OrigAffinity]   ; Parm#2 = RDX = Thread Affinity mask
; test rdx,rdx
; jz @f                ; Skip set affinity mask if memory allocated without NUMA
; call [_SetThreadAffinityMask]
; @@:
; Return, restore RAX = delta TSC
; pop rax
; ret
;- end of locked -

;- yet locked support NUMA for single-thread -
; SingleThreadAffinityGroups:
; Set affinity mask for this thread, store original mask
; mov rcx,[rbx+THCTRL.ThreadHandle]     ; Parm#1 = RCX = Thread handle
; lea rdx,[rbx+THCTRL.ThreadAffinity]   ; Parm#2 = RDX = Pointer to mask + group
; lea r8,[rbx+THCTRL.OrigAffinity]      ; Parm#3 = R8 = Pointer to get original
; xor eax,eax
; mov [r8],rax                          ; Pre-blank original affinity
; cmp [rdx],rax
; je @f                ; Skip set affinity mask if memory allocated without NUMA
; call [_SetThreadGroupAffinity]
; @@:
; Target memory operation, save RAX = delta TSC
; push rbx
; call SingleThreadBenchmark
; pop rbx
; push rax
; Restore original affinity mask
; mov rcx,[rbx+THCTRL.ThreadHandle]     ; Parm#1 = RCX = Thread handle
; lea rdx,[rbx+THCTRL.OrigAffinity]     ; Parm#2 = RDX = Pointer to mask + group
; lea r8,[rbx+THCTRL.ThreadAffinity]    ; Parm#3 = R8 = Pointer to get original
; cmp qword [rdx],0
; je @f                ; Skip set affinity mask if memory allocated without NUMA
; call [_SetThreadGroupAffinity]
; @@:
; Return, restore RAX = delta TSC
; pop rax
; ret
;- end of locked -

