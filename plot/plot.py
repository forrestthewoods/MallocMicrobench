import matplotlib.pyplot as plt
import matplotlib.colors as colors
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import LogFormatter
import math
import csv
import mpl_scatter_density

# Config
maxEntries = 0
csv_filename = "alloc_times_6min_realtimeReplay.csv"
chart_title_alloc = "Doom 3 Memory Analysis - ::malloc - 10x Speed"
chart_title_free = "Doom 3 Memory Analysis - ::free - 10x Speed"
fullscreen = False
show_alloc_graph = True
show_free_graph = True

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
    allocTimestamps = []
    allocTimes = []
    allocSizes = []
    freeData = []
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
            allocTimestamps.append(replayTimestamp)
            allocTimes.append(allocTime)
            allocSizes.append(allocSize)
            if replayFreeTimestamp != 0:
                freeData.append((replayFreeTimestamp, freeTime, allocSize))
            if maxEntries > 0 and len(allocTimestamps) >= maxEntries:
                break
    print("Parse Complete\n")

    # Shared plot data
    def x_labels(tick, pos):
        return f"{tick / 1e9}"

    def y_labels(tick, pos):
        return format_nanoseconds(tick)

    cbar_ticks = [32,64,128,256,512,kilobyte,kilobyte*10, kilobyte*100, megabyte, megabyte*10, mallocMax]

    # Alloc times
    if show_alloc_graph:
        fig = plt.figure(figsize=(16,9))
        ax = fig.add_subplot(1,1,1, projection='scatter_density')
        density = ax.scatter_density(
            x=allocTimestamps, 
            y=allocTimes,
            c=[min(alloc, mallocMax) for alloc in allocSizes], 
            cmap='nipy_spectral', 
            norm=colors.LogNorm(vmin=None,vmax=mallocMax))
        plt.semilogy(basey=10)
        ax.xaxis.set_major_formatter(FuncFormatter(x_labels))
        ax.yaxis.set_major_formatter(FuncFormatter(y_labels))
        ax.set_ylabel("Malloc Time")
        ax.set_ylim(top=1000*1000*1) # 1 millisecond
        ax.set_xlim(left=0)
        ax.set_xlabel("Replay Time (seconds)")
        ax.set_title(chart_title_alloc)
        ax.set_facecolor('#000000')
        fig.colorbar(density, ticks=cbar_ticks, format=ColorbarFormatter())
        
        if fullscreen:
            fig.canvas.manager.full_screen_toggle()  

    # Free times
    if show_free_graph:
        # Sort data by free time
        freeData.sort(key=lambda entry : entry[0])

        # Extract data
        freeTimestamps = [entry[0] for entry in freeData]
        freeTimes = [entry[1] for entry in freeData]
        allocSizes = [min(entry[2], mallocMax) for entry in freeData]

        fig = plt.figure(figsize=(16,9))
        ax = fig.add_subplot(1,1,1, projection='scatter_density')
        density = ax.scatter_density(
            x=freeTimestamps, 
            y=freeTimes,
            c=allocSizes, 
            cmap='nipy_spectral', 
            norm=colors.LogNorm(vmin=None,vmax=mallocMax))
        plt.semilogy(basey=10)
        ax.xaxis.set_major_formatter(FuncFormatter(x_labels))
        ax.yaxis.set_major_formatter(FuncFormatter(y_labels))
        ax.set_ylabel("Free Time")
        ax.set_ylim(top=1000*1000*1) # 1 milliseconds
        ax.set_xlabel("Replay Time (seconds)")
        ax.set_title(chart_title_free)
        ax.set_facecolor('#000000')
        fig.colorbar(density, ticks=cbar_ticks, format=ColorbarFormatter())
        
        if fullscreen:
            fig.canvas.manager.full_screen_toggle()  

    plt.show()

if __name__=="__main__":
    main()

