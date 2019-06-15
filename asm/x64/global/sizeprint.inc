;---------- Print memory block size as Integer.Float ----------;
; Float part is 1 char, use P1-version of Floating Print       ;
; If rounding precision impossible, print as hex               ;
; Only x.5 floating values supported, otherwise as hex         ;
;                                                              ;
; INPUT:   RAX = Number value, units = Bytes                   ;
;          BL  = Force units (override as smallest only)       ;
;                FF = No force units, auto select              ;
;                0 = Bytes, 1 = KB, 2 = MB, 3 = GB, 4 = TB     ;
;          RDI = Destination Pointer (flat)                    ;
; OUTPUT:  RDI = New Destination Pointer (flat)                ;
;                modified because string write                 ;
;--------------------------------------------------------------;
SizePrint64:
push rax rbx rcx rdx rsi
cld
cmp bl,0FFh
je .L22
;--- Adjust to requested units ( Bytes, KB, MB, GB, TB ) ---
mov esi,1
movzx ecx,bl
jrcxz .L11
.L10:
shl rsi,10
loop .L10
.L11:
mov cl,bl
xor edx,edx
div rsi          ; EAX = Integer part, note overflows ignored if explicit units
mov bl,0
call DecimalPrint32
imul eax,edx,10
div rsi          ; EAX = Float part
cmp cl,0
je .L24
push rax
mov al,'.'
stosb
pop rax
jmp .L23
;--- Auto-select units ( Bytes, KB, MB, GB, TB ) by mod=0 criteria ---
.L22:
xor ecx,ecx                 ; ECX = Units selector
test rax,rax
jz .L23                     ; Go if value  = 0
.L0:
mov rbx,rax                 ; RBX = Save previous value
xor edx,edx                 ; RDX = Dividend bits [127-64] = 0
mov esi,1024                ; RSI = Divisor                           
div rsi
mov esi,0FFFFFFFFh
cmp rbx,rsi
ja .L1                      ; Go execute next division if value > 32-bit
test rdx,rdx
jnz .L2                     ; Go print if mod non-zero
.L1:
inc ecx                     ; Units selector + 1
jmp .L0                     ; Make cycle for select optimal units
;--- Check overflow ---
.L2:
cmp ecx,4
ja .L20                     ; Go print hex if units too big
;--- Print value and units ---
mov eax,ebx
.L23:
mov bl,0
call DecimalPrint32         ; Print value
.L24:
mov al,' '
stosb
lea rsi,[U_B]
mov al,cl
mov ah,4
call StringWriteSelected    ; Print units
jmp .L21
;--- Entry point for print as HEX if value too big ---
.L20:
call HexPrint64             ; Print 64-bit hex integer: number of Bytes
mov al,'h'
stosb 
;--- Exit ---
.L21:
pop rsi rdx rcx rbx rax
ret

