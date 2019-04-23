
GuiToUserParms:

; Extract ASM-METHOD, for this option detect CHECKED only
lea rdi,[UserParms]
mov esi,HANDLE_X86_READ            ; Compare pattern = FIRST handle
@@:
call SendMessageGetCheck
jz @f                              ; Go if BST_CHECKED (button state checked)
inc esi
cmp esi,HANDLE_LAST_METHOD         ; Compare with LAST handle
jbe @b                             ; Cycle for radio buttons group
@@:
sub esi,HANDLE_X86_READ            ; 0 = First method toggler
mov [rdi+UPB.OptionAsm],esi        ; Write method number = ESI 

; TODO. Optimize, WS_DISABLED and PARALLEL_NUMA_PG_BASE constants use in the registers

; Extract USE PARALLEL THREADS, for this option detect both GRAY and CHECKED 
xor eax,eax                        ; PARALLEL_NOT_SUP
test dword [PARALLEL_NUMA_PG_BASE + E_PARALLEL_THREADS],WS_DISABLED
jnz @f                             ; Go if toggler button disabled (gray)
mov esi,HANDLE_PARALLEL_THREADS
call SendMessageGetCheck           ; Result = ZF flag
mov eax,PARALLEL_NOT_USED
jnz @f
inc eax                            ; PARALLEL_USED 
@@:
mov [rdi+UPB.OptionParallel],eax   ; Write option state = EAX

; Extract USE HYPER THREADING, for this option detect both GRAY and CHECKED 
xor eax,eax                        ; HT_NOT_SUPPORTED
test dword [PARALLEL_NUMA_PG_BASE + E_HYPER_THREADING],WS_DISABLED
jnz @f                             ; Go if toggler button disabled (gray)
mov esi,HANDLE_HYPER_THREADING
call SendMessageGetCheck           ; Result = ZF flag
mov eax,HT_NOT_USED
jnz @f
inc eax                            ; HT_USED 
@@:
mov [rdi+UPB.OptionHT],eax         ; Write option state = EAX

; Extract USE PROCESSOR GROUPS, for this option detect both GRAY and CHECKED 
xor eax,eax                        ; PG_NOT_SUPPORTED
test dword [PARALLEL_NUMA_PG_BASE + E_PROCESSOR_GROUPS],WS_DISABLED
jnz @f                             ; Go if toggler button disabled (gray)
mov esi,HANDLE_PROCESSOR_GROUPS
call SendMessageGetCheck           ; Result = ZF flag
mov eax,PG_NO_CONTROL
jnz @f
inc eax                            ; PG_USED 
@@:
mov [rdi+UPB.OptionPG],eax         ; Write option state = EAX

; Extract NUMA CONTROL, for this option detect both GRAY and CHECKED
; TODO. Check subtracted constant "HANDLE_NUMA_UNAWARE - 1" see next option, but there 0=L1, here 0=UNAWARE really checked but gray ? REGULARIZE !
xor esi,esi
test dword [PARALLEL_NUMA_PG_BASE + NUMA_UNAWARE],WS_DISABLED
jnz .NumaOff                       ; Go skip if NUMA not sup. by platform, SI=0 
mov esi,HANDLE_NUMA_UNAWARE
@@:
call SendMessageGetCheck
jz @f                              ; Go if BST_CHECKED
inc esi
cmp esi,HANDLE_NUMA_REMOTE         ; Compare with LAST handle
jbe @b                             ; Cycle for radio buttons group
@@:
sub esi,HANDLE_NUMA_UNAWARE - 1    ; 0 = First toggle button
.NumaOff:
mov [rdi+UPB.OptionNUMA],esi       ; Note SIL means bits SI.[7-0]

; Extract TARGET OBJECT (CACHE/DRAM), for this option detect CHECKED only
mov esi,HANDLE_L1_CACHE
@@:
call SendMessageGetCheck
jz @f                              ; Go if BST_CHECKED
inc esi
cmp esi,HANDLE_CUSTOM_BLOCK        ; Compare with LAST handle
jbe @b                             ; Cycle for radio buttons group
@@:
sub esi,HANDLE_L1_CACHE           ; 0 = First toggle button
.ObjectOff:
mov [rdi+UPB.OptionTarget],esi     ; Note SIL means bits SI.[7-0]




; Done
ret