
#include <algorithm>
#include <atomic>
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

// Memory allocator selection
// There are multiple project configurations. Each config should define exactly one target
// These macros help ensure exactly one config is explicitly specified
#ifndef USE_CRT
#define USE_CRT 0
#endif

#ifndef USE_JEMALLOC
#define USE_JEMALLOC 0
#endif 

#ifndef USE_MIMALLOC
#define USE_MIMALLOC 0
#endif 

#ifndef USE_RPMALLOC
#define USE_RPMALLOC 0
#endif 

// Allocator. Pick exactly one.
static_assert(USE_CRT + USE_JEMALLOC + USE_MIMALLOC + USE_RPMALLOC == 1, "Must pick exactly one allocator");

#if USE_CRT
#include <stdlib.h>
#elif USE_JEMALLOC
#include "thirdparty/jemalloc/include/jemalloc.h"
#elif USE_MIMALLOC
// Requires precompiled static lib
#include "thirdparty/mimalloc/mimalloc.h"
#elif USE_RPMALLOC
#include "thirdparty/rpmalloc/rpmalloc.h"

#endif

// If 0 then all mallocs/free on single thread
// If 1 then perform malloc/free on source defined thread
#define THREADED_REPLAY 1

// Config
constexpr double replaySpeed = 1.0;
constexpr const char* journalPath = "c:/temp/doom3_journal.txt";
constexpr const char* resultDir = "c:/temp/";

// Typedefs
using Nanoseconds = std::chrono::nanoseconds;
using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;

// Forward declarations
std::string formatTime(Nanoseconds ns);
std::string formatTime(long long ns);
std::string formatBytes(uint64_t bytes);

// Define allocator implementation
#if USE_CRT
struct Allocator {
    static inline void* alloc(size_t size) { return ::malloc(size); }
    static inline void free(void* ptr) { ::free(ptr); }
    static constexpr const char* name = "crt";
};
#elif USE_JEMALLOC
struct Allocator {
    static void* alloc(size_t size) { return je_malloc(size); }
    static void free(void* ptr) { je_free(ptr); }
    static constexpr const char* name = "jemalloc";
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

// Compute high-precision nanosecond locks via rdtsc
struct RdtscClock {
    static uint64_t now();
    static double nsPerTick();
    static Nanoseconds ticksToNs(uint64_t ticks);
};

enum MemoryOp {
    Alloc,
    Free
};

// jack-of-all-trades struct for memory journal
// contains both input and output because it makes things easy to track and report
struct MemoryEntry {
    // Input
    MemoryOp op = MemoryOp::Alloc;
    uint64_t allocSize = 0; // unused for free
    uint64_t originalPtr = 0;
    uint64_t threadId = 0;
    Nanoseconds originalTimestamp = Nanoseconds{ 0 };

    // Pre-process
    int64_t allocIdx = -1; // help MemoryOp::Free point back to source MemoryOp::Alloc
    
    // Replay Intermediate
    void* replayPtr = nullptr;

    // Output
    Nanoseconds replayAllocTimestamp = Nanoseconds{ 0 };
    Nanoseconds allocTime = Nanoseconds{ 0 };
    Nanoseconds replayFreeTimestamp = Nanoseconds{ 0 };
    Nanoseconds freeTime = Nanoseconds{ 0 };

    static std::vector<MemoryEntry> ParseJournal(const char* filepath);
};

// ----------------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------------
int main()
{
    std::cout << "Hello World!" << std::endl << std::endl;

    // Initialize allocator (if necessary
    std::cout << "Selected Allocator: " << Allocator::name << std::endl << std::endl;
#if USE_RPMALLOC
    std::cout << "Initializing rpmalloc" << std::endl << std::endl;
    rpmalloc_initialize();
#endif

    // Compute and print RDTSC information
    std::cout << "Nanoseconds per RDTSC tick: " << RdtscClock::nsPerTick() << std::endl << std::endl;
    


    // ----------------------------------------------------------------------------------
    // Step 1: Parse Journal
    // ----------------------------------------------------------------------------------
    std::cout << "Parsing log file: " << journalPath << std::endl;
    auto journal = MemoryEntry::ParseJournal(journalPath);
    std::cout << "Parse complete" << std::endl << std::endl;



    // ----------------------------------------------------------------------------------
    // Step 2: Pre-process Journal
    // ----------------------------------------------------------------------------------

    // Compute unique threadIds
    std::unordered_set<uint64_t> threadIds;
    for (auto const& entry : journal) {
        threadIds.insert(entry.threadId);
    }

    // Build pairs of alloc/free
    // Fix-up any alloc/frees whose timing was inconsistent
    {
        std::cout << "Pre-processing replay entries" << std::endl;

        // originalPtr -> sourceIdx
        int numFixups = 0;
        std::unordered_map<uint64_t, size_t> liveAllocs;
        size_t idx = 0;
        while (idx < journal.size()) {
            auto& entry = journal[idx];
            if (entry.op == MemoryOp::Alloc) {
                if (!liveAllocs.contains(entry.originalPtr)) {
                    liveAllocs[entry.originalPtr] = idx;
                }
                else {
                    // Uh oh. Replay Journal allocated on an address that already exists
                    // This _probably_ means the original timestamps came from different
                    // threads and didn't quite agree
                    auto iter = std::find_if(
                        journal.begin() + idx + 1,
                        journal.end(),
                        [&entry](auto const& e) { return e.op == MemoryOp::Free && e.originalPtr == entry.originalPtr; }
                    );

                    if (iter != journal.end()) {
                        // Swap elements
                        size_t idx2 = iter - journal.begin();
                        std::swap(journal[idx], journal[idx2]);

                        // Count number of fixups
                        numFixups += 1;

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
                    entry.allocIdx = iter->second;
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

                        // Count number of fixups
                        numFixups += 1;

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

        std::cout << "Num Fixups:   " << numFixups << std::endl;
        std::cout << "Num Leaks:    " << liveAllocs.size() << std::endl;
        std::cout << "Pre-process complete" << std::endl << std::endl;
    }



    // ----------------------------------------------------------------------------------
    // Step 3: Re-play Journal
    // ----------------------------------------------------------------------------------

    // Stats
    int64_t totalAllocs = 0;
    int64_t totalAllocBytes = 0;
    int64_t curLiveBytes = 0;
    int64_t maxLiveAllocBytes = 0;
    int64_t totalFrees = 0;

    // Re-play journal performing allocs and frees
    {
        std::cout << "Beginning replay" << std::endl;

        using ReplayClock = std::chrono::steady_clock;
        std::atomic<ReplayClock::time_point> replayStart;


        // Flags to track allocation
        std::unique_ptr<std::atomic_bool[]> allocatedFlags{ new std::atomic_bool[journal.size()] };
        for (auto i = 0; i < journal.size(); ++i) {
            allocatedFlags[i] = false;
        }

        const auto waitForTime = [&replayStart](MemoryEntry const& entry, ReplayClock::time_point replayStartTime) {
            // Wait to process this entry until the timestamp
            if (replaySpeed > 0.0) {
                for (;;) {
                    // Get "real" replayTime
                    auto replayTime = ReplayClock::now() - replayStartTime;

                    // Scale replayTime
                    auto scaledReplayTime = Nanoseconds{ (long long)((double)replayTime.count() * replaySpeed) };

                    // Spin until replay
                    if (scaledReplayTime > entry.originalTimestamp) {
                        break;
                    }
                }
            }
        };

        // Lambda that processes a single entry
        // Assumes entry is ready to be processed.
        const auto processOne = [&](MemoryEntry& entry, size_t entryIdx) {
            if (entry.op == MemoryOp::Alloc) {
                auto allocSize = entry.allocSize;

                // Perform and instrument malloc
                auto replayMallocStart = ReplayClock::now();
                auto mallocStart = RdtscClock::now();
                entry.replayPtr = Allocator::alloc(entry.allocSize);
                auto mallocEnd = RdtscClock::now();

                // Store malloc time
                Nanoseconds mallocTime = RdtscClock::ticksToNs(mallocEnd - mallocStart);
                entry.allocTime = mallocTime;
                entry.replayAllocTimestamp = replayMallocStart - replayStart.load();
                allocatedFlags[entryIdx] = true;

                // Update counters
                totalAllocs += 1;
                totalAllocBytes += allocSize;
                curLiveBytes += allocSize;
                maxLiveAllocBytes = std::max(maxLiveAllocBytes, curLiveBytes);
            }
            else {
                // Find alloc
                auto allocIdx = entry.allocIdx;
                auto& allocEntry = journal[allocIdx];
                if (allocEntry.op != MemoryOp::Alloc || allocEntry.originalPtr != entry.originalPtr) {
                    // Pre-processing failed
                    std::abort();
                }

                void* replayPtr = allocEntry.replayPtr;
                uint64_t allocSize = allocEntry.allocSize;

                // Perform and instrument free
                auto replayFreeStart = ReplayClock::now();
                auto freeStart = RdtscClock::now();
                Allocator::free(replayPtr);
                auto freeEnd = RdtscClock::now();

                // Compute free time
                Nanoseconds freeTime = RdtscClock::ticksToNs(freeEnd - freeStart);

                // Store some data back in the alloc entry for logging
                allocEntry.replayFreeTimestamp = replayFreeStart - replayStart.load();
                allocEntry.freeTime = freeTime;
                allocatedFlags[allocIdx] = false;

                // Update counters
                totalFrees += 1;
                curLiveBytes -= allocSize;
            }
        };

        auto replayDuration = std::chrono::nanoseconds{ journal.back().originalTimestamp };
        std::cout << "Replay Duration: " << formatTime(replayDuration) << std::endl;
        std::cout << "Replay Speed: " << replaySpeed << std::endl;

#if THREADED_REPLAY
        size_t numThreads = threadIds.size();

        std::atomic_bool start = false;
        std::vector<std::thread> threads;
        for (auto const& threadId : threadIds) {
            // Run replay for this thread
            auto threadReplayLambda = [
                &start, 
                &journal,
                threadId,
                &allocatedFlags,
                &replayStart,
                &processOne,
                &waitForTime]() 
            {
                // Spin until start
                while (!start) {
                }

                const auto replayStartCopy = replayStart.load();
                size_t threadAllocs = 0;
                size_t threadFrees = 0;

                // Process each journal entry
                for (size_t idx = 0; idx < journal.size(); ++idx) {
                    auto& entry = journal[idx];

                    // Ignore journal entries for other threads
                    if (entry.threadId != threadId) {
                        continue;
                    }

                    // Wait until it's time to process this entry
                    waitForTime(entry, replayStart.load());

                    // If entry is free, wait for alloc to have occurred
                    // Necessary when alloc/free on different threads
                    if (entry.op == MemoryOp::Free) {
                        size_t allocIdx = entry.allocIdx;
                        while (allocatedFlags[allocIdx] == false) {
                            // Spin until this alloc exists
                        }
                    }

                    // Process single entry
                    processOne(entry, idx);

                    if (entry.op == MemoryOp::Alloc) {
                        threadAllocs += 1;
                    }
                    else {
                        threadFrees += 1;
                    }
                }

                std::cout << "Thread " << threadId << " performed " << threadAllocs << " allocs and " << threadFrees << " frees" << std::endl;
            };

            // Create thread to run lambda
            threads.emplace_back(threadReplayLambda);
        }

        // Start the replay
        replayStart = ReplayClock::now();
        start = true;

        // Run all threads to completion
        for (auto& thread : threads) {
            thread.join();
        }

#else
        replayStart = ReplayClock::now();
        for (size_t idx = 0; idx < journal.size(); ++idx){
            auto& entry = journal[idx];

            // Wait until it's time to process this entry
            waitForTime(entry, replayStart.load());

            // Process single entry
            processOne(entry, idx);
        }
#endif

        std::cout << "Replay complete" << std::endl << std::endl;
    }



    // ----------------------------------------------------------------------------------
    // Step 4: Dump Results to File
    // ----------------------------------------------------------------------------------
    {
        std::string speedStr;
        if (replaySpeed <= 0) {
            speedStr = "MaxSpeed";
        }
        else if (replaySpeed == 1.0) {
            speedStr = "1x";
        }
        else if (replaySpeed == 10.0) {
            speedStr = "10x";
        }
        else {
            speedStr = std::format("{:2f}", replaySpeed);
        }

#if THREADED_REPLAY
        std::string threadStr = "MultiThread";
#else
        std::string threadStr = "SingleThread";
#endif

        std::string filepath = std::format("{}doom3_replayreport_{}_{}_{}.csv", 
            resultDir,
            Allocator::name,
            speedStr,
            threadStr);
        std::cout << "Writing alloc times to: " << filepath << std::endl;

        std::ofstream stream(filepath);
        if (!stream.is_open()) {
            std::cout << "Failed to open: " << filepath << std::endl;
        }

        stream << "replayAllocTimestamp,allocTime,allocSize,replayFreeTimestamp,freeTime\n";

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
                << entry.replayAllocTimestamp.count() << ","
                << entry.allocTime.count() << "," 
                << entry.allocSize << ","
                << entry.replayFreeTimestamp.count() << ","
                << entry.freeTime.count()
                << "\n";
        }
        std::cout << "Write complete" << std::endl;
    }



    // ----------------------------------------------------------------------------------
    // Step 5: Print Stats
    // ----------------------------------------------------------------------------------
    
    // Compute data for results
    std::vector<Nanoseconds> allocTimes;
    std::vector<Nanoseconds> freeTimes;
    for (auto const& entry : journal) {
        if (entry.op == MemoryOp::Alloc) {
            allocTimes.push_back(entry.allocTime);
            if (entry.freeTime.count() != 0) {
                freeTimes.push_back(entry.freeTime);
            }
        }
    }

    // Results
    std::sort(allocTimes.begin(), allocTimes.end());
    size_t mallocCount = allocTimes.size();

    std::sort(freeTimes.begin(), freeTimes.end());
    size_t freeCount = freeTimes.size();

    // Compute median alloc size
    std::vector<size_t> allocSizes;
    allocSizes.reserve(mallocCount);
    for (auto const& entry : journal) {
        if (entry.op == MemoryOp::Alloc) {
            allocSizes.push_back(entry.allocSize);
        }
    }
    auto medianIter = allocSizes.begin() + allocSizes.size() / 2;
    std::nth_element(allocSizes.begin(), medianIter, allocSizes.end());
    size_t medianAllocSize = *medianIter;

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
    std::cout << "Median Allocation:    " << formatBytes(medianAllocSize) << std::endl;
    std::cout << "Average Malloc Time:  " << formatTime(totalMallocTimeNs / mallocCount) << std::endl;
    std::cout << "Num Leaked Bytes:     " << formatBytes(curLiveBytes) << std::endl;
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

    // Clean up memory allocators
#if USE_RPMALLOC
    rpmalloc_finalize();
#endif
}

// ----------------------------------------------------------------------------------
// Utility implementations
// ----------------------------------------------------------------------------------
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

uint64_t RdtscClock::now() { 
    return __rdtsc(); 
}

double RdtscClock::nsPerTick() {
    // Compute conversion from rdtscTicks to nanoseconds
    // Computed by lambda that is evaluated exactly once due to static
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

Nanoseconds RdtscClock::ticksToNs(uint64_t ticks) {
    return Nanoseconds{ int64_t((double)ticks * nsPerTick()) };
}
std::vector<MemoryEntry> MemoryEntry::ParseJournal(const char* filepath) {
    std::vector<MemoryEntry> result;
    result.reserve(1000000);

    std::string line;
    std::ifstream file;
    file.open(filepath);

    if (!file.is_open()) {
        std::abort();
    }

    const std::string space_delimiter = " ";
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


// TODO
//   * Implement tracing to detect context switches 
//      * https://docs.microsoft.com/en-us/windows/win32/etw/configuring-and-starting-an-event-tracing-session
//      * https://docs.microsoft.com/en-us/windows/win32/etw/example-that-creates-a-session-and-enables-a-manifest-based-provider
//      * https://caseymuratori.com/blog_0025