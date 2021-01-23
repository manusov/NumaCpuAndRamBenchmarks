;--- Measurement result message box, simple variant without drawings ----------;
;    Can be used for debug, also for improve final product useability on       ;
;    platforms with limited functionality                                      ;
;                                                                              ;
; INPUT:  None at registers                                                    ;
;         Statical text strings used                                           ;
;         MPE control and status block used                                    ;
;                                                                              ;
; OUTPUT: None                                                                 ;
;         Registers save/destroy by Microsoft x64 calling convention           ;
;------------------------------------------------------------------------------;

ResultSimple:
push rbx rsi rdi
cld
; Parameters for text build, write first strings
lea r8,[InputParms]
lea r9,[OutputParms]
lea rsi,[MsgResultSimple]
lea rdi,[TEMP_BUFFER]
call StringWrite

; Start input parameters (1) Method, asm instruction
call StringWrite_CRLF_Equal
push rsi
lea rsi,[AM_Strings_Prefix]     ; RSI = Pointer to strings sequence
mov eax,[r8+IPB.UpdatedAsm]     ; AL = Selector
inc al
mov ah,ASM_RESULT_LIMIT
call StringWriteSelected        ; Write string selected by AL from pool [rsi]
pop rsi
; (2) Operand width, bits
call StringWrite_CRLF_Equal
mov eax,[r8+IPB.OperandWidth]
mov bl,0                        ; BL = template (number of chars), 0 means auto
call DecimalPrint32
; (3) Threads, number of execution threads
call StringWrite_CRLF_Equal
mov eax,[r8+IPB.UpdatedThreads]
call DecimalPrint32
; (4) Hyper-threading, absent/not used/used
call StringWrite_CRLF_Equal
push rsi
lea rsi,[HT_00]
mov eax,[r8+IPB.UpdatedHT]
mov ah,HT_LIMIT
call StringWriteSelected
pop rsi
; (5) Large pages
call StringWrite_CRLF_Equal
push rsi
lea rsi,[LP_00]
mov eax,[r8+IPB.UpdatedLP]
mov ah,LP_LIMIT
call StringWriteSelected
pop rsi
; (6) NUMA profile, none/default/force optimal/force non-optimal
call StringWrite_CRLF_Equal
push rsi
lea rsi,[NUMA_00]
mov eax,[r8+IPB.UpdatedNUMA]
mov ah,NUMA_LIMIT
call StringWriteSelected
pop rsi
; (7) Processor Group profile, none/no control/all groups
call StringWrite_CRLF_Equal
push rsi
lea rsi,[PG_00]
mov eax,[r8+IPB.UpdatedPG]
mov ah,PG_LIMIT
call StringWriteSelected
pop rsi
; (8) Target object profile, L1/L2/L3/L4/RAM/Custom = 0/1/2/3/4/5
call StringWrite_CRLF_Equal
push rsi
lea rsi,[Target_00]
mov eax,[r8+IPB.UpdatedTarget]
mov ah,TARGET_LIMIT
call StringWriteSelected
pop rsi
; (9) Prefetch distance
call StringWrite_CRLF_Equal
push rsi
lea rsi,[PD_00]
mov eax,[r8+IPB.UpdatedDistance]
mov ah,PD_LIMIT
call StringWriteSelected
pop rsi
; (10) Operation block size per thread
call StringWrite_CRLF_Equal
mov rax,[r8+IPB.StartBlockSize]
mov bl,1             ; BL = Units select for size print, old = 0FFh = auto
call SizePrint64     ; Note pssible adaptive, but "15.5K" is optimal, yet "K" fixed
; (11) Measurement profile, brief/careful/brief adaptive/careful adaptive
call StringWrite_CRLF_Equal
push rsi
lea rsi,[Measure_00]
mov eax,[r8+IPB.UpdatedMeasure]
; Reject adaptive modes notation because this results for simple mode
cmp al,MEASURE_B_ADAPTIVE
jne @f
mov al,MEASURE_BRIEF
@@:
cmp al,MEASURE_C_ADAPTIVE
jne @f
mov al,MEASURE_CAREFUL
@@:
mov ah,MEASURE_LIMIT
call StringWriteSelected
pop rsi
; (11) Measurement repeats, num. of same operation repeats for precision
call StringWrite_CRLF_Equal
mov rax,[r8+IPB.MeasureRepeats]  ; Note required support overflows, RAX>4GB, actual for adaptive modes
mov bl,0             ; BL = Template (chars) for decimal print, 0 = no template
call DecimalPrint32

; Start memory allocation parameters
; (1,2) Blocks #1 , #2 bases, two blocks used for copy or dot product
call StringWrite_CRLF
mov ecx,2
lea rbx,[r8+IPB.AllocatedBlock1]
.L0:
call StringWrite_CRLF_Equal
mov rax,[rbx]
add rbx,8
test rax,rax
jz .L2
call HexPrint64
mov al,'h'
stosb
jmp .L1
.L2:
push rsi
lea rsi,[Block_00]
call StringWrite
pop rsi
.L1:
loop .L0
; (3,4) Memory allocated total and per thread
mov ecx,2
lea rdx,[r8+IPB.MemoryTotal]
.L3:
call StringWrite_CRLF_Equal
mov rax,[rdx]
add rdx,8
mov bl,0FFh          ; BL = Units select for size print, 0FFh = auto
call SizePrint64
loop .L3

; Start output parameters, (1) dT(ms)
finit                   ; Initialize FPU
call StringWrite_CRLF
call StringWrite_CRLF_Equal
fild [r9+OPB.OStimerDelta]     ; st0 = delta time in 100 ns units 
push 10000              ; 10000 units * 100 ns = 1000000 nS = 1 ms
fidiv dword [rsp]       ; st0 = delta time in 1 ms units 
fstp qword [rsp]
pop rax                 ; RAX = bitfield as double value
mov bx,0100h            ; Float part is 1 digit, Integer part without template
call DoublePrint
call StringWrite
; (2) dTSC(MHz)
call StringWrite_CRLF_Equal
fld [r9+OPB.TSCfrequencyHz]
push 1000000            ; 1000000 units * 1 Hz = 1 MHz
fidiv dword [rsp]
fstp qword [rsp]
pop rax                 ; RAX = bitfield as double value
mov bx,0100h            ; Float part is 1 digit, Integer part without template
call DoublePrint
; (3) Clocks per Instruction (clocks, CPI)
call StringWrite_CRLF_Equal
mov rax,[r8+IPB.StartBlockSize]  ; RAX = Block size, bytes 
xor edx,edx
mov ecx,[r8+IPB.OperandWidth]    ; Must be not zero, otherwise divide overflow
shr ecx,3                        ; RCX = Bytes per instr., convert bits to bytes
div rcx                          ; RCX = Instructions per block
mov rcx,[r8+IPB.MeasureRepeats]
mul rcx                          ; RAX = Total instructions
push rax
fild qword [rsp]                   ; st0 = total instructions
fild qword [r9+OPB.TSCtimerDelta]  ; st0 = total delta TSC, st1 = total instructions  
fdiv st0,st1
fstp qword [rsp]
pop rax                 ; RAX = bitfield as double value
mov r10,rax
mov bx,0300h            ; Float part is 3 digits, Integer part without template
call DoublePrint

; Start final result select Bandwidth or Latency measurement mode
cmp [r8+IPB.UpdatedAsm],LATENCY_MODE
jae .L9

; Branch for Bandwidth
; (1)  Speed(MBPS)
call StringWrite_CRLF_Equal
mov rax,[r8+IPB.StartBlockSize]    ; RAX = Operational block size, bytes 
mov rcx,[r8+IPB.MeasureRepeats]    ; RCX = Repeats, iterations, note 64-bit     
mul rcx                            ; RAX = BlockSize * Repeats
mov ecx,[r8+IPB.UpdatedThreads]    ; Required all threads same size
mul rcx                            ; RAX = BlockSize * Repeats * Threads
push rax
fild qword [rsp]            ; st0 = total bytes executed 
mov dword [rsp],1000000     ; alternative is 1048576 (binary/decimal MB select)
fidiv dword [rsp]           ; st0 = total megabytes executed
fild [r9+OPB.OStimerDelta]
mov dword [rsp],10000000    ; 10000000 units * 100 ns = 1 second
fidiv dword [rsp]
fdivr st0,st1
fstp qword [rsp]
pop rax                 ; RAX = bitfield as double value
mov bx,0200h            ; Float part is 2 digits, Integer part without template
call DoublePrint
jmp .L10

; Branch for Latency 
; (1) Latency (ns)
.L9:
lea rsi,[MsgResultLatency]
call StringWrite_CRLF_Equal
push r10
fld qword [rsp]
fmul [r9+OPB.TSCperiodNs]
fstp qword [rsp]
pop rax
mov bx,0300h            ; Float part is 3 digits, Integer part without template
call DoublePrint

; Done, text termination
.L10:
mov al,0
stosb 
pop rdi rsi rbx
ret
