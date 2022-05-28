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

For today I'm focused on games. Modern games run between 60 and 144 frames per second. One frame at 144Hz is about 7 milliseconds. That's not very many milliseconds! Except I know for a fact that most games hit the allocator hard. Worst case in theory sets my computer on fire. But what is it in practice?

My goal isn't to come up with a singular, definitive answer. I'm a big fan of napkin math. I want a ballpark guideline of when `malloc` may cause me to miss my target framerate.

# Creating a Journal

My first attempt at a benchmark involved allocating and freeing blocks of random size. Twitter friends correctly scolded me and said that's not good enough. I need real data with real allocation patterns and sizes.

The goal is to create a "journal" of memory operations. It should record `malloc` and `free` operations with their inputs and outputs. Then the journal can be replayed with different allocators to compare performance.

Unfortunately I don't have a suitable personal project for generating this journal. I did a quick search for open source games and the choice was obvious - Doom 3!

It took some time to find a Doom 3 project that had a "just works" Visual Studio project. Eventually I found [RBDOOM-3-BFG](https://github.com/RobertBeckebans/RBDOOM-3-BFG) which only took a little effort to get running.

All memory allocations go through `Mem_Alloc16` and `Mem_Free16` functions defined in [Heap.cpp](https://github.com/RobertBeckebans/RBDOOM-3-BFG/blob/master/neo/idlib/Heap.cpp). Modifying this was trivial. I started with the simplest possible thing and wrote every allocation to disk via `std::fwrite`. It runs a solid 60fps even in debug mode. Ship it!

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

All content for the rest of this post is derived from the same journal. Its a 420 megabyte file containing over 11,000,000 lines. Roughly 5.5 million `mallocs` and 5.5 million `frees`. It leaks 7 megabytes from 8473 `mallocs`, tsk tsk.

The journal covers 7 minutes of game time. I entered the main menu, selected a level, played, died, reloaded, died again, quit to main menu, and quit to desktop. I did this a few times and each run appeared to produce very similar journals. 

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
        auto allocTime = RdtscClock::now() - allocStart;
    } else {
        auto freeStart = RdtscClock::now();
        ::free(entry.ptr);
        auto freeTime = RdtscClock::now() - freeStart;
    }
}
```

This snippet excludes configuration and bookkeeping. I think you get the basic idea.

Running my journal through the new replay system produces the following output:

```
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
Median Allocation:    64 bytes
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
```

Interesting! Average `malloc` time is 57 nanoseconds. That's decent. However p99.9 (1 in 1000) is 2.67 microseconds. That's not great. Worst case is a whopping 230 microseconds, ouch! `free` is similar, but with a shockingly bad worst case performance of 1.55 milliseconds. Yikes! 

What does this mean for hitting a stable 60Hz or 144Hz? Honestly, I don't know. There's a ton of variance. Are the slow allocations because of very large allocations? Do slow allocs occur only during a loading screen or also in the middle of gameplay? Are extreme outliers due to OS context switches? We don't have enough information.

# Visualizing the Journal

We need to graph our raw data to make better sense of it.

First, I took `doom3_journal.txt`, ran a replay, and produced a new `doom3_replayreport.csv`. This replay report contains replay timestamps and replay profiler time for `malloc` and `free`. It looks like this:

```csv
replayAllocTimestamp,allocTime,allocSize,replayFreeTimestamp,freeTime
661500,375,2048,0,0
881700,531,1280,912000,254
911600,268,2560,935400,343
917500,261,3840,0,0
935800,153,1280,961800,101
961600,206,2560,962700,146
962000,248,3840,0,0
984500,447,3760,432968726000,397
993400,18980,3760,432968728400,758
1012500,1354,3760,432968729200,133
1013900,1365,3760,432968842900,161
```

To graph this data I used Python's [matplotlib](https://matplotlib.org/).

My first attempt was a heatmap. This did not go well and was not useful. I soon tried a scatterplot. This worked out great. Unfortunately, Python is a miserably slow language so rendering 11 million points took over 45 seconds. Yikes!

A kind Twitter user pointed me towards a matplotlib extension called [mpl-scatter-density](https://github.com/astrofrog/mpl-scatter-density). This worked phenomenally well and turned 45 seconds into 3 seconds. My biggest bottleneck is actually csv parsing. 

New tools in hand I produced this:

<insert screenshot of crt malloc>
<include button to toggle between malloc and free>

Data visualization is story telling. This story has a lot going on. Let's break it down.

Every single point represents a single call to `malloc`. There are 5.5 million points. The x-axis is replay time, just over 7 minutes. The y-axis is alloc time. Note the y-axis is in **logarithmic scale**. Finally, each pixel has a color which represents the size of that allocation. The color scale is also logarithmic.

There are some very expensive allocations at the very start when the game first boots. At ~30 seconds there are many allocations that are large and expensive as the level loads. There are similar allocations when I die and reload at 2 minutes.

During actual gameplay the vast majority of allocations take from 21 nanoseconds to 500 nanoseconds. Not amazing. Unfortunately there are more than a few allocs sprinkled between 1 and 20 microseconds. Even worse, those expensive allocs are as small as just 16 bytes!

Here's a zoom shot that covers the first gameplay segment.

<insert screenshot of crt malloc zoomed>

# Initial Takeaways

What can we take away from this? A few things.

1. C Runtime (CRT) `malloc` is all over the place
2. Majority of mallocs are under 60 nanoseconds (p90)
3. Almost all mallocs are under 20 microseconds (p99.99)

Is this good or bad? It *feels* bad. The inconsistency is frustrating. However it's important to note that my gameplay ran a rock solid 60fps!

Worst degenerate case is my computer catches fire. Thus far my computer has not caught fire. Nor has `malloc` frozen my computer for 10 seconds. In all of my testing I have not seen a single malloc take more than a few hundred microseconds.

I'm going to make a bold claim:

**Most code can safely call `malloc` and still hit their target framerate.**

I believe this is true even if your code is an audio plugin that needs to compute a buffer in under 4 milliseconds.

The problem isn't `malloc` per se. The problem is if you call `malloc` once then you probably call it more than once. It's death by a thousand cuts. It adds up fast and once you have 1,000 calls to `malloc` it's excruciatingly difficult to unwind them.

# Different allocators

Thus far all testing has been doing using standard C Runtime calls to `malloc` and `free`. CRT `malloc` is infamously slow. Therefore it makes sense to compare different allocators. Since we have a memory journal and replay system we can replay the exact sequence of allocations.

I chose three allocators to compare.

1. [jemalloc](http://jemalloc.net/) - an industrial strength allocator used by FreeBSD, Rust, and more.
2. [mimalloc](https://github.com/microsoft/mimalloc) - a general purpose allocator by Microsoft
3. [rpmalloc](https://github.com/mjansson/rpmalloc) - a lock free general purpose allocator by a now Epic Games employee




# Malloc vs Objected Oriented Bullshit
https://macton.smugmug.com/Other/2008-07-15-by-Eye-Fi/n-xmKDH/


# Revisiting Game Audio

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

## Embedded

If you read this post and want to reply "this is completely wrong because embedded" just stop and delete your snarky comment now. Yes, the rules are extremely different for embedded. Servers, desktops, consoles, smartphones, and embedded all have different requirements and characteristics.


# TODO
* Filter charts based on allocation size?
* Run benchmarks again with a written byte