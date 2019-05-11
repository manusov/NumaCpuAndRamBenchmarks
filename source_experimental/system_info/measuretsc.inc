;--- Measure CPU TSC (Time Stamp Counter) clock frequency, --------------; 
; store results F=Frequency=[Hz].                                        ;
; Call this subroutine only if CPUID and RDTSC both supported.           ;
;                                                                        ;
; INPUT:   RDI = Destination pointer for save TSC frequency and period	 ;
;                                                                        ;
; OUTPUT:  CF flag = Status: 0(NC)=Measured OK, 1(C)=Measurement error	 ;
;          Output RDI and destination memory valid only if CF=0(NC)      ;
;          RDI = Input RDI + 40 , buffer size fixed = 40 bytes           ;
;          Destination memory at [input RDI] = Results                   ;
;           Qword [00-07] = TSC frequency, Hz = delta TSC per 1 second   ;
;           Qword [08-15] = TSC frequency, Hz, as double precision       ;  
;           Qword [16-23] = TSC frequency, MHz, as double precision      ;
;           Qword [24-31] = TSC period, seconds, as double precision     ;
;           Qword [32-40] = TSC period, nanoeconds, as double precis.    ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft x64 calling convention.   ; 
;------------------------------------------------------------------------;

MeasureTsc:
cld                        ; Clear direction, because STOSQ used
push rbx rbp rbp           ; Last push for reserve local variable space
mov rbp,rsp                ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                 ; Make parameters shadow
; Start measure frequency
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
jbe .Error
; Store Frequency, as 64-bit integer value, Hz, delta-TSC per second
stosq                             ; Store RAX to [RDI], advance pointer RDI+8
; Calculate floating-point double precision values
push 1000000000 0 1000000 rax
finit
fild qword [rsp+00]
fld st0
fidiv dword [rsp+08]
fstp qword [rsp+08]   ; MHz
fld1
fdiv st0,st1
fstp qword [rsp+16]   ; Seconds
fild qword [rsp+24]
fdiv st0,st1
fstp qword [rsp+24]   ; Nanoseconds
fstp qword [rsp+00]   ; Hz
; Store floating-point double precision values
pop rax
stosq           ; Store TSC frequency, Hz, floating point double precision
pop rax
stosq           ; Store TSC frequency, MHz, fl. point double precision
pop rax
stosq           ; Store TSC period, Seconds, fl. point double precision
pop rax
stosq           ; Store TSC period, Nanoseconds, fl. point double precis.
; Restore RSP, pop extra registers, exit
clc             ; CF=0 (NC) means CPU clock measured OK
.Exit:
mov rsp,rbp
pop rbp rbp rbx
ret
.Error:
stc             ; CF=1 (CY) means CPU clock measured ERROR
jmp .Exit

