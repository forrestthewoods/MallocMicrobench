
#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>
#include <future>
#include <intrin.h>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#pragma intrinsic(__rdtsc)

// Allocator. Pick one
#define USE_CRT 0
#define USE_MIMALLOC 1
#define USE_RPMALLOC 0
static_assert(USE_CRT + USE_MIMALLOC + USE_RPMALLOC == 1, "Must pick exactly one allocator");

#define THREADED_REPLAY 0

#if USE_CRT
#include <stdlib.h>
constexpr const char* alloc_name = "crt";
#elif USE_MIMALLOC
#include "thirdparty/mimalloc/mimalloc.h"
constexpr const char* alloc_name = "mimalloc";
#elif USE_RPMALLOC
#include "thirdparty/rpmalloc/rpmalloc.h"
constexpr const char* alloc_name = "rpmalloc";
#endif


// Config
constexpr double replaySpeed = 10.0;
constexpr const char* logpath = "C:/temp/doom3_memory_foo_level_died.txt";

// Forward declarations
struct RdtscClock;

// Typedefs
//using Clock = std::chrono::steady_clock;
using Clock = RdtscClock;
using Nanoseconds = std::chrono::nanoseconds;
using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;

std::string formatTime(Nanoseconds ns) {
    auto count = ns.count();
    if (count < 1000) {
        return std::format("{} nanoseconds", count);
    }
    else if (count < 1000 * 1000) {
        return std::format("{:.2f} microseconds", (double)count / 1000);
    }
    else if (count < 1000 * 1000 * 1000) {
        return std::format("{:.2f} milliseconds", (double)count / 1000 / 1000);
    }
    else {
        return std::format("{:.2f} seconds", (double)count / 1000 / 1000 / 1000);
    }
}

std::string formatTime(long long ns) {
    return formatTime(Nanoseconds{ ns });
}

std::string formatBytes(uint64_t bytes) {
    if (bytes < 1024) {
        return std::format("{} bytes", bytes);
    } 
    else if (bytes < 1024 * 1024) {
        return std::format("{} kilobytes", bytes / 1024);
    }
    else  if (bytes < 1024 * 1024 * 1024) {
        return std::format("{} megabytes", bytes / 1024 / 1024);
    }
    else {
        return std::format("{:.2f} gigabytes", (double)bytes / 1024 / 1024 / 1024);
    }
}


#if USE_CRT
struct Allocator {
    static inline void* alloc(size_t size) { return ::malloc(size); }
    static inline void free(void* ptr) { ::free(ptr); }
    static constexpr const char* name = "crt";
};
#elif USE_MIMALLOC
struct Allocator {
    static void* alloc(size_t size) { return ::mi_malloc(size); }
    static void free(void* ptr) { ::mi_free(ptr); }
    static constexpr const char* name = "mimalloc";
};
#elif USE_RPMALLOC
struct Allocator {
    static void* alloc(size_t size) { return ::rpmalloc(size); }
    static void free(void* ptr) { ::rpfree(ptr); }
    static constexpr const char* name = "rpmalloc";
};
#else
#error Could not pick allocator
#endif

struct RdtscClock {
    static uint64_t now() { return __rdtsc(); }

    static double nsPerTick() {
        static double nsPerTick = []() -> double {
            constexpr int intervalMs = 5;
            auto intervalNs = std::chrono::duration_cast<Nanoseconds>(Milliseconds{ intervalMs });
            constexpr size_t numIntervals = 20;

            // Run some samples
            std::vector<uint64_t> ticks;
            for (size_t i = 0; i < numIntervals; ++i) {
                auto chronoStart = std::chrono::high_resolution_clock::now();
                auto rdtscStart = RdtscClock::now();
                decltype(chronoStart) chronoEnd;
                do {
                    chronoEnd = std::chrono::high_resolution_clock::now();
                } while ((chronoEnd - chronoStart) < intervalNs);
                auto rdtscEnd = RdtscClock::now();
                ticks.push_back(rdtscEnd - rdtscStart);
            }

            // Sort results 
            std::sort(ticks.begin(), ticks.end(), std::greater<uint64_t>());

            // Remove slowest (fewest ticks)
            ticks.pop_back();

            uint64_t sum = std::accumulate(ticks.begin(), ticks.end(), uint64_t{ 0 });
            double avgTicksPerInterval = (double)sum / (double)ticks.size();
            double avgNsPerTick = (double)intervalNs.count() / avgTicksPerInterval;

            // Debug spew
            //std::cout << "Fastest nsPerTick: " << (double)intervalNs.count() / (double)ticks.front() << std::endl;
            //std::cout << "Median nsPerTick: " << (double)intervalNs.count() / (double)ticks[ticks.size() / 2] << std::endl;
            //std::cout << "Slowest nsPerTick: " << (double)intervalNs.count() / (double)ticks.back() << std::endl;

            return avgNsPerTick;
        }();

        return nsPerTick;
    }

    static Nanoseconds ticksToNs(uint64_t ticks) {
        return Nanoseconds{ int64_t((double)ticks * nsPerTick()) };
    }
};

enum MemoryOp {
    Alloc,
    Free
};

struct MemoryEntry {
    // Input
    MemoryOp op = MemoryOp::Alloc;
    uint64_t allocSize = 0; // unused for free
    uint64_t originalPtr = 0;
    uint64_t threadId = 0;
    Nanoseconds originalTimestamp = Nanoseconds{ 0 };

    // Intermediate
    int64_t allocIdx = -1; // allocIndex for MemoryOp::Free
    void* replayPtr = 0;

    // Output
    Nanoseconds replayAllocTimestamp = Nanoseconds{ 0 };
    Nanoseconds allocTime = Nanoseconds{ 0 };
    Nanoseconds replayFreeTimestamp = Nanoseconds{ 0 };
    Nanoseconds freeTime = Nanoseconds{ 0 };
};

std::vector<MemoryEntry> ParseMemoryLog(const char* filepath) {
    std::vector<MemoryEntry> result;
    result.reserve(1000000);

    std::string line;
    std::ifstream file;
    file.open(filepath);

    if (!file.is_open()) {
        std::abort();
    }

    const std::string space_delimiter = " ";

    /*
        a 2048 000001BF2B426880 11484 6949200
        a 1280 000001BF2B429170 11484 7407200
        a 2560 000001BF2B4296D0 11484 7441000
        f 000001BF2B429170 11484 7458000
    */

    while (std::getline(file, line)) {
        MemoryEntry entry;
        
        // Parse op type
        entry.op = line[0] == 'a' ? MemoryOp::Alloc : MemoryOp::Free;

        // Pointer work because C++ can't split a string by whitespace
        char* begin = line.data() + 2; // skip first char and first space
        char* const lineEnd = line.data() + line.size();
        char* end = std::find(begin, lineEnd, ' ');
        auto advancePtrs = [&begin, &end, lineEnd]() {
            begin = end + 1;
            end = std::find(begin, lineEnd, ' ');
        };

        // (Optional) Parse size
        uint64_t allocSize = 0;
        if (entry.op == MemoryOp::Alloc) {
            entry.allocSize = strtoull(begin, &end, 10);
            advancePtrs();
        }

        // Parse ptr
        //begin = std::find_if(begin, end, [](char c) { return c != '0'; });
        entry.originalPtr = strtoull(begin, &end, 16);
        advancePtrs();

        // Parse threadId
        entry.threadId = strtoull(begin, &end, 10);
        advancePtrs();

        // Parse timepoint
        entry.originalTimestamp = Nanoseconds{ strtoull(begin, &end, 10) };

        result.push_back(entry);
    }

    return result;
}

int main()
{
    std::cout << "Hello World!" << std::endl << std::endl;
    std::cout << "Selected Allocator: " << Allocator::name << std::endl << std::endl;

#if USE_RPMALLOC
    std::cout << "Initializing rpmalloc" << std::endl << std::endl;
    rpmalloc_initialize();
#endif

    std::cout << "Nanoseconds per RDTSC tick: " << RdtscClock::nsPerTick() << std::endl << std::endl;
    
    // Parse log
    std::cout << "Parsing log file: " << logpath << std::endl;
    auto journal = ParseMemoryLog(logpath);
    std::cout << "Parse complete" << std::endl << std::endl;

    // Compute unique threadIds
    std::unordered_map<uint64_t, size_t> threadOps;
    for (auto const& entry : journal) {
        threadOps[entry.threadId] += 1;
    }

    // Build pairs of alloc/free
    // Fix-up any alloc/frees whose timing was inconsistent
    {
        // originalPtr -> sourceIDX
        std::unordered_map<uint64_t, size_t> liveAllocs;
        size_t idx = 0;
        while (idx < journal.size()) {
            auto& entry = journal[idx];
            if (entry.op == MemoryOp::Alloc) {
                if (!liveAllocs.contains(entry.originalPtr)) {
                    liveAllocs[entry.originalPtr] = idx;
                }
                else {
                    auto iter = std::find_if(
                        journal.begin() + idx + 1,
                        journal.end(),
                        [&entry](auto const& e) { return e.op == MemoryOp::Free && e.originalPtr == entry.originalPtr; }
                    );

                    if (iter != journal.end()) {
                        // Swap elements
                        size_t idx2 = iter - journal.begin();
                        std::swap(journal[idx], journal[idx2]);

                        // Continue without increment idx to process the free
                        continue;
                    }
                    else {
                        // Bad replay data? Re-used address?
                        std::abort();
                    }
                }
            }
            else if (entry.op == MemoryOp::Free) {
                auto iter = liveAllocs.find(entry.originalPtr);
                if (iter != liveAllocs.end()) {
                    entry.allocIdx = idx;
                    liveAllocs.erase(iter);
                }
                else {
                    auto iter = std::find_if(
                        journal.begin() + idx + 1,
                        journal.end(),
                        [&entry](auto const& e) { return e.op == MemoryOp::Alloc && e.originalPtr == entry.originalPtr; }
                    );

                    if (iter != journal.end()) {
                        // Swap elements
                        size_t idx2 = iter - journal.begin();
                        std::swap(journal[idx], journal[idx2]);

                        // Continue without increment idx to process the free
                        continue;
                    }
                    else {
                        // Bad replay data? Re-used address?
                        std::abort();
                    }
                }
            }

            idx += 1;
        }
    }

    // Malloc and Free as fast as possible
    std::unordered_map<uint64_t, std::tuple<void*, uint64_t, size_t>> liveAllocs;
    std::vector<Nanoseconds> allocTimes;
    std::vector<Nanoseconds> freeTimes;
    std::vector<std::pair<MemoryEntry, size_t>> failedAllocs;
    std::vector<std::pair<MemoryEntry, size_t>> failedFrees;

    int64_t totalAllocs = 0;
    int64_t totalAllocBytes = 0;
    int64_t curLiveBytes = 0;
    int64_t maxLiveAllocBytes = 0;
    int64_t totalFrees = 0;

    using ReplayClock = std::chrono::steady_clock;
    auto replayStart = ReplayClock::now();

    auto replayDuration = std::chrono::nanoseconds{ journal.back().originalTimestamp };
    std::cout << "Replay Duration: " << formatTime(replayDuration) << std::endl;
    std::cout << "Replay Speed: " << replaySpeed << std::endl;
    std::cout << std::endl;

    // Re-play journal performing allocs and frees
    {
        std::cout << "Beginning replay" << std::endl;

#if THREADED_REPLAY
        size_t numThreads = threadOps.size();
#else

        float nextReplayMarker = 0.1f;
        size_t idx = 0;
        while (idx < journal.size()) {
            auto& entry = journal[idx];

            // Playback replay
            if (replaySpeed > 0.0) {
                for (;;) {
                    // Get "real" replayTime
                    auto replayTime = ReplayClock::now() - replayStart;

                    // Scale replayTime
                    auto scaledReplayTime = Nanoseconds{ long long((double)replayTime.count() * replaySpeed) };

                    double replayFrac = (double)scaledReplayTime.count() / (double)replayDuration.count();
                    if (replayFrac > nextReplayMarker) {
                        std::cout << "Replay Progress: " << std::format("{}%  RealTime: {}", (int)(nextReplayMarker * 100.0), formatTime(replayTime)) << std::endl;
                        nextReplayMarker += 0.1f;
                    }

                    // Spin until replay
                    if (scaledReplayTime > entry.originalTimestamp) {
                        break;
                    }
                }
            }

            if (entry.op == MemoryOp::Alloc) {
                auto allocSize = entry.allocSize;

                // Perform and instrument malloc
                auto replayMallocStart = ReplayClock::now();
                auto mallocStart = Clock::now();
                auto replayPtr = Allocator::alloc(entry.allocSize);
                auto mallocEnd = Clock::now();

                // Store pointer in live list
                if (!liveAllocs.contains(entry.originalPtr)) {
                    liveAllocs[entry.originalPtr] = std::tuple(replayPtr, allocSize, idx);

                    // Store malloc time
                    Nanoseconds mallocTime = RdtscClock::ticksToNs(mallocEnd - mallocStart);
                    allocTimes.push_back(mallocTime);
                    entry.allocTime = mallocTime;
                    entry.replayAllocTimestamp = replayMallocStart - replayStart;

                    // Update counters
                    totalAllocs += 1;
                    totalAllocBytes += allocSize;
                    curLiveBytes += allocSize;
                    maxLiveAllocBytes = std::max(maxLiveAllocBytes, curLiveBytes);
                }
                else {
                    // source multithreaded alloc/free can sometimes have inconsistent timestamps
                    // we can ignore a few without losing critical information.

                    /*
                    std::cout << "Failed alloc. Idx: [" << idx << "] Size: [" << entry.allocSize << "]  Ptr: [" << entry.ptr << "] Thread: [" << entry.threadId << "]  time: [" << entry.timestamp << "]" << std::endl;
                    size_t existingIdx = std::get<2>(liveAllocs[entry.ptr]);
                    auto const& existingEntry = journal[existingIdx];
                    std::cout << "  Existing: Idx: [" << existingIdx << "] Size: [" << existingEntry.allocSize << "]  Ptr: [" << existingEntry.ptr << "] Thread: [" << existingEntry.threadId << "]  time: [" << existingEntry.timestamp << "]" << std::endl;
                    */

                    failedAllocs.emplace_back(entry, idx);
                }
            }
            else {
                // Find ptr
                auto iter = liveAllocs.find(entry.originalPtr);
                if (iter != liveAllocs.end()) {
                    auto liveAllocTuple = iter->second;
                    void* replayPtr = std::get<0>(liveAllocTuple);
                    uint64_t allocSize = std::get<1>(liveAllocTuple);
                    size_t allocIdx = std::get<2>(liveAllocTuple);

                    // Perform and instrument free
                    auto replayFreeStart = ReplayClock::now();
                    auto freeStart = Clock::now();
                    Allocator::free(replayPtr);
                    auto freeEnd = Clock::now();

                    // Remove pointer from live list
                    liveAllocs.erase(iter);

                    // Store free time
                    Nanoseconds freeTime = RdtscClock::ticksToNs(freeEnd - freeStart);
                    freeTimes.push_back(freeTime);

                    // Store some data back in the alloc entry for logging
                    auto& allocEntry = journal[allocIdx];
                    allocEntry.replayFreeTimestamp = replayFreeStart - replayStart;
                    allocEntry.freeTime = freeTime;

                    // Update counters
                    totalFrees += 1;
                    curLiveBytes -= allocSize;
                }
                else {
                    //std::cout << "Failed free. Idx: [" << idx << "] Size: [" << entry.allocSize << "]  Ptr: [" << entry.ptr << "] Thread: [" << entry.threadId << "]  time: [" << entry.timestamp << "]" << std::endl;
                    failedFrees.emplace_back(entry, idx);
                }
            }

            idx += 1;
        }
#endif

        std::cout << "Replay complete" << std::endl;
        std::cout << std::endl;
    }

    // Dump results to file
    {
        constexpr const char* filepath = "c:/temp/alloc_times.csv";
        std::cout << "Writing alloc times to: " << filepath << std::endl;

        std::ofstream stream(filepath);
        if (!stream.is_open()) {
            std::cout << "Failed to open: " << filepath << std::endl;
        }

        stream << "originalTimestamp,replayAllocTimestamp,allocTime,allocSize,replayFreeTimestamp,freeTime\n";

        for (auto const& entry : journal) {
            // Only consider allocs
            if (entry.op != MemoryOp::Alloc) {
                continue;
            }

            // Skip allocs that failed
            if (entry.allocTime.count() == 0) {
                continue;
            }

            // Write data
            stream 
                << entry.originalTimestamp.count() << ","
                << entry.replayAllocTimestamp.count() << ","
                << entry.allocTime.count() << "," 
                << entry.allocSize << ","
                << entry.replayFreeTimestamp.count() << ","
                << entry.freeTime.count()
                << "\n";
        }
        std::cout << "Write complete" << std::endl;
    }

    // Compute data for results

    // Results
    std::sort(allocTimes.begin(), allocTimes.end());
    size_t mallocCount = allocTimes.size();

    std::sort(freeTimes.begin(), freeTimes.end());
    size_t freeCount = freeTimes.size();

    Nanoseconds totalMallocTimeNs{ 0 };
    for (auto const& allocTime : allocTimes) {
        totalMallocTimeNs += allocTime;
    }

    std::cout << "== Replay Results ==" << std::endl;
    std::cout << "Number of Mallocs:    " << mallocCount << std::endl;
    std::cout << "Number of Frees:      " << freeCount << std::endl;
    std::cout << "Total Allocation:     " << formatBytes(totalAllocBytes) << std::endl;
    std::cout << "Max Live Bytes:       " << formatBytes(maxLiveAllocBytes) << std::endl;
    std::cout << "Average Allocation:   " << formatBytes(totalAllocBytes / mallocCount) << std::endl;
    std::cout << "Average Malloc Time:  " << formatTime(totalMallocTimeNs / mallocCount) << std::endl;
    std::cout << "Num Leaked Allocs:    " << liveAllocs.size() << std::endl;
    std::cout << "Num Leaked Bytes:     " << formatBytes(curLiveBytes) << std::endl;
    std::cout << "Num Failed Allocs:    " << failedAllocs.size() << std::endl;
    std::cout << "Num Failed Frees:     " << failedFrees.size() << std::endl;
    std::cout << std::endl;

    std::cout << "Alloc Time" << std::endl;
    std::cout << "Best:    " << formatTime(allocTimes[0]) << std::endl;
    std::cout << "p1:      " << formatTime(allocTimes[size_t((float)mallocCount * 0.01f)]) << std::endl;
    std::cout << "p10:     " << formatTime(allocTimes[size_t((float)mallocCount * 0.10f)]) << std::endl;
    std::cout << "p25:     " << formatTime(allocTimes[size_t((float)mallocCount * 0.25f)]) << std::endl;
    std::cout << "p50:     " << formatTime(allocTimes[size_t((float)mallocCount * 0.50f)]) << std::endl;
    std::cout << "p75:     " << formatTime(allocTimes[size_t((float)mallocCount * 0.75f)]) << std::endl;
    std::cout << "p90:     " << formatTime(allocTimes[size_t((float)mallocCount * 0.90f)]) << std::endl;
    std::cout << "p95:     " << formatTime(allocTimes[size_t((float)mallocCount * 0.95f)]) << std::endl;
    std::cout << "p98:     " << formatTime(allocTimes[size_t((float)mallocCount * 0.98f)]) << std::endl;
    std::cout << "p99:     " << formatTime(allocTimes[size_t((float)mallocCount * 0.99f)]) << std::endl;
    std::cout << "p99.9:   " << formatTime(allocTimes[size_t((float)mallocCount * 0.999f)]) << std::endl;
    std::cout << "p99.99:  " << formatTime(allocTimes[size_t((float)mallocCount * 0.9999f)]) << std::endl;
    std::cout << "p99.999: " << formatTime(allocTimes[size_t((float)mallocCount * 0.99999f)]) << std::endl;
    std::cout << "Worst:   " << formatTime(allocTimes[mallocCount - 1]) << std::endl << std::endl;

    std::cout << "Free Time" << std::endl;
    std::cout << "Best:    " << formatTime(freeTimes[0]) << std::endl;
    std::cout << "p1:      " << formatTime(freeTimes[size_t((float)freeCount * 0.01f)]) << std::endl;
    std::cout << "p10:     " << formatTime(freeTimes[size_t((float)freeCount * 0.10f)]) << std::endl;
    std::cout << "p25:     " << formatTime(freeTimes[size_t((float)freeCount * 0.25f)]) << std::endl;
    std::cout << "p50:     " << formatTime(freeTimes[size_t((float)freeCount * 0.50f)]) << std::endl;
    std::cout << "p75:     " << formatTime(freeTimes[size_t((float)freeCount * 0.75f)]) << std::endl;
    std::cout << "p90:     " << formatTime(freeTimes[size_t((float)freeCount * 0.90f)]) << std::endl;
    std::cout << "p95:     " << formatTime(freeTimes[size_t((float)freeCount * 0.95f)]) << std::endl;
    std::cout << "p98:     " << formatTime(freeTimes[size_t((float)freeCount * 0.98f)]) << std::endl;
    std::cout << "p99:     " << formatTime(freeTimes[size_t((float)freeCount * 0.99f)]) << std::endl;
    std::cout << "p99.9:   " << formatTime(freeTimes[size_t((float)freeCount * 0.999f)]) << std::endl;
    std::cout << "p99.99:  " << formatTime(freeTimes[size_t((float)freeCount * 0.9999f)]) << std::endl;
    std::cout << "p99.999: " << formatTime(freeTimes[size_t((float)freeCount * 0.99999f)]) << std::endl;
    std::cout << "Worst:   " << formatTime(freeTimes[freeCount - 1]) << std::endl << std::endl;

    std::cout << "Goodbye Cruel World!\n";

#if USE_RPMALLOC
    rpmalloc_finalize();
#endif
}

// TODO
//   * Implement tracing to detect context switches 
//      * https://docs.microsoft.com/en-us/windows/win32/etw/configuring-and-starting-an-event-tracing-session
//      * https://docs.microsoft.com/en-us/windows/win32/etw/example-that-creates-a-session-and-enables-a-manifest-based-provider
//      * https://caseymuratori.com/blog_0025