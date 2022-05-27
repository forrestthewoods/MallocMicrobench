# Benchmarking Malloc with Doom 3
# How Slow is Malloc
# How Long Does it Take to Allocate Memory
# Profiling Malloc with Doom 3
# Benchmarking Malloc with Doom 3

This blog post begins the same way as many of my posts. I nerd sniped myself.

I recently read a blog post about a new library for game audio. It repeated a claim that is conventional wisdom in the audio programming world - "you can't allocate or deallocate memory on the audio thread". I think that statement is [wrong](https://xkcd.com/386/).

(maybe cut) My background is game programming. I'm all about pre-allocating everything and squeezing performance. I've shipped VR games that run a ruthless 90fps. I've fought wars with Unity's terrible garbage collector. This is my jam.

Lately I've been doing quite a bit of audio and audio-like programming lately. My code is reasonably efficient and does pre-allocate quite a bit. However it uses both `malloc` and `mutex`. It also happens to run great and glitch free!

Here's my question: what is the realistic worst-case performance for malloc on a modern machine?

<insert Tim Sweeney tweet>

# Framing the Question

Talking about and measuring performance is hard. What's even the right question here? How long does `malloc` take to call on average? Worst case? 99.9th percentile? 

In any case the boring answer is the same as always, it depends! How much memory are you allocating? How fragmented is your memory? What is your program's allocation pattern? What is your system doing? It always depends.

Allocating memory isn't free. But neither is time spent on complex systems full of error prone lockless programming.

For today I'm focused on games. Modern games run primarily between 60 and 144 frames per second. One frame at 144Hz is about 7 milliseconds. That's not a lot of time. Except I know for a fact that most games hit the allocator hard. Worst case in theory sets my computer on fire. But what is it in practice?

My goal isn't to come up with a singular, definitive answer. I'm a big fan of napkin math. There's a famous list of numbers called [Numbers Every Programmer Should Know](https://gist.github.com/jboner/2841832). I want similar numbers for `malloc`. I want a rough guidline of when `malloc` will cause me to miss my target framerate.

# Creating a Journal

My first attempt at a benchmark involved randomly allocating and freeing blocks of memory. Twitter friends correctly scolded me and said that's not good enough. I need real data with real allocation patterns and sizes.

The goal is to create a "journal" of memory operations. It should record `malloc` and `free` operations with their inputs and outputs. Then the journal can be replayed with different allocators to compare performance.

Unfortunately I don't have a suitable personal project for generating this journal. I did a quick search for open source games and the choice was obvious - Doom 3!

It took some time to find a Doom 3 project that had a "just works" Visual Studio project. Eventually I found [RBDOOM-3-BFG](https://github.com/RobertBeckebans/RBDOOM-3-BFG) which only took a little effort to get running.

All memory allocations go through simple `Mem_Alloc16` and `Mem_Free16` functions in [Heap.cpp](https://github.com/RobertBeckebans/RBDOOM-3-BFG/blob/master/neo/idlib/Heap.cpp). Modifying this was trivial. I did the simplest possible thing and write every allocation to disk. It runs a solid 60fps even in debug mode.

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

# Replaying the Journal

# Visualizing the Journal


# Latency vs Reliability is a Trade-Off

# Conclusion

# Extra Thoughts

## Quake 2
I actually tried Quake 2 before Doom 3. Unfortunately it's too efficient and has virtually no calls to `malloc` once a level has loaded!



# TODO
* Filter charts based on allocation size?
* Run benchmarks again with a written byte