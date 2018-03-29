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
ShowSingleResult:
push rbx rsi rdi
cld
;--- Parameters for text built, write first strings ---
lea rsi,[MsgResult]
lea rdi,[TempBuffer]
mov rdx,rdi                          ; RDX = Parm#2 = Pointer to string
call StringWrite
;--- Start input parameters --- (1) Method, asm instruction ---
push rsi
;-
; Changed at v0.93 (method name selection for SIMPLE mode: text output)
;-
; lea rsi,[Method_01]                  ; OLD: [Method_00] 
; mov al,[SelectMethod]
; mov ah,17
;-
mov al,18
mul [NonTemporalRead]    ; Supports non-temporal read for AMD
mov cl,al
;-
lea rbx,[AM_Selector]    ; RBX = Base address of translation table
mov al,[SelectMethod]    ; AL = User selected method (CPU instruction set)
cmp al,16
ja .BadMethod
inc eax                  ; + 1 because  means first method, not a INVALID value
cmp [OptionTarget],3     ; If Object=RAM=3 force non-temporal mode
jne @f                   ; Skip for other types: L1/L2/L3/Custom
add al,18                ; Skip first 18 entries: Unknown method + 17 methods
add al,cl
;push rax
;mov al,18
;mul [NonTemporalRead]    ; Supports non-temporal read for AMD
;mov cl,al
;pop rax
;add al,cl
@@:
xlat [rbx]                ; AL = Load byte from [ RBX+AL ]
mov ah,29  ; 18+6-1       ; AH = Limit = last acceptable value
.BadMethod:
lea rsi,[AM_Strings]      ; RSI = Pointer to strings sequence
;-
call StringWriteSelected  ; Write string selected by AL from pool [rsi]
pop rsi
;--- (2) Operand width, bits ---
call StringWrite
mov eax,[OperandWidth]
mov bl,0                 ; BL = template (number of chars), 0 means auto
call DecimalPrint32
;--- (3) Threads, number of execution threads ---
call StringWrite
mov eax,[NumThreads]
call DecimalPrint32
;--- (4) Hyper-threading, absent/not used/used ---
call StringWrite
push rsi
lea rsi,[HT_00]
mov al,[OptionHT]
mov ah,2
call StringWriteSelected
pop rsi
;--- (5) NUMA profile, none/default/force optimal/force non-optimal ---
call StringWrite
push rsi
lea rsi,[NUMA_00]
mov al,[OptionNUMA]
mov ah,3
call StringWriteSelected
pop rsi
;--- (6) Target object profile, L1/L2/L3/RAM/Custom = 0/1/2/3/4 ---
call StringWrite
push rsi
lea rsi,[Target_00]
mov al,[OptionTarget]   ; This also select Non-Temporal mode if Object=RAM=3
mov ah,4
call StringWriteSelected
pop rsi
;--- (7) Measurement profile, brief/careful ---
call StringWrite
push rsi
lea rsi,[Measure_00]
mov al,[OptionMeasure]
mov ah,1
call StringWriteSelected
pop rsi
;--- (8,9) Blocks #1 , #2 bases, two blocks used for copy or dot product ---
mov ecx,2
lea rbx,[BlockBase1]
.L0:
call StringWrite
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
;--- (10) Block size ---
call StringWrite
mov rax,[BlockSize]
; call SizePrint     ; changes v0.92
mov bl,0FFh          ; BL = Units select for size print, 0FFh = auto
call SizePrint64
;--- (11) Measurement repeats, num. of same operation repeats for precision ---
call StringWrite
mov eax,[NumRepeats]
mov bl,0             ; BL = Template (chars) for decimal print, 0 = no template
call DecimalPrint32
;--- Start output parameters --- (1) dT(ms) ---
call StringWrite
mov eax,[dTms]
call DecimalPrint32
;--- (2) dTSC(MHz) ---
call StringWrite
mov eax,[dTSCsec]
call FloatPrintP1
;--- (3) dTSC/Iteration(Clks) ---
; REJECTED
;- call StringWrite
;- mov eax,[dTSCiteration]
;- call FloatPrintP3
;--- (4) dTSC/Instruction(Clks) ---
call StringWrite
mov eax,[dTSCinstruction]
call FloatPrintP3
;--- (5) Speed(MBPS) --- 
call StringWrite
mov eax,[SpeedMBPS]
call DecimalPrint32       ; Can use float, but 5 digits is sufficient precision 
;--- Text termination ---
mov al,0
stosb 
;--- Other parameters ---
xor ecx,ecx	                         ; RCX = Parm#1 = Parent window handle or 0
lea r8,[ProductID]                   ; R8  = Parm#3 = Pointer to caption
xor r9d,r9d                          ; R9  = Parm#4 = Message box icon type
;--- Message box and wait user input ---
mov rbx,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
call [MessageBoxA]
mov rsp,rbx
;--- Exit ---
pop rdi rsi rbx
.L9:
ret

