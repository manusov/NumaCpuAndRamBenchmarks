
GetLatencyPattern:
movzx eax,al
movzx eax,word [LatencyPatterns + rax*2]
lea rbx,[LatencyPatternsBase + rax]
ret

; Start offset for compact offsets encoding
LatencyPatternsBase:
; Latency measurement
include 'latency_lcm.inc'
include 'latency_rdrand.inc'
include 'latency_walk.inc'
