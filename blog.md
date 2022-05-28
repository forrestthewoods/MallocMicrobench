# Benchmarking Malloc with Doom 3
# How Slow is Malloc
# How Long Does it Take to Allocate Memory
# Profiling Malloc with Doom 3
# Benchmarking Malloc with Doom 3

This blog post began the same way as many of my posts. I nerd sniped myself.

I recently read a blog post about a new library for game audio. It repeated a claim that is conventional wisdom in the audio programming world - "you can't allocate or deallocate memory on the audio thread". That's a very strong statement, and I'm not convinced it's correct.

Lately I've been doing quite a bit of audio and audio-like programming lately. My code is reasonably efficient and does pre-allocate quite a bit. However it uses both `malloc` and `mutex`. It also happens to run great and glitch free!

This raises a question: what is the realistic worst-case performance for `malloc` on a modern machine?

<insert Tim Sweeney tweet>
https://twitter.com/TimSweeneyEpic/status/1526439480873328640

# Framing the Question

Talking about performance is hard. What's even the right question here? Is it worst case malloc? 99.9th percentile? Total malloc time per frame? I'm not sure.

Answering the question is even harder. The boring answer the same as always, it depends! How big is your allocation? How fragmented are you? What is your program's allocation pattern? Is it multi-threaded? What is your system doing? Are you a long lived server or short lived game? It always depends.

Allocating memory isn't free. But neither is time spent on complex systems full of error prone lockless programming. My spidey sense suggests `malloc` is cheaper than people realize.

For today I'm focused on games. Modern games run between 60 and 144 frames per second. One frame at 144Hz is about 7 milliseconds. That's not a lot of time! Except I know for a fact that most games hit the allocator hard. Worst case in theory sets my computer on fire. But what is it in practice?

My goal isn't to come up with a singular, definitive answer. I'm a big fan of napkin math. I want a rough estimate of when `malloc` will cause me to miss my target framerate.

# Creating a Journal

My first attempt at a benchmark involved randomly allocating and freeing blocks of memory. Twitter friends correctly scolded me and said that's not good enough. I need real data with real allocation patterns and sizes.

The goal is to create a "journal" of memory operations. It should record `malloc` and `free` operations with their inputs and outputs. Then the journal can be replayed with different allocators to compare performance.

Unfortunately I don't have a suitable personal project for generating this journal. I did a quick search for open source games and the choice was obvious - Doom 3!

It took some time to find a Doom 3 project that had a "just works" Visual Studio project. Eventually I found [RBDOOM-3-BFG](https://github.com/RobertBeckebans/RBDOOM-3-BFG) which only took a little effort to get running.

All memory allocations go through simple `Mem_Alloc16` and `Mem_Free16` functions in [Heap.cpp](https://github.com/RobertBeckebans/RBDOOM-3-BFG/blob/master/neo/idlib/Heap.cpp). Modifying this was trivial. I started with the simplest possible thing and wrote every allocation to disk via `std::fwrite`. It runs a solid 60fps even in debug mode. Ship it!

``` cpp
void* fts_allocator::allocInternal(const size_t size) {
	auto allocRelTime = (Clock::now() - _timeBegin);

	// Perform actual malloc
	auto ptr = _aligned_malloc(size, 16);

	// AllocEntry = a allocSize ptr threadId timestamp
	std::array<char, 256> buf;
	int len = std::snprintf(
		buf.data(),
		buf.size(), 
		"a %lld %p %lu %lld\n",
		size, ptr, GetCurrentThreadId(), allocRelTime.count());
	std::fwrite(buf.data(), 1, len, _file);

	return ptr;
}
```

Running `Doom3BFG.exe` now produces a file called `doom3_journal.txt`. This journal records every single `malloc` and `free` from startup to shut down. It looks like this:

```
a 2048 0000023101F264A0 15888 542200
a 1280 0000023101F28020 15888 1098100
a 2560 0000023101F298A0 15888 1130500
f 0000023101F28020 15888 1142000
a 3840 0000023101F2A300 15888 1146900
f 0000023101F298A0 15888 1154900
a 1280 0000023101F28020 15888 1171200
a 2560 0000023101F298A0 15888 1189500
f 0000023101F28020 15888 1191900
a 3840 0000023101F2B260 15888 1202200
f 0000023101F298A0 15888 1207900
```

All content for the rest of this post is derived from the same journal. Its a 315 Mb file containing over 8,000,000 lines. Roughly 4 million `mallocs` and 4 million `frees`. It leaks 4 megabytes from 4538 `mallocs`, tsk tsk.

The journal covers 6 minutes of game time. I entered the main menu, selected a level, played for ~5 minutes, died, returned to main menu, and quit to desktop. I did this a few times and each run appeared to produce very similar journals.

# Replaying the Journal
Next, we need to write code to load and replay the journal. To do this I created a new C++ project called `MallocMicrobench`. The code is very roughly:

```cpp
std::vector<Entry> journal = ParseJournal("doom3_journal.txt");
for (auto& entry : journal) {
    // Spin until journal time
    while (ReplayClock::now() < entry.timepoint) {}

    if (entry.op == Alloc) {
        auto allocStart = RdtscClock::now();
        void* ptr = ::malloc(entry.size);
        auto mallocTime = RdtscClock::now() - allocStart;
    } else {
        auto freeStart = RdtscClock::now();
        ::free(entry.ptr);
        auto freeTime = RdtscClock::now() - freeStart;
    }
}
```

This snippet excludes configuration and bookkeeping. The basic idea is very simple.

Running my journal through the new replay system produces the following output:

TODO: update

```
Parsing log file: C:/temp/doom3_journal.txt
Parse complete

Replay Duration: 325.93 seconds
Beginning replay
Replay complete

== Replay Results ==
Number of Mallocs:    4070475
Number of Frees:      4065937
Total Allocation:     1.66 gigabytes
Max Live Bytes:       330 megabytes
Average Allocation:   437 bytes
Average Malloc Time:  45 nanoseconds
Num Leaked Bytes:     4 megabytes

Alloc Time
Best:    21 nanoseconds
p1:      22 nanoseconds
p10:     23 nanoseconds
p25:     24 nanoseconds
p50:     25 nanoseconds
p75:     31 nanoseconds
p90:     45 nanoseconds
p95:     54 nanoseconds
p98:     86 nanoseconds
p99:     149 nanoseconds
p99.9:   2.67 microseconds
p99.99:  25.91 microseconds
p99.999: 90.95 microseconds
Worst:   231.59 microseconds

Free Time
Best:    21 nanoseconds
p1:      23 nanoseconds
p10:     24 nanoseconds
p25:     25 nanoseconds
p50:     28 nanoseconds
p75:     33 nanoseconds
p90:     43 nanoseconds
p95:     78 nanoseconds
p98:     97 nanoseconds
p99:     120 nanoseconds
p99.9:   572 nanoseconds
p99.99:  6.99 microseconds
p99.999: 49.02 microseconds
Worst:   1.17 milliseconds
```

Interesting! Average `malloc` time is just 45 nanoseconds. Pretty good. However p99.9 is 2.67 microseconds. That's 1 in 1000. Worst case is a whopping 230 microseconds, ouch! `free` is similar, but with a shockingly bad worst case performance of 1.17 milliseconds. Yikes! 

What does this mean for hitting a stable 60Hz or 144Hz? Honestly, I don't know. There's a ton of variance. Are the slow allocations because of very large allocations? Do slow allocs occur only during a loading screen or also in the middle of gameplay? Are extreme outliers due to OS context switches? We don't have enough information.

# Visualizing the Journal

To visualize the data we need to take `doom3_journal.txt`, run a replay, and then produce a new `doom3_replayreport.csv`. It adds replay timestamps and replay profile time for `malloc` and `free`. `free` data is stored adjacent to `alloc` data so it only has 4 million rows. It looks like this:


```csv
replayAllocTimestamp,allocTime,allocSize,replayFreeTimestamp,freeTime
895900,392,2048,0,0
896500,214,1280,897100,480
896800,202,2560,897800,208
897600,160,3840,0,0
898100,171,1280,898500,143
898300,141,2560,903200,145
898700,4487,3840,0,0
903400,191,3760,596526100,247
903600,1207,3760,596526300,227
904900,1205,3760,596526600,107
```

To better understand the data I decided to build a visualization using Python's [matplotlib](https://matplotlib.org/).

My first attempt was a heatmap. This did not go well and was not useful. I soon tried a scatterplot. This worked out great. Unfortunately, Python is a miserably slow language so rendering 8 million points took over 30 seconds. Yikes!

A kind Twitter user pointed me towards a matplotlib extension called [mpl-scatter-density](https://github.com/astrofrog/mpl-scatter-density). This worked phenomenally well and turned 40 seconds into 3 seconds. My biggest bottleneck is actually csv parsing. 

New tools in tow I produced this:

<insert screenshot of crt malloc>
<include button to toggle between malloc and free>

Data visualization is story telling. What story


# Latency vs Reliability is a Trade-Off

# Conclusion

# Extra Thoughts

## Hardware Setup
All tests were run on a Windows 10 desktop with i7-8700k and 32Gb of RAM. My computer was not doing any obvious work. However I did not close every background application to produce the fastest possible result. My computer was in a state typical for playing games.

I have not tested macOS or Linux. The replay code will require a few changes to work on other platforms.

## Quake 2
I actually tried Quake 2 before Doom 3. Unfortunately it's too efficient and has virtually no calls to `malloc` once a level has loaded!

The one exception, `malloc` is called every single frame on player input. You can see this in `cmd.c` (link)[https://github.com/id-Software/Quake-2/blob/master/qcommon/cmd.c#L677]. So close to being `malloc` free!


## Using RDTSC
`std::chrono` clocks have a maximum precision of 100 nanoseconds. My initial replay system used `std::chrono::high_resolution_clock` and my report produced malloc times of either 0ns or 100ns. This is clearly insufficient for what I'm attempting to measure.

Using RDTSC is tricky. The conversion from ticks to nanoseconds requires procedurally measuring RDTSC against another timer. I performed twenty 5-millisecond spins to calibrate. The result does vary. My i7-8700k machine computes 0.270564 nanoseconds per tick. The inverse of which almost perfectly matches my 3.7 GHz CPU.


# TODO
* Filter charts based on allocation size?
* Run benchmarks again with a written byte