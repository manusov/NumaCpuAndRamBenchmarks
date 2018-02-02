;---------- Memory allocation subroutine -----------------------------------;
; Use Heap or NUMA-aware method = F (Options)                               ;
;                                                                           ;
; INPUT:   None                                                             ;
;          Variables state                                                  ;
; OUTPUT:  ZF = Error flag: 0(NZ)=OK, 1(Z)=Memory allocation error          ;
;          None, variables settings only                                    ;
;---------------------------------------------------------------------------;
MemoryAllocation:
push rbx rcx rdx rax rsi rbp r8 r9 r10 r11 rax  ; R8-R11 because API
mov rbp,rsp           ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack because Win API used
sub rsp,32                   ; Make parameters shadow
;---Load pointer to variables, blank value ---
lea rbx,[MemoryRequestSize]
mov qword [rbx+24],0  ; [MemoryAlignedBase], 0 means error

;--- Select Heap or NUMA functions ---
; NUMA affinity masks initialization note
; if NUMA API supported: mask for node, passed as argument when memory allocation 
; if NUMA API not supported, mask=0, thread must skip set mask, if mask=0 
lea rdi,[ThreadsControl+32]
lea rsi,[NumaNodes]
mov r12d,[rsi+00]
cmp r12d,2            ; R12D = Number of NUMA nodes
jae AllocNumaBranch   ; If Nodes > 1 , go to NUMA branch

;- v0.92 changes Heap to VirtualAlloc for non-NUMA memory allocation
; ;--- Heap Branch ---
; ;--- Create heap, get and save heap id ---
; mov ecx,HEAP_NO_SERIALIZE  ; Parm#1 = RCX = Heap allocation options
; xor edx,edx       ; Parm#2 = RDX = Initial size, 0=One page (with grow later)
; xor r8d,r8d       ; Parm#3 = R8  = Maximum size, 0=Enable grow for allocate large blocks
; call [HeapCreate]
; mov [rbx+08],rax  ; [MemoryHeapID], Output = RAX = Heap handle, 0=Error
; test rax,rax
; jz MemoryExit     ; Go if heap creation error
; ;--- Allocate memory by required size, get and save allocated memory base ---
; xchg rcx,rax      ; Parm#1 = RCX = Heap handle
; mov edx,HEAP_NO_SERIALIZE  ; Parm#2 = RDX = Heap allocation options
; mov r8d,[rbx+00]  ; Parm#3 = R8  = Size, the number of bytes to be allocated
; call [HeapAlloc]
;-

xor ecx,ecx                ; Parm#1 = Base virtual address = auto
mov rdx,[rbx+00]           ; Parm#2 = Memory block size, bytes
mov r8d,MEM_COMMIT         ; Parm#3 = Allocation type
mov r9d,PAGE_READWRITE     ; Parm#4 = Page protection attribytes
call [VirtualAlloc]
test rax,rax               ; Output = linear address or 0 (if allocation error)
jz MemoryExit              ; Go if memory allocation error

;-

mov [rbx+16],rax  ; [MemoryTrueBase], Output = RAX = Pointer to allocated block, 0=Error, this used for release
;--- Built regions list for threads ---
; Note if region small (for cache memory), sub-regions located with 64MB step
MemorySetup:
test rax,rax
jz MemoryExit          ; Go if memory allocation error
call AddressAlign4KB   ; Alignment = Page = 4KB
mov [rbx+24],rax       ; [MemoryAlignedBase]
;--- Initializing multithread data ---
mov rdx,[rbx+00] ; RDX = Get allocated block size
xor dx,dx        ; Clear alignment addend
shr rdx,1        ; RDX = Block size / 2, second half for multithreading
add rax,rdx      ; RAX = Base address of second half of allocated memory block
shr rdx,4        ; RCX = RCX/16 , calculate sub-blocks sizes for each thread
mov rbx,[SystemInfo.dwActiveProcessorMask]
mov ecx,16             ; RCX = Threads counter
cld
@@:
mov qword [rdi-16],0   ; Clear affinity mask = f (options) for Non-NUMA allocation
mov qword [rdi+72],rbx ; Clear true affinity mask for Non-NUMA allocation
stosq                  ; Store address
add rax,rdx            ; Select next memory block of 16
add rdi,128-8          ; Select next thread of 16
loop @b                ; Cycle for 16 threads
;--- Exit ---
MemoryExit:
jmp GlobalExitAPI

;--- NUMA branch ---
; RBX	= Pointer to memory control block
; RSI+4 = Nodes masks list, for check node numbers validity
; RDI	= Thread control blocks . Block base
; R12	= Number of NUMA nodes
;---
AllocNumaBranch:
push r12 r13 r14 r15       ; Additional save , 6 registers: stack aligned
;--- Get current process handle ---
call [GetCurrentProcess]   ; Return RAX = Handle
test rax,rax
jz .MemoryExit1 		  ; Go error if Handle=0
xchg r13,rax			    ; R13 = Handle
;--- Allocate memory, first half for single thread  ---
cmp qword [rsi+04],0
je .MemoryExit1 		  ; Go error if Mask = 0 for Node Number = 0
mov rcx,r13			      ; Parm#1 = RCX = Handle
xor rdx,rdx			      ; Parm#2 = RDX = Base address, 0 means selected by function
mov r8,[rbx]			    ; R8 = Block total size
shr r8,1			        ; Parm#3 = R8  = Memory block size / 2 , first half for single thread
add r8,4096			      ; Restore reserve for alignment
mov r9,00003000h		  ; Parm#4 = R9  = Allocation type: RESERVE(2000h) + COMMIT(1000h)
push rdx			        ; Parm#6 = NUMA node preferred = 0 , use RDX=0 for compact encoding
pushq 04h			        ; Parm#5 = Pages protection attributes = Read/Write
;---
sub rsp,32			      ; Create parameters shadow
mov rax,[_VirtualAllocExNuma]
test rax,rax
jz .NoValloc1
call rax 
.NoValloc1:
add rsp,32+16			    ; Remove parameters shadow + parameters 5,6
;---
mov [rbx+16],rax		  ; [MemoryTrueBase], Output = RAX = Pointer to allocated block, 0=Error, this used for release
test rax,rax
jz .MemoryExit1 		  ; Go error if memory allocation failed
call AddressAlign4KB  ; Alignment = Page = 4KB
mov [rbx+24],rax		  ; [MemoryAlignedBase]
;--- Prepare blocks base, step, count ---
; RSI+4 = Nodes masks list, for check node numbers validity
; RDI	= Thread control blocks . Block base
; R12	= Number of NUMA nodes, already loaded
; R13	= Current process handle, already loaded
; R14	= Threads counter, loaded here
; R15	= Block size
;---
mov rax,[rbx]			    ; RAX = Block size
shr rax,4+1			      ; RAX = Sub-block size
lea r15,[rax+4096]		; R15 = Memory size with alignment reserve
;--- Allocate memory, second half for multi thread  ---
mov r14d,16			      ; EBP = Threads counter
xor ebx,ebx			      ; EBX = Node number
.NumaAlloc:
mov rcx,r13			      ; Parm#1 = RCX = Handle
xor rdx,rdx			      ; Parm#2 = RDX = Base address, 0 means selected by function
mov r8,r15			      ; Parm#3 = R8 = Block size
mov r9,00003000h		  ; Parm#4 = R9  = Allocation type: RESERVE(2000h) + COMMIT(1000h)
push rbx			        ; Parm#6 = NUMA node preferred = 0 , use RDX=0 for compact encoding
pushq 04h			        ; Parm#5 = Pages protection attributes = Read/Write
;---
sub rsp,32			      ; Create parameters shadow
mov rax,[_VirtualAllocExNuma]
test rax,rax
jz .NoValloc2
call rax 
.NoValloc2:
add rsp,32+16			    ; Remove parameters shadow + parameters 5,6
;---
test rax,rax
jz .MemoryExit1
;--- Store result = block base ---
mov rcx,[rsi+4+rbx*8]
test rcx,rcx			      ; Don't use JRCXZ, because must set ZF flag
jz .MemoryExit1 		    ; Go error if this node number invalid, affinity mask = 0
mov [rdi+72],rcx        ; Store true affinity mask
mov qword [rdi-16],0    ; Blank affinity mask = F(options)
mov [rdi+80],rax		    ; Store true base address, can be used for release block
mov [rdi+88],r15		    ; Store true size, can be used for release block
call AddressAlign4KB    ; Alignment = Page = 4KB
cld
stosq
;--- Select next node ---
inc ebx
cmp ebx,r12d
jb .InRange
xor ebx,ebx			    ; Clear node number if all nodes scanned
.InRange:
;--- Select next thread ---
add rdi,128-8
dec r14d
jnz .NumaAlloc
;--- Exit ---
or al,1 			    ; This for ZF=1(NZ) means no errors
.MemoryExit1:
pop r15 r14 r13 r12
jmp MemoryExit

;---
; Small additional subroutine for memory page alignment
; Must don't touch value if already aligned
; INPUT:   RAX = Address
; OUTPUT:  RAX = Aligned address
;---
AddressAlign4KB:
test eax,0FFFh
jz @f                           ; Go skip if already aligned
and rax,0FFFFFFFFFFFFF000h	    ; Alignment = Page = 4KB
add rax,1000h			              ; This prevent use address before block start
@@:
ret

