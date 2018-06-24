;--- Strings for vector brief test, with text result only ---

; Processor clock, this string after CPU name
MsgResultBrief  DB  0Ah,0Dh, 'TSC clock (MHz) = ',0

; Processor CPUID features
                DB  0Ah,0Dh,0Ah,0Dh, 'Processor features, detect by CPUID',0
                DB  0Ah,0Dh, 'AVX 256-bit : ',0
                DB  0Ah,0Dh, 'AVX2 256-bit : ',0
                DB  0Ah,0Dh, 'AVX3 512-bit = AVX512F (Foundation) : ',0
                DB  0Ah,0Dh, 'AVX512CD (Conflict Detection) : ',0
                DB  0Ah,0Dh, 'AVX512PF (Prefetch) : ',0
                DB  0Ah,0Dh, 'AVX512ER (Exponential and Reciprocal) : ',0
                DB  0Ah,0Dh, 'AVX512VL (Vector Length) : ',0
                DB  0Ah,0Dh, 'AVX512BW (Byte and Word) : ',0
                DB  0Ah,0Dh, 'AVX512DQ (Doubleword and Quadword) : ',0
                DB  0Ah,0Dh, 'AVX512IFMA (Integer Fused Multiply and Add) : ',0
                DB  0Ah,0Dh, 'AVX512VBM (Vector Byte Manipulation) : ',0
                DB  0  ; Second zero is sequence terminator

; OS context features
                DB  0Ah,0Dh,0Ah,0Dh, 'OS context management features, detect by XCR0',0
                DB  0Ah,0Dh, 'SSE128 registers XMM[0-15] bits [0-127] : ',0
                DB  0Ah,0Dh, 'AVX256 registers YMM[0-15] bits [128-255] : ',0
                DB  0Ah,0Dh, 'AVX512 registers ZMM[0-15] bits[256-511] : ',0
                DB  0Ah,0Dh, 'AVX512 registers ZMM[16-31] bits[0-511] : ',0
                DB  0Ah,0Dh, 'AVX512 predicate registers K[0-7] : ',0
                DB  0  ; Second zero is sequence terminator

; Instructions timings
                DB  0Ah,0Dh,0Ah,0Dh, 'Instruction timings per 1 core (TSC clocks and nanoseconds)',0
                DB  0Ah,0Dh, 'SSE128 read : ',0             
                DB  0Ah,0Dh, 'SSE128 write : ',0             
                DB  0Ah,0Dh, 'SSE128 copy : ',0             
                DB  0Ah,0Dh, 'AVX256 read : ',0             
                DB  0Ah,0Dh, 'AVX256 write : ',0             
                DB  0Ah,0Dh, 'AVX256 copy : ',0             
                DB  0Ah,0Dh, 'AVX512 read : ',0             
                DB  0Ah,0Dh, 'AVX512 write : ',0             
                DB  0Ah,0Dh, 'AVX512 copy : ',0             
                DB  0Ah,0Dh, 'SQRTPD xmm : ',0             
                DB  0Ah,0Dh, 'VSQRTPD ymm : ',0             
                DB  0Ah,0Dh, 'VSQRTPD zmm : ',0             
                DB  0Ah,0Dh, 'FCOS : ',0             
                DB  0Ah,0Dh, 'FSINCOS : ',0             
                DB  0  ; Second zero is sequence terminator

; Strings for results
; Strings "supported" , "not supported"
Nsup            DB 'not supported',0
Sup             DB 'supported',0
NsupCPU         DB 'not supported by CPU',0
NsupOS          DB 'not supported by OS',0


                