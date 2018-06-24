;--- Strings for simple mode test, with text result only ---

; INPUT parameters, measurement start conditions
MsgResult   DB  'Simple mode benchmarks, options settings:'
            DB  0Ah,0Dh,0Ah,0Dh, 'method = ',0
            DB  0Ah,0Dh, 'instruction operand width (Bits) = ',0
            DB  0Ah,0Dh, 'threads = ',0
            DB  0Ah,0Dh, 'hyper-threading = ',0
            DB  0Ah,0Dh, 'large pages = ',0
            DB  0Ah,0Dh, 'NUMA profile = ',0
            DB  0Ah,0Dh, 'target object profile = ',0
            DB  0Ah,0Dh, 'measurement profile = ',0
            DB  0Ah,0Dh, 'block #1 base = ',0
            DB  0Ah,0Dh, 'block #2 base = ',0
            DB  0Ah,0Dh, 'block size per thread = ',0
            DB  0Ah,0Dh, 'measurement repeats = ',0

; OUTPUT parameters, measurements results
            DB  0Ah,0Dh,0Ah,0Dh,'measurements results:'
            DB  0Ah,0Dh,0Ah,0Dh, 'dT (ms) = ',0
            DB  '  + 1000 ms measure CPUCLK + heating'
            DB  0Ah,0Dh, 'dTSC/Sec (MHz) = ',0
            DB  0Ah,0Dh, 'dTSC/Instruction per thread (Clks) = ',0
            DB  0Ah,0Dh,0Ah,0Dh, 'Speed (MBPS) = ',0            
            DB  0