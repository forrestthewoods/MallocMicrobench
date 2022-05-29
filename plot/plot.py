import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.colors as colors
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import LogFormatter
import math
import csv
import mpl_scatter_density
import cmasher as cmr

# Config
maxEntries = 0 # 0 = All
fullscreen = True
prepare_alloc_graph = True
prepare_free_graph = False
show_plot = False
save_pngs = True
y_max = 1000*1000*1

# Replays
replays = {
    "crtmalloc_1x_writebyte": {
        "csv_filename" : "doom3_replayreport_crt_1x_WriteByte.csv",
        "chart_title" : "crtmalloc - 1x Speed - WriteByte",
    },
    "crtalloc_10x_writenone": {
        "csv_filename" : "doom3_replayreport_crt_10x_WriteNone.csv",
        "chart_title" : "crtmalloc - 10x Speed - WriteNone",
    },
    "crtmalloc_10x_writebyte": {
        "csv_filename" : "doom3_replayreport_crt_10x_WriteByte.csv",
        "chart_title" : "crtmalloc - 10x Speed - WriteByte",
    },
    "crtmalloc_10x_writeall": {
        "csv_filename" : "doom3_replayreport_crt_10x_WriteAll.csv",
        "chart_title" : "crtmalloc - 10x Speed - WriteAll",
    },
    "crtmalloc_25x_writebyte": {
        "csv_filename" : "doom3_replayreport_crt_25x_WriteByte.csv",
        "chart_title" : "crtmalloc - 25x Speed - WriteByte",
    },
    "crtmalloc_max_writebyte": {
        "csv_filename" : "doom3_replayreport_crt_MaxSpeed_WriteByte.csv",
        "chart_title" : "crtmalloc - Max Speed - WriteByte",
    },
}

# Replays to process
#selected_replays = None # None = All
selected_replays = [
    "crtmalloc_1x_writebyte", 
    "crtmalloc_10x_writebyte", 
    "crtmalloc_25x_writebyte",
    "crtmalloc_max_writebyte",]

# Labels
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

    replays_to_process = selected_replays if selected_replays != None else replays.keys()

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
                replayTimestamp = float(row[0])
                allocTime = float(row[1])
                allocSize = float(row[2])
                replayFreeTimestamp = float(row[3])
                freeTime = float(row[4])
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
            nsPerMinute = 60e9
            nsPerSecond = 1e9
            minutes = int(tick / nsPerMinute)
            seconds = int((tick - minutes*nsPerMinute) / nsPerSecond)
            return f"{minutes}:{seconds:02d}"

        def y_labels(tick, pos):
            return format_nanoseconds(tick)

        cbar_ticks = [32,64,128,256,512,kilobyte,kilobyte*10, kilobyte*100, megabyte, megabyte*10, mallocMax]
        cmap = cmr.get_sub_cmap('nipy_spectral', 0.03, 0.96)

        # Alloc times
        if prepare_alloc_graph:
            fig = plt.figure(figsize=(20,11.25))
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
            ax.set_ylabel("Alloc Time")
            ax.set_ylim(bottom=3,top=y_max)
            ax.set_xlabel("Replay Time")
            ax.set_title(f"{title_prefix} - Allocate Memory - {chart_title}")
            ax.set_facecolor('#000000')
            fig.colorbar(density, ticks=cbar_ticks, format=ColorbarFormatter())

            if fullscreen:
                fig.canvas.manager.full_screen_toggle()  
            
            if save_pngs:
                save_filename = csv_filename[:-4] + "_alloc.png"
                fig.savefig(f"screenshots/{save_filename}", bbox_inches='tight')

        # Free times
        if prepare_free_graph:
            # Sort data by free time
            freeData.sort(key=lambda entry : entry[0])

            # Extract data
            freeTimestamps = [entry[0] for entry in freeData]
            freeTimes = [entry[1] for entry in freeData]
            allocSizes = [min(entry[2], mallocMax) for entry in freeData]

            fig = plt.figure(figsize=(20,11.25))
            ax = fig.add_subplot(1,1,1, projection='scatter_density')
            density = ax.scatter_density(
                x=freeTimestamps, 
                y=freeTimes,
                c=allocSizes, 
                cmap=cmap, 
                norm=colors.LogNorm(vmin=None,vmax=mallocMax))
            plt.semilogy(basey=10)
            ax.xaxis.set_major_formatter(FuncFormatter(x_labels))
            ax.yaxis.set_major_formatter(FuncFormatter(y_labels))
            ax.set_ylabel("Free Time")
            ax.set_ylim(bottom=3, top=y_max)
            ax.set_xlabel("Replay Time (seconds)")
            ax.set_title(f"{title_prefix} - Free Memory - {chart_title}")
            ax.set_facecolor('#000000')
            fig.colorbar(density, ticks=cbar_ticks, format=ColorbarFormatter())

            if fullscreen:
                fig.canvas.manager.full_screen_toggle()  

            if save_pngs:
                save_filename = csv_filename[:-4] + "_free.png"
                fig.savefig(f"screenshots/{save_filename}", bbox_inches='tight')

    if show_plot:
        plt.show()

if __name__=="__main__":
    main()

