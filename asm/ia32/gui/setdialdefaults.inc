;--- Set default state for options, called when Create Window 0 ----;
;    and when click "Defaults" button.                              ;
;                                                                   ;
; INPUT:   EBX = Parent window handle                               ;
;          Sysinfo and Window 0 variables only                      ;
; OUTPUT:  None in the CPU registers,                               ; 
;          window 0 variables only                                  ;
;-------------------------------------------------------------------; 

SetDialogueDefaults:
cld				           ; Clear direction, because STOSQ used
pushad

; Options, set Method Selector = F (CPU features),
; Pre-set togglers (radio buttons and checkboxes)
mov edi,WS_DISABLED              ; EDI = Operand, must be nonvolatile
lea esi,[METHODS_BASE]           ; ESI local label for compact offsets
test [esi+AVX512_METHOD_1],edi   ; Check availability: AVX512 Read
mov al,HANDLE_AVX512_READ
jz @f                            ; Go with Handle=AL if checkbox enabled
test [esi+AVX256_METHOD_1],edi   ; Check availability: AVX256 Read
mov al,HANDLE_AVX256_READ
jz @f                            ; Go with Handle=AL if checkbox enabled
test [esi+SSE128_METHOD_1],edi
mov al,HANDLE_SSE128_READ
jz @f
mov al,HANDLE_X86_READ
@@:
movzx eax,al
push eax                         ; Parm#4 = Selected radiobutton or checkbox 
push HANDLE_LAST_METHOD          ; Parm#3 = Last radiobutton or checkbox
push HANDLE_X86_READ             ; Parm#2 = First radiobutton or checkbox
push ebx                         ; Parm#1 = Parent handle
call [CheckRadioButton]          ; Set radio button

; Options, set "Default prefetch distance" if SSE supported
test [esi+SSE128_METHOD_1],edi
jnz @f
push HANDLE_DISTANCE_DEFAULT     ; Parm#4 = Selected radiobutton or checkbox 
push HANDLE_DISTANCE_LONG        ; Parm#3 = Last radiobutton or checkbox
push HANDLE_DISTANCE_DEFAULT     ; Parm#2 = First radiobutton or checkbox
push ebx                         ; Parm#1 = Parent handle
call [CheckRadioButton]          ; Set radio button
@@:

; Options, set "Use Parallel Threads" button = off, if enabled
lea esi,[PARALLEL_NUMA_PG_BASE]
test [esi+E_PARALLEL_THREADS],edi
jnz @f                           ; Go skip if checkbox disabled
push 0                           ; Parm#4 = Selected=0 for clear to "0" 
push HANDLE_PARALLEL_THREADS     ; Parm#3 = Last radiobutton or checkbox 
push HANDLE_PARALLEL_THREADS     ; Parm#2 = First radiobutton or checkbox
push ebx                         ; Parm#1 = Parent handle 
call [CheckRadioButton]
@@:

; Options, set "Use Hyper-Threading" button = off, unconditionally
test [esi+E_HYPER_THREADING],edi
jnz @f                           ; Go skip if checkbox disabled
push 0                           ; Selected=0 for clear to "0"
push HANDLE_HYPER_THREADING
push HANDLE_HYPER_THREADING
push ebx
call [CheckRadioButton]
@@:

; Options, set "Use Processor Groups" button = off, unconditionally
test [esi+E_PROCESSOR_GROUPS],edi
jnz @f                           ; Go skip if checkbox disabled
push 0                           ; Selected=0 for clear to "0"
push HANDLE_PROCESSOR_GROUPS
push HANDLE_PROCESSOR_GROUPS
push ebx
call [CheckRadioButton]
@@:

; Options, set "Use Large Pages" button = off, if enabled
test [esi+E_LARGE_PAGES],edi
jnz @f                           ; Go skip if checkbox disabled
push 0                           ; Selected=0 for clear to "0"
push HANDLE_LARGE_PAGES
push HANDLE_LARGE_PAGES
push ebx 
call [CheckRadioButton]
@@:

; Options, NUMA Absent, Local, Remote
test [esi+NUMA_FORCE_LOCAL],edi  ; Check NUMA present, ZF=1(Z) if present
mov eax,HANDLE_NUMA_LOCAL        ; Selected="Local" if NUMA present
jz @f                            ; Go if NUMA present, for use "Local"
mov eax,HANDLE_NUMA_UNAWARE      ; Selected="No control" if NUMA absent
@@:
push eax
push HANDLE_NUMA_REMOTE
push HANDLE_NUMA_UNAWARE 
push ebx
call [CheckRadioButton]

; Options, L1,L2,L3 Caches, RAM, Custom Block
lea esi,[CACHES_BASE]
mov al,HANDLE_L1_CACHE
test [esi+L1_CACHE],edi 
jz @f                            ; Go if L1 present, radiobutton=L1
mov al,HANDLE_L2_CACHE
test [esi+L2_CACHE],edi
jz @f                            ; Go if L2 present, radiobutton=L2
mov al,HANDLE_L3_CACHE
test [esi+L3_CACHE],edi
jz @f                            ; Go if L3 present, radiobutton=L3
mov al,HANDLE_DRAM               ; Otherwise radiobutton=RAM
@@:
movzx eax,al
push eax
push HANDLE_CUSTOM_BLOCK
push HANDLE_L1_CACHE
push ebx
call [CheckRadioButton]

; Options, Carefully mode (slow)
; Updated for default method is careful adaptive
push HANDLE_CAREFUL_ADAPTIVE_MEASUREMENT  ; selected at group, this to "1" 
push HANDLE_CAREFUL_ADAPTIVE_MEASUREMENT  ; last at group, group to "0"
push HANDLE_BRIEF_MEASUREMENT             ; first at group, group to "0" 
push ebx
call [CheckRadioButton]

; Options, Approximation mode
push HANDLE_APPROXIMATION_NONE   ; selected at group, this for set to "1"
push HANDLE_APPROXIMATION_X32    ; last at group, group for clear to "0"
push HANDLE_APPROXIMATION_NONE   ; first at group, group for clear to "0"
push ebx
call [CheckRadioButton]

; Options, Editable field for custom block size
push Edit_Size            ; Parm#4 = Source buffer
push 0                    ; Parm#3 = Don't used
push WM_SETTEXT           ; Parm#2 = Command, "WM" means Window Message
push dword [SubHandles - 64 + HANDLE_CUSTOM_EDIT*4]  ; Parm#1 = Destination element handle 

; Select block size
lea esi,[SystemParameters.CpuCache.CpuidL1Data]
mov eax,[esi+00]          ; EAX = L1 Data Cache size
mov edx,[esi+04]
test eax,eax
jz @f                     ; Go try next level if Size = 0
test edx,edx
jz .PrintBlock            ; Go print if valid, if Size < 4GB (high bits = 0)
@@:
mov eax,[esi+12]          ; EDX:EAX = L2 Unified Cache size
mov edx,[esi+16]
test eax,eax
jz @f                     ; Go try next level if Size = 0
test edx,edx
jz .PrintBlock            ; Go print if valid, if Size < 4GB (high bits = 0)
@@:
mov eax,[esi+24]          ; EDX:EAX = L3 Unified Cache size
test eax,eax                       
jz @f                     ; Go try next level if Size = 0
test edx,edx
jz .PrintBlock            ; Go print if valid, if Size < 4GB (high bits = 0)
@@:
mov eax,00010000h         ; Set default size = 64KB if all invalid

; Print block size  
.PrintBlock:
lea edi,[Edit_Size]
call HexPrint32
mov al,0
stosb

; Send message with size string
call [SendMessage]

; Disable edit block size field
push 0            ; Parm#4 = Not used
push 0            ; Parm#3 = Select: 0=Disable, R13 from previous step
push WM_ENABLE    ; Parm#2 = Message
push dword [SubHandles - 64 + HANDLE_CUSTOM_EDIT*4]  ; Parm#1 = Handle 
call [SendMessage]                  

; Disable prefetch distance radio buttons
push 0            ; Parm#2 = Select: 0=Disable
push dword [SubHandles-64+HANDLE_DISTANCE_DEFAULT*4]  ; Parm#1 = Handle
call [EnableWindow]
push 0            ; Parm#2 = Select: 0=Disable
push dword [SubHandles-64+HANDLE_DISTANCE_MEDIUM*4]   ; Parm#1 = Handle
call [EnableWindow]
push 0            ; Parm#2 = Select: 0=Disable
push dword [SubHandles-64+HANDLE_DISTANCE_LONG*4]     ; Parm#1 = Handle
call [EnableWindow]

; Done
popad
ret

