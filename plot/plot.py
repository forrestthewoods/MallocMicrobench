import matplotlib.pyplot as plt
import matplotlib.colors as colors
from matplotlib.ticker import LogFormatter
import math
import csv

# Config
maxEntries = 100000
csv_filename = "alloc_times_6min_realtimeReplay.csv"
chart_title = "Doom 3 Memory Analysis - ::malloc - Realtime Replay"

# Constants
kilobyte = 1024.0
megabyte = kilobyte * 1024.0
gigabyte = megabyte * 1024.0

# Utilities
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
    elif bytes == 1024:
        return "1 kilobyte"
    elif bytes < 1024 * 1024:
        return f"{bytes/1024:.0f} kilobytes"
    elif bytes == 1024 * 1024:
        return "1 megabyte" 
    elif bytes < 1024 * 1024 * 1024:
        return f"{bytes/1024/1024} megabytes"
    else:
        return f"{1024.0/1024.0/1024.0} gigabytes"

class ColorbarFormatter(LogFormatter):
    def __call__(self, x, pos = None):
        return format_bytes(x)

def main():
    # Parse data
    mallocMax = 100 * megabyte
    mallocMaxLog = math.log(mallocMax)

    print("Parsing data")
    timestamps = []
    allocTimes = []
    allocSizes = []
    csvData = []
    with open(csv_filename) as csv_file:
        reader = csv.reader(csv_file)
        
        # skip header
        next(reader, None)

        # process data
        for row in reader:
            originalTimestamp = float(row[0])
            replayTimestamp = float(row[1])
            allocTime = float(row[2])
            allocSize = float(row[3])
            replayFreeTimestamp = float(row[4])
            freeTime = float(row[5])
            timestamps.append(replayTimestamp)
            allocTimes.append(allocTime)
            allocSizes.append(allocSize)
            if maxEntries > 0 and len(timestamps) >= maxEntries:
                break
    print("Parse Complete\n")

    # Need to normalize allocSizes to [0,1] for color
    def clamp(v,lo,hi):
        return max(lo, min(v, hi))

    # Normalize allocSizes to [0,1]
    # Used fix upper limited that clamped
    def normalize_z(v):
        clamp(math.log(min(v, mallocMax))/mallocMaxLog, 0.0, 1.0)

    normalizedAllocSizes = [normalize_z(alloc) for alloc in allocSizes]
    clampedAllocSizes = [min(alloc, mallocMax) for alloc in allocSizes]

    # Scatter plot
    if True:
        from matplotlib.ticker import FuncFormatter

        def x_labels(tick, pos):
            return f"{tick / 1e9}"

        def y_labels(tick, pos):
            return format_nanoseconds(tick)

        def z_labels(tick, pos):
            return format_bytes(tick)

        cbar_ticks = [32,64,128,256,512,kilobyte,kilobyte*10, kilobyte*100, megabyte, megabyte*10, mallocMax]


        fig,ax = plt.subplots(1,1, figsize=(16,9))
        plt.scatter(
            x=timestamps, 
            y=allocTimes,
            c=clampedAllocSizes, 
            s=0.2, 
            cmap='nipy_spectral', 
            norm=colors.LogNorm(vmin=None,vmax=mallocMax))
        plt.semilogy(basey=10)
        ax.xaxis.set_major_formatter(FuncFormatter(x_labels))
        ax.yaxis.set_major_formatter(FuncFormatter(y_labels))
        ax.set_ylabel("Malloc Time")
        ax.set_xlabel("Replay Time (seconds)")
        ax.set_title(chart_title)
        ax.set_facecolor('#101010')
        cb = plt.colorbar(ticks=cbar_ticks, format=ColorbarFormatter())
        fig.canvas.manager.full_screen_toggle() # toggle fullscreen mode        

        plt.show()

if __name__=="__main__":
    main()

