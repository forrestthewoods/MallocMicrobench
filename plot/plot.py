import matplotlib.pyplot as plt
import math
#import numpy as np
import csv


#def heatmap2d(arr: np.ndarray):
def heatmap2d(arr):
    #plt.imshow(arr, cmap='viridis')
    plt.imshow(arr, cmap='gist_yarg')
    plt.colorbar()
    plt.show()


data = [
    [1,2,0,4,5],
    [2,3,0,5,6],
    [3,4,5,6,7],
    [4,5,6,7,8],
]

#test_array = np.arange(100 * 100).reshape(100, 100)
heatmap2d(data)

# Parse data
maxAllocTime = 0
logBase = 10

entries = []
with open("alloc_times.csv") as csv_file:
    reader = csv.reader(csv_file)
    
    # skip header
    next(reader, None)

    # process data
    for row in reader:
        timestamp = float(row[0])
        allocTime = float(row[1])
        entries.append((timestamp, allocTime))
        maxAllocTime = max(maxAllocTime, allocTime)

maxTimestamp = entries[len(entries)-1][0]
logMaxAllocTime = math.log(maxAllocTime, logBase)

# Create 100x100 heatmap
width = 10
height = 10
data = [[0] * width] * height

# Convert to grid for heatmap
for entry in entries:
    timestamp = entry[0]
    allocTime = entry[1]
    logAllocTime = math.log(allocTime,logBase) 

    timebucket = min(int(timestamp / maxTimestamp * width), width - 1)
    allocbucket = min(int(logAllocTime / logMaxAllocTime * height), height - 1)

    data[timebucket][allocbucket] += 1

heatmap2d(data)
