;--- Stop measurement and drawings session, release resources -----------------;
;    used for draw performance = f( block size ) function.                     ;
;    Include terminate  DrawSessionThread  subroutine as parallel thread       ;
;                                                                              ;
; INPUT:   None at CPU registers                                               ;
;                                                                              ;
; OUTPUT:  CF = status flag, 0(NC)=No errors, 1(C)=Error                       ;  
;                                                                              ;
; All CPU registers can be corrupted                                           ;
;------------------------------------------------------------------------------;

; TODO.
; Required input-output parameters detalization and error type status definition.

DrawSessionStop:

call SessionStop    ; De-initializing, release events, threads, memory
jc .Exit            ; Go if error returned, AL = Error code

; Align stack and create parameters shadow, required for use WinAPI
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32

; Initializing pointer
lea rsi,[DrawThreadParms]
cld

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
.Exit:
ret

; Exit point for errors detected
.Error:
stc
jmp .L90
