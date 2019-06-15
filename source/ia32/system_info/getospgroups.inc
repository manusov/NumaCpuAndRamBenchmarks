;--- Get number of processor groups, ------------------------------------;
; actual for support systems with > 64 logical processors                ;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save Processor Groups info      ;
;                                                                        ;
; OUTPUT:  EDI = Input EDI + 8, output size is 2 DWORDS                  ;
;                Dword[0-3] = Active processor groups count              ; 
;                Dword[4-7] = Total processors count at all groups       ;                
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

GetProcessorGroups:
; Detect processor groups
mov eax,[_GetActiveProcessorGroupCount]
test eax,eax
jz .NoGroups  ; Go skip if this Win API function not found by dynamical import  
call eax      ; EAX = this subroutine address after dynamical import
.NoGroups:
movzx eax,ax
stosd         ; Store groups count, 16-bit valid, 0 means simple single group

; Detect total processors count per all groups
mov eax,[_GetActiveProcessorCount]
test eax,eax
jz .NoProcessorsTotal      ; Go skip if this Win API function not found by dynamical import 
push ALL_PROCESSOR_GROUPS  ; Parm#1 = Group number or ALL GROUPS
call eax                   ; EAX = this subroutine address after dynamical import
.NoProcessorsTotal:
stosd                      ; Store total (all groups) processors count, 32-bit value, 

; Done
ret

