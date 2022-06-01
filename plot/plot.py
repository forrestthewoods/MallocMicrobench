from re import X
import cmasher as cmr
import csv
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.colors as colors
from matplotlib import scale as mscale
from matplotlib import transforms as mtransforms
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import LogFormatter
import math
import mpl_scatter_density

# Config
maxEntries = 0 # 0 = All
prepare_alloc_graph = True
prepare_free_graph = False
prepare_p99 = True
show_plot = False
save_pngs = True
save_large_pngs = True
y_max = 1000*1000*2
dpi_lo = 80
dpi_hi = 200

# Replays
replays = {
    # CRT
    "crtmalloc_1x_writebyte": {
        "csv_filename" : "doom3_replayreport_crt_1x_WriteByte.csv",
        "chart_title" : "crtmalloc - 1x Speed - WriteByte",
        "friendly_name": "crt",
    },
    "crtalloc_10x_writenone": {
        "csv_filename" : "doom3_replayreport_crt_10x_WriteNone.csv",
        "chart_title" : "crtmalloc - 10x Speed - WriteNone",
        "friendly_name": "crt",
    },
    "crtmalloc_10x_writebyte": {
        "csv_filename" : "doom3_replayreport_crt_10x_WriteByte.csv",
        "chart_title" : "crtmalloc - 10x Speed - WriteByte",
        "friendly_name": "crt",
    },
    "crtmalloc_10x_writeall": {
        "csv_filename" : "doom3_replayreport_crt_10x_WriteAll.csv",
        "chart_title" : "crtmalloc - 10x Speed - WriteAll",
        "friendly_name": "crt",
    },
    "crtmalloc_25x_writebyte": {
        "csv_filename" : "doom3_replayreport_crt_25x_WriteByte.csv",
        "chart_title" : "crtmalloc - 25x Speed - WriteByte",
        "friendly_name": "crt",
    },
    "crtmalloc_max_writebyte": {
        "csv_filename" : "doom3_replayreport_crt_MaxSpeed_WriteByte.csv",
        "chart_title" : "crtmalloc - Max Speed - WriteByte",
        "friendly_name": "crt",
    },

    # dlmalloc
    "dlmalloc_1x_writebyte": {
        "csv_filename" : "doom3_replayreport_dlmalloc_1x_SingleThread_WriteByte.csv",
        "chart_title" : "dlmalloc - 1x Speed - SingleThreaded - WriteByte",
        "friendly_name": "dlmalloc",
    },

    # HeapAlloc
    "HeapAlloc_1x_writebyte": {
        "csv_filename" : "doom3_replayreport_HeapAlloc_1x_WriteByte.csv",
        "chart_title" : "HeapAlloc - 1x Speed - WriteByte",
        "friendly_name": "HeapAlloc",
    },

    # jemalloc
    "jemalloc_1x_writebyte": {
        "csv_filename" : "doom3_replayreport_jemalloc_1x_WriteByte.csv",
        "chart_title" : "jemalloc - 1x Speed - WriteByte",
        "friendly_name": "jemalloc",
    },
    "jemalloc_10x_writebyte": {
        "csv_filename" : "doom3_replayreport_jemalloc_10x_WriteByte.csv",
        "chart_title" : "jemalloc - 10x Speed - WriteByte",
        "friendly_name": "jemalloc",
    },

    # mimalloc
    "mimalloc_1x_writebyte": {
        "csv_filename" : "doom3_replayreport_mimalloc_1x_WriteByte.csv",
        "chart_title" : "mimalloc - 1x Speed - WriteByte",
        "friendly_name": "mimalloc",
    },
    "mimalloc_10x_writebyte": {
        "csv_filename" : "doom3_replayreport_mimalloc_10x_WriteByte.csv",
        "chart_title" : "mimalloc - 10x Speed - WriteByte",
        "friendly_name": "mimalloc",
    },

    # rpmalloc
    "rpmalloc_1x_writebyte": {
        "csv_filename" : "doom3_replayreport_rpmalloc_1x_WriteByte.csv",
        "chart_title" : "rpmalloc - 1x Speed - WriteByte",
        "friendly_name": "rpmalloc",
    },
    "rpmalloc_10x_writebyte": {
        "csv_filename" : "doom3_replayreport_rpmalloc_10x_WriteByte.csv",
        "chart_title" : "rpmalloc - 10x Speed - WriteByte",
        "friendly_name": "rpmalloc",
    },

    # tlsf
    "tlsf_1x_writebyte": {
        "csv_filename" : "doom3_replayreport_tlsf_1x_SingleThread_WriteByte.csv",
        "chart_title" : "tlsf - 1x Speed - SingleThreaded - WriteByte",
        "friendly_name": "tlsf",
    },
    "tlsf_10x_writebyte": {
        "csv_filename" : "doom3_replayreport_tlsf_10x_SingleThread_WriteByte.csv",
        "chart_title" : "tlsf - 10x Speed - SingleThreaded - WriteByte",
        "friendly_name": "tlsf",
    },
}

# Replays to process
#selected_replays = None # None = All
selected_replays = ["crtmalloc_1x_writebyte", "HeapAlloc_1x_writebyte"]
#selected_replays = ["crtmalloc_1x_writebyte", "dlmalloc_1x_writebyte", "jemalloc_1x_writebyte", "mimalloc_1x_writebyte", "rpmalloc_1x_writebyte", "tlsf_1x_writebyte"]
percentile_replays = [
    "crtmalloc_1x_writebyte", 
    "dlmalloc_1x_writebyte",  
    "HeapAlloc_1x_writebyte", 
    "jemalloc_1x_writebyte", 
    "mimalloc_1x_writebyte", 
    "rpmalloc_1x_writebyte", 
    "tlsf_1x_writebyte"]

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

    p99_data = {}

    for replay in replays_to_process:

        replay_entry = replays[replay]
        csv_filename = replay_entry["csv_filename"]
        chart_title = replay_entry["chart_title"]
        replay_friendly_name = replay_entry["friendly_name"]

        print(f"Parsing data: {csv_filename}")
        allocTimestamps = []
        allocTimes = []
        freeTimes = []
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
                if maxEntries > 0 and len(allocTimes) + len(freeTimes) >= maxEntries:
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

            save_filename = csv_filename[:-4] + "_alloc"
            if save_pngs:
                fig.savefig(f"screenshots/{save_filename}.png", bbox_inches='tight', dpi=dpi_lo)

            if save_large_pngs:
                fig.savefig(f"screenshots/{save_filename}_large.png", bbox_inches='tight', dpi=dpi_hi)

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
            ax.set_xlabel("Replay Time")
            ax.set_title(f"{title_prefix} - Free Memory - {chart_title}")
            ax.set_facecolor('#000000')
            fig.colorbar(density, ticks=cbar_ticks, format=ColorbarFormatter())

            save_filename = csv_filename[:-4] + "_free"
            if save_pngs:
                fig.savefig(f"screenshots/{save_filename}.png", bbox_inches='tight', dpi=dpi_lo)

            if save_large_pngs:
                fig.savefig(f"screenshots/{save_filename}_large.png", bbox_inches='tight', dpi=dpi_hi)

        # Store malloc/free times for p99 plot
        if prepare_p99 and replay in percentile_replays:
            if len(freeTimes) == 0:
                freeTimes = [entry[1] for entry in freeData]

            # Sort times
            allocTimes.sort()
            freeTimes.sort()

            p99_data[replay_friendly_name] = {
                "allocs": allocTimes,
                "frees": freeTimes
            }

    if prepare_p99: 
        # Utility
        def clamp(v, min, max):
            if v < min:
                return min
            elif v > max:
                return max
            else:
                return v

        def lerp(frac, a, b):
            return a*(1-frac) + b*frac

        def lerp_map_range(v, in_min, in_max, out_min, out_max):
            v = clamp(v, in_min, in_max)
            frac = clamp(float(v - in_min) / float(in_max - in_min), 0.0, 1.0)
            return lerp(frac, out_min, out_max)

        fig = plt.figure(figsize=(20,11.25))
        ax = fig.add_subplot()

        plt.semilogy(basey=10)
        #ax.xaxis.set_major_formatter(FuncFormatter(x_labels))
        ax.yaxis.set_major_formatter(FuncFormatter(y_labels))
        ax.set_ylabel("Alloc Time")
        ax.set_xlim(left=0, right=101)
        ax.set_ylim(bottom=3, top=y_max)
        ax.set_xlabel("Percentile")
        ax.set_title(f"Alloc Time (Percentile)")
        ax.set_facecolor('#000000')

        # colors from https://spectrum.adobe.com/page/color-for-data-visualization/
        p99_colors = ['#0fb5ae', '#4046ca', '#f68511', '#de3d82', '#7e84fa', '#72e06a']
        
        colorIdx = 0
        for key in p99_data:
            allocs = p99_data[key]["allocs"]

            buckets = []

            # Carefully define buckets
            def appendHelper(min, max, step):
                i = min
                while i < max:
                    buckets.append(i)
                    i += step
            appendHelper(1.0, 99.9, 0.1)
            appendHelper(99.9, 99.99, 0.01)
            appendHelper(99.99, 99.999, 0.001)
            appendHelper(99.999, 99.9999, 0.0001)

            # Lerp all values from p99.9999 to p100
            p_lo = 99.9999
            p_hi = 100.0
            idx_lo = int(p_lo/100 * len(allocs))
            idx_hi = len(allocs) - 1
            for idx in range(idx_lo, idx_hi + 1):
                p = lerp_map_range(idx, idx_lo, idx_hi, p_lo, p_hi)
                buckets.append(p)

            buckets.append(100)

            # TODO: insert all points between 99.9999 and 100

            alloc_bucket_values = []
            for bucket in buckets:
                idx = bucket/100.0 * len(allocs)
                idx = int(min(idx, len(allocs) - 1))
                alloc_bucket_values.append(allocs[idx])

            ax.plot(buckets,alloc_bucket_values,label=key,color=p99_colors[colorIdx % len(p99_colors)])
            colorIdx = colorIdx + 1
        
        ax.legend(loc='upper left', prop={'size': 14})

        # Save two images. One full graph, one zoomed on p95
        if save_pngs:
            # Full size
            ax.set_xlim(left=0, right=101)

            if save_pngs:
                fig.savefig(f"screenshots/percentile_alloc.png", bbox_inches='tight', dpi=dpi_lo)
            
            if save_large_pngs:
                fig.savefig(f"screenshots/percentile_alloc_large.png", bbox_inches='tight', dpi=dpi_hi)

            # Zoom image
            def log_map_range(v, in_min, in_max, out_min, out_max):
                v = clamp(v, in_min, in_max)
                frac = clamp((v - in_min) / (in_max - in_min), 0.0, 1.0)
                if frac == 0:
                    return out_min
                scaled_frac = math.log(lerp(frac, 1, 10), 10)
                result = lerp(scaled_frac, out_min, out_max)
                return result

            def transform_one(value):
                sections = 5
                section_size = 9 / sections
                def section_marker(i):
                    return 90 + section_size*i

                if value <= 99.0:
                    return log_map_range(value, 90.0, 99.0, section_marker(0), section_marker(1))
                elif value <= 99.9:
                    return log_map_range(value, 99.0, 99.9, section_marker(1), section_marker(2))
                elif value <= 99.99:
                    return log_map_range(value, 99.9, 99.99, section_marker(2), section_marker(3))
                elif value <= 99.999:
                    return log_map_range(value, 99.99, 99.999, section_marker(3), section_marker(4))
                elif value <= 99.9999:
                    return log_map_range(value, 99.999, 99.9999, section_marker(4), section_marker(5))
                else:
                    return lerp_map_range(value, 99.9999, 100.0, section_marker(5), 100.0)

            def transform_arr(values_arr):
                return [transform_one(value) for value in values_arr]

            def x_scale(values):
                return [transform_arr(values_inner) for values_inner in values]

            ax.set_xscale('functionlog', functions=[lambda x: x_scale(x), lambda x: x])
            
            # Set major ticks
            ticks = [90, 99, 99.9, 99.99, 99.999, 99.9999, 100]
            tick_labels = ["p90", "p99", "p99.9", "p99.99", "p99.999", "p99.9999", "p100"]
            ax.set_xticks(ticks)
            ax.set_xticklabels(tick_labels)
            ax.set_xlim(left=90, right=101)

            # Set minor ticks
            def xlabel_helper(tick, pos):
                return ''
            ax.xaxis.set_minor_formatter(FuncFormatter(xlabel_helper))
            ax.xaxis.set_minor_locator(plt.FixedLocator([
                91,92,93,94,95,96,97,98,
                99.1, 99.2, 99.3, 99.4, 99.5, 99.6, 99.7, 99.8,
                99.91, 99.92, 99.93, 99.94, 99.95, 99.96, 99.97, 99.98,
                99.991, 99.992, 99.993, 99.994, 99.995, 99.996, 99.997, 99.998,
                99.9991, 99.9992, 99.9993, 99.9994, 99.9995, 99.9996, 99.9997, 99.9998,
                ]))

            if save_pngs:
                fig.savefig(f"screenshots/percentile_alloc_zoomed.png", bbox_inches='tight', dpi=dpi_lo)

            if save_large_pngs:
                fig.savefig(f"screenshots/percentile_alloc_zoomed_large.png", bbox_inches='tight', dpi=dpi_hi)


    if show_plot:
        plt.show()

if __name__=="__main__":
    main()

