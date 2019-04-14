
;------------------------------------------------------------------------------;
;            Strings for simple mode test, with text result only.              ;
;------------------------------------------------------------------------------;

; INPUT parameters, measurement start conditions
MsgResultSimple   DB  'Simple mode benchmarks, options settings:',0
                  DB  0Dh,0Ah, 'method',0
                  DB  'instruction operand width (Bits)',0
                  DB  'threads',0
                  DB  'hyper-threading',0
                  DB  'large pages',0
                  DB  'NUMA profile',0
                  DB  'target object profile',0
                  DB  'measurement profile',0
                  DB  'block #1 base',0
                  DB  'block #2 base',0
                  DB  'block size per thread',0
                  DB  'measurement repeats',0

; OUTPUT parameters, measurements results
                  DB  0Dh,0Ah,'measurements results:',0
                  DB  0Dh,0Ah, 'dT (ms)',0
                  DB  '  + 1000 ms measure CPUCLK + heating',0
                  DB  'dTSC/Sec (MHz)',0
                  DB  'dTSC/Instruction per thread (Clks)',0
                 
MsgResultMBPS     DB  0Dh,0Ah, 'Speed (MBPS)',0            
MsgResultLatency  DB  0Dh,0Ah, 'Latency (ns)',0
