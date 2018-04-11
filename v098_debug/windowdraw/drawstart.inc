;-------------------------------------------------------------------;
; Benchmarks drawings speed=f(size or address), process open        ;
; Benchmarks mode settings = F(Options)                             ;
; Get options required for run                                      ; 
; simple and drawings benchmarks after dialogue                     ;
; Procedure 1 of 3 = DrawStart, DrawProgress, DrawStop              ;
;                                                                   ;
; INPUT:  Dialogue control block, parameters from OS API            ;
; OUTPUT: Options variables fields, used for benchmarks control     ;
;                                                                   ;
;-------------------------------------------------------------------;

DrawStart:

; Get options 
call SimpleStart

; Load base address for compact offsets
lea r15,[BasePoint]

; Prepare method name string for visual
; IMPORTANT patch v0.98.0 = non-temporal read use BOTH FOR INTEL AND AMD
mov al,18
mul [InputParms.NonTemporalRead]  ; Supports non-temporal read
mov cl,al
lea rsi,[AM_Strings]              ; RSI = Pointer to strings sequence
lea rbx,[AM_Selector]             ; RBX = Pointer to array of strings numbers
mov al,[InputParms.SelectMethod]  ; RAX = OptionMethod, CPU instruction set
cmp al,16
ja .BadMethod
inc eax                  ; + 1 because  means first method, not a INVALID value
cmp [InputParms.OptionTarget],3     ; If Object=RAM=3 force non-temporal mode
jne @f                   ; Skip for other types: L1/L2/L3/Custom
add al,18                ; Skip first 18 entries: Unknown method + 17 methods
add al,cl
@@:
xlat [rbx]                           ; AL = Load byte from [ RBX+AL ]
cmp al,31  ; old=29 , v0.98.2 patch  ; AH = Limit = last acceptable value
ja .BadMethod
mov ah,al
.SelectMethod:           ; First string always skipped
lodsb
cmp al,0
jne .SelectMethod        ; Find zero for skip one string
dec ah
jnz .SelectMethod        ; Skip required number of strings
.BadMethod:
sub rsi,r15
mov [DialogueContent_Win1+HANDLE_TEXT_METHOD1*16-256+12],si

; Set string visual under drawings sub-window: Object (L1,L2,L3,RAM, Custom)
lea rsi,[Unknown_Object]          ; RSI = Pointer to objects strings sequence
mov ah,[InputParms.OptionTarget]  ; AH = OptionBlock
movzx edx,ah                      ; DL = Copy for later use
cmp ah,4
ja .BadObject   
inc ah
.SelectObject:                    ; First string always skipped
lodsb
cmp al,0
jne .SelectObject
dec ah
jnz .SelectObject 
.BadObject:
sub rsi,r15
mov [DialogueContent_Win1+HANDLE_TEXT_OBJECT1*16-256+12],si

; Set number of instructions for current X
; This default settings for prevent use undefined state of this variable,
; benchmark procedure overwrite it when drawing Y=F(X)
mov [BenchInstructions],1
; Save registers used for prints
push rbx rdi
mov bl,0
; Print number of threads, see also at drawprogress.inc
lea rdi,[Threads_Number]
mov word [rdi],'? '
mov byte [rdi+2],0
mov eax,[InputParms.NumThreads]
test eax,eax
jz @f                 ; Skip if number of threads = 0
cmp eax,64
ja @f                 ; Skip if Number of threads > 64 (means error, not sup.)
call DecimalPrint32
@@:
; Print number of repeats
lea rdi,[Repeats_Number]
push rdi 
mov ecx,9
mov al,' '
rep stosb
pop rdi 
mov eax,[InputParms.NumRepeats]
call DecimalPrint32
; Print Normal or Large pages option state
lea rax,[NPages_String]
cmp [InputParms.OptionLP],2
jne @f 
lea rax,[LPages_String]
@@:
sub rax,r15
mov [DialogueContent_Win1+HANDLE_LARGE_PAGES1*16-256+12],ax
pop rdi rbx

; Set bytes per instruction(s) or hardware iteration cycle
mov eax,[InputParms.OperandWidth]
shr eax,3                        ; Convert from bits to bytes
mov [BenchBPI],rax

; Set instructions per pixel X and other X-parameters
; Example#1 : 1 unstruction = 32 bytes , 32 * 2 = 64 bytes per pixel for X
; Example#2 : 1 unstruction = 8 bytes  , 8 * 8 = 64 bytes per pixel for X
xchg ecx,eax                     ; ECX = Bytes per Instruction

; Calculating BPP = Bytes per pixel, get block size first
; Here valid RDX = OptionBlock
; DL=4 means custom block size mode 
mov rax,[BlockSizeEdit]          ; RAX = Custom Block size
cmp dl,4
jae .SizeReady                   ; Go if selected object = Custom or 4+
; DL=3 means RAM benchmark mode 
mov rax,[InputParms.BlockSize]   ; RAX = RAM Block size
cmp dl,3
je .SizeReady                    ; Go if selected object = RAM
; Otherwise Cache benchmark mode
imul edx,edx,12                  ; 12 bytes per cache entry
mov rax,[rdx+CacheL1Data]        ; RAX = Cache size, bytes, L1,L2,L3
.SizeReady:
; RAX = Analysed object size, this size must be at X-center of screen
; BPP = Bytes per Pixel = RAX / Pixels at half X
; For prevent approximation, BPP must be aligned by 64
mov esi,SUBWINX/2
xor edx,edx
div esi                      ; EAX = BPP, Bytes per pixel 

; Select units: Bytes, Kilobytes, Megabytes
lea rdi,[SelectUnits]
mov byte [rdi],0             ; BYTE [SelectUnits] = 0 , Bytes/KB/MB
cmp eax,512
jb @f                        ; Go if BPP < 0.5K , Units=Bytes 
inc byte [rdi]
add eax,0000003FFh
and eax,0FFFFFC00h
cmp eax,512*1024             ; Go if BPP < 0.5M , Units=Kilobytes
jb @f
inc byte [rdi]
add eax,0000FFFFFh
and eax,0FFF00000h
@@:                          ; Otherwise Units=Megabytes

; Here: EAX=BPP, ECX=BPI, set instructions per pixel
add eax,00000003Fh
and eax,0FFFFFFC0h           ; EAX = BPP, aligned by 64 
push rax
cdq                          ; Clear EDX = 0
div ecx                      ; EAX = BPP / BPI 
mov dword [Xmultiplier],eax
pop rax

; Set default Maximum MBPS for Y-sizing at first pass
; Note at next passes Y-size selected automatic as F(Maximum Detected MBPS)
mov [MaximumMBPS],0 

; Override defaults for Cache&RAM mode, Set units per grid Y
mov [ValueGridY],DEFAULT_Y_MBPS_PER_GRID

; Set total MBPS per Y, for all sub-screen
mov [Ydivisor],DEFAULT_Y_MBPS_PER_PIXEL 

; Set units per grid X
mov edx,GRIDSTEPX
mov cl,[rdi]
imul ecx,ecx,10
shr eax,cl
mul edx
xchg edx,eax
mov [ValueGridX],edx

; Exit
ret

