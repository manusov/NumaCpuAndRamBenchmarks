;------------------------------------------------------------------------;
; Get number of processor groups,                                        ;
; actual for support systems with > 64 logical processors                ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save Processor Groups info      ;
; OUTPUT:  RDI = Input RDI + 8, output size is 2 DWORDS                  ;
;                Dword[0-3] = Active processor groups count              ; 
;                Dword[4-7] = Total processors count at all groups       ;                
;                                                                        ;
; Can destroy registers, volatile by Microsoft x64 calling convention.   ; 
;------------------------------------------------------------------------;
GetProcessorGroups:
push rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack because WinAPI call
sub rsp,32                   ; Make parameter shadow because WinAPI call
; Pre-clear output
mov qword [rdi],0
; Detect processor groups
mov rax,[_GetActiveProcessorGroupCount]
test rax,rax
jz .NoGroups
call rax     ; RAX = this subroutine address after dynamical import
.NoGroups:
movzx eax,ax
stosd        ; Store groups count, 16-bit valid, 0 means simple single group
; Detect total processors count per all groups
mov rax,[_GetActiveProcessorCount]
test rax,rax
jz .NoProcessorsTotal 
mov ecx,ALL_PROCESSOR_GROUPS
call rax             ; RAX = this subroutine address after dynamical import
.NoProcessorsTotal:
stosd                ; Store total (all groups) processors count, 32-bit value, 
mov rsp,rbp          ; 0 means use legacy method
pop rbp
ret

