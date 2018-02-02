;--- Window class descriptor for user interface window ---
; WNDCLASSEX class descriptor structure (for Win64):
; Name             Size	 Comments
;--------------------------------------------------------------
; cbSize           dd  Size of this structure, bytes
; style            dd  Window style info and bitmap
; lpfnWndProc      dq  Pointer to Window Procedure for call back
; cbClsExtra       dd  Number of extra cleared bytes after win. class struc.
; cbWndExtra       dd  Number of extra cleared bytes after window instance
; hInstance        dq  Handle for instance, contain window proc. for the class
; hIcon            dq  Handle to the class icon (icon resource) , 0=default
; hCursor          dq  Handle to the class cursor (cursor resource) , 0=default
; hbrBackground    dq  Handle to the class background brush or color value
; lpszMenuName     dq  Pointer to resource name - class menu , 0=None
; lpszClassName    dq  Pointer to class name string or atom
; hIconSm          dq  Handle to the small icon
;-------------------------------------------------------------- 

Dialogue_Win0	  WNDCLASSEX sizeof.WNDCLASSEX,0,WindowProc_Win0,0,0,0,0,0,COLOR_BTNFACE+1,0,ClassWindow0,0
Dialogue_Win1	  WNDCLASSEX sizeof.WNDCLASSEX,0,WindowProc_Win1,0,0,0,0,0,COLOR_BTNFACE+1,0,ClassWindow1,0

ClassWindow0  DB  'FASM64WIN0',0  ; Class for Window 0 (my object)
ClassWindow1  DB  'FASM64WIN1',0  ; Class for Window 1 (my object)
ClassButton   DB  'BUTTON',0      ; Class BUTTON (ref. to windows)
ClassStatic   DB  'STATIC',0      ; Class STATIC (ref. to windows)
ClassEdit     DB  'EDIT',0        ; Class EDIT (ref. to windows)

