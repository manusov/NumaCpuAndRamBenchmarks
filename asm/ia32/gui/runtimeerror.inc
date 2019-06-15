;--- Output error reporting GUI window, used for -------------------; 
;    "Vector brief", "Run simple", "Run drawings".                  ;
;                                                                   ;
; INPUT:   AL = Error code                                          ;
; OUTPUT:  None                                                     ;
; Registers destroy by Microsoft ia32 calling convention            ;  
;-------------------------------------------------------------------;

RuntimeError:
lea ecx,[RunErrorMemoryA]  ; Parm#2, pointer to message string #0
cmp al,0                   ; AL = 0 means memory allocation error
je @f
lea ecx,[RunErrorMemoryR]  ; Parm#2, pointer to message string #1
cmp al,1                   ; AL = 1 means memory release error 
je @f
lea ecx,[RunErrorTimings]  ; Parm#2, pointer to message string #2
cmp al,2                   ; AL = 2 means timings measurement error
je @f                      ; Otherwise address arithmetic error
lea ecx,[RunErrorArithm]   ; Parm#2, pointer to message string #3
@@:  
push MB_ICONERROR    ; Parm#4, Message Box Icon Error = MB_ICNERROR
push 0               ; Parm#3, Caption=0=error message, otherwise pointer to caption
push ecx             ; Parm#2, pointer to message string #3
push 0               ; Parm#1, parent window handle = none
call [MessageBoxA]
ret
