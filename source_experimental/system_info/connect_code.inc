;------------------------------------------------------------------------------;
;     System Information Library: connect include files with subroutines.      ;
;------------------------------------------------------------------------------;

include 'checkcpuid.inc'      ; Check CPUID instruction and functions available
include 'sysfncload.inc'      ; Load optional WinAPI by dynamical import
include 'getacpi.inc'         ; Get and parse ACPI tables, use WinAPI
include 'getcpucache.inc'     ; Get CPU cache by CPUID instruction
include 'getcpufeatures.inc'  ; Get CPU features by CPUID/XGETBV instructions 
include 'getcpuname.inc'      ; Get CPU name by CPUID instruction
include 'getlargepages.inc'   ; Get large pages feature availability by WinAPI
include 'getosmemory.inc'     ; Get memory information by WinAPI
include 'getosnuma.inc'       ; Get NUMA topology by WinAPI
include 'getospgroups.inc'    ; Get processor groups by WinAPI, for >64 CPUs
include 'getostopology.inc'   ; Get system topology by WinAPI
include 'measuretsc.inc'      ; Measure TSC (Time Stamp Counter) frequency

