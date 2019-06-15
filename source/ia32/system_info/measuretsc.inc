;--- Measure CPU TSC (Time Stamp Counter) clock frequency, --------------; 
; store results F=Frequency=[Hz].                                        ;
; Call this subroutine only if CPUID and RDTSC both supported.           ;
;                                                                        ;
; INPUT:   EDI = Destination pointer for save TSC frequency and period	 ;
;                                                                        ;
; OUTPUT:  CF flag = Status: 0(NC)=Measured OK, 1(C)=Measurement error	 ;
;          Output EDI and destination memory valid only if CF=0(NC)      ;
;          RDI = Input EDI + 40 , buffer size fixed = 40 bytes           ;
;          Destination memory at [input EDI] = Results                   ;
;           Qword [00-07] = TSC frequency, Hz = delta TSC per 1 second   ;
;           Qword [08-15] = TSC frequency, Hz, as double precision       ;  
;           Qword [16-23] = TSC frequency, MHz, as double precision      ;
;           Qword [24-31] = TSC period, seconds, as double precision     ;
;           Qword [32-40] = TSC period, nanoeconds, as double precis.    ;
;                                                                        ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.  ; 
;------------------------------------------------------------------------;

MeasureTsc:
cld                        ; Clear direction, because STOSQ used
push ebx esi ebp eax eax   ; Last 2 pushes = reserve space for local variables
mov ebp,esp                ; EBP = pointer to local variables
; Start measure frequency, get current change
push ebp                          ; Parm#1 = pointer to output 64-bit variable
call [GetSystemTimeAsFileTime]    ; Get current count
mov esi,[ebp]
; Wait for start 1 second interval
@@:
push ebp
call [GetSystemTimeAsFileTime]    ; Get next count for wait 100 ns
cmp esi,[ebp]
je @b
; Set time interval = 1 second
mov esi,[ebp + 0]
mov ebx,[ebp + 4]
add esi,10000000                  ; 10^7 * 100ns = 1 second
adc ebx,0
; Get start TSC
rdtsc
push edx eax
; Wait for end 1 second interval
@@:
push ebp
call [GetSystemTimeAsFileTime]    ; Get count for wait 1 second
cmp [ebp + 4],ebx
jb @b
cmp [ebp + 0],esi
jb @b
; Get end TSC, calculate delta-TSC
rdtsc
pop esi ebx
sub eax,esi
sbb edx,ebx     ; EDX:EAX = Delta TSC per 1 second = frequency, Hz
jb .Error       ; Go error if frequency < 0               
mov ecx,eax
or ecx,edx
jz .Error       ; Go error if frequency = 0

; Store Frequency, as 64-bit integer value, Hz, delta-TSC per second
stosd                   ; Store EAX = Frequency, low dword
xchg eax,edx
stosd                   ; Store EAX = Frequency, high dword

; Load variables and constants into stack
push 0 1000000000 
push 0 0
push 0 1000000
push eax edx

; Calculate floating-point double precision values: frequency and period
finit
fild qword [esp + 00]
fld st0
fidiv dword [esp + 08]
fstp qword [esp + 08]   ; Frequency, MHz
fld1
fdiv st0,st1
fstp qword [esp + 16]   ; Period, Seconds
fild qword [esp + 24]
fdiv st0,st1
fstp qword [esp + 24]   ; Period, Nanoseconds
fstp qword [esp + 00]   ; Frequency, Hz

; Store floating-point double precision values
; Store TSC frequency, Hz, floating point double precision
; Store TSC frequency, MHz, floating point double precision
; Store TSC period, Seconds, floating point double precision
; Store TSC period, Nanoseconds, floating point double precision
mov ecx,8
@@:
pop eax
stosd
loop @b

; Restore RSP, pop extra registers, exit
clc             ; CF=0 (NC) means CPU clock measured OK
.Exit:
pop eax eax ebp esi ebx
ret
.Error:
stc             ; CF=1 (CY) means CPU clock measured ERROR
jmp .Exit

