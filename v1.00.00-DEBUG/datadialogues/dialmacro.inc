;--- Element descriptor for user interface window element ---
; Used as dialogue descriptors entry
; Parms list: Class, Name, Xbase, Ybase, Xsize, Ysize, Style 

MACRO DIALELEMENT  x1, x2, x4, x5, x6, x7, x3
{
DW  x7, x6, x5, x4       ; Ysize, Xsize, Ybase, Xbase, if Ysize=0 means stop
DD  x3                   ; Window style, numeric constants combination
DW  x2-BasePoint         ; Short pointer to Object Name, 0=None
DW  x1-BasePoint         ; Short pointer to Object Class Name
}
; Terminator
MACRO DIALSTOP
{
DW  0                    ; Ysize=0 means stop
}

