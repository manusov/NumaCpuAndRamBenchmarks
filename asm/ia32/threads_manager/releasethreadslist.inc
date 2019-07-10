;---------- Close handles at threads list: events and threads -----------------;
;           Before close handles, release threads                              ;
;                                                                              ;
; INPUT:   ECX = Pointer to Threads Control List                               ;
;          EDX = Number of threads                                             ;
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;                                                                              ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.        ;
;------------------------------------------------------------------------------;

ReleaseThreadsList:
cld
push ebx esi edi ebp 
; Reload input parameters to non-volatile registers
mov esi,ecx                    ; ESI = Pointer to Threads list
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
mov ecx,[esi + THCTRL.ThreadHandle]  ; ECX = THREAD handle
jecxz @f                             ; Skip if this entry not allocated
push 0                               ; Parm#2 = Thread exit code
push ecx                             ; Parm#1 = THREAD handle 
call [TerminateThread]               ; Call WinAPI function
test eax,eax                         ; Status=0 means error
jz .ReleaseError                     ; Go exit if error
@@:
; Close Start Events
mov ecx,[esi + THCTRL.EventStart]    ; ECX = Load EVENT handle
jecxz @f                             ; Skip if this entry not allocated
push ecx                             ; Parm#1 = EVENT handle 
call [CloseHandle]                   ; Call WinAPI function
test eax,eax                         ; Status=0 means error
jz .ReleaseError                     ; Go exit if error 
@@:
; Close Done Events
mov ecx,[esi + THCTRL.EventDone]     ; ECX = Load EVENT handle
jecxz @f                             ; Skip if this entry not allocated
push ecx                             ; Parm#1 = EVENT handle
call [CloseHandle]                   ; Call WinAPI function
test eax,eax                         ; Status=0 means error
jz .ReleaseError                     ; Go exit if error 
@@:
; Close Threads
mov ecx,[esi + THCTRL.ThreadHandle]  ; ECX = Load THREAD handle
jecxz @f                             ; Skip if this entry not allocated
push ecx                             ; Parm#1 = EVENT handle
call [CloseHandle]                   ; Call WinAPI function
test eax,eax                         ; Status=0 means error
jz .ReleaseError                     ; Go exit if error
@@:
; Cycle for release threads
dec edi                             ; Count threads entries, for used entries
jz .ThreadsDone                     ; Go exit if used threads done before limit
; Limit cycle for maximum number of threads
add esi,THCTRL_SIZE                 ; Select next thread entry
dec ebx                             ; Count threads entries, for limit
jnz .ReleaseThreads                 ; Cycle for all entries

; Exit points
.ThreadsDone:
clc                  ; CF=0 means status = no errors 
.ExitRelease:
pop ebp edi esi ebx
ret
.ReleaseError:
stc                  ; CF=1 means status = error detected
jmp .ExitRelease

