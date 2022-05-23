#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>
#include <intrin.h>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>

#pragma intrinsic(__rdtsc)

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
    else {
        return std::format("{:.2f} milliseconds", (double)count / 1000 / 1000);
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
    MemoryOp op = MemoryOp::Alloc;
    uint64_t allocSize = 0; // unused for free
    uint64_t ptr = 0;
    uint64_t threadId;
    Nanoseconds timestamp;
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
        entry.ptr = strtoull(begin, &end, 16);
        advancePtrs();

        // Parse threadId
        entry.threadId = strtoull(begin, &end, 10);
        advancePtrs();

        // Parse timepoint
        entry.timestamp = Nanoseconds{ strtoull(begin, &end, 10) };

        result.push_back(entry);
    }

    return result;
}

int main()
{
    std::cout << "Hello World!" << std::endl << std::endl;

    auto journal = ParseMemoryLog("C:/temp/doom3_memory_startup.txt");

    std::cout << "Nanoseconds per RDTSC tick: " << RdtscClock::nsPerTick() << std::endl << std::endl;

    // Config
    constexpr const uint64_t kilobyte = 1024;
    constexpr const uint64_t megabyte = 1024 * 1024;
    constexpr const uint64_t gigabyte = 1024 * 1024 * 1024;

    constexpr const size_t minAllocSize = 64;
    constexpr const size_t maxAllocSize = 8192;
    constexpr const bool writeByte = false;
    constexpr const bool clearBytes = true;
    constexpr const bool freePtrRightAway = false;
    constexpr const bool sleepBeforeMalloc = false;
    constexpr const size_t mallocsBeforeSleep = 1;
    constexpr const bool yieldBeforeMalloc = false;
    constexpr const long long sleepMilliseconds = 1;
    constexpr const size_t numIterations = 4;

    constexpr const std::chrono::milliseconds maxAllocTime{ 3000 };

    constexpr const uint64_t maxAllocAmount = 2 * gigabyte;


    // RNG
    std::random_device randomDevice;
    std::default_random_engine randomEngine(randomDevice());
    std::uniform_int_distribution<size_t> rng(minAllocSize, maxAllocSize);
    auto getAllocSize = [&]() -> size_t {
        return rng(randomEngine);
    };

    for (size_t i = 0; i < numIterations; ++i) {
        std::cout << "Iteration: " << i << std::endl;

        // Result data
        std::vector<long long> mallocTimes;
        mallocTimes.reserve(4000000);
        std::vector<void*> pointers;
        pointers.reserve(4000000);
        uint64_t allocatedBytes = 0;

        // Loop
        auto loopStart = Clock::now();
        while (Clock::ticksToNs(Clock::now() - loopStart) < maxAllocTime && allocatedBytes < maxAllocAmount) {
            Nanoseconds mallocTime{ 0 };

            size_t allocSize = getAllocSize();

            // Sleep under the assumption that we probably won't get context switched after waking up
            if (sleepBeforeMalloc && mallocTimes.size() % mallocsBeforeSleep == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds{ sleepMilliseconds });
            }

            if (yieldBeforeMalloc) {
                std::this_thread::yield();
            }

            auto mallocStart = Clock::now();
            auto ptr = ::malloc(allocSize);
            if (writeByte) {
                auto bytePtr = reinterpret_cast<uint8_t*>(ptr);
                bytePtr[0] = 42;
            }
            if (clearBytes) {
                std::memset(ptr, 42, allocSize);
            }
            mallocTime += Clock::ticksToNs(Clock::now() - mallocStart);
            allocatedBytes += allocSize;

            if (freePtrRightAway) {
                ::free(ptr);
            }
            else {
                pointers.push_back(ptr);
            }

            mallocTimes.push_back(mallocTime.count());
        }

        // Results
        std::sort(mallocTimes.begin(), mallocTimes.end());
        
        // Debug worst times
        //auto reverseTimes = mallocTimes;
        //std::reverse(reverseTimes.begin(), reverseTimes.end());

        size_t mallocCount = mallocTimes.size();
        long long totalMallocTime = std::accumulate(mallocTimes.begin(), mallocTimes.end(), (long long)0);
        Nanoseconds totalMallocTimeNs{ totalMallocTime };

        std::cout << "Number of Mallocs:   " << mallocCount << std::endl;
        std::cout << "Total Allocation:    " << formatBytes(allocatedBytes) << std::endl;
        std::cout << "Average Allocation:  " << formatBytes(allocatedBytes / mallocCount) << std::endl;
        std::cout << "Average Malloc Time: " << formatTime(totalMallocTimeNs / mallocCount) << std::endl << std::endl;

        std::cout << "Best:    " << formatTime(mallocTimes[0]) << std::endl;
        std::cout << "p1:      " << formatTime(mallocTimes[size_t((float)mallocCount * 0.01f)]) << std::endl;
        std::cout << "p10:     " << formatTime(mallocTimes[size_t((float)mallocCount * 0.10f)]) << std::endl;
        std::cout << "p25:     " << formatTime(mallocTimes[size_t((float)mallocCount * 0.25f)]) << std::endl;
        std::cout << "p50:     " << formatTime(mallocTimes[size_t((float)mallocCount * 0.50f)]) << std::endl;
        std::cout << "p75:     " << formatTime(mallocTimes[size_t((float)mallocCount * 0.75f)]) << std::endl;
        std::cout << "p90:     " << formatTime(mallocTimes[size_t((float)mallocCount * 0.90f)]) << std::endl;
        std::cout << "p95:     " << formatTime(mallocTimes[size_t((float)mallocCount * 0.95f)]) << std::endl;
        std::cout << "p98:     " << formatTime(mallocTimes[size_t((float)mallocCount * 0.98f)]) << std::endl;
        std::cout << "p99:     " << formatTime(mallocTimes[size_t((float)mallocCount * 0.99f)]) << std::endl;
        std::cout << "p99.9:   " << formatTime(mallocTimes[size_t((float)mallocCount * 0.999f)]) << std::endl;
        std::cout << "p99.99:  " << formatTime(mallocTimes[size_t((float)mallocCount * 0.9999f)]) << std::endl;
        std::cout << "p99.999: " << formatTime(mallocTimes[size_t((float)mallocCount * 0.99999f)]) << std::endl;
        std::cout << "Worst:   " << formatTime(mallocTimes[mallocCount - 1]) << std::endl << std::endl;

        // Free any pointers
        for (auto* ptr : pointers) {
            ::free(ptr);
        }
        pointers.clear();
    }

    std::cout << "Goodbye Cruel World!\n";

}

// TODO
//   * Implement tracing to detect context switches 
//      * https://docs.microsoft.com/en-us/windows/win32/etw/configuring-and-starting-an-event-tracing-session
//      * https://docs.microsoft.com/en-us/windows/win32/etw/example-that-creates-a-session-and-enables-a-manifest-based-provider
//      * https://caseymuratori.com/blog_0025