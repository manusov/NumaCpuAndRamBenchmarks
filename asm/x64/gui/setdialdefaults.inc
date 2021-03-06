;--- Set default state for options, called when Create Window 0 ----;
;    and when click "Defaults" button.                              ;
;                                                                   ;
; INPUT:   RBX = Parent window handle                               ;
;          Sysinfo and Window 0 variables only                      ;
; OUTPUT:  None in the CPU registers,                               ; 
;          window 0 variables only                                  ;
;-------------------------------------------------------------------; 

SetDialogueDefaults:
cld				           ; Clear direction, because STOSQ used
push rbx rcx rdx rax rsi rbp r8 r9 r10 r11 rax	; R8-R11 because API
mov rbp,rsp                      ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h       ; Align stack because Win API used 
sub rsp,32                       ; Make parameters shadow

; Options, set Method Selector = F (CPU features),
; Pre-set togglers (radio buttons and checkboxes)
mov edi,WS_DISABLED              ; EDI = Operand, must be nonvolatile
lea rsi,[METHODS_BASE]           ; RSI local label for compact offsets
test [rsi+AVX512_METHOD_1],edi   ; Check availability: AVX512 Read
mov al,HANDLE_AVX512_READ
jz @f                            ; Go with Handle=AL if checkbox enabled
test [rsi+AVX256_METHOD_1],edi   ; Check availability: AVX256 Read
mov al,HANDLE_AVX256_READ
jz @f                            ; Go with Handle=AL if checkbox enabled
test [rsi+SSE128_METHOD_1],edi
mov al,HANDLE_SSE128_READ
jz @f
mov al,HANDLE_X86_READ
@@:
mov rcx,rbx                   ; RCX = Parm#1 = Parent handle
mov edx,HANDLE_X86_READ       ; RDX = Parm#2 = First radiobutton or checkbox          
mov r8d,HANDLE_LAST_METHOD    ; R8  = Parm#3 = Last radiobutton or checkbox
movzx r9d,al                  ; R9  = Parm#4 = Selected radiobutton or checkbox 
call [CheckRadioButton]       ; Set radio button

; Options, set "Default prefetch distance" if SSE supported
test [rsi+SSE128_METHOD_1],edi
jnz @f
mov rcx,rbx                      ; RCX = Parm#1 = Parent handle
mov edx,HANDLE_DISTANCE_DEFAULT  ; RDX = Parm#2 = First radiobutton or checkbox          
mov r8d,HANDLE_DISTANCE_LONG     ; R8  = Parm#3 = Last radiobutton or checkbox
mov r9d,HANDLE_DISTANCE_DEFAULT  ; R9  = Parm#4 = Selected radiobutton or checkbox 
call [CheckRadioButton]          ; Set radio button
@@:

; Options, set "Use Parallel Threads" button = off, if enabled
lea rsi,[PARALLEL_NUMA_PG_BASE]
test [rsi+E_PARALLEL_THREADS],edi
jnz @f                           ; Go skip if checkbox disabled
mov rcx,rbx
mov edx,HANDLE_PARALLEL_THREADS
mov r8d,edx                                 
xor r9d,r9d                      ; Selected=0 for clear to "0" 
call [CheckRadioButton]
@@:

; Options, set "Use Hyper-Threading" button = off, if enabled
test [rsi+E_HYPER_THREADING],edi
jnz @f                           ; Go skip if checkbox disabled
mov rcx,rbx
mov edx,HANDLE_HYPER_THREADING
mov r8d,edx
xor r9d,r9d                      ; Selected=0 for clear to "0"
call [CheckRadioButton]
@@:

; Options, set "Use Processor Groups" button = off, if enabled
test [rsi+E_PROCESSOR_GROUPS],edi
jnz @f                           ; Go skip if checkbox disabled
mov rcx,rbx
mov edx,HANDLE_PROCESSOR_GROUPS
mov r8d,edx                                 
xor r9d,r9d                      ; Selected=0 for clear to "0" 
call [CheckRadioButton]
@@:

; Options, set "Use Large Pages" button = off, if enabled
test [rsi+E_LARGE_PAGES],edi
jnz @f                           ; Go skip if checkbox disabled
mov rcx,rbx
mov edx,HANDLE_LARGE_PAGES
mov r8d,edx                                 
xor r9d,r9d                      ; Selected=0 for clear to "0" 
call [CheckRadioButton]
@@:

; Options, NUMA Absent, Local, Remote
test [rsi+NUMA_FORCE_LOCAL],edi  ; Check NUMA present, ZF=1(Z) if present
mov rcx,rbx
mov edx,HANDLE_NUMA_UNAWARE
mov r8d,HANDLE_NUMA_REMOTE                  
mov r9d,HANDLE_NUMA_LOCAL        ; Selected="Local" if NUMA present
jz @f                            ; Go if NUMA present, for use "Local"
mov r9d,HANDLE_NUMA_UNAWARE      ; Selected="No control" if NUMA absent
@@:
call [CheckRadioButton]

; Options, L1,L2,L3 Caches, RAM, Custom Block
lea rsi,[CACHES_BASE]
mov al,HANDLE_L1_CACHE
test [rsi+L1_CACHE],edi 
jz @f                                       ; Go if L1 present, radiobutton=L1
mov al,HANDLE_L2_CACHE
test [rsi+L2_CACHE],edi
jz @f                                       ; Go if L2 present, radiobutton=L2
mov al,HANDLE_L3_CACHE
test [rsi+L3_CACHE],edi
jz @f                                       ; Go if L3 present, radiobutton=L3
mov al,HANDLE_DRAM                          ; Otherwise radiobutton=RAM
@@:
mov rcx,rbx
mov edx,HANDLE_L1_CACHE
mov r8d,HANDLE_CUSTOM_BLOCK
movzx r9d,al
call [CheckRadioButton]

; Options, Carefully mode (slow)
; Updated for default method is careful adaptive
mov rcx,rbx
mov edx,HANDLE_BRIEF_MEASUREMENT             ; first at group, group to "0"
mov r8d,HANDLE_CAREFUL_ADAPTIVE_MEASUREMENT  ; last at group, group to "0"
;- mov r9d,HANDLE_CAREFUL_MEASUREMENT        ; selected at group, this to "1"
mov r9d,HANDLE_CAREFUL_ADAPTIVE_MEASUREMENT  ; selected at group, this to "1"
call [CheckRadioButton]

; Options, Approximation mode
mov rcx,rbx
mov edx,HANDLE_APPROXIMATION_NONE   ; first at group, group for clear to "0"
mov r8d,HANDLE_APPROXIMATION_X32    ; last at group, group for clear to "0"
mov r9d,HANDLE_APPROXIMATION_NONE   ; selected at group, this for set to "1"
call [CheckRadioButton]

; Options, Editable field for custom block size
mov rcx,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]  ; RCX = Parm#1 = Destination element handle
mov edx,WM_SETTEXT           ; RDX = Parm#2 = Command, "WM" means Window Message
xor r8d,r8d                  ; R8  = Parm#3 = Don't used
lea r9,[Edit_Size]           ; R9  = Parm#4 = Source buffer  

; Select block size
lea rsi,[SystemParameters.CpuCache.CpuidL1Data]
mov rbx,0FFFFFFFF00000000h
mov rax,[rsi+00]             ; RAX = L1 Data Cache size
test rax,rax
jz @f                        ; Go try next level if Size = 0
test rax,rbx
jz .PrintBlock               ; Go print if valid, if Size < 4GB (high bits = 0)
@@:
mov rax,[rsi+12]             ; RAX = L2 Unified Cache size
test rax,rax
jz @f                        ; Go try next level if Size = 0
test rax,rbx
jz .PrintBlock               ; Go print if valid, if Size < 4GB (high bits = 0)
@@:
mov rax,[rsi+24]             ; RAX = L3 Unified Cache size
test rax,rax                       
jz @f                        ; Go try next level if Size = 0
test rax,rbx
jz .PrintBlock               ; Go print if valid, if Size < 4GB (high bits = 0)
@@:
mov eax,00010000h            ; Set default size = 64KB if all invalid
; Print block size  
.PrintBlock:
mov rdi,r9
call HexPrint32
mov al,0
stosb

; Send message with size string
call [SendMessage]

; Disable edit block size field
mov rcx,[SubHandles-128+HANDLE_CUSTOM_EDIT*8]  ; RCX = Parm#1 = Handle
mov edx,WM_ENABLE       ; RDX = Parm#2 = Message
xor r8d,r8d             ; R8  = Parm#3 = Select: 0=Disable
xor r9d,r9d             ; R9  = Parm#4 = Not used
call [SendMessage]                  

; Disable prefetch distance radio buttons
mov rcx,[SubHandles-128+HANDLE_DISTANCE_DEFAULT*8]  ; RCX = Parm#1 = Handle
xor edx,edx             ; RDX = Parm#2 = Select: 0=Disable
call [EnableWindow]
mov rcx,[SubHandles-128+HANDLE_DISTANCE_MEDIUM*8]   ; RCX = Parm#1 = Handle
xor edx,edx             ; RDX = Parm#2 = Select: 0=Disable
call [EnableWindow]
mov rcx,[SubHandles-128+HANDLE_DISTANCE_LONG*8]     ; RCX = Parm#1 = Handle
xor edx,edx             ; RDX = Parm#2 = Select: 0=Disable
call [EnableWindow]

; Done, registers restore and return, R8-R11 because API, volatile
mov rsp,rbp
pop rax r11 r10 r9 r8 rbp rsi rax rdx rcx rbx    ; R8-R11 because API, volatile
ret


