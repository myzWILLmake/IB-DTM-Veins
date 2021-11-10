import matplotlib.pyplot as plt
import numpy as np
import csv
import os

directory = 'data'
args = ['25_100', '25_150', '10_150', '25_200', '25_250']
args_axis = ['25*4=100', '25*6=150', '10*15=150', '25*8=200', '25*10=250']
data = {}

def open_data(fname, arg):
    read_data = []
    args_tmp = arg.split('_')
    slots = int(args_tmp[0])
    print(args_tmp)
    with open(fname) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            if int(row[0]) < 40:
                read_data.append(int(row[1]) * slots * 2)
    return read_data

for filename in os.listdir(directory):
    f = os.path.join(directory, filename)
    if os.path.isfile(f):
        for arg in args:
            tmp = '2_' + arg
            if f.find(tmp) != -1:
                print(f)
                number = f[-1]
                datatmp = open_data(f, arg)
                if arg not in data.keys():
                    data[arg] = []
                data[arg].extend(datatmp)

plot_data = []
    
for arg in args:
    plot_data.append(np.array(data[arg]))

fig, ax = plt.subplots(figsize = (10, 7))

# ax = fig.add_axes(plot_axis)
ax.boxplot(plot_data, labels=args_axis)

plt.title("First detected time of malicious vehicles")
plt.show()
