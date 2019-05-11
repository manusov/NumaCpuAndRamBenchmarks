;------------------------------------------------------------------------------;
;          Library of performance patterns for bandwidth measurement:          ;
;                    connect include files with subroutines.                   ;
;------------------------------------------------------------------------------;

;--- Get pointer to performance pattern subroutine by ID ---;
; INPUT:   AL  = Subroutine ID                              ;
; OUTPUT:  RBX = Subroutine entry point                     ;
;-----------------------------------------------------------; 
GetBandwidthPattern:
movzx eax,al
imul eax,eax,6   ; sizeof.BANDWIDTH_ENTRY
movzx eax,word [BandwidthPatterns + rax]
lea rbx,[BandwidthPatternsBase + rax]
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
; Temporal (cacheable read and write) mode
include 'read_mov64.inc'
include 'write_mov64.inc'
include 'copy_mov64.inc'
include 'modify_not64.inc'
include 'write_stosq.inc'
include 'copy_movsq.inc'
include 'read_mmx64.inc'
include 'write_mmx64.inc'
include 'copy_mmx64.inc'
include 'read_sse128.inc'
include 'write_sse128.inc'
include 'copy_sse128.inc'
include 'read_avx256.inc'
include 'write_avx256.inc'
include 'copy_avx256.inc'
include 'read_avx512.inc'
include 'write_avx512.inc'
include 'copy_avx512.inc'
include 'dot_fma256.inc'
include 'dot_fma512.inc'
; Non-temporal (non-cacheable write) mode
include 'ntwrite_sse128.inc'
include 'ntcopy_sse128.inc'
include 'ntwrite_avx256.inc'
include 'ntcopy_avx256.inc'
include 'ntwrite_avx512.inc'
include 'ntcopy_avx512.inc'
; Non-temporal (non-cacheable read and write) mode
include 'ntread_sse128.inc'
include 'ntrcopy_sse128.inc'
include 'ntread_avx256.inc'
include 'ntrcopy_avx256.inc'
include 'ntread_avx512.inc'
include 'ntrcopy_avx512.inc'
; Non-temporal (non-cacheable read-by-prefetch and write) mode
include 'ntpread_sse128.inc'
include 'ntpread_avx256.inc'

