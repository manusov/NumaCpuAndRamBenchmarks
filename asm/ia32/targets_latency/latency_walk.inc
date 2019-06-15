;---------- Memory performance patterns ---------------------------------------;
; INPUT:   ESI = Block #1 pointer (32-bit flat) ,                              ;
;                here used to load walk list entry point                       ;
;          EDI = Block #2 pointer (32-bit flat) , used for walk list           ;
;          ECX = Block length, units = instructions                            ;
;          EBX:EBP = Number of measurement repeats, EBX=High32, EBP=Low32      ;
; OUTPUT:  None                                                                ;
;          Registers corrupted                                                 ;
;------------------------------------------------------------------------------;

Pattern_Latency_Walk:

mov ecx,[esi]         ; qword [esi] = walk list entry point, Block #1
WalkRepeat:
mov eax,ecx

DumpStart_Latency_Walk:
.WalkLinked:          ; Walk linked list, use fetched data as next address
mov edx,[eax+0]
mov eax,[eax+4]       ; Get duplicated from high dword
test eax,eax
jnz .WalkLinked
DumpStop_Latency_Walk:

sub ebp,1
sbb ebx,0
mov eax,ebp
or eax,ebx
jnz WalkRepeat
ret
