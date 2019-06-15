;---------- Print double precision value --------------------------------------;
; x87 FPU used, required x87 presence validation by CPUID before call this.    ;
;                                                                              ;
; INPUT:   RAX = Double precision number                                       ;
;          BL  = Number of digits in the INTEGER part,                         ;
;                used for add left non-signed zeroes.                          ; 
;                BL=0 means not print left unsigned zeroes.                    ;
;          BH  = Number of digits in the FLOAT part,                           ;
;                used as precision control.                                    ;
;          RDI = Destination text buffer pointer                               ;
;                                                                              ;
; OUTPUT:  RDI = Modified by text string write                                 ;  
;------------------------------------------------------------------------------;
DoublePrint:
;--- Entry ---
push rax rbx rcx rdx r8 r9 r10 r11
cld
;--- Detect special cases for DOUBLE format, yet unsigned indication ---
mov rdx,07FFFFFFFFFFFFFFFh
and rdx,rax
jz .FP64_Zero                ; Go if special cases = 0.0  or  -0.0
mov rcx,07FF8000000000000h
cmp rdx,rcx
je .FP64_QNAN                ; Go if special case = QNAN (Quiet Not a Number)
mov rcx,07FF0000000000000h
cmp rdx,rcx
je .FP64_INF                 ; Go if special case = INF (Infinity)
ja .FP64_NAN                 ; Go if special case = NAN (Not a Number)
;--- Initializing FPU x87 ---
finit
;--- Change rounding mode from default (nearest) to truncate ---  
push rax     ; save input value
push rax     ; reserve space
fstcw [rsp]
pop rax
or ax,0C00h  ; correct Rounding Control, RC = FPU CW bits [11-10]
push rax
fldcw [rsp]
pop rax
;--- Load input value, note rounding mode already changed ---
fld qword [rsp]
pop rax
;--- Separate integer and float parts --- 
fld st0         ; st0 = value   , st1 = value copy
frndint         ; st0 = integer , st1 = value copy
fxch            ; st0 = value copy , st1 = integer
fsub st0,st1    ; st0 = float , st1 = integer
;--- Build divisor = f(precision selected) --- 
mov eax,1
movzx ecx,bh    ; BH = count digits after "."
jrcxz .L6
@@:
imul rax,rax,10
loop @b
.L6:
;--- Build float part as integer number --- 
push rax
fimul dword [rsp]
pop rax
;--- Extract signed Binary Coded Decimal (BCD) to R9:R8, float part .X ---
push rax rax  ; Make frame for stack variable, used for x87 write data
fbstp [rsp]   ; Store BCD integer and pop, destination is 80 bit = 10 bytes
pop r8 r9     ; R9:R8 = data from x87 write
;--- Extract signed Binary Coded Decimal (BCD) to R11:R10, integer part X. ---
push rax rax  ; Make frame for stack variable, used for x87 write data
fbstp [rsp]   ; Store BCD integer and pop, destination is 80 bit = 10 bytes
pop r10 r11     ; R11:R10 = data from x87 write
;--- Check sign of integer and float part --- 
bt r11,15     ; R11 bit 15 is bit 79 of 80-bit x87 operand (integer part)
setc dl       ; DL = Sign of integer part
bt r9,15      ; R9 bit 15 is bit 79 of 80-bit x87 operand (floating part)
setc dh       ; DH = Sign of floating part
;--- Go error if sign of integer and float part mismatch ---

;-
; This comparision and error branching rejected 
; because bug with -1.0 "-" AND "+", CHECK IF SIGN SAVED ?
; cmp dx,0100h
; je .Error
; cmp dx,0001h
; je .Error
; Write "-" if one of signs "-".
;-

test dx,dx
jz @f            ; Go skip write "-" if both integer/floating signs "+"

;--- Write "-" if negative value ---
mov al,'-'
stosb
@@:
;--- Write INTEGER part --- 
mov dl,0         ; DL = flag "minimum one digit always printed"
mov ecx,18       ; RCX = maximum number of digits in the integer part 
.L3:             ; Cycle for digits in the INTEGER part
mov al,r11l
shr al,4         ; AL = current digit
cmp cl,1
je .L2           ; Go print if last pass, otherwise .X instead 0.X
cmp cl,bl
jbe .L2          ; Go print if required by formatting option, BL=count
test dl,dl
jnz .L2          ; Go print, if digits sequence already beginned
test al,al
jz .L1           ; Otherwise, can go skip print if digit = 0 
.L2:
mov dl,1
or al,30h
stosb            ; Write current ASCII digit
.L1:
shld r11,r10,4   ; Positioning digits sequence at R11:R10 pair
shl r10,4
loop .L3         ; Cycle for digits in the INTEGER part
;--- Write decimal point ---
test bh,bh
jz .Exit        ; Skip if not print float part
mov al,'.'
stosb
;--- Write FLOATING part ---
std                  ; Write from right to left 
movzx ecx,bh         ; RCX = digits count     
lea rdi,[rdi+rcx]    ; RDI = After last digit (char) position
push rdi
dec rdi
.L4:                 ; Cycle for digits in the FLOATING part
mov al,r8l
and al,0Fh
or al,30h
stosb
shrd r8,r9,4         ; Positioning digits sequence at R9:R8 pair
shr r9,4
loop .L4             ; Cycle for digits in the FLOATING part
pop rdi
cld                  ; Restore strings increment mode
;--- Go exit subroutine ---
jmp .Exit
;--- Write strings for different errors types ---
.FP64_Zero:					; Zero
mov eax,'0.0 '
jmp .FP64_02
.FP64_INF:          ; "INF" = Infinity, yet unsigned infinity indicated
mov eax,'INF '
jmp .FP64_02
.FP64_NAN:
mov eax,'NAN '      ; "NAN" = (Signaled) Not a number
jmp .FP64_02
.FP64_QNAN:
mov eax,'QNAN'      ; "QNAN" = Quiet not a number
.FP64_02:
stosd
jmp .Exit
.Error:
mov al,'?'
stosb
.Exit:
;--- Exit with re-initialize x87 FPU --- 
finit
pop r11 r10 r9 r8 rdx rcx rbx rax
ret

