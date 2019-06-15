;------------------------------------------------------------------------------;
;                  This performance patterns used in the                       ; 
;              "Run simple", "Run drawings" benchmark scenarios.               ;
;           Library of performance patterns for latency measurement:           ;
;                    connect include files with subroutines.                   ;
;------------------------------------------------------------------------------;

;--- Get pointer to performance pattern subroutines by ID ------;
; INPUT:   AL  = Subroutine ID                                  ;
; OUTPUT:  EBX = Subroutine #1 entry point, walk list builder   ;
;          EAX = Subroutine #2 entry point, walker              ; 
;---------------------------------------------------------------; 
GetLatencyPattern:
movzx eax,al                             ; This clears bits EAX[31-8]
imul eax,eax,6                           ; 6 = sizeof.LATENCY_ENTRY
movzx eax,word [LatencyPatterns + eax]   ; Read 16-bit compact offset
lea ebx,[LatencyPatternsBase + eax]      ; Calculate 32-bit absolute address
lea eax,[Pattern_Latency_Walk]
ret

;--- Get pointer to performance pattern cycle body for dump by ID ---;
;    Also returns used instruction operand width, bits               ; 
; INPUT:   AL  = Subroutine ID                                       ;
; OUTPUT:  EAX = Dump fragment base address                          ;
;          EDX = Dump fragment length, bytes                         ;
;          ECX = Used instruction operand width, bits                ;          
;--------------------------------------------------------------------; 
GetLatencyDump:
movzx eax,al
imul eax,eax,6   ; sizeof.BANDWIDTH_ENTRY
movzx ecx,byte [LatencyPatterns + eax + 5]
movzx edx,byte [LatencyPatterns + eax + 4]
movzx eax,word [LatencyPatterns + eax + 2]
add eax,LatencyPatternsBase
ret

; Start offset for compact offsets encoding
LatencyPatternsBase:
; Latency measurement
include 'latency_lcm.inc'      ; Build walk list, use linear congruental method
include 'latency_rdrand.inc'   ; Build walk list, use RDRAND instruction
include 'latency_walk.inc'     ; Walk on pre-builded walk list
