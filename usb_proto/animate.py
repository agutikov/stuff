#!/usr/bin/python

from pprint import pprint
import operator
import collections
import time
import sys

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

f = open(sys.argv[1])

interval_ms = 500
sample_per_interval = 100

fig = plt.figure()
ax = fig.add_subplot(111)
line, = ax.plot(range(0, sample_per_interval), range(0, sample_per_interval))


def update(data):
	global sample_per_interval
	
	if len(data) == sample_per_interval:
		line.set_ydata(data)
		min_y = min(data)
		max_y = max(data)
		print(min_y, max_y)
		ax.set_ylim(min_y - abs(min_y)/10, max_y + abs(max_y)/10)
	else:
		print("wrong data length")
	return line,


def data_gen():
	global f
	global sample_per_interval
	
	while True:
		values = []
			
		for i in range(0, sample_per_interval):
			
			line = f.readline()
			if not line:
				f.close()
				exit()
			
			values.append(int(line))
			
		yield values


ani = animation.FuncAnimation(fig, update, data_gen, blit=False, repeat=True, interval=interval_ms)


try:
	plt.show()
except KeyboardInterrupt:
	pass

f.close()

