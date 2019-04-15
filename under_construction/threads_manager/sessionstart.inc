
; TODO. 
; Regularize simple/brief/drawings calling conventions, what registers destroyed.

SessionStart:

lea rsi,[UserParms]
lea rdi,[InputParms]
lea rbx,[SystemParameters]

; Setup benchmark method, 
; assembler routine = f (test method, test object, system configuration)
mov eax,[rsi + UPB.OptionAsm]
mov edx,[rsi + UPB.OptionTarget]
cmp eax,ASM_LIMIT
ja .L0
cmp edx,TARGET_LIMIT
ja .L0
mov ecx,[rbx + SYSPARMS.NonTemporalRead]
inc ecx
imul ecx,ecx,AM_BYTE_COUNT
cmp edx,TARGET_DRAM
je .L2
xor ecx,ecx
.L2:
movzx eax,byte [AM_Selector + rax + rcx]
jmp .L1
.L0:
xor eax,eax
.L1:
mov [rdi + IPB.UpdatedAsm],eax

; Setup tested object
mov [rdi + IPB.UpdatedTarget],edx

; Setup asm. routine address, set address and memory dump region, here EAX valid
push rbx
cmp eax,LATENCY_MODE
jae .L3
push rax
call GetBandwidthPattern  
pop rax
call GetBandwidthDump
jmp .L4
.L3:
push rax
call GetLatencyPattern  
pop rax
call GetLatencyDump
.L4:
mov [rdi + IPB.PatternRoutine],rbx
mov [rdi + IPB.DumpMethodStart],rax
mov [rdi + IPB.DumpMethodLength],edx
pop rbx

; Setup number of threads, use platform total number of threads if auto
mov eax,1
cmp [rsi + UPB.OptionParallel],PARALLEL_USED
jne .L5
mov rax,[rbx + SYSPARMS.NumaNodesMasks]
mov edx,1
xor ecx,ecx
.L6:
test rax,rdx
jz .L7
inc ecx     ; ECX = processors at first NUMA domain
.L7:
shl rdx,1
jnz .L6
mov eax,[rbx + SYSPARMS.SystemInfo + SYSTEM_INFO.dwNumberOfProcessors]  ; EAX = processors at current group
mov edx,[rbx + SYSPARMS.ProcessorsTotal]                                ; EDX = processors at all groups







.L5:
mov [rdi + IPB.UpdatedThreads],eax

; Setup Hyper-Threading mode


; Setup Processor Group mode


; Setup NUMA mode


; Setup Large Page mode


; Setup measurement precision (calibration repeats) mode


; Setup measurement scenario as block sizes sequence


; Setup Processor Groups parameters


; Setup NUMA domains parameters


; Setup page size


; Setup memory allocation parameters, align allocated size by page size


; Build NUMA nodes list, 
; first step before (without) memory allocation


; NUMA-aware memory allocation, 
; second step with memory allocation after nodes list build


; Build threads list




 


ret