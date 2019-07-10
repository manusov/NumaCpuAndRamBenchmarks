;---------- Thread routine, called by OS callback mechanism -------------------;
; Variant without support Processor Groups, systems with <= 64 logical CPUs    ;
;                                                                              ;
; INPUT:   RCX = Parm#1 = Thread ID, 0-63                                      ;
; OUTPUT:  None, because no return                                             ;
;------------------------------------------------------------------------------;

ThreadRoutineSingleGroup:
push rbx rsi rdi rbp r12 r13 r14 r15
mov rax,rsp
and rsp,0FFFFFFFFFFFFFFF0h
push rax
sub rsp,32 + 8                ; Build parameters shadow, 4 qwords + alignment

; Calculate thread control entry offset = f(thread id)
cmp ecx,MAX_THREADS_PER_GROUP
jb @f
xor ecx,ecx                   ; Force thread 0 if bad thread number
@@:
imul ecx,ecx,THCTRL_SIZE
lea r15,[ThreadsList + rcx]   ; R15 = Address for block for this thread

; This point for restart thread
.ThreadRepeat:
mov rbx,r15
; Set affinity mask for this thread
mov rcx,[rbx + THCTRL.ThreadHandle]     ; Parm#1 = RCX = Thread handle
mov rdx,[rbx + THCTRL.ThreadAffinity]   ; Parm#2 = RDX = Thread Affinity mask
test rdx,rdx    ; Check for affinity mask = 0
jz @f           ; Skip set affinity mask if memory allocated without NUMA
call [_SetThreadAffinityMask]
@@:

; Target memory performance operation
call RunTarget

; Pre-clear thread run event
mov rcx,[r15 + THCTRL.EventStart]  ; RCX = Parm#1 = Handle for this thread
call [ResetEvent]                  ; Call WinAPI function
; Signaling thread terminated
mov rcx,[r15 + THCTRL.EventDone]   ; RCX = Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function
; Wait for thread re-run
mov rcx,[r15 + THCTRL.EventStart]  ; RCX = Parm#1 = Handle for this thread
mov edx,-1                         ; RDX = Parm#2 = Wait infinite, no timeout
call [WaitForSingleObject]

; Repeat measurement iteration in this thread
;- jmp .ThreadRepeat
cmp [PrimaryChildsBreak],0
je .ThreadRepeat

; Signaling thread terminated
mov rcx,[r15 + THCTRL.EventDone]   ; RCX = Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function

; Return from thread to OS caller
add rsp,32 + 8
pop rax
mov rsp,rax
pop r15 r14 r13 r12 rbp rdi rsi rbx
xor eax,eax
ret


;---------- Thread routine, called by OS callback mechanism -------------------;
; Variant with support Processor Groups, systems with >64 logical CPUs         ;
;                                                                              ;
; INPUT:   RCX = Parm#1 = Thread ID, 0-255                                     ;
; OUTPUT:  None, because no return                                             ;
;------------------------------------------------------------------------------;

ThreadRoutineMultiGroup:
push rbx rsi rdi rbp r12 r13 r14 r15
mov rax,rsp
and rsp,0FFFFFFFFFFFFFFF0h
push rax
sub rsp,32 + 8                ; Build parameters shadow, 4 qwords + alignment

; Calculate thread control entry offset = f(thread id)
cmp ecx,MAX_THREADS
jb @f
xor ecx,ecx                   ; Force thread 0 if bad thread number
@@:
imul ecx,ecx,THCTRL_SIZE
lea r15,[ThreadsList + rcx]   ; R15 = Address for block for this thread

; This point for restart thread
.ThreadRepeat:
mov rbx,r15
; Set affinity mask for this thread
mov rcx,[rbx + THCTRL.ThreadHandle]     ; Parm#1 = RCX = Thread handle
lea rdx,[rbx + THCTRL.ThreadAffinity]   ; Parm#2 = RDX = Pointer to mask + group
lea r8,[rbx + THCTRL.OrigAffinity]      ; Parm#3 = R8 = Pointer to get original
cmp qword [rdx],0    ; Check for affinity mask = 0
je @f                ; Skip set affinity mask if memory allocated without NUMA
call [_SetThreadGroupAffinity]      ; This changed for Processor Groups
@@:

; Target memory performance operation
call RunTarget

; Pre-clear thread run event
mov rcx,[r15 + THCTRL.EventStart]  ; RCX = Parm#1 = Handle for this thread
call [ResetEvent]                  ; Call WinAPI function
; Signaling thread terminated
mov rcx,[r15 + THCTRL.EventDone]   ; RCX = Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function
; Wait for thread re-run
mov rcx,[r15 + THCTRL.EventStart]  ; RCX = Parm#1 = Handle for this thread
mov edx,-1                         ; RDX = Parm#2 = Wait infinite, no timeout
call [WaitForSingleObject]

; Repeat measurement iteration in this thread
;- jmp .ThreadRepeat
cmp [PrimaryChildsBreak],0
je .ThreadRepeat

; Signaling thread terminated
mov rcx,[r15 + THCTRL.EventDone]   ; RCX = Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function

; Return from thread to OS caller
add rsp,32 + 8
pop rax
mov rsp,rax
pop r15 r14 r13 r12 rbp rdi rsi rbx
xor eax,eax
ret


