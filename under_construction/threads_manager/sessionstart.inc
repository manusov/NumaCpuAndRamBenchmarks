
; TODO. 
; Regularize simple/brief/drawings calling conventions, what registers destroyed.

SessionStart:

lea rsi,[UserParms]
lea rdi,[InputParms]
lea rbx,[SystemParameters]


; Setup benchmark method, 
; assembler routine = f (test method, test object, system configuration)


; Setup asm. routine address, set address and memory dump region


; Setup tested object



; Setup number of threads, use platform total number of threads if auto


; Setup Hyper-Threading mode


; Setup Processor Group mode


; Setup NUMA mode


; Setup Large Page mode


; Setup measurement precision (calibration repeats) mode


; Setup measurement scenario as block sizes sequence


; Setup Processor Groups parameters


; Setup NUMA domains parameters


; Setup page size


; Setup memory allocation parameters, align allocated size by page size


; Build NUMA nodes list, 
; first step before (without) memory allocation


; NUMA-aware memory allocation, 
; second step with memory allocation after nodes list build


; Build threads list




 


ret