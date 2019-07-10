;---------- Close handles at threads list: events and threads -----------------;
;           Before close handles, release threads                              ;
;                                                                              ;
; INPUT:   RCX = Pointer to Threads Control List                               ;
;          EDX = Number of threads                                             ;
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;                                                                              ;
; Can destroy registers, volatile by Microsoft x64 calling convention.         ;
;------------------------------------------------------------------------------;

ReleaseThreadsList:
cld
push rbx rsi rdi rbp 
mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
sub rsp,32                     ; Build parameters shadow because WinAPI used
; Reload input parameters to non-volatile registers
mov rsi,rcx                    ; RSI = Pointer to Threads list
mov edi,edx                    ; EDI = Number of used Threads
; Set nodes list limit
mov ebx,MAX_THREADS            ; EBX = Maximum number of Threads

; Terminate threads routines
call ThreadsReStartBreak
; Delay for return from thread routine
call DelayThreadReturn

; Cycle for release threads
.ReleaseThreads:
; Terminate threads
mov rcx,[rsi + THCTRL.ThreadHandle]  ; RCX = Parm#1 = THREAD handle
jrcxz @f                             ; Skip if this entry not allocated
xor edx,edx                          ; RDX = Parm#2 = Thread exit code
call [TerminateThread]               ; Call WinAPI function
test rax,rax                         ; Status=0 means error
jz .ReleaseError                     ; Go exit if error
@@:
; Close Start Events
mov rcx,[rsi + THCTRL.EventStart]    ; RCX = Parm#1 = Load EVENT handle
jrcxz @f                             ; Skip if this entry not allocated
call [CloseHandle]                   ; Call WinAPI function
test rax,rax                         ; Status=0 means error
jz .ReleaseError                     ; Go exit if error 
@@:
; Close Done Events
mov rcx,[rsi + THCTRL.EventDone]     ; RCX = Parm#1 = Load EVENT handle
jrcxz @f                             ; Skip if this entry not allocated
call [CloseHandle]                   ; Call WinAPI function
test rax,rax                         ; Status=0 means error
jz .ReleaseError                     ; Go exit if error 
@@:
; Close Threads
mov rcx,[rsi + THCTRL.ThreadHandle]  ; RCX = Parm#1 = Load THREAD handle
jrcxz @f                             ; Skip if this entry not allocated
call [CloseHandle]                   ; Call WinAPI function
test rax,rax                         ; Status=0 means error
jz .ReleaseError                     ; Go exit if error
@@:
dec edi                             ; Count threads entries, for used entries
jz .ThreadsDone                     ; Go exit if used threads done before limit
; Cycle for release threads
add rsi,THCTRL_SIZE                 ; Select next thread entry
dec ebx                             ; Count threads entries, for limit
jnz .ReleaseThreads                 ; Cycle for all entries

; Exit points
.ThreadsDone:
clc                  ; CF=0 means status = no errors 
.ExitRelease:
mov rsp,rbp          ; Restore RSP at subroutine entry, because stack alignment
pop rbp rdi rsi rbx
ret
.ReleaseError:
stc                  ; CF=1 means status = error detected
jmp .ExitRelease

