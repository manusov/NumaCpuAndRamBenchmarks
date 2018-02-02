;------------------------------------------------------------------------------;
;                  System parameters, detected at start                        ;
;------------------------------------------------------------------------------;

align 64

SystemParameters:

CpuVendorString  DB  12 DUP (?)      ; Result of CPUID#0
                 DB  ?               ; Terminator location 
CpuNameString    DB  48 DUP (?)      ; Result of CPUID#80000002h-80000004h
                 DB  ?               ; Terminator location
CpuSignature     DD  ?               ; Resut EAX of CPUID#1 = TFMS

;--- Special flags ---
; D0=SSE, D1=AVX, D2=FMA, D3=AVX2, D4=AVX512F (foundation part of AVX512),
; D[5-6]=Reserved, D7=HT
; Since v0.93 bit D5 = validation SSE4.1 for 128-bit VMOVNTDQA NT Read
; HT bit can be updated by WinAPI results, note CPU can return HT support=1
; when CPU support HT but HT disabled by CMOS setup option or OS
;---
CpuSpecFlags     DB  ?               ; Special flags

;--- TSC Frequency (Hz) and Period (fs), 1 fs = 10^-15 seconds ---
TscClockHz       DQ  ?               ; Time Stamp Counter frequency, Hz
TscPeriodFs      DQ  ?               ; Time Stamp Counter period, Fs

;--- Cache info format ---
; Qword=Size, bytes,
; Word=Maximum threads at this level
; Word=Maximum APIC IDs per package
; For Trace Cache size in micro operations, not bytes
;---
CacheTrace       DQ  ?	             ; Instruction trace cache
                 DW  ? , ?
CacheL1Code      DQ  ?	             ; L1 instruction cache
                 DW  ? , ?
CacheL1Data      DQ  ?	             ; L1 data cache
                 DW  ? , ?
CacheL2Unified   DQ  ?	             ; L2 unified cache
                 DW  ? , ?
CacheL3Unified   DQ  ?	             ; L3 unified cache
                 DW  ? , ?

;--- MADT = Multiple APICs Descripton Table extracted info ---
; Text and numbers format:
; 6 bytes [00-05] = OEM ID (ACPI MADT relative), text
; 8 bytes [06-13] = Manufacturer Model ID (ACPI MADT relative), text
; 4 bytes [14-17] = OEM revision (ACPI MADT relative), number
;---
MadtText         DB  14 DUP (?)      ; ACPI MADT Text Tags
MadtOemRevision  DD  ?               ; OEM Revision, 32-bit number
MadtLocalApics	 DD  ?               ; Number of CPU Local APICs
MadtIoApics      DD  ?               ; Number of I/O APICs


;--- SRAT = System (Static) Resources Affinity Table extracted info ---
; Text format same as MADT, see upper block
;---
SratText         DB  14 DUP (?)      ; ACPI SRAT Text Tags
SratOemRevision  DD  ?	             ; OEM Revision, 32-bit number
SratDomains      DD  ?               ; Number of NUMA Domains
SratCpus         DD  ?               ; Number of CPUs (as Local APICs)
SratRams         DD  ?	             ; Number of RAMs (as Regions)
		  
;--- NUMA = Non Uniform Memory Access info, maximum for 64 NUMA Nodes ---
NumaNodes        DD  ?	             ; Number of NUMA Nodes
NumaNodesMasks   DQ  64 DUP (?)      ; Affinity masks for 64 nodes maximum

;--- Operating System base info ---
; wProcessorArchitecture      dw ?
; wReserved                   dw ?
; dwPageSize                  dd ?
; lpMinimumApplicationAddress dq ?
; lpMaximumApplicationAddress dq ?
; dwActiveProcessorMask       dq ?
; dwNumberOfProcessors	      dd ?
; dwProcessorType             dd ?
; dwAllocationGranularity     dd ?
; wProcessorLevel             dw ?
; wProcessorRevision          dw ?
;---
SystemInfo	 SYSTEM_INFO   ; Length = 48 bytes

;--- Memory info --
; dwLength                    dd ?
; dwMemoryLoad                dd ?
; ullTotalPhys                dq ?
; ullAvailPhys                dq ?
; ullTotalPageFile            dq ?
; ullAvailPageFile            dq ?
; ullTotalVirtual             dq ?
; ullAvailVirtual             dq ?
; ullAvailExtendedVirtual     dq ?
;---
MemoryInfo	      DB  64 DUP (?)     ; Length = 64 bytes

;--- 32 bytes CACHE SIZE INFO ---
; Qword [000-007] = L1 instruction cache size (bytes)
; Qword [008-015] = L1 data cache size (bytes)
; Qword [016-023] = L2 unified cache size (bytes)
; Qword [024-031] = L3 unified cache size (bytes)
;--- 16 bytes CACHE TOPOLOGY INFO ---
; Dword [032-035] = Threads per L1 instruction cache
; Dword [036-039] = Threads per L1 data cache
; Dword [040-043] = Threads per L2 unified cache 
; Dword [044-047] = Threads per L3 unified cache 
;--- 16 bytes MULTIPROCESSING TOPOLOGY INFO ---
; Dword [048-051] = Threads per one core (SMT)
; Dword [052-055] = Cores per one package (or per one NUMA node)
; Dword [056-059] = Total NUMA nodes per platform
; Dword [060-063] = Total CPU packages per platform 
;---

TopologicalInfo   DB  64 DUP (?)     ; Length = 64 bytes

