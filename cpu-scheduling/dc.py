"""
Tyrel Hiebert
CSC 360 - Assignment 3
July 15, 2018
"""

import subprocess, math

quantumVals = [50, 100, 250, 500, 1000]
dispatchVals = [0, 5, 10, 15, 20, 25]
trials = []
tasks = 2000
seed = 8825

# Run simulations and accumulate outputs
print("Running simulations...")
for quantum in quantumVals:
    for dispatch in dispatchVals:
        command = './simgen '+str(tasks)+' '+str(seed)+' | ./rrsim --quantum '+str(quantum)+' --dispatch '+str(dispatch)
        output = subprocess.getoutput(command)
        trials.append(output)
print("Done.")

# Pull wait and turnaround times from trials and calc averages
print("Calculating averages...")
waitVals = []
avgWaitVals = []
taVals = []
avgTaVals = []
for trial in trials:
    trial = trial.split('\n')
    for outputLine in trial:
        if "EXIT" in outputLine:
            line = outputLine.split(' ')
            wait = int(line[3][2:])
            ta = int(line[4][3:])
            waitVals.append(wait)
            taVals.append(ta)
    waitCount = 0
    waitSum = 0
    for val in waitVals:
        waitSum += val
        waitCount += 1
    avgWaitVals.append(math.floor(waitSum / waitCount))
    taCount = 0
    taSum = 0
    for val in taVals:
        taSum += val
        taCount += 1
    avgTaVals.append(math.floor(taSum / taCount))
    waitVals = []
    taVals = []
print("Done.")

# save averages to text
print("Saving output...")
outputFile = open("output.txt", 'w')
outputFile.write("quantum dispatch wait turnaround\n")
keys = []
for quantum in quantumVals:
    for dispatch in dispatchVals:
        key = str(quantum) + " " + str(dispatch)
        keys.append(key)
for i in range(len(quantumVals) * len(dispatchVals)):
    outputLine = str(keys[i]) + " " + str(avgWaitVals[i]) + " " + str(avgTaVals[i])
    outputFile.write(outputLine + "\n")
print("Success!")