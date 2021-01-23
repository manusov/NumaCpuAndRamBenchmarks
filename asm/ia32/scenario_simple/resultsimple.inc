;--- Measurement result message box, simple variant without drawings ----------;
;    Can be used for debug, also for improve final product useability on       ;
;    platforms with limited functionality                                      ;
;                                                                              ;
; INPUT:  None at registers                                                    ;
;         Statical text strings used                                           ;
;         MPE control and status block used                                    ;
;                                                                              ;
; OUTPUT: None                                                                 ;
;         Registers save/destroy by Microsoft ia32 calling convention          ;
;------------------------------------------------------------------------------;

ResultSimple:
push ebx esi edi ebp
cld

; Parameters for text build, write first strings
lea esi,[MsgResultSimple]
lea edi,[TEMP_BUFFER]
call StringWrite

; Start input parameters (1) Method, asm instruction
call StringWrite_CRLF_Equal
push esi
lea esi,[AM_Strings_Prefix]             ; ESI = Pointer to strings sequence
mov eax,[InputParms + IPB.UpdatedAsm]   ; AL = Selector
inc al              
mov ah,ASM_RESULT_LIMIT
call StringWriteSelected        ; Write string selected by AL from pool [esi]
pop esi
; (2) Operand width, bits
call StringWrite_CRLF_Equal
mov eax,[InputParms + IPB.OperandWidth]
mov bl,0                        ; BL = template (number of chars), 0 means auto
call DecimalPrint32
; (3) Threads, number of execution threads
call StringWrite_CRLF_Equal
mov eax,[InputParms + IPB.UpdatedThreads]
call DecimalPrint32
; (4) Hyper-threading, absent/not used/used
call StringWrite_CRLF_Equal
push esi
lea esi,[HT_00]
mov eax,[InputParms + IPB.UpdatedHT]
mov ah,HT_LIMIT
call StringWriteSelected
pop esi
; (5) Large pages
call StringWrite_CRLF_Equal
push esi
lea esi,[LP_00]
mov eax,[InputParms + IPB.UpdatedLP]
mov ah,LP_LIMIT
call StringWriteSelected
pop esi
; (6) NUMA profile, none/default/force optimal/force non-optimal
call StringWrite_CRLF_Equal
push esi
lea esi,[NUMA_00]
mov eax,[InputParms + IPB.UpdatedNUMA]
mov ah,NUMA_LIMIT
call StringWriteSelected
pop esi
; (7) Processor Group profile, none/no control/all groups
call StringWrite_CRLF_Equal
push esi
lea esi,[PG_00]
mov eax,[InputParms + IPB.UpdatedPG]
mov ah,PG_LIMIT
call StringWriteSelected
pop esi
; (8) Target object profile, L1/L2/L3/L4/RAM/Custom = 0/1/2/3/4/5
call StringWrite_CRLF_Equal
push esi
lea esi,[Target_00]
mov eax,[InputParms + IPB.UpdatedTarget]
mov ah,TARGET_LIMIT
call StringWriteSelected
pop esi
; (9) Prefetch distance
call StringWrite_CRLF_Equal
push esi
lea esi,[PD_00]
mov eax,[InputParms + IPB.UpdatedDistance]
mov ah,PD_LIMIT
call StringWriteSelected
pop esi
; (10) Operation block size per thread
call StringWrite_CRLF_Equal
mov eax,[InputParms + IPB.StartBlockSize]
xor edx,edx
mov bl,1             ; BL = Units select for size print, old = 0FFh = auto
call SizePrint64     ; Note pssible adaptive, but "15.5K" is optimal, yet "K" fixed
; (11) Measurement profile, brief/careful/brief adaptive/careful adaptive
call StringWrite_CRLF_Equal
push esi
lea esi,[Measure_00]
mov eax,[InputParms + IPB.UpdatedMeasure]
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
pop esi
; (12) Measurement repeats, num. of same operation repeats for precision
call StringWrite_CRLF_Equal
mov eax,dword [InputParms + IPB.MeasureRepeats]  ; Note required support overflows, RAX>4GB, actual for adaptive modes
mov bl,0                                         ; BL = Template (chars) for decimal print, 0 = no template
call DecimalPrint32

; Start memory allocation parameters
; (1,2) Blocks #1 , #2 bases, two blocks used for copy or dot product
call StringWrite_CRLF
mov ecx,2
lea ebx,[InputParms + IPB.AllocatedBlock1]
.L0:
call StringWrite_CRLF_Equal
mov eax,[ebx]
add ebx,4
test eax,eax
jz .L2
call HexPrint32
mov al,'h'
stosb
jmp .L1
.L2:
push esi
lea esi,[Block_00]
call StringWrite
pop esi
.L1:
loop .L0
; (3,4) Memory allocated total and per thread
mov ecx,2
lea edx,[InputParms + IPB.MemoryTotal]
.L3:
call StringWrite_CRLF_Equal
push edx
mov eax,[edx + 0]
xor edx,edx
mov bl,0FFh          ; BL = Units select for size print, 0FFh = auto
call SizePrint64
pop edx
add edx,4
loop .L3

; Start output parameters, (1) dT(ms)
finit                   ; Initialize FPU
call StringWrite_CRLF
call StringWrite_CRLF_Equal
fild [OutputParms + OPB.OStimerDelta]     ; st0 = delta time in 100 ns units 
push 0 10000            ; 10000 units * 100 ns = 1000000 nS = 1 ms
fidiv dword [esp]       ; st0 = delta time in 1 ms units 
fstp qword [esp]
pop eax edx             ; EDX:EAX = bitfield as double value
mov bx,0100h            ; Float part is 1 digit, Integer part without template
call DoublePrint
call StringWrite
; (2) dTSC(MHz)
call StringWrite_CRLF_Equal
fld [OutputParms + OPB.TSCfrequencyHz]
push 0 1000000            ; 1000000 units * 1 Hz = 1 MHz
fidiv dword [esp]
fstp qword [esp]
pop eax edx             ; EDX:EAX = bitfield as double value
mov bx,0100h            ; Float part is 1 digit, Integer part without template
call DoublePrint
; (3) Clocks per Instruction (clocks, CPI)
call StringWrite_CRLF_Equal
mov eax,[InputParms + IPB.StartBlockSize]   ; EDX:EAX = Block size, bytes 
xor edx,edx
mov ecx,[InputParms +IPB.OperandWidth]   ; Must be not zero, otherwise divide overflow
shr ecx,3                    ; ECX = Bytes per instr., convert bits to bytes
div ecx                      ; ECX = Instructions per block
mov ecx,dword [InputParms + IPB.MeasureRepeats]  ; Note overflow possible 32/64 !
mul ecx                          ; RAX = Total instructions
push edx eax
fild qword [esp]                   ; st0 = total instructions
fild qword [OutputParms + OPB.TSCtimerDelta]  ; st0 = total delta TSC, st1 = total instructions  
fdiv st0,st1
fstp qword [esp]
pop eax edx             ; EDX:EAX = bitfield as double value
mov bx,0300h            ; Float part is 3 digits, Integer part without template
call DoublePrint

; Start final result select Bandwidth or Latency measurement mode
cmp [InputParms + IPB.UpdatedAsm],LATENCY_MODE
jae .L9

; Branch for Bandwidth
; (1)  Speed(MBPS)
call StringWrite_CRLF_Equal
push edx eax
fild [InputParms + IPB.MeasureRepeats]   ; st0 = Repeats, iterations, note 64-bit
fimul [InputParms + IPB.StartBlockSize]  ; st0 * Operational block size, bytes 
fimul [InputParms + IPB.UpdatedThreads]   ; Required all threads same size
; At this point, st0 = BlockSize * Repeats * Threads
mov dword [esp],1000000     ; alternative is 1048576 (binary/decimal MB select)
fidiv dword [esp]           ; st0 = total megabytes executed
fild [OutputParms + OPB.OStimerDelta]
mov dword [esp],10000000    ; 10000000 units * 100 ns = 1 second
fidiv dword [esp]
fdivr st0,st1
fstp qword [esp]
pop eax edx             ; EDX:EAX = bitfield as double value
mov bx,0200h            ; Float part is 2 digits, Integer part without template
call DoublePrint
jmp .L10

; Branch for Latency 
; (1) Latency (ns)
.L9:
lea esi,[MsgResultLatency]
call StringWrite_CRLF_Equal
push edx eax
fld qword [esp]
fmul [OutputParms + OPB.TSCperiodNs]
fstp qword [esp]
pop eax edx
mov bx,0300h            ; Float part is 3 digits, Integer part without template
call DoublePrint

; Done, text termination
.L10:
mov al,0
stosb 
pop ebp edi esi ebx
ret
