;---------- Update threads list, set new size at bytes and instructions ------;
;                                                                             ;
; INPUT:   RCX = Pointer to Threads Control List                              ;
;          RDX = Pointer to Input Parameters Block (IPB)                      ;
;          R8D = Number of threads                                            ;
;          R9  = Block size, bytes                                            ; 
;                                                                             ;
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                       ;
;                                                                             ;
; Note.                                                                       ;
; Limit number of updated entries by input R8D (not all 256 threads) can      ;
; improve quality of measurements when call this subroutine in the            ;
; drawing cycle.                                                              ;
; Can destroy registers, volatile by Microsoft x64 calling convention.        ;
;-----------------------------------------------------------------------------;
ThreadsUpdateBlockSize:
mov rax,r9                           ; RAX  = new block size, units = bytes
mov r10d,[rdx + IPB.OperandWidth]    ; R10D = operand width, bits 
shr r10d,3                           ; R10D = operand width, bytes, means / 8
jz .Error                            ; Go error if width = 0
xor edx,edx
div r10                           ; RAX = new block size, units = INSTRUCTIONS 
; Entries update cycle
@@:
; Update block size, bytes, this field must be updated per draw iteration
mov [rcx + THCTRL.SizeBytes],r9
; Update block size, instructions, this field must be updated per draw iteration
mov [rcx + THCTRL.SizeInst],rax
; Cycle for all threads
add rcx,THCTRL_SIZE          ; RCX = threads entries pointer
dec r8d                      ; R8D = threads entries counter
jnz @b 
; Exit points
clc             ; CF = 0 means no errors
ret
.Error:
stc             ; CF = 1 means error
ret


;--- Same operation for thread routine, -----------------------------------;
;    this required for latency measurement mode,                           ;
;    when change fill pattern to walk pattern                              ;
;                                                                          ;
; INPUT:   RCX = Pointer to Threads Control List                           ;
;          R8D = Number of threads                                         ;
;          R9  = Pointer to performance pattern subroutine                 ; 
;                                                                          ;
; OUTPUT:  None                                                            ;
;--------------------------------------------------------------------------;
ThreadsUpdatePattern:
@@:
mov [rcx + THCTRL.EntryPoint],r9   ; R9 = pointer to pattern routine
; Cycle
add rcx,THCTRL_SIZE                ; RCX = threads entries pointer
dec r8d                            ; R8D = threads entries counter
jnz @b 
ret
 
;--- Same operation for adaptive mode repeats count, ----------------------;
;    this required for adaptive repeats mode,                              ;
;    when change Repeats count = F ( Block size )                          ;
;                                                                          ;
; INPUT:   RCX = Pointer to Threads Control List                           ;
;          RDX = Pointer to Input Parameters Block (IPB)                   ;
;          R8D = Number of threads                                         ;
;                                                                          ;
; OUTPUT:  None                                                            ;
;--------------------------------------------------------------------------;
ThreadsUpdateAdaptive:
mov r11,rdx                          ; R11 = IPB base pointer
mov rax,[r11 + IPB.AdaptiveProduct]  ; RAX = total constant adaptive count 
mov r10,[r11 + IPB.StartBlockSize]   ; R10 = block size, units = bytes
xor edx,edx
div r10                          ; RAX = repeats = total constant / block size 
; Prevent overflow, if repeats count = 0, set repeats count = 1
test rax,rax
jnz @f                           ; Go skip correction if repeats > 0
inc eax                          ; correction, if repeats = 0, set repeats = 1 
@@:
; Update IPB, note R11 = IPB base pointer
mov [r11 + IPB.MeasureRepeats],rax  ; Update repeats count, note 64-bit
; Cycle for fill threads list
@@:
mov [rcx + THCTRL.Repeats],rax   ; RAX = updated repeats count, note 64-bit
add rcx,THCTRL_SIZE              ; RCX = threads entries pointer
dec r8d                          ; R8D = threads entries counter
jnz @b 
ret

