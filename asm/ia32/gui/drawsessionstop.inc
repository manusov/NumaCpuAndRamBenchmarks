;--- Stop measurement and drawings session, release resources ---------------;
;    used for draw performance = f( block size ) function.                   ;
;    Include terminate  DrawSessionThread  subroutine as parallel thread     ;
;                                                                            ;
; INPUT:   None at CPU registers                                             ;
;                                                                            ;
; OUTPUT:  CF = status flag, 0(NC)=No errors, 1(C)=Error                     ;
;          AL = error status, valid only if error detected (when CF=1)       ; 
;               see file "thread manager\sessionstop.inc" for decode         ;   
;                                                                            ;
; All CPU registers can be corrupted, this rule for complex modules:         ;
; directory threads_manager\:                                                ;
;   sessionstart.inc, sessionstop.inc, sessionprogress.inc.                  ;
; directory gui\:                                                            ;
;   drawsessionstart.inc, drawsessionstop.inc                                ;
;   but not drawsessionthread.inc because OS callback                        ;       
; Remember about PUSH-POP in the callers.                                    ;
;----------------------------------------------------------------------------;

DrawSessionStop:

call SessionStop    ; De-initializing, release events, threads, memory
jc .Exit            ; Go if error returned, AL = Error code

; Initializing pointer
lea esi,[DrawThreadParms]
cld

; Close handles, halt and remove thread with DrawSessionThread  subroutine
; Terminate threads
mov ecx,[esi + DTHP.ThreadHandle]   ; ECX = THREAD handle
jecxz @f                            ; Skip if this entry not allocated
push 0                              ; Parm#2 = Thread exit code
push ecx                            ; Parm#1 = THREAD handle 
call [TerminateThread]              ; Call WinAPI function
test eax,eax                        ; Status=0 means error
jz .Error                           ; Go exit if error
@@:
; Close Start Events
lodsd                               ; EAX = Load EVENT handle
test eax,eax
jz @f                               ; Skip if this entry not allocated
push eax                            ; Parm#1 = EVENT handle
call [CloseHandle]                  ; Call WinAPI function
test eax,eax                        ; Status=0 means error
jz .Error                           ; Go exit if error 
@@:
; Close Done Events
lodsd                               ; EAX = Load EVENT handle
test eax,eax
jz @f                               ; Skip if this entry not allocated
push eax                            ; Parm#1 = EVENT handle 
call [CloseHandle]                  ; Call WinAPI function
test eax,eax                        ; Status=0 means error
jz .Error                           ; Go exit if error 
@@:
; Close Threads
lodsd                               ; EAX = Load THREAD handle 
test eax,eax
jz @f                               ; Skip if this entry not allocated
push eax                            ; Parm#1 = THREAD handle
call [CloseHandle]                  ; Call WinAPI function
test eax,eax                        ; Status=0 means error
jz .Error                           ; Go exit if error
@@:

; Done, parallel thread with  DrawSessionThread  subroutine HALTED and REMOVED
clc
.L90:
.Exit:                 ; Note AL = error status must be valid at this point
ret

; Exit point for errors detected
.Error:
stc
jmp .L90


