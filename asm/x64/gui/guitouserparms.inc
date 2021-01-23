;--- Extract options settings data from GUI widgets to User Parameters Block --;
;                                                                              ;
; INPUT:   AL = Run context: 0 = by "Run simple" button                        ;
;                            1 = by "Run drawings" button                      ;
;          Run context used for memory allocation, multiplicated if draw mode  ;
;          GUI control structures                                              ; 
;                                                                              ;
; OUTPUT:  None at registers                                                   ;
;          UPB = User parameters block, extracted options values               ;  
;                                                                              ;
; All CPU registers can be corrupted                                           ;
;------------------------------------------------------------------------------;

; Note for optimizing, use EBX = WS_DISABLED required conditions:
;  1) RBX not used for other purposes at this subroutine.
;  2) RBX can be destroyed by this subroutine, reloaded after it call.
;  3) RBX not destroyed by called subroutine: SendMessageGetCheck.  

GuiToUserParms:

; Setup User Parameters Block (UPB) pointer and data pattern for widgets check
lea rdi,[UserParms]
mov ebx,WS_DISABLED

; Setup run context for memory allocation multiplication mode control
movzx eax,al
mov [rdi + UPB.RunContext],eax  

; Extract ASM-METHOD, for this option detect CHECKED only
mov esi,HANDLE_X86_READ            ; Compare pattern = FIRST handle
@@:
call SendMessageGetCheck
jz @f                              ; Go if BST_CHECKED (button state checked)
inc esi
cmp esi,HANDLE_LAST_METHOD         ; Compare with LAST handle
jbe @b                             ; Cycle for radio buttons group
@@:
sub esi,HANDLE_X86_READ            ; 0 = First method toggler
mov [rdi + UPB.OptionAsm],esi      ; Write method number = ESI 

; Extract PREFETCH DISTANCE, for this option detect CHECKED only
mov esi,HANDLE_DISTANCE_DEFAULT    ; Compare pattern = FIRST handle
@@:
call SendMessageGetCheck
jz @f                              ; Go if BST_CHECKED (button state checked)
inc esi
cmp esi,HANDLE_DISTANCE_LONG       ; Compare with LAST handle
jbe @b                             ; Cycle for radio buttons group
@@:
sub esi,HANDLE_DISTANCE_DEFAULT    ; 0 = First method toggler, ... , 3 = not used
mov [rdi + UPB.OptionDistance],esi ; Write option state = ESI 

; Extract USE PARALLEL THREADS, for this option detect both GRAY and CHECKED 
xor eax,eax                        ; PARALLEL_NOT_SUP
test dword [PARALLEL_NUMA_PG_BASE + E_PARALLEL_THREADS],ebx  ; EBX = WS_DISABLED
jnz @f                             ; Go if toggler button disabled (gray)
mov esi,HANDLE_PARALLEL_THREADS
call SendMessageGetCheck           ; Result = ZF flag
mov eax,PARALLEL_NOT_USED
jnz @f
inc eax                            ; PARALLEL_USED 
@@:
mov [rdi + UPB.OptionParallel],eax  ; Write option state = EAX

; Extract USE HYPER THREADING, for this option detect both GRAY and CHECKED 
xor eax,eax                        ; HT_NOT_SUPPORTED
test dword [PARALLEL_NUMA_PG_BASE + E_HYPER_THREADING],ebx  ; EBX = WS_DISABLED
jnz @f                             ; Go if toggler button disabled (gray)
mov esi,HANDLE_HYPER_THREADING
call SendMessageGetCheck           ; Result = ZF flag
mov eax,HT_NOT_USED
jnz @f
inc eax                            ; HT_USED 
@@:
mov [rdi + UPB.OptionHT],eax       ; Write option state = EAX

; Extract USE PROCESSOR GROUPS, for this option detect both GRAY and CHECKED 
xor eax,eax                        ; PG_NOT_SUPPORTED
test dword [PARALLEL_NUMA_PG_BASE + E_PROCESSOR_GROUPS],ebx  ; EBX = WS_DISABLED
jnz @f                             ; Go if toggler button disabled (gray)
mov esi,HANDLE_PROCESSOR_GROUPS
call SendMessageGetCheck           ; Result = ZF flag
mov eax,PG_NO_CONTROL
jnz @f
inc eax                            ; PG_USED 
@@:
mov [rdi + UPB.OptionPG],eax       ; Write option state = EAX

; Extract NUMA CONTROL, for this option detect both GRAY and CHECKED
xor esi,esi
test dword [PARALLEL_NUMA_PG_BASE + NUMA_UNAWARE],ebx  ; EBX = WS_DISABLED
jnz .NumaOff                       ; Go skip if NUMA not sup. by platform, SI=0 
mov esi,HANDLE_NUMA_UNAWARE
@@:
call SendMessageGetCheck
jz @f                              ; Go if BST_CHECKED
inc esi
cmp esi,HANDLE_NUMA_REMOTE         ; Compare with LAST handle
jbe @b                             ; Cycle for radio buttons group
@@:
sub esi,HANDLE_NUMA_UNAWARE - 1    ; 0 = Gray, 1 = First toggle button active
.NumaOff:
mov [rdi + UPB.OptionNUMA],esi     ; Old note SIL means bits SI.[7-0]

; Extract TARGET OBJECT (CACHE/DRAM), for this option detect CHECKED only
mov esi,HANDLE_L1_CACHE
@@:
call SendMessageGetCheck
jz @f                              ; Go if BST_CHECKED
inc esi
cmp esi,HANDLE_CUSTOM_BLOCK        ; Compare with LAST handle
jbe @b                             ; Cycle for radio buttons group
@@:
sub esi,HANDLE_L1_CACHE            ; 0 = First toggle button
mov [rdi + UPB.OptionTarget],esi   ; Note SIL means bits SI.[7-0]

; Extract USE LARGE PAGES, for this option detect both GRAY and CHECKED 
xor eax,eax                        ; LP_NOT_SUPPORTED
test dword [LARGE_PAGES_BASE + E_LARGE_PAGES],ebx  ; EBX = WS_DISABLED
jnz @f                             ; Go if toggler button disabled (gray)
mov esi,HANDLE_LARGE_PAGES
call SendMessageGetCheck           ; Result = ZF flag
mov eax,LP_NOT_USED
jnz @f
inc eax                            ; LP_USED 
@@:
mov [rdi + UPB.OptionLP],eax       ; Write option state = EAX

; Extract MEASUREMENT MODE (FAST/SLOW/ADAPTIVE), for this option detect CHECKED only
mov esi,HANDLE_BRIEF_MEASUREMENT
@@:
call SendMessageGetCheck
jz @f                                         ; Go if BST_CHECKED
inc esi
cmp esi,HANDLE_CAREFUL_ADAPTIVE_MEASUREMENT   ; Compare with LAST handle
jbe @b                                        ; Cycle for radio buttons group
@@:
sub esi,HANDLE_BRIEF_MEASUREMENT              ; 0 = First toggle button
mov [rdi + UPB.OptionMeasure],esi             ; Write option state = ESI

; Extract MEASUREMENT APPROXIMATION MODE, for this option detect CHECKED only
mov esi,HANDLE_APPROXIMATION_NONE
@@:
call SendMessageGetCheck
jz @f                                         ; Go if BST_CHECKED
inc esi
cmp esi,HANDLE_APPROXIMATION_X32              ; Compare with LAST handle
jbe @b                                        ; Cycle for radio buttons group
@@:
sub esi,HANDLE_APPROXIMATION_NONE             ; 0 = First toggle button
mov [rdi + UPB.OptionApprox],esi              ; Write option state = ESI

; Extract CUSTOM BLOCK SIZE, from user-editable field to UPB qword
mov rax,[BlockSizeEdit]
mov [rdi + UPB.CustomBlockStart],rax

; Done
ret

