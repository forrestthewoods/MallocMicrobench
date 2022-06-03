# ----------------------------------------------------------------
# doom3_replayreport_crt_1x_WriteByte
# ----------------------------------------------------------------
Hello World!

Selected Allocator: crt

Write Strategy: Write 1 per 4096
Nanoseconds per RDTSC tick: 0.270564

Parsing log file: c:/temp/doom3_journal.txt
Parse complete

Pre-processing replay entries
Num Fixups:           3
Num Leaks:            8473
Cross-thread Frees:   160875
Pre-process complete

Beginning replay
Replay Duration: 433.59 seconds
Replay Speed: 1
Thread 12408 performed 188 allocs and 170 frees
Thread 25524 performed 1 allocs and 0 frees
Thread 19832 performed 120186 allocs and 117035 frees
Thread 19604 performed 116095 allocs and 113475 frees
Thread 8860 performed 1329134 allocs and 1320185 frees
Thread 7360 performed 3966105 allocs and 3972371 frees
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_crt_1x_WriteByte.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  57 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     22 nanoseconds
p25:     23 nanoseconds
p50:     31 nanoseconds
p75:     45 nanoseconds
p90:     58 nanoseconds
p95:     86 nanoseconds
p98:     192 nanoseconds
p99:     316 nanoseconds
p99.9:   3.07 microseconds
p99.99:  25.75 microseconds
p99.999: 95.01 microseconds
Worst:   200.74 microseconds

Free Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     23 nanoseconds
p25:     25 nanoseconds
p50:     32 nanoseconds
p75:     38 nanoseconds
p90:     78 nanoseconds
p95:     94 nanoseconds
p98:     184 nanoseconds
p99:     281 nanoseconds
p99.9:   941 nanoseconds
p99.99:  8.83 microseconds
p99.999: 40.22 microseconds
Worst:   1.55 milliseconds

Goodbye Cruel World!

# ----------------------------------------------------------------
# doom3_replayreport_dlmalloc_1x_SingleThread_WriteByte
# ----------------------------------------------------------------
Hello World!

Selected Allocator: dlmalloc

Write Strategy: Write 1 per 4096
Nanoseconds per RDTSC tick: 0.27049

Parsing log file: c:/temp/doom3_journal.txt
Parse complete

Pre-processing replay entries
Num Fixups:           3
Num Leaks:            8473
Cross-thread Frees:   160875
Pre-process complete

Beginning replay
Replay Duration: 433.59 seconds
Replay Speed: 1
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_dlmalloc_1x_SingleThread_WriteByte.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  37 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    16 nanoseconds
p1:      18 nanoseconds
p10:     19 nanoseconds
p25:     20 nanoseconds
p50:     24 nanoseconds
p75:     28 nanoseconds
p90:     42 nanoseconds
p95:     63 nanoseconds
p98:     115 nanoseconds
p99:     198 nanoseconds
p99.9:   1.33 microseconds
p99.99:  4.85 microseconds
p99.999: 15.26 microseconds
Worst:   218.95 microseconds

Free Time
Best:    16 nanoseconds
p1:      18 nanoseconds
p10:     20 nanoseconds
p25:     21 nanoseconds
p50:     23 nanoseconds
p75:     32 nanoseconds
p90:     64 nanoseconds
p95:     100 nanoseconds
p98:     148 nanoseconds
p99:     188 nanoseconds
p99.9:   467 nanoseconds
p99.99:  1.51 microseconds
p99.999: 17.42 microseconds
Worst:   214.04 microseconds

Goodbye Cruel World!


# ----------------------------------------------------------------
# doom3_replayreport_HeapAlloc_1x_WriteByte
# ----------------------------------------------------------------
Hello World!

Selected Allocator: HeapAlloc

Write Strategy: Write 1 per 4096
Nanoseconds per RDTSC tick: 0.270564

Parsing log file: c:/temp/doom3_journal.txt
Parse complete

Pre-processing replay entries
Num Fixups:           3
Num Leaks:            8473
Cross-thread Frees:   160875
Pre-process complete

Beginning replay
Replay Duration: 433.59 seconds
Replay Speed: 1
Thread 12408 performed 188 allocs and 170 frees
Thread 25524 performed 1 allocs and 0 frees
Thread 19832 performed 120186 allocs and 117035 frees
Thread 19604 performed 116095 allocs and 113475 frees
Thread 8860 performed 1329134 allocs and 1320185 frees
Thread 7360 performed 3966105 allocs and 3972371 frees
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_HeapAlloc_1x_WriteByte.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  58 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     22 nanoseconds
p25:     23 nanoseconds
p50:     34 nanoseconds
p75:     45 nanoseconds
p90:     61 nanoseconds
p95:     91 nanoseconds
p98:     206 nanoseconds
p99:     385 nanoseconds
p99.9:   2.88 microseconds
p99.99:  26.21 microseconds
p99.999: 93.72 microseconds
Worst:   261.03 microseconds

Free Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     23 nanoseconds
p25:     24 nanoseconds
p50:     33 nanoseconds
p75:     40 nanoseconds
p90:     79 nanoseconds
p95:     101 nanoseconds
p98:     202 nanoseconds
p99:     316 nanoseconds
p99.9:   1.10 microseconds
p99.99:  9.42 microseconds
p99.999: 42.25 microseconds
Worst:   1.21 milliseconds

Goodbye Cruel World!

# ----------------------------------------------------------------
# doom3_replayreport_jemalloc_1x_WriteByte
# ----------------------------------------------------------------
Hello World!

Selected Allocator: jemalloc

Write Strategy: Write 1 per 4096
Nanoseconds per RDTSC tick: 0.270564

Parsing log file: c:/temp/doom3_journal.txt
Parse complete

Pre-processing replay entries
Num Fixups:           3
Num Leaks:            8473
Cross-thread Frees:   160875
Pre-process complete

Beginning replay
Replay Duration: 433.59 seconds
Replay Speed: 1
Thread 12408 performed 188 allocs and 170 frees
Thread 25524 performed 1 allocs and 0 frees
Thread 19832 performed 120186 allocs and 117035 frees
Thread 19604 performed 116095 allocs and 113475 frees
Thread 8860 performed 1329134 allocs and 1320185 frees
Thread 7360 performed 3966105 allocs and 3972371 frees
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_jemalloc_1x_WriteByte.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  36 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    5 nanoseconds
p1:      7 nanoseconds
p10:     7 nanoseconds
p25:     8 nanoseconds
p50:     9 nanoseconds
p75:     18 nanoseconds
p90:     22 nanoseconds
p95:     35 nanoseconds
p98:     204 nanoseconds
p99:     661 nanoseconds
p99.9:   2.82 microseconds
p99.99:  10.76 microseconds
p99.999: 23.05 microseconds
Worst:   841.86 microseconds

Free Time
Best:    9 nanoseconds
p1:      10 nanoseconds
p10:     10 nanoseconds
p25:     11 nanoseconds
p50:     19 nanoseconds
p75:     25 nanoseconds
p90:     31 nanoseconds
p95:     40 nanoseconds
p98:     112 nanoseconds
p99:     361 nanoseconds
p99.9:   1.87 microseconds
p99.99:  3.69 microseconds
p99.999: 24.81 microseconds
Worst:   2.35 milliseconds

Goodbye Cruel World!

# ----------------------------------------------------------------
# doom3_replayreport_mimalloc_1x_WriteByte
# ----------------------------------------------------------------
Hello World!

Selected Allocator: mimalloc

Write Strategy: Write 1 per 4096
Nanoseconds per RDTSC tick: 0.270564

Parsing log file: c:/temp/doom3_journal.txt
Parse complete

Pre-processing replay entries
Num Fixups:           3
Num Leaks:            8473
Cross-thread Frees:   160875
Pre-process complete

Beginning replay
Replay Duration: 433.59 seconds
Replay Speed: 1
Thread 12408 performed 188 allocs and 170 frees
Thread 25524 performed 1 allocs and 0 frees
Thread 19832 performed 120186 allocs and 117035 frees
Thread 19604 performed 116095 allocs and 113475 frees
Thread 8860 performed 1329134 allocs and 1320185 frees
Thread 7360 performed 3966105 allocs and 3972371 frees
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_mimalloc_1x_WriteByte.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  20 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    4 nanoseconds
p1:      5 nanoseconds
p10:     5 nanoseconds
p25:     5 nanoseconds
p50:     5 nanoseconds
p75:     7 nanoseconds
p90:     9 nanoseconds
p95:     31 nanoseconds
p98:     75 nanoseconds
p99:     198 nanoseconds
p99.9:   1.63 microseconds
p99.99:  5.94 microseconds
p99.999: 23.71 microseconds
Worst:   990.11 microseconds

Free Time
Best:    4 nanoseconds
p1:      5 nanoseconds
p10:     5 nanoseconds
p25:     5 nanoseconds
p50:     6 nanoseconds
p75:     7 nanoseconds
p90:     8 nanoseconds
p95:     14 nanoseconds
p98:     46 nanoseconds
p99:     85 nanoseconds
p99.9:   259 nanoseconds
p99.99:  633 nanoseconds
p99.999: 11.39 microseconds
Worst:   161.56 microseconds

Goodbye Cruel World!


# ----------------------------------------------------------------
# doom3_replayreport_rpmalloc_1x_WriteByte
# ----------------------------------------------------------------
Hello World!

Selected Allocator: rpmalloc

Initializing rpmalloc

Write Strategy: Write 1 per 4096
Nanoseconds per RDTSC tick: 0.270371

Parsing log file: c:/temp/doom3_journal.txt
Parse complete

Pre-processing replay entries
Num Fixups:           3
Num Leaks:            8473
Cross-thread Frees:   160875
Pre-process complete

Beginning replay
Replay Duration: 433.59 seconds
Replay Speed: 1
Thread 12408 performed 188 allocs and 170 frees
Thread 25524 performed 1 allocs and 0 frees
Thread 19832 performed 120186 allocs and 117035 frees
Thread 19604 performed 116095 allocs and 113475 frees
Thread 8860 performed 1329134 allocs and 1320185 frees
Thread 7360 performed 3966105 allocs and 3972371 frees
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_rpmalloc_1x_WriteByte.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  12 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    4 nanoseconds
p1:      4 nanoseconds
p10:     5 nanoseconds
p25:     5 nanoseconds
p50:     5 nanoseconds
p75:     6 nanoseconds
p90:     8 nanoseconds
p95:     16 nanoseconds
p98:     33 nanoseconds
p99:     61 nanoseconds
p99.9:   1.36 microseconds
p99.99:  8.05 microseconds
p99.999: 20.70 microseconds
Worst:   216.73 microseconds

Free Time
Best:    4 nanoseconds
p1:      5 nanoseconds
p10:     5 nanoseconds
p25:     5 nanoseconds
p50:     5 nanoseconds
p75:     6 nanoseconds
p90:     12 nanoseconds
p95:     18 nanoseconds
p98:     44 nanoseconds
p99:     87 nanoseconds
p99.9:   248 nanoseconds
p99.99:  505 nanoseconds
p99.999: 11.20 microseconds
Worst:   172.82 microseconds

Goodbye Cruel World!


# ----------------------------------------------------------------
# doom3_replayreport_tlsf_1x_SingleThread_WriteByte
# ----------------------------------------------------------------
Hello World!

Selected Allocator: tlsf

Initializing tlsf. Pool size: 3.00 gigabytes

Write Strategy: Write 1 per 4096
Nanoseconds per RDTSC tick: 0.270564

Parsing log file: c:/temp/doom3_journal.txt
Parse complete

Pre-processing replay entries
Num Fixups:           3
Num Leaks:            8473
Cross-thread Frees:   160875
Pre-process complete

Beginning replay
Replay Duration: 433.59 seconds
Replay Speed: 1
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_tlsf_1x_SingleThread_WriteByte.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  19 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    5 nanoseconds
p1:      6 nanoseconds
p10:     7 nanoseconds
p25:     14 nanoseconds
p50:     15 nanoseconds
p75:     20 nanoseconds
p90:     26 nanoseconds
p95:     34 nanoseconds
p98:     70 nanoseconds
p99:     136 nanoseconds
p99.9:   315 nanoseconds
p99.99:  989 nanoseconds
p99.999: 15.14 microseconds
Worst:   30.34 microseconds

Free Time
Best:    4 nanoseconds
p1:      5 nanoseconds
p10:     5 nanoseconds
p25:     7 nanoseconds
p50:     11 nanoseconds
p75:     20 nanoseconds
p90:     29 nanoseconds
p95:     52 nanoseconds
p98:     101 nanoseconds
p99:     139 nanoseconds
p99.9:   353 nanoseconds
p99.99:  643 nanoseconds
p99.999: 14.09 microseconds
Worst:   119.92 microseconds

Goodbye Cruel World!

# ----------------------------------------------------------------
# doom3_replayreport_crt_1x_WriteNone
# ----------------------------------------------------------------
Hello World!

Selected Allocator: crt

Write Strategy: NoWrite
Nanoseconds per RDTSC tick: 0.270565

Parsing log file: c:/temp/doom3_journal.txt
Parse complete

Pre-processing replay entries
Num Fixups:           3
Num Leaks:            8473
Cross-thread Frees:   160875
Pre-process complete

Beginning replay
Replay Duration: 433.59 seconds
Replay Speed: 1
Thread 12408 performed 188 allocs and 170 frees
Thread 25524 performed 1 allocs and 0 frees
Thread 19832 performed 120186 allocs and 117035 frees
Thread 19604 performed 116095 allocs and 113475 frees
Thread 8860 performed 1329134 allocs and 1320185 frees
Thread 7360 performed 3966105 allocs and 3972371 frees
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_crt_1x_WriteNone.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  58 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     22 nanoseconds
p25:     24 nanoseconds
p50:     31 nanoseconds
p75:     45 nanoseconds
p90:     60 nanoseconds
p95:     93 nanoseconds
p98:     216 nanoseconds
p99:     371 nanoseconds
p99.9:   2.81 microseconds
p99.99:  26.18 microseconds
p99.999: 89.04 microseconds
Worst:   320.60 microseconds

Free Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     23 nanoseconds
p25:     25 nanoseconds
p50:     31 nanoseconds
p75:     38 nanoseconds
p90:     80 nanoseconds
p95:     102 nanoseconds
p98:     200 nanoseconds
p99:     310 nanoseconds
p99.9:   1.02 microseconds
p99.99:  8.86 microseconds
p99.999: 39.00 microseconds
Worst:   1.57 milliseconds

Goodbye Cruel World!


# ----------------------------------------------------------------
# doom3_replayreport_crt_1x_WriteAll
# ----------------------------------------------------------------
Hello World!

Selected Allocator: crt

Write Strategy: memset all
Nanoseconds per RDTSC tick: 0.270517

Parsing log file: c:/temp/doom3_journal.txt
Parse complete

Pre-processing replay entries
Num Fixups:           3
Num Leaks:            8473
Cross-thread Frees:   160875
Pre-process complete

Beginning replay
Replay Duration: 433.59 seconds
Replay Speed: 1
Thread 12408 performed 188 allocs and 170 frees
Thread 25524 performed 1 allocs and 0 frees
Thread 19832 performed 120186 allocs and 117035 frees
Thread 19604 performed 116095 allocs and 113475 frees
Thread 8860 performed 1329134 allocs and 1320185 frees
Thread 7360 performed 3966105 allocs and 3972371 frees
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_crt_1x_WriteAll.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  136 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    21 nanoseconds
p1:      23 nanoseconds
p10:     33 nanoseconds
p25:     71 nanoseconds
p50:     85 nanoseconds
p75:     106 nanoseconds
p90:     128 nanoseconds
p95:     183 nanoseconds
p98:     372 nanoseconds
p99:     563 nanoseconds
p99.9:   5.53 microseconds
p99.99:  59.84 microseconds
p99.999: 299.84 microseconds
Worst:   9.25 milliseconds

Free Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     23 nanoseconds
p25:     25 nanoseconds
p50:     32 nanoseconds
p75:     40 nanoseconds
p90:     79 nanoseconds
p95:     104 nanoseconds
p98:     209 nanoseconds
p99:     315 nanoseconds
p99.9:   1.08 microseconds
p99.99:  10.01 microseconds
p99.999: 195.29 microseconds
Worst:   4.62 milliseconds

Goodbye Cruel World!