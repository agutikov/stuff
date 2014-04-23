#!/usr/bin/python


import sys
import os
import io
from pprint import pprint




def time2str (t):
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

def str2time (s):
	a = s.split(" ")
	v = float(a[0].replace(',','.'))
	mul = {
		's' : 1.0,
		'ms' : 1000.0,
		'us' : 1000000.0,
		'ns' : 1000000000.0
	}[a[1]]
	return v / mul

def xor(data):
	result = 0
	for b in data:
		result ^= b
	return result

def lrc(data):
	result = 0
	for b in data:
		result = (result + b) & 0xFF
	return (((result ^ 0xFF) + 1) & 0xFF)

def cs(data):
	result = 1
	for b in data:
		result = (result + b) & 0xFF
	return result



def ms (i):
	return i / 1000000.0


def csv_line (line):
	return list(map(lambda el: el.strip(" \""), line.split("\",\"")))

def csv_parse (text_single_string):
	text = text_single_string.split(os.linesep)
	data = {"header" : [], "data" : []}
	data["header"] = csv_line(text[0])
	for line in text[1:]:
		if line:
			data["data"].append(csv_line(line))
	return data




txt = ""

with open(sys.argv[1], "rb") as f:
	txt = bytearray(f.read()).replace(b"\xE2\x80\x8A", b"\x20").decode("utf-8")

data = csv_parse(txt)

#pprint(data)


# "index","start-time","end-time","event?","event-type","RxD event","TxD event","RxD data","TxD data"
class octet:
	index = 0
	start = 0.0
	end = 0.0
	dur = 0.0
	event = False
	ev_type = ""
	rx_event = ""
	tx_event = ""
	val = 0
	def __init__ (self, str_lst):
		self.index = int(str_lst[0])
		self.start = str2time(str_lst[1])
		self.end = str2time(str_lst[2])
		self.dur = self.end - self.start
		self.event = str_lst[3] == "true"
		self.ev_type = str_lst[4]
		self.rx_event = str_lst[5]
		self.tx_event = str_lst[6]
		if not self.event:
			if str_lst[7]:
				self.val = int(str_lst[7])
			else:
				self.val = int(str_lst[8])

	def str (self):
		if self.event:
			return ("event: %s %s %s" % (self.ev_type, self.rx_event, self.tx_event))
		else:
			return ("%d, %s, %s, %s, 0x%02X" %
				(self.index, time2str(self.start), time2str(self.end), time2str(self.dur), self.val))

	def value (self):
		if self.event:
			return ("event: %s %s %s" % (self.ev_type, self.rx_event, self.tx_event))
		else:
			return ("0x%02X" % self.val)

octets = []

for lst in data["data"]:
	octets.append(octet(lst))

delays = []

for idx, b in enumerate(octets):
	delays.append(b.start - (0 if idx == 0 else octets[idx-1].end))

messages = []
tmp = []

for idx, b in enumerate(octets):
	# print(time2str(delays[idx]))
	# print(b.str())
	if delays[idx] > 0.02:
		# print("\n" + time2str(delays[idx]) + "\n")
		if tmp:
			messages.append([delays[idx], tmp])
			tmp = []
	if not b.event:
		tmp.append(b.val)
	# print(b.value())


print("\n")

def printhexarr (a):
	print(("%02X"+", %02X"*(len(a)-1)) % tuple(a))

for m in messages:
	pprint(m[0])
	printhexarr(m[1])
	print("xor: 0x%02X, lrc: 0x%02X, cs: 0x%02X" % (xor(m[1][:-1]), lrc(m[1][:-1]), cs(m[1][:-1])))






















