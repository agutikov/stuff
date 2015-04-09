#!/usr/bin/python


import sys
from matplotlib import pyplot as plt
from pprint import pprint


channels_count = len(sys.argv) - 1

data = [None] * channels_count

for i in range(0, channels_count):
	with open(sys.argv[i + 1]) as f:
		lines = f.readlines()
	#	pprint(lines)
		data[i] = list(map(lambda line: int(line), lines))

fig, axes = plt.subplots(nrows=channels_count, sharex=True, sharey=True)

if channels_count > 1:
	for i in range(0, channels_count):
		axes[i].plot(data[i])
else:
	axes.plot(data[0])
	
fig.subplots_adjust(hspace=0)

plt.show()


