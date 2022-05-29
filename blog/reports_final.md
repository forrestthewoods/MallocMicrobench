# ----------------------------------------------------------------
# crt 10x WriteStrategy=0
# ----------------------------------------------------------------
Hello World!

Selected Allocator: crt

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
Replay Speed: 10
Thread 12408 performed 188 allocs and 170 frees
Thread 25524 performed 1 allocs and 0 frees
Thread 19832 performed 120186 allocs and 117035 frees
Thread 19604 performed 116095 allocs and 113475 frees
Thread 8860 performed 1329134 allocs and 1320185 frees
Thread 7360 performed 3966105 allocs and 3972371 frees
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_crt_10x_WriteNone.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  54 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     25 nanoseconds
p25:     26 nanoseconds
p50:     37 nanoseconds
p75:     44 nanoseconds
p90:     54 nanoseconds
p95:     63 nanoseconds
p98:     110 nanoseconds
p99:     180 nanoseconds
p99.9:   2.40 microseconds
p99.99:  26.72 microseconds
p99.999: 103.46 microseconds
Worst:   157.25 microseconds

Free Time
Best:    21 nanoseconds
p1:      23 nanoseconds
p10:     24 nanoseconds
p25:     27 nanoseconds
p50:     32 nanoseconds
p75:     36 nanoseconds
p90:     73 nanoseconds
p95:     83 nanoseconds
p98:     112 nanoseconds
p99:     166 nanoseconds
p99.9:   699 nanoseconds
p99.99:  7.14 microseconds
p99.999: 41.65 microseconds
Worst:   1.35 milliseconds

Goodbye Cruel World!

# ----------------------------------------------------------------
# crt 10x WriteStrategy=1
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
Replay Speed: 10
Thread 12408 performed 188 allocs and 170 frees
Thread 25524 performed 1 allocs and 0 frees
Thread 19832 performed 120186 allocs and 117035 frees
Thread 19604 performed 116095 allocs and 113475 frees
Thread 8860 performed 1329134 allocs and 1320185 frees
Thread 7360 performed 3966105 allocs and 3972371 frees
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_crt_10x.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  52 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    21 nanoseconds
p1:      24 nanoseconds
p10:     25 nanoseconds
p25:     26 nanoseconds
p50:     33 nanoseconds
p75:     44 nanoseconds
p90:     51 nanoseconds
p95:     62 nanoseconds
p98:     111 nanoseconds
p99:     180 nanoseconds
p99.9:   2.56 microseconds
p99.99:  25.85 microseconds
p99.999: 93.90 microseconds
Worst:   162.17 microseconds

Free Time
Best:    21 nanoseconds
p1:      24 nanoseconds
p10:     25 nanoseconds
p25:     26 nanoseconds
p50:     31 nanoseconds
p75:     36 nanoseconds
p90:     73 nanoseconds
p95:     84 nanoseconds
p98:     110 nanoseconds
p99:     163 nanoseconds
p99.9:   685 nanoseconds
p99.99:  7.03 microseconds
p99.999: 35.74 microseconds
Worst:   1.32 milliseconds

Goodbye Cruel World!

# ----------------------------------------------------------------
# crt 10x WriteStrategy=2
# ----------------------------------------------------------------
Hello World!

Selected Allocator: crt

Write Strategy: memset all
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
Replay Speed: 10
Thread 12408 performed 188 allocs and 170 frees
Thread 25524 performed 1 allocs and 0 frees
Thread 19832 performed 120186 allocs and 117035 frees
Thread 19604 performed 116095 allocs and 113475 frees
Thread 8860 performed 1329134 allocs and 1320185 frees
Thread 7360 performed 3966105 allocs and 3972371 frees
Replay complete

Writing alloc times to: c:/temp/doom3_replayreport_crt_10x_WriteAll.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:    64 bytes
Average Malloc Time:  97 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    21 nanoseconds
p1:      24 nanoseconds
p10:     27 nanoseconds
p25:     38 nanoseconds
p50:     49 nanoseconds
p75:     61 nanoseconds
p90:     94 nanoseconds
p95:     115 nanoseconds
p98:     180 nanoseconds
p99:     306 nanoseconds
p99.9:   4.89 microseconds
p99.99:  61.65 microseconds
p99.999: 306.77 microseconds
Worst:   9.08 milliseconds

Free Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     23 nanoseconds
p25:     26 nanoseconds
p50:     32 nanoseconds
p75:     36 nanoseconds
p90:     70 nanoseconds
p95:     84 nanoseconds
p98:     111 nanoseconds
p99:     165 nanoseconds
p99.9:   728 nanoseconds
p99.99:  8.18 microseconds
p99.999: 181.16 microseconds
Worst:   4.05 milliseconds

Goodbye Cruel World!