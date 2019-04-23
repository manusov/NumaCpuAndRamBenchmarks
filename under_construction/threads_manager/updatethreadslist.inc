;---------- Update threads list, set new size at bytes and instructions -------;
;                                                                              ;
; INPUT:   RCX = Pointer to Threads Control List                               ;
;          RDX = Pointer to Input Parameters Block (IPB)                       ;
;          R8D = Number of threads                                             ;
;          R9  = Block size, bytes                                             ; 
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;                                                                              ;
; Note.                                                                        ;
; Limit number of updated entries by input R8D (not all 256 threads) can       ;
; improve quality of measurements when call this subroutine in the             ;
; drawing cycle.                                                               ;
; Can destroy registers, volatile by Microsoft x64 calling convention.         ;
;------------------------------------------------------------------------------;

ThreadsUpdateBlockSize:
mov rax,r9
mov r10d,[rdx + IPB.OperandWidth]
shr r10d,3
jz .Error
xor edx,edx
div r10 

@@:
; Update block size, bytes, this field must be updated per draw iteration
mov [rcx + THCTRL.SizeBytes],r9
; Update block size, instructions, this field must be updated per draw iteration
mov [rcx + THCTRL.SizeInst],rax
; Cycle
add rcx,THCTRL_SIZE
dec r8d
jnz @b 

; Exit points
clc
ret
.Error:
stc
ret


; Same operation for thread routine,
; this required for latency measurement mode,
; when change fill pattern to walk pattern

ThreadsUpdatePattern:
@@:
mov [rcx + THCTRL.EntryPoint],r9
; Cycle
add rcx,THCTRL_SIZE
dec r8d
jnz @b 
ret
 
