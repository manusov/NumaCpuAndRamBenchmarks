;--- Window 1 elements list ---
; Window 1 = Measurement window, drawings Y=F(X).

; If x2=BasePoint, Result=StringsNull-StringsNull=0
START_HANDLE_WIN1 = 16
DialogueContent_Win1:
; Buttons and ASM method string
HANDLE_BUTTON_RUN1    = 16
HANDLE_BUTTON_CANCEL1 = 17
HANDLE_TEXT_METHOD1   = 18
HANDLE_TEXT_OBJECT1   = 19
HANDLE_TEXT_REPEATS1  = 20
; "AM" label name prefix means "Assembler Method" means CPU Instruction Set
DIALELEMENT  ClassButton , TextRestart     , 0570 , 0500 , 0080 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON  ; 16 = Restart button
DIALELEMENT  ClassButton , TextCancel      , 0660 , 0500 , 0080 , 0024 , WS_VISIBLE+WS_CHILD+BS_PUSHBUTTON  ; 17 = Cancel button
;DIALELEMENT ClassStatic , AM_00           , 0017 , 0492 , 0400 , 0020 , WS_VISIBLE+WS_CHILD                ; 18 = Selected ASM method string , patch v0.98.2
DIALELEMENT  ClassStatic , AM_00           , 0017 , 0492 , 0500 , 0020 , WS_VISIBLE+WS_CHILD                ; 18 = Selected ASM method string
DIALELEMENT  ClassStatic , Unknown_Object  , 0017 , 0510 , 0200 , 0020 , WS_VISIBLE+WS_CHILD                ; 19 = Selected analysed object string
DIALELEMENT  ClassStatic , Repeats_String  , 0195 , 0510 , 0200 , 0020 , WS_VISIBLE+WS_CHILD                ; 20 = Number of repeats string
; Terminator
DIALSTOP
; End of list

