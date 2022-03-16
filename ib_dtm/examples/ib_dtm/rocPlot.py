import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as st
import csv
import os
import math

directory = 'data'
args = ['D-TM', 'IB-100']
labels = {
    'IB-100': 'PoS Blockchain-based Trust Management', 
    'D-TM': 'Distributed Trust Management',
}

tvn = {}
tvm = {}
mintv = 100
maxtv = -100

def open_data(fname, arg):
    epoch = 0
    if arg not in tvn:
        tvn[arg] = {}
        tvm[arg] = {}
    with open(fname) as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith("epoch"):
                epoch = int(line[5:])
                if epoch not in tvn[arg]:
                    tvn[arg][epoch] = []
                if epoch not in tvm[arg]:
                    tvm[arg][epoch] = []
            else:
                idx = line.index(":")
                vid = int(line[:idx])
                tv = int(line[idx+1:])
                if vid < 40:
                    tvm[arg][epoch].append(tv)
                else:
                    tvn[arg][epoch].append(tv)
                global mintv
                global maxtv
                if tv < mintv:
                    mintv = tv
                if tv > maxtv:
                    maxtv = tv
    
for filename in os.listdir(directory):
    f = os.path.join(directory, filename)
    if os.path.isfile(f):
        if f.find("tvo") != -1:
            for arg in args:
                if f.find(arg) != -1:
                    epochs = open_data(f, arg)

print(tvm)


def cal_roc(arg):
    roc = {}

    for threshold in range(mintv-2, maxtv+2):
        Pcnt = 0
        TP = 0
        Ncnt = 0
        FP = 0
        epochs = sorted(tvm[arg].keys())
        maxepoch = epochs[-1]
        for epoch in range(maxepoch-10, maxepoch+1):
            # processing TPR
            Pcnt = Pcnt + len(tvm[arg][epoch])
            for tv in tvm[arg][epoch]:
                if tv < threshold:
                    TP = TP+1
            # processing FPR
            Ncnt = Ncnt + len(tvn[arg][epoch])
            for tv in tvn[arg][epoch]:
                if tv < threshold:
                    FP = FP+1
        
        TPR = TP / float(Pcnt)
        FPR = FP / float(Ncnt)

        if FPR not in roc:
            roc[FPR] = TPR

    return roc

print(cal_roc("D-TM"))

roc_data = {}
for arg in args:
    roc_data[arg] = cal_roc(arg)

fig, ax = plt.subplots(figsize=(5, 5))
ax.grid(axis='both')
ax.set_xlabel('False Positive Rate')
ax.set_ylabel('True Positive Rate')

for arg in args:
    x_axis_data = sorted(roc_data[arg].keys())
    x_axis = np.array(x_axis_data)
    y_axis_data = []
    for x in x_axis_data:
        y_axis_data.append(roc_data[arg][x])
    y_axis = np.array(y_axis_data)

    ax.plot(x_axis, y_axis, "-", label=labels[arg])
    ax.legend(loc=4)

random_x_axis_data = []
for i in range(0, 11):
    random_x_axis_data.append(i / float(10))

random_x_axis = np.array(random_x_axis_data)
random_y_axis = np.array(random_x_axis_data)
ax.plot(random_x_axis, random_y_axis, "--", label="Random Classifier")
ax.legend(loc=4)

plt.plot(0,1,'ro', label="Perfect Classifier")
ax.legend(loc=4) 

plt.show()
