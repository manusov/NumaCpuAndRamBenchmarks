;-------------------------------------------------------------------;
; Simple Benchmarks Execution.                                      ;
;                                                                   ;
; INPUT:   Control context initialized by options settings.         ;   
; OUTPUT:  None                                                     ;
;                                                                   ;
;-------------------------------------------------------------------;

ExecuteSimple:

; Set benchmarks options = F (User dialogue results)
call SimpleStart
; Run benchmarks
call SimpleProgress
jc SimpleError
; Show benchmarks results
call SimpleStop
ret

; Error reporting branch, used also from drawings context
SimpleError:
xor ecx,ecx                ; Parm#1, parent window handle = none
lea rdx,[RunErrorMemoryA]  ; Parm#2, pointer to message string #0
cmp al,0
je @f
lea rdx,[RunErrorMemoryR]  ; Parm#2, pointer to message string #1
cmp al,1
je @f
lea rdx,[RunErrorTimings]  ; Parm#2, pointer to message string #2
cmp al,2
je @f
lea rdx,[RunErrorArithm]   ; Parm#2, pointer to message string #3
@@:  
xor r8d,r8d                ; Parm#3, Caption=0=error message, otherwise pointer to caption
mov r9,MB_ICONERROR        ; Parm#4, Message Box Icon Error = MB_ICNERROR
call [MessageBoxA]
ret
