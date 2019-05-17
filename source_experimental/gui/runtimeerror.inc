;--- Output error reporting GUI window, used for -------------------; 
;    "Vector brief", "Run simple", "Run drawings".                  ;
;                                                                   ;
; INPUT:   AL = Error code                                          ;
; OUTPUT:  None                                                     ;
; Registers destroy by Microsoft x64 calling convention             ;  
;-------------------------------------------------------------------;

RuntimeError:
push rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32

xor ecx,ecx                ; Parm#1, parent window handle = none
lea rdx,[RunErrorMemoryA]  ; Parm#2, pointer to message string #0
cmp al,0                   ; AL = 0 means memory allocation error
je @f
lea rdx,[RunErrorMemoryR]  ; Parm#2, pointer to message string #1
cmp al,1                   ; AL = 1 means memory release error 
je @f
lea rdx,[RunErrorTimings]  ; Parm#2, pointer to message string #2
cmp al,2                   ; AL = 2 means timings measurement error
je @f                      ; Otherwise address arithmetic error
lea rdx,[RunErrorArithm]   ; Parm#2, pointer to message string #3
@@:  
xor r8d,r8d                ; Parm#3, Caption=0=error message, otherwise pointer to caption
mov r9d,MB_ICONERROR       ; Parm#4, Message Box Icon Error = MB_ICNERROR
call [MessageBoxA]

mov rsp,rbp
pop rbp
ret
