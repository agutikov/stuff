#!/usr/bin/python

import sys
from pprint import pprint
import re
import binascii
import struct

R = re.compile('data<(.*)> s')

sample = bytearray()


channels_count = 27
sample_size = channels_count * 2

channel_files = [None] * channels_count

for ch in range(0, channels_count):
	channel_files[ch] = open('channels/%02d.txt' % ch, 'w+')

sample_count = 0

with open(sys.argv[1]) as f:
	while True:
		line = f.readline()
		if not line:
			break
#		print(line)
		a = R.search(line)
		if not a:
			continue
		data_txt_hex = a.group(1)
#		print(data_txt_hex)
		data_bytes = bytearray.fromhex(data_txt_hex)
#		print(binascii.hexlify(data_bytes))
		if len(data_bytes) > 5 and data_bytes[1] == 0x01 and data_bytes[2] == 0x60:
#			print(len(data_bytes), binascii.hexlify(data_bytes))
			samples_data = data_bytes[3:-2]
#			print(len(samples_data), binascii.hexlify(samples_data))
			sample = sample + samples_data
			if len(sample) >= sample_size:
#				print(''.join('{:02x}'.format(x) for x in sample[:sample_size]))
				sample_54 = sample[:sample_size]
				sample = sample[sample_size:]
				signal = struct.unpack('h'*channels_count, sample_54)
#				print(' '.join('{:d}'.format(x) for x in signal))
				for ch in range(0, channels_count):
					channel_files[ch].write("%d %d\n" % (sample_count, signal[ch]))
				sample_count += 1


for ch in range(0, channels_count):
	channel_files[ch].close()














