;==============================================================================;
;                                                                              ;
;               NCRB (NUMA CPU&RAM Benchmarks). Win64 Edition.                 ;
;                           (C)2019 IC Book Labs.                              ;
;                                                                              ;
;  This file is main module: translation object, interconnecting all modules.  ;
;                                                                              ;
;         Translation by Flat Assembler version 1.73.04 (April 30, 2018)       ;
;                         http://flatassembler.net/                            ;
;                                                                              ;
;       Edit by FASM Editor 2.0, use this editor for correct tabulations.      ;
;               http://asmworld.ru/instrumenty/fasm-editor-2-0/                ;
;                                                                              ;
;==============================================================================;

; UNDER CONSTRUCTION.

; TODO. Unload dynamical import, advapi 32.
; TODO. Message if build main window error.
; TODO. Warning message if some functions not loaded.
; TODO. Error decode and GUI box with message by becchmarks results,
;       check CF flag after subroutines.

; Template for debug method 1 of 3 = Template debug.
; ( Use also 2 = Application debug, 3 = Window debug )

include 'win64a.inc'

format PE64 GUI
entry start
section '.text' code readable executable
start:

sub rsp,8*5


xor ecx,ecx
call [ExitProcess]

section '.data' data readable writeable
align 4096
BUFFER_SIZE  EQU  16384
Buffer DB BUFFER_SIZE DUP (?)

section '.idata' import data readable writeable
library user32, 'USER32.DLL', kernel32, 'KERNEL32.DLL', gdi32, 'GDI32.DLL'
include 'api\user32.inc'    ; Win API, user interface
include 'api\gdi32.inc'     ; Win API, graphice 
include 'api\kernel32.inc'  ; Win API, OS standard kernel functions

