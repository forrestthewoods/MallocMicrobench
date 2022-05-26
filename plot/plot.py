import matplotlib.pyplot as plt
import matplotlib.colors as colors
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import LogFormatter
import math
import csv
import mpl_scatter_density
import cmasher as cmr

# Config
maxEntries = 0
fullscreen = True
prepare_alloc_graph = True
prepare_free_graph = True
show_plot = False
save_pngs = True

# Replays
replays = {
    "crt_1x": {
        "csv_filename" : "alloc_times_6min_realtimeReplay.csv",
        "chart_title" : "CRT - 1x Speed",
    },
    "mimalloc_10x" : {
        "csv_filename" : "alloc_times_6min_10xspeed_mimalloc.csv",
        "chart_title" : "mimalloc - 10x Speed",
    },
    "rpmalloc_10x" : {
        "csv_filename" : "alloc_times_6min_10xspeed_rpmalloc.csv",
        "chart_title" : "rpmalloc - 10x Speed",
    }
}

# Labels
selected_replay = None # None == All
title_prefix = "Doom 3 Memory Analysis"

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

    replays_to_process = [selected_replay] if selected_replay != None else replays.keys()

    for replay in replays_to_process:

        replay_entry = replays[replay]
        csv_filename = replay_entry["csv_filename"]
        chart_title = replay_entry["chart_title"]

        print(f"Parsing data: {csv_filename}")
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
        cmap = cmr.get_sub_cmap('nipy_spectral', 0.03, 0.96)
        #cmap = 'nipy_spectral'

        # Alloc times
        if prepare_alloc_graph:
            fig = plt.figure(figsize=(24,13.5))
            ax = fig.add_subplot(1,1,1, projection='scatter_density')
            density = ax.scatter_density(
                x=allocTimestamps, 
                y=allocTimes,
                c=[min(alloc, mallocMax) for alloc in allocSizes], 
                cmap=cmap, 
                norm=colors.LogNorm(vmin=None,vmax=mallocMax))
            plt.semilogy(basey=10)
            ax.xaxis.set_major_formatter(FuncFormatter(x_labels))
            ax.yaxis.set_major_formatter(FuncFormatter(y_labels))
            ax.set_ylabel("Malloc Time")
            ax.set_ylim(bottom=3,top=1000*1000*1) # 1 millisecond
            ax.set_xlabel("Replay Time (seconds)")
            ax.set_title(f"{title_prefix} - Allocate Memory - {chart_title}")
            ax.set_facecolor('#000000')
            fig.colorbar(density, ticks=cbar_ticks, format=ColorbarFormatter())
            
            if fullscreen:
                fig.canvas.manager.full_screen_toggle()  
            
            if save_pngs:
                save_filename = csv_filename[:-4] + "_alloc.png"
                fig.savefig(f"screenshots/{save_filename}")

        # Free times
        if prepare_free_graph:
            # Sort data by free time
            freeData.sort(key=lambda entry : entry[0])

            # Extract data
            freeTimestamps = [entry[0] for entry in freeData]
            freeTimes = [entry[1] for entry in freeData]
            allocSizes = [min(entry[2], mallocMax) for entry in freeData]

            fig = plt.figure(figsize=(24,13.5))
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
            ax.set_ylim(bottom=3, top=1000*1000*1) # 1 milliseconds
            ax.set_xlabel("Replay Time (seconds)")
            ax.set_title(f"{title_prefix} - Free Memory - {chart_title}")
            ax.set_facecolor('#000000')
            fig.colorbar(density, ticks=cbar_ticks, format=ColorbarFormatter())
            
            if fullscreen:
                fig.canvas.manager.full_screen_toggle()  

            if save_pngs:
                save_filename = csv_filename[:-4] + "_free.png"
                fig.savefig(f"screenshots/{save_filename}")

        if show_plot:
            plt.show()

if __name__=="__main__":
    main()

