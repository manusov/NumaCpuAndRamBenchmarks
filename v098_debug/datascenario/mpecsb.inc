;------------------------------------------------------------------------------;
; MPECSB = Memory Performance Engine control and status block                  ;
;------------------------------------------------------------------------------;

; Test input and output operands located by alignment-optimal ordering
; and 64-byte cache line allocation

; Test input parameters layout declaration
struct IPB
BlockBase1        dq  ?    ; Base address of target memory block 
BlockBase2        dq  ?    ; Base address of second target memory block, if used
BlockSize         dq  ?    ; Size of each memory block, bytes
OperandWidth      dd  ?    ; Used instructions operand width, bits 
NumThreads        dd  ?    ; Number of execution threads, 1 if single thread
NumRepeats        dd  ?    ; Number of measurement repeats
SelectMethod      db  ?    ; Select test ASM method, one of procedure
OptionHT          db  ?    ; Hyper-threading: 0=None, 1=Present, not used, 2=Used
OptionNUMA        db  ?    ; NUMA: 0=None, 1=No control, 2=Optimal, 3=Non optim.
OptionTarget      db  ?    ; Target object: 0=L1, 1=L2, 2=L3, 3=RAM, 4=Custom
OptionMeasure     db  ?    ; Measurement: 0=brief, 1=careful
; Performance pattern internal parameters
; SSE4.1 validation required for 128-bit MOVNTDQA
; AVX2 validation required for 256-bit VMOVNTDQA
; Note non-temporal read recommended for AMD CPU only,
; Note non-temporal write recommended for both Intel and AMD
; IMPORTANT patch v0.98.0 = non-temporal read use BOTH FOR INTEL AND AMD
NonTemporalRead   db  ?    ; 0 = Default NT write only, 1 = SSE4.1 modify, 2 = AVX2 modify
ends
; Test input parameters
align 64
InputParms  IPB

; Test output parameters layout declaration
struct OPB
TSCfrequencyHz    dq  ?   ; TSC measured clock, Hz, 64-bit long integer
OStimerDelta      dq  ?   ; Units = 100 ns, 64-bit long integer 
TSCtimerDelta     dq  ?   ; Units = 1 TSC clock, 64-bit long integer 
ends
; Test output parameters
align 64
OutputParms  OPB

; Field for custom block size edit
BlockSizeEdit     DQ  ?    ; CUSTOM for selection: L1/L2/L3/Custom

; Benchmarks select and routine dump support
PatternRoutine    DQ  ?  ; Pointer to target benchmark routine
DumpMethodStart   DQ  ?  ; Start of fragment, visualized as dump during benchmarks
DumpMethodLength  DD  ?  ; Length of fragment, visualized as dump during benchmarks, bytes

; Threads management lists, must be sequentally located, this fact used in code
align 64
ThreadsControl    DB  THREADS_CONTROL_SIZE DUP (?) 
EventsControl     DB  EVENTS_CONTROL_SIZE  DUP (?) 
NumaControl       DB  NUMA_CONTROL_SIZE    DUP (?)
