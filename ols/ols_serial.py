#!/usr/bin/python

import zipfile
import sys
import os.path
from io import StringIO
import math
from pprint import pprint
import operator
from collections import defaultdict
import functools



############## process command line args #######################################

print("\n".join(sys.argv))
print(len(sys.argv))

if len(sys.argv) < 2:
	print("argc < 2")
	quit()

if not os.path.isfile(sys.argv[1]):
	print(sys.argv[1] + " is not a file")
	quit()

zip_filename = sys.argv[1]

print(zip_filename)


################ Open zip file, find data, read text, split to strings #########

with zipfile.ZipFile(zip_filename, "r") as zfile:
	for name in zfile.namelist():
		print(name)
		if name == "data.ols":
			ols_data = zfile.read(name)

if not ols_data:
	print("ols data not found")
	quit()

# print(ols_data)

text = ols_data.decode("utf-8")
strings = text.split("\n")


############### Parse header ###################################################

# header example:
#
# ;Size: 512
# ;Rate: 20000
# ;Channels: 8
# ;EnabledChannels: 255
# ;TriggerPosition: 2454
# ;Compressed: true
# ;AbsoluteLength: 24576
# ;CursorEnabled: true
#

# data_strings = filter(lambda s: s.find("@") != -1, strings)

data_strings = []

rate = 0
period = 0

for s in strings:
	if len(s) > 0:
		if s[0] == ';':
			print(s)
			ss = s.split(":")
			if ss[0] == ";Rate":
				rate = int(ss[1])
				period = 1 / float(rate)
		else:
			data_strings.append(s)


def timestr (t):
	if t >= 1:
		return ("%.1f" % t) + "s"
	elif t < 1 and t >= 1/1000:
		return ("%.1f" % (t*1000)) + "ms"
	elif t < 1/1000 and t >= 1/1000000:
		return ("%.1f" % (t*1000000)) + "us"
	elif t < 1/1000000 and t > 0:
		return ("%.1f" % (t*1000000000)) + "ns"
	else:
		return "0.0"

print("sampling period = " + timestr(period))

########## parse data strings to list of fields ################################
#
# [0] = bit value,
# [1] = start time,
# [2] = duration,
# [3] = number of bits
#

data = []

prev_level = 1 if (int(data_strings[0][0]) == 0) else 0

for s in data_strings:
#	print(s)
	fields = s.split("@")
	level = int(fields[0])
	if (prev_level != level):
		data.append([level, int(fields[1])])
		prev_level = level
	else:
		data[-1][1] = int(fields[1])

for idx, d in enumerate(data):
	dur = 0
	if (idx < len(data)-1):
		dur = data[idx+1][1] - d[1];
	d.append(dur)
	# print(timestr(period*dur))

# pprint(data)

# pprint(list(filter(lambda x: x[2] == 1, data)))

####################### statistics of signal level length ######################

# количество появлений длительностей уровней сигналов
stat = {}

for d in data:
	if d[2] in stat:
		stat[d[2]] += 1
	else:
		stat[d[2]] = 1

# pprint(stat)

def dict2tuplist (d):
	lst = []
	for k in d.keys():
		lst.append((k, d[k]))
	return lst

# pprint(dict2tuplist(stat))

# те-же количества появлений длительностей уровней сигналов
# но в виде списка пар
_stat = dict2tuplist(stat)

# отсортированный список пар (длительность - количество появлений)
sstat = sorted(_stat, key=lambda x: x[1], reverse=True)

'''
for s in sstat:
	print("%d" % s[0] + " : %d" % s[1])
'''

# вспомогательный словарь
# для каждого количества появлений - список длительностей
_sstat = defaultdict(list)

for s in sstat:
	_sstat[s[1]].append(s[0])

# отсортированный список пар (количество появлений - список длительностей)
__sstat = sorted(dict2tuplist(_sstat), key=lambda x: x[0], reverse=True)

for s in __sstat:
	print( ("%d times appears: " % s[0]) +
        ", ".join(map(lambda x: ("%d (%s)" % (x, timestr(period*x))), s[1]))
        )



################## get bit duration ############################################

# idea: group durations in clusters with nearest values

durations = sorted(map(lambda x: x[0], sstat))

diffs = []

for idx, dur in enumerate(durations):
	if idx == 0:
		diffs.append(0)
	else:
		diffs.append(dur - durations[idx-1])


# pprint(durations)
# pprint(diffs)

clustered = []
tmp = []

def min_greater (lst, floor):
	return functools.reduce(lambda m, v: m if m<v and m>floor else v, lst, floor)

threshold = min_greater(durations, 2) / 2 - 1

print("clustering threshold = %d" % threshold)

for idx, dif in enumerate(diffs):
	if dif > threshold:
		clustered.append(tmp)
		tmp = []
	tmp.append(durations[idx])

pprint(clustered)


# bit sequences durations
# map duration to number of bits
bit_seq_durs = {}

def avg (lst):
	return functools.reduce(lambda a,b: a+b, lst) / len(lst)

bit_dur = avg(clustered[1])

pprint(bit_dur)


for c in clustered[1:]:
	a = avg(c)
	b = a / bit_dur
	print("%d %8.1f %8.1f %8d" % (len(c), a, b, round(b)))


quit()


################################################################################

durations = []

for idx, d in enumerate(data):
	print(("%d" % idx) + " : " + ("%d" % d[0]) + ", " + ("%d" % d[1]) + ", " + ("%d" % d[2]))
	# print(("%d" % d[1]) + ", " + ("%d" % d[2]))
	durations.append(d[2])

bit_dur = min(durations[:-2])

print("probably bit lenth = " + ("%d" % bit_dur))



floor_add = 2

for d in data:
	bit_count = ((d[2] + floor_add) / bit_dur)
	# print(("%d" % d[1]) + ", " + ("%d" % d[2]) + ", " + ("%d" % bit_count))
	d.append(int(bit_count))
	if d[3] < 16:
		print(("%d" % d[1]) * d[3])
	else:
		print("_________" + ("%d" % d[1]) + " for " + ("%d" % d[2]) + " sample periods")


messages = []
curr_msg = ""

for d in data:
	if d[3] < 16:
		curr_msg += ("%d" % d[1]) * d[3]
	elif len(curr_msg) > 0:
		messages.append(curr_msg)
		curr_msg = ""


for m in messages:
	print(
		(" %d " % len(m)) +
		(" %d " % int(len(m)/8)) +
		(" %d " % (len(m)%8))
		)
	# print(m)
	# m += "1" * (len(m)%8)
	print(m[0:31])
	print(m[31:])
	bits = [m[i:i+8] for i in range(0, len(m), 8)]
	# print(bits)
	bytes = map(lambda s: int(s, 2), bits)
	# for b in bytes:
	# 	print("{0:08b}".format(b) + "  ", end='')

	for idx, b in enumerate(bytes):
		print(("%02X" % b) + " ", end='')


	print("\n")


