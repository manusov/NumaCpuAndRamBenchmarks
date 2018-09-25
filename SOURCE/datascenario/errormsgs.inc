;------------------------------------------------------------------------------;
;                             Error messages.                                  ;
;               See also non-fatal warnings: file warnapistr.inc.              ;
;------------------------------------------------------------------------------;

; Error messages, visualized if can't start application
; Application cannot start with this errors
MessageNoCpuid	  DB  'CPUID instruction not supported or locked',0
MessageNoCpuid1   DB  'CPUID function 1 not supported or locked',0
MessageNoX87      DB  'x87 Floating Point Unit not supported or locked',0
MessageNoTsc      DB  'Time Stamp Counter not supported or locked',0
MessageClkError   DB  'Error measuring TSC frequency',0
MessageMApiError  DB  'Memory information API return error',0
MessageTApiError  DB  'CPU topological information API return error',0
; MessageMemError DB  'Memory allocation error',0
MessageBadWindow  DB  'Error initializing dialogue window',0

; Error messages, visualized in runtime
; Cannot execute required test if this errors detected
RunErrorMemoryA   DB  'Benchmarks buffer memory allocation error',0  
RunErrorMemoryR   DB  'Benchmarks buffer memory release error',0
RunErrorTimings   DB  'Benchmarks timings measurement error',0 
RunErrorArithm    DB  'Benchmarks address arithmetic error',0
