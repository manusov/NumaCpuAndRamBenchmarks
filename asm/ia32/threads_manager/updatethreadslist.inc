;---------- Update threads list, set new size at bytes and instructions ------;
;                                                                             ;
; INPUT:   EBX = Pointer to Threads Control List                              ;
;          EDX = Pointer to Input Parameters Block (IPB)                      ;
;          ECX = Number of threads                                            ;
;          EAX = Block size, bytes                                            ; 
;                                                                             ;
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                       ;
;                                                                             ;
; Note.                                                                       ;
; Limit number of updated entries by input EAX (not all 256 threads) can      ;
; improve quality of measurements when call this subroutine in the            ;
; drawing cycle.                                                              ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.       ;
;-----------------------------------------------------------------------------;
ThreadsUpdateBlockSize:
push ebx esi edi

mov esi,eax                        ; ESI  = new block size, units = bytes
mov edi,[edx + IPB.OperandWidth]   ; EDI = operand width, bits 
shr edi,3                          ; EDI = operand width, bytes, means / 8
jz .Error                          ; Go error if width = 0
xor edx,edx
div edi                            ; EAX = new block size, units = INSTRUCTIONS 

; Entries update cycle
@@:
; Update block size, bytes, this field must be updated per draw iteration
mov [ebx + THCTRL.SizeBytes],esi
; Update block size, instructions, this field must be updated per draw iteration
mov [ebx + THCTRL.SizeInst],eax
; Cycle for all threads
add ebx,THCTRL_SIZE          ; EBX = threads entries pointer
loop @b                      ; ECX = threads entries counter

; Exit points
clc             ; CF = 0 means no errors
.L0:
pop edi esi ebx
.Error:
stc             ; CF = 1 means error
jmp .L0


;--- Same operation for thread routine, -----------------------------------;
;    this required for latency measurement mode,                           ;
;    when change fill pattern to walk pattern                              ;
;                                                                          ;
; INPUT:   EBX = Pointer to Threads Control List                           ;
;          ECX = Number of threads                                         ;
;          EAX = Pointer to performance pattern subroutine                 ; 
;                                                                          ;
; OUTPUT:  None                                                            ;
;--------------------------------------------------------------------------;
ThreadsUpdatePattern:
@@:
mov [ebx + THCTRL.EntryPoint],eax   ; EAX = pointer to pattern routine
; Cycle
add ebx,THCTRL_SIZE                 ; EBX = threads entries pointer
loop @b                             ; ECX = threads entries counter
ret
 
;--- Same operation for adaptive mode repeats count, ----------------------;
;    this required for adaptive repeats mode,                              ;
;    when change Repeats count = F ( Block size )                          ;
;                                                                          ;
; INPUT:   EBX = Pointer to Threads Control List                           ;
;          EDX = Pointer to Input Parameters Block (IPB)                   ;
;          ECX = Number of threads                                         ;
;          EAX = Block size, bytes                                         ; 
;                                                                          ;
; OUTPUT:  None                                                            ;
;--------------------------------------------------------------------------;

; TODO.
; 32/64 division bug for AdaptiveProduct.

ThreadsUpdateAdaptive:
push ebx esi edx

mov esi,[edx + IPB.StartBlockSize]             ; ESI = block size, units = bytes
mov eax,dword [edx + IPB.AdaptiveProduct + 0]  ; EDX:EAX = total constant adaptive count 
mov edx,dword [edx + IPB.AdaptiveProduct + 4]
div esi                                        ; EAX = repeats = total constant / block size 

; Prevent overflow, if repeats count = 0, set repeats count = 1
test eax,eax
jnz @f                           ; Go skip correction if repeats > 0
inc eax                          ; correction, if repeats = 0, set repeats = 1 
@@:

; Update IPB, note R11 = IPB base pointer
pop edx
mov dword [edx + IPB.MeasureRepeats + 0],eax  ; Update repeats count, note 64-bit
mov dword [edx + IPB.MeasureRepeats + 4],0 

; Cycle for fill threads list
@@:
mov dword [ebx + THCTRL.Repeats + 0],eax   ; EAX = updated repeats count, note 64-bit
mov dword [ebx + THCTRL.Repeats + 4],0
add ebx,THCTRL_SIZE              ; EBX = threads entries pointer
loop @b                          ; ECX = threads entries counter

pop esi ebx
ret

