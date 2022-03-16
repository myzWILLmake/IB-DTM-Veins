import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as st
import csv
import os
import math

directory = 'data'
args = ['IB-DTM', 'D-TM']
labels = {
    'IB-DTM': 'PoS Blockchain-based Trust Management', 
    'D-TM': 'Distributed Trust Management'
}
TP = {}
TN = {}
FP = {}
epochs = 0
def open_data(fname, arg):
    tmp_TP = []
    tmp_TN = []
    tmp_FP = []
    cnt = 0
    with open(fname) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            tmp_TP.append(int(row[1]))
            tmp_TN.append(int(row[2]))
            tmp_FP.append(int(row[3]))
            cnt += 1
    if arg not in TP.keys():
        TP[arg] = []
        TN[arg] = []
        FP[arg] = []
    TP[arg].append(np.array(tmp_TP))
    TN[arg].append(np.array(tmp_TN))
    FP[arg].append(np.array(tmp_FP))
    return cnt
    
for filename in os.listdir(directory):
    f = os.path.join(directory, filename)
    if os.path.isfile(f):
        if f.find("malicious") != -1:
            for arg in args:
                if f.find(arg) != -1:
                    epochs = open_data(f, arg)

x_axis = np.arange(epochs)
fig, ax = plt.subplots(figsize = (6, 4))
ax.grid(axis='both')
ax.set_xlabel('Epoch #')
ax.set_ylabel('Flagged vehicles num')
# ax.set_ylabel('Mistake rate')
lines = []
for arg in args:
    arg_TP = np.array(TP[arg])
    arg_TN = np.array(TN[arg])
    arg_FP = np.array(FP[arg])
    y_axis = np.empty(epochs)
    stddev = np.empty(epochs)

    for i in range(0, epochs):
        stds = []
        means = []
        col = arg_TP[:, i]
        # col = arg_TN[:, i]

        y_axis[i] = np.mean(col)
        stddev[i] = np.std(col)

    ax.plot(x_axis, y_axis, ".-", label=labels[arg])
    print(arg)
    ax.legend(loc=4)
    ax.fill_between(x_axis, y_axis+stddev, y_axis-stddev, alpha=0.6)
    
    # for i in range(epochs):
    #     rate = []
    #     col = arg_FP[:, i]
    #     y_axis[i] = np.sum(col) / (160.0 * len(col))

    # ax.plot(x_axis, y_axis, ".-", label=arg)
    # ax.legend()

# line_value = 160
# line_data = []
# for i in range(epochs):
#     line_data.append(line_value)
# ax.plot(x_axis, line_data, '--')

# plt.title('True Negative')

plt.show()
