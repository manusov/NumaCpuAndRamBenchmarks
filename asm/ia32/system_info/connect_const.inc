;------------------------------------------------------------------------------;
; System Information Library: constants and variables with pre-defined values. ; 
;------------------------------------------------------------------------------;

;------------------------------------------------------------------------------;
;                  System signatures for configuration detect.                 ;
;------------------------------------------------------------------------------;
Signature_Intel  DB  'GenuineIntel'
Signature_AMD    DB  'AuthenticAMD'

;-
; ;------------------------------------------------------------------------------;
; ; Data structure for L1 trace cache detection,                                 ; 
; ; this actual for Intel Prescott CPUs.                                         ;
; ;------------------------------------------------------------------------------;
; ; x1 = Cache Descriptor value, for CPUID function 2
; ; x2 = Trace Cache size, KuOps (kilo micro-operations)
; MACRO TRACE_CACHE_ID x1, x2
; {
; DB x1, x2
; }
; ; List terminator
; MACRO TRACE_CACHE_END
; {
; DB 0
; }
; 
; IntelTraceCaches:
; TRACE_CACHE_ID   070h , 12   ; { 0x70, "trace L1 cache, 12 K킣Ps, 8 ways" }
; TRACE_CACHE_ID   071h , 16   ; { 0x71, "trace L1 cache, 16 K킣Ps, 8 ways" }
; TRACE_CACHE_ID   072h , 32   ; { 0x72, "trace L1 cache, 32 K킣Ps, 8 ways" }
; TRACE_CACHE_ID   073h , 64   ; { 0x73, "trace L1 cache, 64 K킣Ps, 8 ways" }
; TRACE_CACHE_END
;-

;------------------------------------------------------------------------------;
; Data structure for Intel Cache Detection                                     ; 
; This actual for CPUs not support function                                    ; 
; CPUID 00000004h = Deterministic Cache Info,                                  ; 
; Note if runs under Windows XP 32-bit, this information cannot be detected    ;
; by Win API GetLogicalProcessorInformation, means required use:               ;
; CPUID 00000002h = Get Cache Descriptors, with decoding by this data          ;
;------------------------------------------------------------------------------;

;--- This original list imported from Java Cpuid utility ---
;    { 0x00, "null descriptor (=unused descriptor)" } ,
;    { 0x01, "code TLB, 4K pages, 4 ways, 32 entries" } ,
;    { 0x02, "code TLB, 4M pages, fully, 2 entries" } ,
;    { 0x03, "data TLB, 4K pages, 4 ways, 64 entries" } ,
;    { 0x04, "data TLB, 4M pages, 4 ways, 8 entries" } ,
;    { 0x05, "data TLB, 4M pages, 4 ways, 32 entries" } , 
;    { 0x06, "code L1 cache, 8 KB, 4 ways, 32 byte lines" } , 
;    { 0x08, "code L1 cache, 16 KB, 4 ways, 32 byte lines" } , 
;    { 0x09, "code L1 cache, 32 KB, 4 ways, 64 byte lines" } , 
;    { 0x0A, "data L1 cache, 8 KB, 2 ways, 32 byte lines" } , 
;    { 0x0B, "code TLB, 4M pages, 4 ways, 4 entries" } , 
;    { 0x0C, "data L1 cache, 16 KB, 4 ways, 32 byte lines" } ,
;    { 0x0D, "data L1 cache, 16 KB, 4 ways, 64 byte lines (ECC)" } , 
;    { 0x0E, "data L1 cache, 24 KB, 6 ways, 64 byte lines" } , 
;    { 0x10, "data L1 cache, 16 KB, 4 ways, 32 byte lines (IA-64)" } , 
;    { 0x15, "code L1 cache, 16 KB, 4 ways, 32 byte lines (IA-64)" } , 
;    { 0x1A, "code and data L2 cache, 96 KB, 6 ways, 64 byte lines (IA-64)" } , 
;    { 0x1D, "code and data L2 cache, 128 KB, 2 ways, 64 byte lines" } , 
;    { 0x21, "code and data L2 cache, 256 KB, 8 ways, 64 byte lines" } , 
;    { 0x22, "code and data L3 cache, 512 KB, 4 ways (!), 64 byte lines, dual-sectored" } , 
;    { 0x23, "code and data L3 cache, 1024 KB, 8 ways, 64 byte lines, dual-sectored" } , 
;    { 0x24, "code and data L2 cache, 1024 KB, 16 ways, 64 byte lines" } , 
;    { 0x25, "code and data L3 cache, 2048 KB, 8 ways, 64 byte lines, dual-sectored" } , 
;    { 0x29, "code and data L3 cache, 4096 KB, 8 ways, 64 byte lines, dual-sectored" } , 
;    { 0x2C, "data L1 cache, 32 KB, 8 ways, 64 byte lines" } ,  
;    { 0x30, "code L1 cache, 32 KB, 8 ways, 64 byte lines" } ,  
;    { 0x39, "code and data L2 cache, 128 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x3A, "code and data L2 cache, 192 KB, 6 ways, 64 byte lines, sectored" } ,  
;    { 0x3B, "code and data L2 cache, 128 KB, 2 ways, 64 byte lines, sectored" } ,  
;    { 0x3C, "code and data L2 cache, 256 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x3D, "code and data L2 cache, 384 KB, 6 ways, 64 byte lines, sectored" } ,  
;    { 0x3E, "code and data L2 cache, 512 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x40, "no integrated L2 cache (P6 core) or L3 cache (P4 core)" } ,  
;    { 0x41, "code and data L2 cache, 128 KB, 4 ways, 32 byte lines" } ,  
;    { 0x42, "code and data L2 cache, 256 KB, 4 ways, 32 byte lines" } ,  
;    { 0x43, "code and data L2 cache, 512 KB, 4 ways, 32 byte lines" } ,  
;    { 0x44, "code and data L2 cache, 1024 KB, 4 ways, 32 byte lines" } ,  
;    { 0x45, "code and data L2 cache, 2048 KB, 4 ways, 32 byte lines" } ,  
;    { 0x46, "code and data L3 cache, 4096 KB, 4 ways, 64 byte lines" } ,  
;    { 0x47, "code and data L3 cache, 8192 KB, 8 ways, 64 byte lines" } ,  
;    { 0x48, "code and data L2 cache, 3072 KB, 12 ways, 64 byte lines" } ,  
;    { 0x49, "code and data L3 cache, 4096 KB, 16 ways, 64 byte lines (P4) or"
;           + " code and data L2 cache, 4096 KB, 16 ways, 64 byte lines (Core 2)" } ,   
;    { 0x4A, "code and data L3 cache, 6144 KB, 12 ways, 64 byte lines" } ,  
;    { 0x4B, "code and data L3 cache, 8192 KB, 16 ways, 64 byte lines" } ,  
;    { 0x4C, "code and data L3 cache, 12288 KB, 12 ways, 64 byte lines" } ,  
;    { 0x4D, "code and data L3 cache, 16384 KB, 16 ways, 64 byte lines" } ,  
;    { 0x4E, "code and data L2 cache, 6144 KB, 24 ways, 64 byte lines" } ,  
;    { 0x4F, "code TLB, 4K pages, ???, 32 entries" } ,  
;    { 0x50, "code TLB, 4K/4M/2M pages, fully, 64 entries" } ,  
;    { 0x51, "code TLB, 4K/4M/2M pages, fully, 128 entries" } ,  
;    { 0x52, "code TLB, 4K/4M/2M pages, fully, 256 entries" } ,  
;    { 0x55, "code TLB, 2M/4M, fully, 7 entries" } ,  
;    { 0x56, "L0 data TLB, 4M pages, 4 ways, 16 entries" } ,  
;    { 0x57, "L0 data TLB, 4K pages, 4 ways, 16 entries" } ,  
;    { 0x59, "L0 data TLB, 4K pages, fully, 16 entries" } ,  
;    { 0x5A, "L0 data TLB, 2M/4M, 4 ways, 32 entries" } ,  
;    { 0x5B, "data TLB, 4K/4M pages, fully, 64 entries" } ,  
;    { 0x5C, "data TLB, 4K/4M pages, fully, 128 entries" } ,  
;    { 0x5D, "data TLB, 4K/4M pages, fully, 256 entries" } ,  
;    { 0x60, "data L1 cache, 16 KB, 8 ways, 64 byte lines, sectored" } ,  
;    { 0x61, "code TLB, 4K pages, fully, 48 entries" } ,  
;    { 0x63, "data TLB, 2M/4M pages, 4-way, 32-entries, and"
;           + " data TLB, 1G pages, 4-way, 4 entries" } ,   
;    { 0x64, "data TLB, 4K pages, 4-way, 512 entries" } ,  
;    { 0x66, "data L1 cache, 8 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x67, "data L1 cache, 16 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x68, "data L1 cache, 32 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x6A, "L0 data TLB, 4K pages, 8-way, 64 entries" } ,  
;    { 0x6B, "data TLB, 4K pages, 8-way, 256 entries" } ,  
;    { 0x6C, "data TLB, 2M/4M pages, 8-way, 126 entries" } ,  
;    { 0x6D, "data TLB, 1G pages, fully, 16 entries" } ,  
;    { 0x70, "trace L1 cache, 12 K킣Ps, 8 ways" } ,  
;    { 0x71, "trace L1 cache, 16 K킣Ps, 8 ways" } ,  
;    { 0x72, "trace L1 cache, 32 K킣Ps, 8 ways" } ,  
;    { 0x73, "trace L1 cache, 64 K킣Ps, 8 ways" } ,  
;    { 0x76, "code TLB, 2M/4M pages, fully, 8 entries" } ,  
;    { 0x77, "code L1 cache, 16 KB, 4 ways, 64 byte lines, sectored (IA-64)" } ,  
;    { 0x78, "code and data L2 cache, 1024 KB, 4 ways, 64 byte lines" } ,  
;    { 0x79, "code and data L2 cache, 128 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
;    { 0x7A, "code and data L2 cache, 256 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
;    { 0x7B, "code and data L2 cache, 512 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
;    { 0x7C, "code and data L2 cache, 1024 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
;    { 0x7D, "code and data L2 cache, 2048 KB, 8 ways, 64 byte lines" } ,  
;    { 0x7E, "code and data L2 cache, 256 KB, 8 ways, 128 byte lines, sect. (IA-64)" } ,  
;    { 0x7F, "code and data L2 cache, 512 KB, 2 ways, 64 byte lines" } ,  
;    { 0x80, "code and data L2 cache, 512 KB, 8 ways, 64 byte lines" } ,  
;    { 0x81, "code and data L2 cache, 128 KB, 8 ways, 32 byte lines" } ,  
;    { 0x82, "code and data L2 cache, 256 KB, 8 ways, 32 byte lines" } ,  
;    { 0x83, "code and data L2 cache, 512 KB, 8 ways, 32 byte lines" } ,  
;    { 0x84, "code and data L2 cache, 1024 KB, 8 ways, 32 byte lines" } ,  
;    { 0x85, "code and data L2 cache, 2048 KB, 8 ways, 32 byte lines" } ,  
;    { 0x86, "code and data L2 cache, 512 KB, 4 ways, 64 byte lines" } ,  
;    { 0x87, "code and data L2 cache, 1024 KB, 8 ways, 64 byte lines" } ,  
;    { 0x88, "code and data L3 cache, 2048 KB, 4 ways, 64 byte lines (IA-64)" } ,  
;    { 0x89, "code and data L3 cache, 4096 KB, 4 ways, 64 byte lines (IA-64)" } ,  
;    { 0x8A, "code and data L3 cache, 8192 KB, 4 ways, 64 byte lines (IA-64)" } ,  
;    { 0x8D, "code and data L3 cache, 3072 KB, 12 ways, 128 byte lines (IA-64)" } ,  
;    { 0x90, "code TLB, 4K...256M pages, fully, 64 entries (IA-64)" } ,  
;    { 0x96, "data L1 TLB, 4K...256M pages, fully, 32 entries (IA-64)" } ,  
;    { 0x9B, "data L2 TLB, 4K...256M pages, fully, 96 entries (IA-64)" } ,  
;    { 0xA0, "data TLB, 4K pages, fully, 32 entries" } ,  
;    { 0xB0, "code TLB, 4K pages, 4 ways, 128 entries" } ,  
;    { 0xB1, "code TLB, 4M pages, 4 ways, 4 entries and"
;           + " code TLB, 2M pages, 4 ways, 8 entries " } ,  
;    { 0xB2, "code TLB, 4K pages, 4 ways, 64 entries" } ,  
;    { 0xB3, "data TLB, 4K pages, 4 ways, 128 entries" } ,  
;    { 0xB4, "data TLB, 4K pages, 4 ways, 256 entries" } ,  
;    { 0xB5, "code TLB, 4K pages, 8 ways, 64 entries" } ,  
;    { 0xB6, "code TLB, 4K pages, 8 ways, 128 entries" } ,  
;    { 0xBA, "data TLB, 4K pages, 4 ways, 64 entries" } ,  
;    { 0xC0, "data TLB, 4K/4M pages, 4 ways, 8 entries" } ,  
;    { 0xC1, "L2 code and data TLB, 4K/2M pages, 8 ways, 1024 entries" } ,  
;    { 0xC2, "data TLB, 2M/4M pages, 4 ways, 16 entries" } ,  
;    { 0xC3, "L2 code and data TLB, 4K/2M pages, 6 ways, 1536 entries and"
;           + " L2 code and data TLB, 1G pages, 4 ways, 16 entries" } ,   
;    { 0xC4, "data TLB, 2M/4M pages, 4-way, 32 entries" } ,  
;    { 0xCA, "L2 code and data TLB, 4K pages, 4 ways, 512 entries" } ,  
;    { 0xD0, "code and data L3 cache, 512-kb, 4 ways, 64 byte lines" } ,  
;    { 0xD1, "code and data L3 cache, 1024-kb, 4 ways, 64 byte lines" } ,  
;    { 0xD2, "code and data L3 cache, 2048-kb, 4 ways, 64 byte lines" } ,  
;    { 0xD6, "code and data L3 cache, 1024-kb, 8 ways, 64 byte lines" } ,  
;    { 0xD7, "code and data L3 cache, 2048-kb, 8 ways, 64 byte lines" } ,  
;    { 0xD8, "code and data L3 cache, 4096-kb, 8 ways, 64 byte lines" } ,  
;    { 0xDC, "code and data L3 cache, 1536-kb, 12 ways, 64 byte lines" } ,  
;    { 0xDD, "code and data L3 cache, 3072-kb, 12 ways, 64 byte lines" } ,  
;    { 0xDE, "code and data L3 cache, 6144-kb, 12 ways, 64 byte lines" } ,  
;    { 0xE2, "code and data L3 cache, 2048-kb, 16 ways, 64 byte lines" } ,  
;    { 0xE3, "code and data L3 cache, 4096-kb, 16 ways, 64 byte lines" } ,  
;    { 0xE4, "code and data L3 cache, 8192-kb, 16 ways, 64 byte lines" } ,  
;    { 0xEA, "code and data L3 cache, 12288-kb, 24 ways, 64 byte lines" } ,  
;    { 0xEB, "code and data L3 cache, 18432-kb, 24 ways, 64 byte lines" } ,  
;    { 0xEC, "code and data L3 cache, 24576-kb, 24 ways, 64 byte lines" } ,  
;    { 0xF0, "64 byte prefetching" } ,  
;    { 0xF1, "128 byte prefetching" } ,  
;    { 0xFF, "query standard level 0000_0004h instead" } , 
;    };
;---

;--- Macro for decoding control ---

MACRO L1T x1, x2
{           ; 1 = ID for Trace Cache decode 
DB 1, x1    ; x2 = Cache Descriptor by Intel
DW x2       ; x2 = Cache Size, kilobytes 
}

MACRO L1I x1, x2
{           ; 1 = ID for L1 Instruction Cache decode
DB 2, x1    ; x1 = Cache Descriptor by Intel
DW x2       ; x2 = Cache Size, kilobytes 
}

MACRO L1D x1, x2
{           ; 2 = ID for L1 Data Cache decode
DB 3, x1    ; x1 = Cache Descriptor by Intel
DW x2       ; x2 = Cache Size, kilobytes 
}

MACRO L2U x1, x2
{           ; 3 = ID for L2 Unified Cache decode
DB 4, x1    ; x1 = Cache Descriptor by Intel
DW x2       ; x2 = Cache Size, kilobytes 
}

MACRO L3U x1, x2
{           ; 4 = ID for L3 Unified Cache decode
DB 5, x1    ; x1 = Cache Descriptor by Intel
DW x2       ; x2 = Cache Size, kilobytes 
}

MACRO END_CACHE
{     ; List terminator
DB 0  ; 0 = End of list
}

;--- This list contains cache only information from original Java Cpuid code,
; TLB and some special information rejected for current purpose

CacheDescriptorsDecoder:
L1I 006h, 8      ; { 0x06, "code L1 cache, 8 KB, 4 ways, 32 byte lines" } , 
L1I 008h, 16     ; { 0x08, "code L1 cache, 16 KB, 4 ways, 32 byte lines" } , 
L1I 009h, 32     ; { 0x09, "code L1 cache, 32 KB, 4 ways, 64 byte lines" } , 
L1D 00Ah, 8      ; { 0x0A, "data L1 cache, 8 KB, 2 ways, 32 byte lines" } , 
L1D 00Ch, 16     ; { 0x0C, "data L1 cache, 16 KB, 4 ways, 32 byte lines" } ,
L1D 00Dh, 16     ; { 0x0D, "data L1 cache, 16 KB, 4 ways, 64 byte lines (ECC)" } , 
L1D 00Eh, 24     ; { 0x0E, "data L1 cache, 24 KB, 6 ways, 64 byte lines" } , 
L1D 010h, 16     ; { 0x10, "data L1 cache, 16 KB, 4 ways, 32 byte lines (IA-64)" } ,
L1I 015h, 16     ; { 0x15, "code L1 cache, 16 KB, 4 ways, 32 byte lines (IA-64)" } , 
L2U 01Ah, 96     ; { 0x1A, "code and data L2 cache, 96 KB, 6 ways, 64 byte lines (IA-64)" } , 
L2U 01Dh, 128    ; { 0x1D, "code and data L2 cache, 128 KB, 2 ways, 64 byte lines" } , 
L2U 021h, 256    ; { 0x21, "code and data L2 cache, 256 KB, 8 ways, 64 byte lines" } ,
L3U 022h, 512    ; { 0x22, "code and data L3 cache, 512 KB, 4 ways (!), 64 byte lines, dual-sectored" } , 
L3U 023h, 1024   ; { 0x23, "code and data L3 cache, 1024 KB, 8 ways, 64 byte lines, dual-sectored" } ,
L2U 024h, 1024   ; { 0x24, "code and data L2 cache, 1024 KB, 16 ways, 64 byte lines" } , 
L3U 025h, 2048   ; { 0x25, "code and data L3 cache, 2048 KB, 8 ways, 64 byte lines, dual-sectored" } , 
L3U 029h, 4096   ; { 0x29, "code and data L3 cache, 4096 KB, 8 ways, 64 byte lines, dual-sectored" } , 
L1D 02Ch, 32     ; { 0x2C, "data L1 cache, 32 KB, 8 ways, 64 byte lines" } ,  
L1I 030h, 32     ; { 0x30, "code L1 cache, 32 KB, 8 ways, 64 byte lines" } ,  
L2U 039h, 128    ; { 0x39, "code and data L2 cache, 128 KB, 4 ways, 64 byte lines, sectored" } ,  
L2U 03Ah, 192    ; { 0x3A, "code and data L2 cache, 192 KB, 6 ways, 64 byte lines, sectored" } ,  
L2U 03Bh, 128    ; { 0x3B, "code and data L2 cache, 128 KB, 2 ways, 64 byte lines, sectored" } ,  
L2U 03Ch, 256    ; { 0x3C, "code and data L2 cache, 256 KB, 4 ways, 64 byte lines, sectored" } ,  
L2U 03Dh, 384    ; { 0x3D, "code and data L2 cache, 384 KB, 6 ways, 64 byte lines, sectored" } ,  
L2U 03Eh, 512    ; { 0x3E, "code and data L2 cache, 512 KB, 4 ways, 64 byte lines, sectored" } ,  
L2U 041h, 128    ; { 0x41, "code and data L2 cache, 128 KB, 4 ways, 32 byte lines" } ,  
L2U 042h, 256    ; { 0x42, "code and data L2 cache, 256 KB, 4 ways, 32 byte lines" } ,  
L2U 043h, 512    ; { 0x43, "code and data L2 cache, 512 KB, 4 ways, 32 byte lines" } ,  
L2U 044h, 1024   ; { 0x44, "code and data L2 cache, 1024 KB, 4 ways, 32 byte lines" } ,  
L2U 045h, 2048   ; { 0x45, "code and data L2 cache, 2048 KB, 4 ways, 32 byte lines" } ,  
L3U 046h, 4096   ; { 0x46, "code and data L3 cache, 4096 KB, 4 ways, 64 byte lines" } ,  
L3U 047h, 8192   ; { 0x47, "code and data L3 cache, 8192 KB, 8 ways, 64 byte lines" } ,  
L2U 048h, 3072   ; { 0x48, "code and data L2 cache, 3072 KB, 12 ways, 64 byte lines" } ,  
; L3U 049h, 4096   ; { 0x49, "code and data L3 cache, 4096 KB, 16 ways, 64 byte lines (P4) or"
;                  ;  + " code and data L2 cache, 4096 KB, 16 ways, 64 byte lines (Core 2)" } ,   
L3U 04Ah, 6144   ; { 0x4A, "code and data L3 cache, 6144 KB, 12 ways, 64 byte lines" } ,  
L3U 04Bh, 8192   ; { 0x4B, "code and data L3 cache, 8192 KB, 16 ways, 64 byte lines" } ,  
L3U 04Ch, 12288  ; { 0x4C, "code and data L3 cache, 12288 KB, 12 ways, 64 byte lines" } ,  
L3U 04Dh, 16384  ; { 0x4D, "code and data L3 cache, 16384 KB, 16 ways, 64 byte lines" } ,
L2U 04Eh, 6144   ; { 0x4E, "code and data L2 cache, 6144 KB, 24 ways, 64 byte lines" } ,  
L1D 060h, 16     ; { 0x60, "data L1 cache, 16 KB, 8 ways, 64 byte lines, sectored" } ,  
L1D 066h, 8      ; { 0x66, "data L1 cache, 8 KB, 4 ways, 64 byte lines, sectored" } ,  
L1D 067h, 16     ; { 0x67, "data L1 cache, 16 KB, 4 ways, 64 byte lines, sectored" } ,  
L1D 068h, 32     ; { 0x68, "data L1 cache, 32 KB, 4 ways, 64 byte lines, sectored" } ,  
L1T 070h, 12     ; { 0x70, "trace L1 cache, 12 K킣Ps, 8 ways" } ,  
L1T 071h, 16     ; { 0x71, "trace L1 cache, 16 K킣Ps, 8 ways" } ,  
L1T 072h, 32     ; { 0x72, "trace L1 cache, 32 K킣Ps, 8 ways" } ,  
L1T 073h, 64     ; { 0x73, "trace L1 cache, 64 K킣Ps, 8 ways" } ,  
L1I 077h, 16     ; { 0x77, "code L1 cache, 16 KB, 4 ways, 64 byte lines, sectored (IA-64)" } ,  
L2U 078h, 1024   ; { 0x78, "code and data L2 cache, 1024 KB, 4 ways, 64 byte lines" } ,  
L2U 079h, 128    ; { 0x79, "code and data L2 cache, 128 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
L2U 07Ah, 256    ; { 0x7A, "code and data L2 cache, 256 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
L2U 07Bh, 512    ; { 0x7B, "code and data L2 cache, 512 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
L2U 07Ch, 1024   ; { 0x7C, "code and data L2 cache, 1024 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
L2U 07Dh, 2048   ; { 0x7D, "code and data L2 cache, 2048 KB, 8 ways, 64 byte lines" } ,  
L2U 07Eh, 256    ; { 0x7E, "code and data L2 cache, 256 KB, 8 ways, 128 byte lines, sect. (IA-64)" } ,  
L2U 07Fh, 512    ; { 0x7F, "code and data L2 cache, 512 KB, 2 ways, 64 byte lines" } ,  
L2U 080h, 512    ; { 0x80, "code and data L2 cache, 512 KB, 8 ways, 64 byte lines" } ,  
L2U 081h, 128    ; { 0x81, "code and data L2 cache, 128 KB, 8 ways, 32 byte lines" } ,  
L2U 082h, 256    ; { 0x82, "code and data L2 cache, 256 KB, 8 ways, 32 byte lines" } ,  
L2U 083h, 512    ; { 0x83, "code and data L2 cache, 512 KB, 8 ways, 32 byte lines" } ,  
L2U 084h, 1024   ; { 0x84, "code and data L2 cache, 1024 KB, 8 ways, 32 byte lines" } ,  
L2U 085h, 2048   ; { 0x85, "code and data L2 cache, 2048 KB, 8 ways, 32 byte lines" } ,  
L2U 086h, 512    ; { 0x86, "code and data L2 cache, 512 KB, 4 ways, 64 byte lines" } ,  
L2U 087h, 1024   ; { 0x87, "code and data L2 cache, 1024 KB, 8 ways, 64 byte lines" } ,
L3U 088h, 2048   ; { 0x88, "code and data L3 cache, 2048 KB, 4 ways, 64 byte lines (IA-64)" } ,  
L3U 089h, 4096   ; { 0x89, "code and data L3 cache, 4096 KB, 4 ways, 64 byte lines (IA-64)" } ,  
L3U 08Ah, 8192   ; { 0x8A, "code and data L3 cache, 8192 KB, 4 ways, 64 byte lines (IA-64)" } ,  
L3U 08Dh, 3072   ; { 0x8D, "code and data L3 cache, 3072 KB, 12 ways, 128 byte lines (IA-64)" } ,  
L3U 0D0h, 512    ; { 0xD0, "code and data L3 cache, 512-kb, 4 ways, 64 byte lines" } ,  
L3U 0D1h, 1024   ; { 0xD1, "code and data L3 cache, 1024-kb, 4 ways, 64 byte lines" } ,  
L3U 0D2h, 2048   ; { 0xD2, "code and data L3 cache, 2048-kb, 4 ways, 64 byte lines" } ,  
L3U 0D6h, 1024   ; { 0xD6, "code and data L3 cache, 1024-kb, 8 ways, 64 byte lines" } ,  
L3U 0D7h, 2048   ; { 0xD7, "code and data L3 cache, 2048-kb, 8 ways, 64 byte lines" } ,  
L3U 0D8h, 4096   ; { 0xD8, "code and data L3 cache, 4096-kb, 8 ways, 64 byte lines" } ,  
L3U 0DCh, 1536   ; { 0xDC, "code and data L3 cache, 1536-kb, 12 ways, 64 byte lines" } ,  
L3U 0DDh, 3072   ; { 0xDD, "code and data L3 cache, 3072-kb, 12 ways, 64 byte lines" } ,  
L3U 0DEh, 6144   ; { 0xDE, "code and data L3 cache, 6144-kb, 12 ways, 64 byte lines" } ,  
L3U 0E2h, 2048   ; { 0xE2, "code and data L3 cache, 2048-kb, 16 ways, 64 byte lines" } ,  
L3U 0E3h, 4096   ; { 0xE3, "code and data L3 cache, 4096-kb, 16 ways, 64 byte lines" } ,  
L3U 0E4h, 8192   ; { 0xE4, "code and data L3 cache, 8192-kb, 16 ways, 64 byte lines" } ,  
L3U 0EAh, 12288  ; { 0xEA, "code and data L3 cache, 12288-kb, 24 ways, 64 byte lines" } ,  
L3U 0EBh, 18432  ; { 0xEB, "code and data L3 cache, 18432-kb, 24 ways, 64 byte lines" } ,  
L3U 0ECh, 24576  ; { 0xEC, "code and data L3 cache, 24576-kb, 24 ways, 64 byte lines" } ,  
END_CACHE        ; List terminator

;------------------------------------------------------------------------------;
;   Data for load some system functions as dynamical import - constants pool.  ;
; This functions not declared in the import section and required manually load ;
; for Windows XP x64 compatibility reasons. Otherwise abort when interpreting  ;
; import section under Windows XP x64, because some API functions not found.   ;
;------------------------------------------------------------------------------;
; Library name KERNEL32.DLL
NameKernel32:                         ; Libraries names
DB  'KERNEL32.DLL',0
; Library name ADVAPI32.DLL
NameAdvapi32:
DB  'ADVAPI32.DLL',0
; Functions names at KERNEL32.DLL
FunctionsNamesKernel32:               ; Functions names
DB  'GlobalMemoryStatusEx',0
DB  'GetNumaHighestNodeNumber',0
DB  'GetNumaNodeProcessorMask',0
DB  'VirtualAllocExNuma',0
DB  'SetThreadAffinityMask',0                
DB  'GetSystemFirmwareTable',0
DB  'GetLogicalProcessorInformation',0
DB  'GetLargePageMinimum',0
DB  'GetActiveProcessorCount',0           ; added for support > 64 processors 
DB  'GetActiveProcessorGroupCount',0      ; added for support > 64 processors
DB  'GetNumaNodeProcessorMaskEx', 0       ; added for support > 64 processors
DB  'SetThreadGroupAffinity',0            ; added for support > 64 processors
DB  0                           ; List must be terminared by additional 00 byte
; Functions names at ADVAPI32.DLL
FunctionsNamesAdvapi32:
DB  'OpenProcessToken',0                  ; added for Large Pages support
DB  'AdjustTokenPrivileges',0             ; added for Large Pages support
DB  0                           ; List must be terminared by additional 00 byte

;------------------------------------------------------------------------------;
; This function used for show warning when 32-bit NCRB runs under Win64,       ;
; this is possible, but non-optimal configuration, can use NCRB x64            ;
; for this platform.                                                           ;
;------------------------------------------------------------------------------;
FunctionNameWoW64:
DB  'IsWow64Process',0
DB  0

