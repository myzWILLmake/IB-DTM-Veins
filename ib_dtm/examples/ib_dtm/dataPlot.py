import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as st
import csv
import os
import math

# directory = 'data_2021_11_04'
# args = ['25_100', '25_150', '10_150', '25_200', '25_250']
# args_axis = ['25*4=100', '25*6=150', '10*15=150', '25*8=200', '25*10=250']
directory = 'data'
args = ['10_100_300', '10_150_300', '10_200_500']
args_axis = ['10slots 10epochs', '10slots 15epochs', '10slots 20epochs']
data = {}
error_stat = {}
mean_stat = {}
median_stat = {}
percentile_stat = {}

def open_data(fname, arg):
    read_data = []
    args_tmp = arg.split('_')
    slots = int(args_tmp[0])
    delay = 300
    if (len(args_tmp) > 2):
        delay = int(args_tmp[2])
    cnt = 0
    with open(fname) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            if int(row[0]) < 40:
                read_data.append(int(row[1]) * slots * 2 - delay)
            else:
                cnt += 1
    while (len(read_data) < 40):
        read_data.append(2000 - delay)
    return read_data, cnt

def adjacent_values(vals, q1, q3):
    upper_adjacent_value = q3 + (q3 - q1) * 1.5
    upper_adjacent_value = np.clip(upper_adjacent_value, q3, vals[-1])

    lower_adjacent_value = q1 - (q3 - q1) * 1.5
    lower_adjacent_value = np.clip(lower_adjacent_value, vals[0], q1)
    return lower_adjacent_value, upper_adjacent_value

def set_axis_style(ax, labels):
    ax.xaxis.set_tick_params(direction='out')
    ax.xaxis.set_ticks_position('bottom')
    ax.set_xticks(np.arange(1, len(labels) + 1))
    ax.set_xticklabels(labels)
    ax.set_xlim(0.25, len(labels) + 0.75)
    ax.set_ylabel('detection time (second)')
    # ax.set_xlabel('Sample name')

for filename in os.listdir(directory):
    f = os.path.join(directory, filename)
    if os.path.isfile(f):
        for arg in args:
            tmp = '2_' + arg
            if f.find(tmp) != -1:
                datatmp, cnt = open_data(f, arg)
                if arg not in data.keys():
                    data[arg] = []
                    error_stat[arg] = []
                    mean_stat[arg] = []
                    median_stat[arg] = []
                    percentile_stat[arg] = []
                data[arg].extend(datatmp)
                
                error_stat[arg].append(cnt)
                nptmp = np.array(datatmp)
                mean_stat[arg].append(np.mean(nptmp))
                median_stat[arg].append(np.median(nptmp))
                percentile_stat[arg].append(np.percentile(nptmp, 90))

# print(error_stat)

plot_data = []

for arg in args:
    # change to np.array
    data[arg].sort()
    npdata = np.array(data[arg])
    plot_data.append(npdata)

fig, ax = plt.subplots(figsize = (10, 7))
parts = ax.violinplot(
    plot_data, showmeans=False, showmedians=False, 
    showextrema=False)

for pc in parts['bodies']:
    # pc.set_facecolor('#D43F3A')
    pc.set_edgecolor('black')
    pc.set_alpha(1)

quartile1, medians, quartile3 = np.percentile(plot_data, [25, 50, 75], axis=1)
print(quartile1, medians, quartile3)
whiskers = np.array([
    adjacent_values(sorted_array, q1, q3)
    for sorted_array, q1, q3 in zip(plot_data, quartile1, quartile3)])
print(whiskers)
whiskers_min, whiskers_max = whiskers[:, 0], whiskers[:, 1]

inds = np.arange(1, len(medians) + 1)
ax.scatter(inds, medians, marker='o', color='white', s=30, zorder=3)
ax.vlines(inds, quartile1, quartile3, color='k', linestyle='-', lw=5)
ax.vlines(inds, whiskers_min, whiskers_max, color='k', linestyle='-', lw=1)

set_axis_style(ax, args_axis)

plt.subplots_adjust(bottom=0.15, wspace=0.05)
plt.title("First detected time of malicious vehicles")


# calculate confidence interval
for arg in mean_stat:
    data = mean_stat[arg]
    mean, sigma = np.mean(data), np.std(data)/np.sqrt(len(data))
    ci = st.norm.interval(alpha=0.95, loc=mean, scale=sigma)
    print(arg, ci, mean)
print()
for arg in median_stat:
    data = median_stat[arg]
    median, sigma = np.mean(data), np.std(data)/np.sqrt(len(data))
    ci = st.norm.interval(alpha=0.95, loc=median, scale=sigma)
    print(arg, ci, median)
print()
for arg in percentile_stat:
    data = percentile_stat[arg]
    percentile, sigma = np.mean(data), np.std(data)/np.sqrt(len(data))
    ci = st.norm.interval(alpha=0.95, loc=percentile, scale=sigma)
    print(arg, ci, percentile)
print()
# calculate error proportion confidence interval
for arg in error_stat:
    data = error_stat[arg]
    total = 160*len(data)
    proportion = float(sum(data)) / total
    interval = 1.96 * math.sqrt(proportion * (1-proportion) / total)
    ci = (proportion-interval, proportion+interval)
    print(arg, ci, proportion, interval)

plt.show()
