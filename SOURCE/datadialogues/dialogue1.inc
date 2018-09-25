;------------------------------------------------------------------------------;
;                         Window 1 elements list.                              ;
;------------------------------------------------------------------------------;

; Window 1 = Measurement window, drawings Y=F(X).
; List of GUI window elements descriptors.

; If x2=BasePoint, Result=StringsNull-StringsNull=0
START_HANDLE_WIN1 = 16
DialogueContent_Win1:
; Buttons and ASM method string
HANDLE_BUTTON_RUN1    = 16
HANDLE_BUTTON_CANCEL1 = 17
HANDLE_TEXT_METHOD1   = 18
HANDLE_TEXT_OBJECT1   = 19
HANDLE_THREADS1       = 20
HANDLE_TEXT_REPEATS1  = 21
HANDLE_LARGE_PAGES1   = 22
HANDLE_NUMA_MODE1     = 23
; "AM" label name prefix means "Assembler Method" means CPU Instruction Set
DIALELEMENT  ClassButton , TextRestart     , 0570 , 0500 , 0080 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON  ; 16 = Restart button
DIALELEMENT  ClassButton , TextCancel      , 0660 , 0500 , 0080 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON  ; 17 = Cancel button
DIALELEMENT  ClassStatic , AM_00           , 0017 , 0492 , 0500 , 0020 , WS_VISIBLE+WS_CHILD                ; 18 = Selected ASM method string
DIALELEMENT  ClassStatic , Unknown_Object  , 0017 , 0510 , 0200 , 0020 , WS_VISIBLE+WS_CHILD                ; 19 = Selected analysed object string
DIALELEMENT  ClassStatic , Threads_String  , 0125 , 0510 , 0125 , 0020 , WS_VISIBLE+WS_CHILD                ; 20 = Number of threads string
DIALELEMENT  ClassStatic , Repeats_String  , 0222 , 0510 , 0125 , 0020 , WS_VISIBLE+WS_CHILD                ; 21 = Number of repeats string
DIALELEMENT  ClassStatic , NPages_String   , 0358 , 0510 , 0125 , 0020 , WS_VISIBLE+WS_CHILD                ; 22 = Large pages string
DIALELEMENT  ClassStatic , NumaOff_String  , 0448 , 0510 , 0108 , 0020 , WS_VISIBLE+WS_CHILD                ; 23 = NUMA mode string
; Terminator
DIALSTOP
; End of list

