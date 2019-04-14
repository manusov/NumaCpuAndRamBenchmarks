;------------------------------------------------------------------------------;
; Measurement result message box, simple variant without drawings.             ;
; Can be used for debug, also for improve final product useability on          ;
; platforms with limited functionality                                         ;
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

;--- OLD ---
; Start input parameters, (1) Method, asm instruction
; IMPORTANT patch v0.98.0 = non-temporal read use BOTH FOR INTEL AND AMD
;push rsi
;mov al,18
;mul [InputParms.NonTemporalRead]     ; Supports non-temporal read
;mov cl,al
;lea rbx,[AM_Selector]                ; RBX = Base address of translation table
;mov al,[InputParms.SelectMethod]     ; AL = User selected method (CPU instruction set)
;cmp al,16
;ja .BadMethod
;inc eax                            ; + 1 because  means first method, not a INVALID value
;cmp [InputParms.OptionTarget],3    ; If Object=RAM=3 force non-temporal mode
;jne @f                             ; Skip for other types: L1/L2/L3/Custom
;add al,18                          ; Skip first 18 entries: Unknown method + 17 methods
;add al,cl
;@@:
;xlat [rbx]                           ; AL = Load byte from [ RBX+AL ]
;mov ah,32  ; old=31 , v0.99.0 patch  ; AH = Limit = last acceptable value
;.BadMethod:
;lea rsi,[AM_Strings]      ; RSI = Pointer to strings sequence
;call StringWriteSelected  ; Write string selected by AL from pool [rsi]
;pop rsi
;--- OLD ---

; Start input parameters (1) Method, asm instruction
call StringWrite_CRLF_Equal
push rsi
lea rsi,[AM_Strings_Prefix]     ; RSI = Pointer to strings sequence
mov eax,[r8+IPB.UpdatedAsm]    ; AL = Selector
inc al
mov ah,AM_LIMIT
call StringWriteSelected        ; Write string selected by AL from pool [rsi]
pop rsi

;--- OLD ---
; (2) Operand width, bits
;call StringWrite
;mov eax,[InputParms.OperandWidth]
;mov bl,0                 ; BL = template (number of chars), 0 means auto
;call DecimalPrint32
;--- OLD ---

; (2) Operand width, bits
call StringWrite_CRLF_Equal
mov eax,[r8+IPB.OperandWidth]
mov bl,0                        ; BL = template (number of chars), 0 means auto
call DecimalPrint32

;--- OLD ---
; (3) Threads, number of execution threads
;call StringWrite
;mov eax,[InputParms.NumThreads]
;call DecimalPrint32
;--- OLD ---

; (3) Threads, number of execution threads
call StringWrite_CRLF_Equal
mov eax,[r8+IPB.UpdatedThreads]
call DecimalPrint32

;--- OLD ---
; (4) Hyper-threading, absent/not used/used
;call StringWrite
;push rsi
;lea rsi,[HT_00]
;mov al,[InputParms.OptionHT]
;mov ah,2
;call StringWriteSelected
;pop rsi
;--- OLD ---

; (4) Hyper-threading, absent/not used/used
call StringWrite_CRLF_Equal
push rsi
lea rsi,[HT_00]
mov eax,[r8+IPB.UpdatedHT]
mov ah,HT_LIMIT
call StringWriteSelected
pop rsi

;--- OLD ---
; (5) Large pages
;call StringWrite
;push rsi
;lea rsi,[LP_00]
;mov al,[InputParms.OptionLP]
;mov ah,2
;call StringWriteSelected
;pop rsi
;--- OLD ---

; (5) Large pages
call StringWrite_CRLF_Equal
push rsi
lea rsi,[LP_00]
mov eax,[r8+IPB.UpdatedLP]
mov ah,LP_LIMIT
call StringWriteSelected
pop rsi

;--- OLD ---
; (6) NUMA profile, none/default/force optimal/force non-optimal
;call StringWrite
;push rsi
;lea rsi,[NUMA_00]
;mov al,[InputParms.OptionNUMA]
;mov ah,3
;call StringWriteSelected
;pop rsi
;--- OLD ---

; (6) NUMA profile, none/default/force optimal/force non-optimal
call StringWrite_CRLF_Equal
push rsi
lea rsi,[NUMA_00]
mov eax,[r8+IPB.UpdatedNUMA]
mov ah,NUMA_LIMIT
call StringWriteSelected
pop rsi

;--- OLD ---
; (7) Target object profile, L1/L2/L3/RAM/Custom = 0/1/2/3/4
;call StringWrite
;push rsi
;lea rsi,[Target_00]
;mov al,[InputParms.OptionTarget]  ; This also select Non-Temporal mode if Object=RAM=3
;mov ah,4
;call StringWriteSelected
;pop rsi
;--- OLD ---

; (7) Target object profile, L1/L2/L3/L4/RAM/Custom = 0/1/2/3/4/5
call StringWrite_CRLF_Equal
push rsi
lea rsi,[Target_00]
mov eax,[r8+IPB.UpdatedTarget]
mov ah,TARGET_LIMIT
call StringWriteSelected
pop rsi

;--- OLD ---
; (8) Measurement profile, brief/careful
;call StringWrite
;push rsi
;lea rsi,[Measure_00]
;mov al,[InputParms.OptionMeasure]
;mov ah,1
;call StringWriteSelected
;pop rsi
;--- OLD ---

; (8) Measurement profile, brief/careful/brief adaptive/careful adaptive
call StringWrite_CRLF_Equal
push rsi
lea rsi,[Measure_00]
mov eax,[r8+IPB.UpdatedMeasure]
mov ah,MEASURE_LIMIT
call StringWriteSelected
pop rsi

;--- OLD ---
; (9,10) Blocks #1 , #2 bases, two blocks used for copy or dot product
;mov ecx,2
;lea rbx,[InputParms.BlockBase1]
;.L0:
;call StringWrite
;mov rax,[rbx]
;add rbx,8
;test rax,rax
;jz .L2
;call HexPrint64
;mov al,'h'
;stosb
;jmp .L1
;.L2:
;push rsi
;lea rsi,[Block_00]
;call StringWrite
;pop rsi
;.L1:
;loop .L0
;--- OLD ---

; (9,10) Blocks #1 , #2 bases, two blocks used for copy or dot product
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

;--- OLD ---
; (11) Block size
;call StringWrite
;mov rax,[InputParms.BlockSize]
;mov bl,0FFh          ; BL = Units select for size print, 0FFh = auto
;call SizePrint64
;--- OLD ---

; (11) Block size
call StringWrite_CRLF_Equal
mov rax,[r8+IPB.StartBlockSize]
mov bl,0FFh          ; BL = Units select for size print, 0FFh = auto
call SizePrint64

;--- OLD ---
; (12) Measurement repeats, num. of same operation repeats for precision
;call StringWrite
;mov eax,[InputParms.NumRepeats]
;mov bl,0             ; BL = Template (chars) for decimal print, 0 = no template
;call DecimalPrint32
;--- OLD ---

; (12) Measurement repeats, num. of same operation repeats for precision
call StringWrite_CRLF_Equal
mov eax,[r8+IPB.MeasureRepeats]
mov bl,0             ; BL = Template (chars) for decimal print, 0 = no template
call DecimalPrint32

;--- OLD ---
; Start output parameters --- (1) dT(ms)
;finit                   ; Initialize FPU
;call StringWrite
;fild [OutputParms.OStimerDelta]     ; st0 = delta time in 100 ns units 
;push 10000              ; 10000 units * 100 ns = 1000000 nS = 1 ms
;fidiv dword [rsp]       ; st0 = delta time in 1 ms units 
;fstp qword [rsp]
;pop rax                 ; RAX = bitfield as double value
;mov bx,0100h            ; Float part is 1 digit, Integer part without template
;call DoublePrint
;--- OLD ---

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

;--- OLD ---
; (2) dTSC(MHz)
;call StringWrite
;fild [OutputParms.TSCfrequencyHz]
;push 1000000            ; 1000000 units * 1 Hz = 1 MHz
;fidiv dword [rsp]
;fstp qword [rsp]
;pop rax                 ; RAX = bitfield as double value
;mov bx,0100h            ; Float part is 1 digit, Integer part without template
;call DoublePrint
;--- OLD ---

; (2) dTSC(MHz)
call StringWrite_CRLF_Equal
fild [r9+OPB.TSCfrequencyHz]
push 1000000            ; 1000000 units * 1 Hz = 1 MHz
fidiv dword [rsp]
fstp qword [rsp]
pop rax                 ; RAX = bitfield as double value
mov bx,0100h            ; Float part is 1 digit, Integer part without template
call DoublePrint

;--- OLD ---
; (3) Clocks per Instruction (clocks)
;call StringWrite
;mov rax,[InputParms.BlockSize]      ; RAX = Block size, bytes 
;xor edx,edx
;mov ecx,[InputParms.OperandWidth]   ; Must be not zero, otherwise divide overflow
;shr ecx,3                    ; RCX = Bytes per instr., convert bits to bytes
;div rcx                      ; RCX = Instructions per block
;mov ecx,[InputParms.NumRepeats]
;mul rcx                      ; RAX = Total instructions
;push rax
;fild qword [rsp]                        ; st0 = total instructions
;fild qword [OutputParms.TSCtimerDelta]  ; st0 = total delta TSC, st1 = total instructions  
;fdiv st0,st1
;fstp qword [rsp]
;pop rax                 ; RAX = bitfield as double value
;mov bx,0300h            ; Float part is 3 digits, Integer part without template
;call DoublePrint
;--- OLD ---

; (3) Clocks per Instruction (clocks)
call StringWrite_CRLF_Equal
mov rax,[r8+IPB.StartBlockSize]  ; RAX = Block size, bytes 
xor edx,edx
mov ecx,[r8+IPB.OperandWidth]    ; Must be not zero, otherwise divide overflow
shr ecx,3                        ; RCX = Bytes per instr., convert bits to bytes
div rcx                          ; RCX = Instructions per block
mov ecx,[r8+IPB.MeasureRepeats]
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

;--- OLD ---
; (4) Speed(MBPS) 
;call StringWrite
;mov rax,[InputParms.BlockSize]
;mov ecx,[InputParms.NumRepeats]
;mul rcx                            ; RAX = BlockSize * Repeats
;mov ecx,[InputParms.NumThreads]    ; Required all threads same size
;mul rcx                            ; RAX = BlockSize * Repeats * Threads
;push rax
;fild qword [rsp]            ; st0 = total bytes executed 
;; Select binary or decimal megabytes
;; mov dword [rsp],1048576
;  mov dword [rsp],1000000
;; end of patched
;fidiv dword [rsp]           ; st0 = total megabytes executed
;fild [OutputParms.OStimerDelta]
;mov dword [rsp],10000000    ; 10000000 units * 100 ns = 1 second
;fidiv dword [rsp]
;fdivr st0,st1
;fstp qword [rsp]
;pop rax                 ; RAX = bitfield as double value
;mov bx,0200h            ; Float part is 2 digits, Integer part without template
;call DoublePrint
;--- OLD ---

cmp [r8+IPB.UpdatedAsm],LATENCY_MODE
jae .L9

; (4 for Bandwidth) Speed(MBPS)
call StringWrite_CRLF_Equal
mov rax,[r8+IPB.StartBlockSize]
mov ecx,[r8+IPB.MeasureRepeats]
mul rcx                            ; RAX = BlockSize * Repeats
mov ecx,[r8+IPB.UpdatedThreads]    ; Required all threads same size
mul rcx                            ; RAX = BlockSize * Repeats * Threads
push rax
fild qword [rsp]            ; st0 = total bytes executed 
; Select binary or decimal megabytes
; mov dword [rsp],1048576
  mov dword [rsp],1000000
; end of patched
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

; (4 for Latency) Speed(MBPS)
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

.L10:
; Text termination
mov al,0
stosb 

; Input parameters for build message box
;xor ecx,ecx             ; RCX = Parm#1 = Parent window handle or 0
;lea rdx,[TEMP_BUFFER]   ; RDX = Dialogue window Parm#2 = Pointer to string
;lea r8,[PRODUCT_ID]     ; R8  = Parm#3 = Pointer to caption
;xor r9d,r9d             ; R9  = Parm#4 = Message box icon type

; Message box and wait user input
;mov rbx,rsp
;and rsp,0FFFFFFFFFFFFFFF0h
;sub rsp,32
;call [MessageBoxA]
;mov rsp,rbx

; Exit
pop rdi rsi rbx
ret
