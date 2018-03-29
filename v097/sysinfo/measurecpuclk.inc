;------------------------------------------------------------------------;
; Measure CPU TSC (Time Stamp Counter) clock frequency, store results    ;
; F=Frequency=[Hz], T=Period=[fs]. 1 femtosecond = 10^-15 seconds.       ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save TSC frequency and period	 ;
;                                                                        ;
; OUTPUT:  CF flag = Status: 0(NC)=Measured OK, 1(C)=Measurement error	 ;
;          Output RDI and destination memory valid only if CF=0(NC)      ;
;          RDI = Input RDI + 16 , buffer size fixed = 16 bytes           ;
;          Destination memory at [input RDI] = Results                   ;
;          Qword [00-07] = TSC Frequency, Hz, F = Delta TSC per 1 second ;
;          Qword [08-15] = TSC Period, Fs, T=1/F                         ;
;------------------------------------------------------------------------;
MeasureCpuClk:
cld                       ; Clear direction, because STOSQ used
push rbx rcx rdx rax rsi rbp r8 r9 r10 r11 rax	; R8-R11 because API
mov rbp,rsp               ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                ; Make parameters shadow
;--- Start measure frequency ---
mov rcx,rbp
call [GetSystemTimeAsFileTime]    ; Get current count
mov rsi,[rbp]
@@:
mov rcx,rbp
call [GetSystemTimeAsFileTime]    ; Get next count for wait 100 ns
cmp rsi,[rbp]
je @b
mov rsi,[rbp]
add rsi,10000000                  ; 10^7 * 100ns = 1 second
rdtsc
shl rdx,32
lea rbx,[rax+rdx]                 ; RBX = 64-bit TSC at operation start
@@:
mov rcx,rbp
call [GetSystemTimeAsFileTime]    ; Get count for wait 1 second
cmp rsi,[rbp]
ja @b
rdtsc
shl rdx,32
or rax,rdx                        ; RAX = 64-bit TSC at operation end
sub rax,rbx                       ; RAX = Delta TSC
;--- Store Frequency ---
stosq
;--- Calculate Period ---
xchg rcx,rax                      ; RCX = Divisor, XCHG (not MOV) for compact
stc                               ; Set CF=1(C), error indicator
jrcxz ExitCpuClk                  ; Go skip if error: divisor=0
mov rax,1000000000000000          ; RAX = Femtoseconds per one second
cqo                               ; RDX = 0, dividend high 64 bits
div rcx                           ; T=1/F, because RAX=10^15 here T=Femtoseconds
;--- Store Period ---
stosq
;--- Restore RSP, pop extra registers, exit ---
clc                               ; CF=0 (NC) means CPU clock measured OK
ExitCpuClk:
GlobalExitAPI:
mov rsp,rbp
pop rax r11 r10 r9 r8 rbp         ; R8-R11 because API
ExitCpuId4:
jmp ExitCpuId2

