
MACRO LATENCY_ENTRY  x1, x2, x3, x4   ; 6 bytes per entry, compact encoding
{
DW x1-LatencyPatternsBase     ; Target routine entry point
DW x2-LatencyPatternsBase     ; Dump fragment start, dump show at drawings Y=F(X)
DB x3-x2                      ; Dump fragment size
DB x4                         ; Data Bytes per Instruction,
}                             ; example YMM register op. = 32 bytes (256 bit) 

align 2
LatencyPatterns:
; Include files connect for performance patterns
; Latency measurement, note for both fill methods (LCM, RDRAND) dump walk method only
LATENCY_ENTRY  Pattern_Latency_LCM    , DumpStart_Latency_Walk , DumpStop_Latency_Walk  , 8   ; ID = 0
LATENCY_ENTRY  Pattern_Latency_RDRAND , DumpStart_Latency_Walk , DumpStop_Latency_Walk  , 8   ; ID = 1  
LATENCY_ENTRY  Pattern_Latency_Walk   , DumpStart_Latency_Walk , DumpStop_Latency_Walk  , 8   ; ID = 2   
