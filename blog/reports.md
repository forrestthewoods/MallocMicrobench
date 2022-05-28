# ----------------------------------------------------------------
# doom3_replayreport_crt_1x_MultiThread
# ----------------------------------------------------------------

Hello World!

Selected Allocator: crt

Nanoseconds per RDTSC tick: 0.270564

Parsing log file: c:/temp/doom3_journal.txt
Parse complete

Pre-processing replay entries
Num Fixups:   3
Num Leaks:    8473
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

Writing alloc times to: c:/temp/doom3_replayreport_crt_1x_MultiThread.csv
Write complete
== Replay Results ==
Number of Mallocs:    5531709
Number of Frees:      5523236
Total Allocation:     2.47 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   480 bytes
Median Allocation:   64 bytes
Average Malloc Time:  57 nanoseconds
Num Leaked Bytes:     7 megabytes

Alloc Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     22 nanoseconds
p25:     24 nanoseconds
p50:     35 nanoseconds
p75:     46 nanoseconds
p90:     58 nanoseconds
p95:     84 nanoseconds
p98:     186 nanoseconds
p99:     276 nanoseconds
p99.9:   2.76 microseconds
p99.99:  25.78 microseconds
p99.999: 100.11 microseconds
Worst:   235.77 microseconds

Free Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     23 nanoseconds
p25:     25 nanoseconds
p50:     33 nanoseconds
p75:     40 nanoseconds
p90:     77 nanoseconds
p95:     93 nanoseconds
p98:     184 nanoseconds
p99:     272 nanoseconds
p99.9:   920 nanoseconds
p99.99:  7.35 microseconds
p99.999: 35.39 microseconds
Worst:   1.55 milliseconds

Goodbye Cruel World!