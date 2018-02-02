;--- Window 1 (drawings) execution controls ---

; Benchmarks units control
; Used indexed access from this base, include next groups, don't reorder variables!
ValueGridX         DD  ?      ; Units per horizontal grid cell
ValueGridY         DD  ?      ; Units per vertical grid cell
SelectUnits        DB  ?      ; 0=Bytes, 1=Kilobytes, 2=Megabytes

; Benchmarks statistics and drawings X-counter
; Used indexed access from this base, include next groups, don't reorder variables!
StatClksMinimum    DQ  ?      ; Minimum clocks per iteration
StatClksMaximum    DQ  ?      ; Maximum clocks per iteration
StatClksAverage    DQ  ?      ; Average clocks per iteration
StatClksSum        DQ  ?      ; Clocks sum for averaging
DrawCount          DD  ?      ; Pixels counter for X-progress when drawing
DrawPreviousY      DD  ?      ; Previous coordinate for vertical lines draw if required, when ABS(X(i)-X(i+1)) > 1

; Benchmark block size and repeats
; Used indexed access from this base, include next groups, don't reorder variables!
BenchRepeats       DQ  ?      ; Number of repeats for measurement
BenchInstructions  DQ  ?      ; Number of target instructions at all block (not cycle only), example=VMOVAPD
BenchBPI           DQ  ?      ; Number of bytes per target instruction

; Benchmark parameters ---
Xmultiplier        DD  ?      ; XTimerTick * Xmultiplier = RCX for performance pattern
Ydivisor           DD  ?      ; ( ( Frequency * Decimal Megabytes transferred ) / clocks ) / Ydivisor = DrawYCoordinateOffset

; Auto Y-scale (by Restart/ReSize button at Window 1)
MaximumMBPS        DD  ?      ; Maximum detected Megabytes per Second for auto Y-scale

