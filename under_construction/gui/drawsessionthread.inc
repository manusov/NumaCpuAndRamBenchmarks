
DrawSessionThread:

; Entry point
push rbx rsi rdi rbp r12 r13 r14 r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
push rbp
sub rsp,32

; Load pointer to thread control structure
lea r15,[DrawThreadParms]

; Signaling thread START
mov rcx,[r15 + DTHP.EventStart]   ; RCX = Parm#1 = Handle for thread start event
call [SetEvent]                   ; Call WinAPI function

; Measurement initialization

; Measurement cycle
.BackgroundProcess:

cmp [r15 + DTHP.MeasureCounter],MEASURE_POINTS_COUNT
jae .BackgroundSkip

; DEBUG
mov ecx,90000h
loop $
; DEBUG
mov eax,[r15 + DTHP.MeasureCounter]
imul eax,eax,16
lea rdi,[r15 + DTHP.MeasureArray + rax]
mov eax,2
stosq
mov eax,1
stosq
; DEBUG

;cmp [r15 + DTHP.MeasureBreak],0
;jne .BackgroundBreak 
;inc [r15 + DTHP.MeasureCounter]
;cmp [r15 + DTHP.MeasureCounter],MEASURE_POINTS_COUNT
;jb .BackgroundProcess 

inc [r15 + DTHP.MeasureCounter]

.BackgroundSkip:
cmp [r15 + DTHP.MeasureBreak],0
je .BackgroundProcess 

.BackgroundBreak: 
; mov [r15 + DTHP.MeasureBreak],1

; Signaling thread STOP
mov rcx,[r15 + DTHP.EventDone]    ; RCX = Parm#1 = Handle for thread done event
call [SetEvent]                   ; Call WinAPI function

; Exit point
add rsp,32
pop rbp
mov rsp,rbp
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret