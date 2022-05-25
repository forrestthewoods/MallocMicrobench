import matplotlib.pyplot as plt
import math
import csv

kilobyte = 1024.0
megabyte = kilobyte * 1024.0
gigabyte = megabyte * 1024.0

def format_nanoseconds(ns):
    if ns < 1000:
        return f"{ns} ns"
    elif ns < 1000 * 1000:
        return f"{ns/1000} μs"
    elif ns < 1000 * 1000 * 1000:
        return f"{ns/1000/1000} ms"
    else:
        return f"{ns/1000/1000/1000} s"

def format_bytes(bytes):
    if bytes < 1024:
        return f"{bytes:.0f} bytes"
    elif bytes < 1024 * 1024:
        return f"{bytes/1024:.0f} kilobytes"
    elif bytes < 1024 * 1024 * 1024:
        return f"{bytes/1024/1024:.2f} megabytes"
    else:
        return f"{float(bytes)/1024.0/1024.0/1024.0:.2f} gigabytes"

def main():
    # Parse data
    maxEntries = 100000
    chart_title = "Doom 3 Memory Analysis - Malloc"
    mallocMax = 50 * megabyte
    mallocMaxLog = math.log(mallocMax)

    print("Parsing data")
    timestamps = []
    allocTimes = []
    allocSizes = []
    with open("alloc_times.csv") as csv_file:
        reader = csv.reader(csv_file)
        
        # skip header
        next(reader, None)

        # process data
        for row in reader:
            timestamp = float(row[0])
            allocTime = float(row[1])
            allocSize = float(row[2])
            timestamps.append(timestamp)
            allocTimes.append(allocTime)
            allocSizes.append(allocSize)
            if maxEntries > 0 and len(timestamps) >= maxEntries:
                break
    print("Parse Complete\n")

    # Need to normalize allocSizes to [0,1] for color
    def clamp(v,lo,hi):
        return max(lo, min(v, hi))

    #maxAllocSize = max(allocSizes)
    #maxLogAllocSize = math.log(maxAllocSize)
    #print(f"Max alloc size: {max(allocSizes)}")
    #print(f"Max alloc size: {maxLogAllocSize}")
    #normalizedAllocSizes = [clamp(math.log(min(alloc, maxAllocSize))/maxLogAllocSize,0.0,1.0) for alloc in allocSizes]

    # Normalize allocSizes to [0,1]
    # Used fix upper limited that clamped
    normalizedAllocSizes = [clamp(math.log(min(alloc, mallocMax))/mallocMaxLog,0.0,1.0) for alloc in allocSizes]

    # Scatter plot
    if True:
        from matplotlib.ticker import FuncFormatter

        def x_labels(tick, pos):
            return f"{tick / 1e9}"

        def y_labels(tick, pos):
            return format_nanoseconds(tick)


        fig,ax = plt.subplots(1,1, figsize=(16,9))
        plt.scatter(x=timestamps, y=allocTimes, c=normalizedAllocSizes, s=0.1, cmap='jet')
        plt.semilogy(basey=10)
        ax.xaxis.set_major_formatter(FuncFormatter(x_labels))
        ax.yaxis.set_major_formatter(FuncFormatter(y_labels))
        ax.set_ylabel("Malloc Time")
        ax.set_xlabel("Game time (seconds)")
        ax.set_title(chart_title)

        # Define a color bar to provide a third dimension (alloc size)
        num_cbar_ticks = int(10)

        print(f"Debug 0: {format_bytes(mallocMax)}")
        
        debug = [mallocMax for i in range(num_cbar_ticks)]
        print(f"Debug 1: {debug}")

        debug = [i for i in range(num_cbar_ticks)]
        print(f"Debug 2: {debug}")

        debug = [0 if i == 0 else i/num_cbar_ticks for i in range(num_cbar_ticks + 1)]
        print(f"Debug 3: {debug}")

        debug = [0 if i == 0 else i/num_cbar_ticks*mallocMax for i in range(num_cbar_ticks + 1)]
        print(f"Debug 5: {debug}")

        debug = [0 if i == 0 else math.log(i/num_cbar_ticks*mallocMax) for i in range(num_cbar_ticks + 1)]
        print(f"Debug 6: {debug}")
        
        debug = [format_bytes(b) for b in debug]
        print(f"Debug 7: {debug}")

        cbar_labels = ["0" if i == 0 else format_bytes(math.log(i*mallocMax)) for i in range(num_cbar_ticks)]
        print(f"Cbar Labels: {cbar_labels}")
        #cbar_labels = [str(i) for i in range(num_cbar_ticks)]
        cbar = plt.colorbar()
        #cbar = plt.colorbar(ticks=[i for i in range(num_cbar_ticks)])
        #cbar.ax.set_yticklabels(cbar_labels)

        plt.show()

if __name__=="__main__":
    main()

