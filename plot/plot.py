import matplotlib.pyplot as plt
import math
import csv


# Parse data
maxAllocTime = 0
logBase = 10

timestamps = []
allocTimes = []
maxEntries = 0
with open("alloc_times.csv") as csv_file:
    reader = csv.reader(csv_file)
    
    # skip header
    next(reader, None)

    # process data
    for row in reader:
        timestamp = float(row[0])
        allocTime = float(row[1])
        timestamps.append(timestamp)
        allocTimes.append(allocTime)
        if maxEntries > 0 and len(timestamps) >= maxEntries:
            break
        #maxAllocTime = max(maxAllocTime, allocTime)

#maxTimestamp = entries[len(entries)-1][0]
#logMaxAllocTime = math.log(maxAllocTime, logBase)

# Scatter plot
if True:
    from matplotlib.ticker import FuncFormatter

    def x_labels(tick, pos):
        return f"{tick / 1e9}"

    def y_labels(tick, pos):
        if tick < 1000:
            return f"{tick} ns"
        elif tick < 1000 * 1000:
            return f"{tick/1000} μs"
        elif tick < 1000 * 1000 * 1000:
            return f"{tick/1000/1000} ms"
        else:
            return f"{tick/1000/1000/1000} s"

    fig,ax = plt.subplots(1,1, figsize=(16,9))
    plt.scatter(x=timestamps, y=allocTimes, s=0.1)
    plt.semilogy(basey=10)
    ax.xaxis.set_major_formatter(FuncFormatter(x_labels))
    ax.yaxis.set_major_formatter(FuncFormatter(y_labels))
    ax.set_ylabel("Malloc Time")
    ax.set_xlabel("Game time (seconds)")
    ax.set_title("Doom 3 Memory Analysis - Malloc")
    plt.show()

# Heatmap
if False:
    width = 100
    height = 10
    data = [ [0]*width for i in range(height)]

    # Buckets
    allocTimeLimit = [30]

    # Convert to grid for heatmap
    for entry in entries:
        timestamp = entry[0]
        allocTime = entry[1]
        logAllocTime = math.log(allocTime,logBase) 

        timebucket = min(int(timestamp / maxTimestamp * width), width - 1)
        allocbucket = min(int(logAllocTime / logMaxAllocTime * height), height - 1)
        #allocbucket = min(int(allocTime / maxAllocTime * height), height - 1)

        data[allocbucket][timebucket] += 1

    print(f"Max timestamp: {timestamp}")
    print(f"Max alloc time: {maxAllocTime}")
    print(f"Max alloc time (log): {logMaxAllocTime}")

    for row in data:
        print(row)

    heatmap2d(data)
    #print(data)
