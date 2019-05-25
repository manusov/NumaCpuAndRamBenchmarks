;------------------------------------------------------------------------------;
;                  This performance patterns used in the                       ; 
;              "Run simple", "Run drawings" benchmark scenarios.               ;
;          Library of performance patterns for bandwidth measurement:          ;
;                    connect include files with subroutines.                   ;
;------------------------------------------------------------------------------;

;--- Get pointer to performance pattern subroutine by ID ---;
; INPUT:   AL  = Subroutine ID                              ;
; OUTPUT:  RBX = Subroutine entry point                     ;
;-----------------------------------------------------------; 
GetBandwidthPattern:
movzx eax,al                               ; This clears bits RAX[63-8]
imul eax,eax,6                             ; 6 = sizeof.BANDWIDTH_ENTRY
movzx eax,word [BandwidthPatterns + rax]   ; Read 16-bit compact offset
lea rbx,[BandwidthPatternsBase + rax]      ; Calculate 64-bit absolute address      
ret

;--- Get pointer to performance pattern cycle body for dump by ID ---;
;    Also returns used instruction operand width, bits               ; 
; INPUT:   AL  = Subroutine ID                                       ;
; OUTPUT:  RAX = Dump fragment base address                          ;
;          EDX = Dump fragment length, bytes                         ;
;          ECX = Used instruction operand width, bits                ;          
;--------------------------------------------------------------------; 
GetBandwidthDump:
movzx eax,al
imul eax,eax,6   ; sizeof.BANDWIDTH_ENTRY
movzx ecx,byte [BandwidthPatterns + rax + 5]
movzx edx,byte [BandwidthPatterns + rax + 4]
movzx eax,word [BandwidthPatterns + rax + 2]
add rax,BandwidthPatternsBase
ret

; Start offset for compact offsets encoding
BandwidthPatternsBase:
; Include files connect for performance patterns
; Temporal (cacheable read and write) mode, temporal is default cacheable access
include 'read_mov64.inc'       ; Read by MOV (64 bit)
include 'write_mov64.inc'      ; Write by MOV (64 bit) 
include 'copy_mov64.inc'       ; Copy by MOV + MOV (64 bit)
include 'modify_not64.inc'     ; Modify = Read-Modify-Write by NOT (64 bit) 
include 'write_stosq.inc'      ; Write by REP STOSQ (64 bit)
include 'copy_movsq.inc'       ; Copy by REP MOVSQ (64 bit)
include 'read_mmx64.inc'       ; Read by MOVQ (64 bit)
include 'write_mmx64.inc'      ; Write by MOVQ (64 bit)
include 'copy_mmx64.inc'       ; Copy by MOVQ + MOVQ (64 bit)
include 'read_sse128.inc'      ; Read by MOVAPS (128 bit) 
include 'write_sse128.inc'     ; Write by MOVAPS (128 bit)
include 'copy_sse128.inc'      ; Copy by MOVAPS + MOVAPS (128 bit)
include 'read_avx256.inc'      ; Read by VMOVAPD (256 bit)
include 'write_avx256.inc'     ; Write by VMOVAPD (256 bit)
include 'copy_avx256.inc'      ; Copy by VMOVAPD + VMOVAPD (256 bit)
include 'read_avx512.inc'      ; Read by VMOVAPD (512 bit)  
include 'write_avx512.inc'     ; Write by VMOVAPD (512 bit)
include 'copy_avx512.inc'      ; Copy by VMOVAPD + VMOVAPD (512 bit)
include 'dot_fma256.inc'       ; Fused multiply and add, VFMADD231PD (256 bit)
include 'dot_fma512.inc'       ; Fused multiply and add, VFMADD231PD (512 bit) 
; Non-temporal (non-cacheable write) mode
include 'ntwrite_sse128.inc'   ; Write by MOVNTPS (128 bit)
include 'ntcopy_sse128.inc'    ; Copy by PREFETCHNTA + MOVAPS + MOVNTPS (128 bit)
include 'ntwrite_avx256.inc'   ; Write by VMOVNTPD (256 bit)
include 'ntcopy_avx256.inc'    ; Copy by MOVAPD + MOVNTPD (256 bit)
include 'ntwrite_avx512.inc'   ; Write by VMOVNTPD (512 bit)
include 'ntcopy_avx512.inc'    ; Copy by MOVAPD + MOVNTPD (512 bit) 
; Non-temporal (non-cacheable read and write) mode
include 'ntread_sse128.inc'    ; Read by MOVNTDQA (128 bit) 
include 'ntrcopy_sse128.inc'   ; Copy by MOVNTDQA + MOVNTPD (128 bit)
include 'ntread_avx256.inc'    ; Read by VMOVNTDQA (256 bit)
include 'ntrcopy_avx256.inc'   ; Copy by VMOVNTDQA + VMOVNTPD (256 bit)  
include 'ntread_avx512.inc'    ; Read by VMOVNTDQA (512 bit)
include 'ntrcopy_avx512.inc'   ; Copy by VMOVNTDQA + VMOVNTPD (512 bit)
; Non-temporal (non-cacheable read-by-prefetch and write) mode
include 'ntpread_sse128.inc'   ; Read by PREFETCHNTA + MOVAPS (128 bit)
include 'ntpread_avx256.inc'   ; Read by PREFETCHNTA + VMOVAPD (256 bit)

