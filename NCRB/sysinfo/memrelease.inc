;---------- Memory release subroutine --------------------------------------;
; Use Heap or NUMA-aware method = F (Options)                               ;
;	                                                                          ;
; INPUT:   R15 = Global variables pointer                                   ;
;          Variables state                                                  ;
; OUTPUT:  None, variables settings only                                    ;
;---------------------------------------------------------------------------;
MemoryRelease:
push rbx rcx rdx rax rsi rbp r8 r9 r10 r11 rax  ; R8-R11 because API
mov rbp,rsp                  ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack because Win API used
sub rsp,32                   ; Make parameters shadow
;---Load pointer to variables, blank value ---
lea rbx,[MemoryRequestSize]

;--- Select Heap or NUMA functions ---
cmp dword [NumaNodes],2
jae ReleaseNumaBranch	   ; If Nodes > 1 , go to NUMA branch

;- v0.92 changes Heap to VirtualAlloc for non-NUMA memory allocation
; ;--- Heap Branch ---
; mov rcx,[rbx+08]	         ; [MemoryHeapID], Parm#1 = RCX = Heap handle
; mov edx,HEAP_NO_SERIALIZE  ; Parm#2 = RDX = Heap allocation options
; mov r8,[rbx+16]            ; [MemoryTrueBase], Parm#3 = R8  = A pointer to the memory block to be freed
; call [HeapFree]
; test rax,rax               ; Output = RAX = nonzero if operation OK, 0=Error
; jz MemoryExit2		         ; Go if memory allocation error
; ;--- HeapDestroy subroutine parameters ---
; mov rcx,[rbx+08]           ; [MemoryHeapID], Parm#1 = RCX = Heap handle 
; call [HeapDestroy]

;-

mov rcx,[rbx+16]       ; Parm#1 = Block true allocated base, for release
xor edx,edx            ; Parm#2 = Block size, 0 means release all allocated
mov r8d,MEM_RELEASE    ; Parm#3 = Release mode
call [VirtualFree]

;-

; At this point status ignored, output = RAX = nonzero if operation OK, 0=Error
MemoryExit2:
jmp MemoryExit

;--- NUMA branch ---
ReleaseNumaBranch:
;--- Get current process handle ---
call [GetCurrentProcess]   ; Return RAX = Handle
xchg rdi,rax           ; Copy to non-volatile register
;--- Release first half, single thread ---
mov rcx,rdi            ; Parm#1 = RCX = Process Handle
mov rdx,[rbx+16]       ; Parm#2 = Address
;- mov r8,[rbx]        ; R8 = Block total size
;- shr r8,1            ; Parm#3 = R8  = Memory block size / 2 , first half for single thread
;- add r8,4096         ; Restore reserve for alignment
xor r8d,r8d            ; Parm#3 = R8 = Release size, here must be 0 = Entire block
mov r9d,MEM_RELEASE	   ; Parm#4 = Release type
call [VirtualFreeEx]
test rax,rax
jz .MemoryExit3
;--- Release second half, multi thread ---
lea rsi,[ThreadsControl+112]
mov ebx,16
.ReleaseNuma:
cld
mov rcx,rdi		         ; Parm#1 = RCX = Process Handle
lodsq
xchg rdx,rax		       ; Parm#2 = Address
xor r8d,r8d		         ; Parm#3 = R8  = Memory block size / 2 , first half for single thread
mov r9d,MEM_RELEASE	   ; Parm#4 = Release type
call [VirtualFreeEx]
test rax,rax
jz .MemoryExit3
add rsi,128-8
dec ebx
jnz .ReleaseNuma
;--- Exit ---
.MemoryExit3:
jmp MemoryExit2

