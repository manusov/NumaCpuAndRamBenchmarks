
GetLatencyPattern:
movzx eax,al
imul eax,eax,6   ; sizeof.LATENCY_ENTRY
movzx eax,word [LatencyPatterns + rax]
lea rbx,[LatencyPatternsBase + rax]
ret

GetLatencyDump:
movzx eax,al
imul eax,eax,6   ; sizeof.BANDWIDTH_ENTRY
movzx edx,byte [LatencyPatterns + rax + 4]
movzx eax,word [LatencyPatterns + rax + 2]
add rax,LatencyPatternsBase
ret

; Start offset for compact offsets encoding
LatencyPatternsBase:
; Latency measurement
include 'latency_lcm.inc'
include 'latency_rdrand.inc'
include 'latency_walk.inc'
