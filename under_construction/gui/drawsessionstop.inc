
DrawSessionStop:

call SessionStop


; Align stack and create parameters shadow, required for use WinAPI
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32

; Initializing pointer
lea rsi,[DrawThreadParms]
cld

; Wait for measurement thread done, this event at thread exit point
mov rcx,[rsi + DTHP.EventDone]   ; RCX = Parm#1 = Handle for done event
mov edx,10000                    ; RDX = Parm#2 = 10 seconds
call [WaitForSingleObject]
test rax,rax                     ; special encoding, 0 means event detected OK 
jnz .Error

; Close handles, halt and remove thread with DrawSessionThread  subroutine
; Terminate threads
mov rcx,[rsi + DTHP.ThreadHandle]   ; RCX = Parm#1 = THREAD handle
jrcxz @f                            ; Skip if this entry not allocated
xor edx,edx                         ; RDX = Parm#2 = Thread exit code
call [TerminateThread]              ; Call WinAPI function
test rax,rax                        ; Status=0 means error
jz .Error                           ; Go exit if error
@@:
; Close Start Events
lodsq
xchg rcx,rax                        ; RCX = Parm#1 = Load EVENT handle
jrcxz @f                            ; Skip if this entry not allocated
call [CloseHandle]                  ; Call WinAPI function
test rax,rax                        ; Status=0 means error
jz .Error                           ; Go exit if error 
@@:
; Close Done Events
lodsq
xchg rcx,rax                        ; RCX = Parm#1 = Load EVENT handle
jrcxz @f                            ; Skip if this entry not allocated
call [CloseHandle]                  ; Call WinAPI function
test rax,rax                        ; Status=0 means error
jz .Error                           ; Go exit if error 
@@:
; Close Threads
lodsq
xchg rcx,rax                        ; RCX = Parm#1 = Load THREAD handle
jrcxz @f                            ; Skip if this entry not allocated
call [CloseHandle]                  ; Call WinAPI function
test rax,rax                        ; Status=0 means error
jz .Error                           ; Go exit if error
@@:

; Done, parallel thread with  DrawSessionThread  subroutine HALTED and REMOVED
clc
.L90:
mov rsp,rbp
ret

; Exit point for errors detected
.Error:
stc
jmp .L90
