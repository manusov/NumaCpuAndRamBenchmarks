;------------------------------------------------------------------------------;
;        Flags means Windows 0, 1 already initialized, pre-blanked to 0.       ;
;------------------------------------------------------------------------------;

Win0_Init     DB  0   ; Enable events handling by Window 0 = main
Win1_Init     DB  0   ; Enable events handling by Window 1 = drawings 
Win1_Running  DB  0   ; Flag for drawings and timer

