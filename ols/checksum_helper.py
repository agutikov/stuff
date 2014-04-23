#!/usr/bin/python

import binascii
import array
import string
import sys


# this values is for signal level LOW=0/HIGH=1 and MSB first bit order
packets_low0_msb = [ bytearray([0x50, 0xE0, 0x05, 0xF8, 0x80, 0x10, 0x87]),
	          bytearray([0x50, 0xE0, 0x2D, 0x08, 0x80, 0x10, 0x87]),
	          bytearray([0x50, 0xE0, 0x2D, 0x29, 0x80, 0x10, 0x87]),
	          bytearray([0x50, 0xE0, 0x2D, 0xE9, 0x80, 0x10, 0x87])]

def bitmask (x):
	return 0x1 << x

def reflect (v, b):
	t = v
	for i in range(0, b):
		if t & 0x01:
			v |= bitmask((b - 1) - i)
		else:
			v &= ~bitmask((b - 1) - i)
		t >>= 1
	return v


############### print all possible interpretation of signal in a single table

print("\n" \
"FIELDS: hex = binary" \
"[0] LOW=0, HIGH=1, big-endian" \
"[1] LOW=0, HIGH=1, little-endian" \
"[2] LOW=1, HIGH=0, big-endian" \
"[3] LOW=1, HIGH=0, little-endian" \
"")

for bb in packets_low0_msb :
	for b in bb:
		print(("%0.2X" % b) + " = " + "{0:08b}".format(b) + ", " + \
		      ("%0.2X" % reflect(b, 8)) + " = " + "{0:08b}".format(reflect(b, 8)) + ", " + \
		      ("%0.2X" % (~b & 0xFF)) + " = " + "{0:08b}".format((~b & 0xFF)) + ", " + \
		      ("%0.2X" % reflect((~b & 0xFF), 8)) + " = " + "{0:08b}".format(reflect((~b & 0xFF), 8)))
	print("")

print("")


############### fill each message arrays and print them


# low signal logical value, bit order, function for convertion from
class Desc:
	signal_logic = 0
	bit_order = 0
	convert = lambda b: b

	signal_logic_values = ["LOW=0, HIGH=1", "LOW=1, HIGH=0"]
	bit_order_strings = ["big-endian", "little-endian"]

	def __init__ (self, logic, order, convert):
		self.signal_logic = logic
		self.bit_order = order
		self.convert = convert

	def name (self):
		return self.signal_logic_values[self.signal_logic] + ", " + self.bit_order_strings[self.bit_order]


desc = [ Desc(0, 0, lambda b: b),
	 Desc(0, 1, lambda b: reflect(b, 8)),
	 Desc(1, 0, lambda b: (~b & 0xFF)),
	 Desc(1, 1, lambda b: reflect((~b & 0xFF), 8))
	]

packets = []

for d in desc:
	print(d.name())
	tmp_msg = []
	for bb in packets_low0_msb:
		tmp = []
		for b in bb:
			tmp.append(d.convert(b))
		tmp_msg.append(bytearray(tmp))
		print(binascii.hexlify(bytearray(tmp)))
	packets.append(tmp_msg)
	print("")

print("")


############### user-defined checksum functions

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


# ported implementation from http://www.ross.net/crc/download/crc_v3.txt

class Crc:
	width = 0
	poly = 0
	init = 0
	refin = False
	refout = False
	xorout = 0
	check = 0
	name = ""

	def __init__ (self, width, poly, init, refin, refout, xorout, check, name):
		self.width = width
		self.poly = poly
		self.init = init
		self.refin = refin
		self.refout = refout
		self.xorout = xorout
		self.check = check
		self.name = name

	def wildmask (self):
		return (1 << self.width) - 1
		# return (((0x01 << (self.width - 1)) - 1) << 1) | 0x01

	reg = 0

	def ini (self):
		self.reg = self.init

	def nxt (self, ch):
		uch = ch
		topbit = bitmask(self.width - 1)
		if self.refin:
			uch = reflect(uch, 8);
		self.reg ^= uch << (self.width - 8);
		for i in range(8):
			if self.reg & topbit:
				self.reg = (self.reg << 1) ^ self.poly
			else:
				self.reg <<= 1
			self.reg &= self.wildmask()

	def crc (self):
		if self.refout:
			return self.xorout ^ reflect(self.reg, self.width)
		else:
			return self.xorout ^ self.reg

	def calc (self, block):
		self.ini()
		for b in block:
			self.nxt(b)
		return self.crc()

	def do_check(self):
		print("name = " + self.name)
		print("width = " + ("%d" % self.width))
		print("poly = " + ("0x%0.2X" % self.poly))
		print("init = " + ("0x%0.2X" % self.init))
		print("refin = " + ("%r" % self.refin))
		print("refout = " + ("%r" % self.refout))
		print("xorout = " + ("0x%0.2X" % self.xorout))
		print("check = " + ("0x%0.2X" % self.check))
		test = bytearray([0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39])
		# print binascii.hexlify(test)
		calculated_check = self.calc(test)
		print("calculated check = " + ("0x%X" % calculated_check))
		if calculated_check != self.check:
			print("WARNING! Check and calculated check are not equal!")
		print("")



# http://reveng.sourceforge.net/crc-catalogue/1-15.htm#crc.cat-bits.8

crcs = [Crc(8, 0x07, 0x00, False, False, 0x00, 0xf4, "Crc-8"),
	Crc(8, 0x9b, 0xff, False, False, 0x00, 0xda, "Crc-8/CDMA2000"),
	Crc(8, 0x39, 0x00, True,  True,  0x00, 0x15, "Crc-8/DARC"),
	Crc(8, 0xd5, 0x00, False, False, 0x00, 0xbc, "Crc-8/DVB-S2"),
	Crc(8, 0x1d, 0xff, True,  True,  0x00, 0x97, "Crc-8/EBU"),
	Crc(8, 0x1d, 0xfd, False, False, 0x00, 0x7e, "Crc-8/I-CODE"),
	Crc(8, 0x07, 0x00, False, False, 0x55, 0xa1, "Crc-8/ITU"),
	Crc(8, 0x31, 0x00, True,  True,  0x00, 0xa1, "Crc-8/MAXIM"),
	Crc(8, 0x07, 0xff, True,  True,  0x00, 0xd0, "Crc-8/ROHC"),
	Crc(8, 0x9b, 0x00, True,  True,  0x00, 0x25, "Crc-8/WCDMA")
	]


def simple_do_check (name, call):
	print("name = " + name)
	print("")

class ChecksumFunction:
	calc = lambda data: 0
	do_check = lambda : simple_do_check("", null)
	def __init__ (self, calc, do_check):
		self.calc = calc
		self.do_check = do_check

checksum_functions = []

checksum_functions.append(ChecksumFunction(xor, lambda : simple_do_check("xor", xor)))
checksum_functions.append(ChecksumFunction(lrc, lambda : simple_do_check("lrc", lrc)))
checksum_functions.append(ChecksumFunction(cs, lambda : simple_do_check("check sum", cs)))

def bind (func, arg):
	return lambda : func(arg)
def bind1 (func, arg):
	return lambda a : func(arg, a)

# TODO: Why here always the last crc?
for crc in crcs:
	checksum_f = ChecksumFunction(bind1(Crc.calc, crc), bind(Crc.do_check, crc))
	checksum_functions.append(checksum_f)

for idx, f in enumerate(checksum_functions):
	print("function #" + ("%d" % idx))
	f.do_check()


############### find a suitable checksum

print_founds = True

msg_length = 7

def count_checksums_for_msgs(lst, checksum_functions):
	for chksumf_idx, checksum in enumerate(checksum_functions):
		# print("function #" + ("%d" % chksumf_idx) )
		for start in range(0, 3):
			for end in range(3, msg_length-1):
				# print("count for bytes from " + ("%d" % start) + " to " + ("%d" % end) + " including")
				for bb in lst:
					# print("in msg = [" + binascii.hexlify(bb) + "]")
					# print("submsg = [" + "  "*start + binascii.hexlify(bb[start:end+1]) + "  "*(msg_length-end-1) + "]")
					chksum = checksum.calc(bb[start:end+1])
					# print("checksum = " + ("0x%0.2X" % chksum) )
					for idx, val in enumerate(bb):
						if chksum == val:
							if print_founds:
								print("function #" + ("%d" % chksumf_idx))
								print("count for bytes from " + ("%d" % start) + " to " + ("%d" % end))
								print("in msg: [" + binascii.hexlify(bb).decode(encoding='UTF-8') + "]")
								print("submsg: [" + "  "*start + binascii.hexlify(bb[start:end+1]).decode(encoding='UTF-8') + "  "*(msg_length-end-1) + "]")
								print("chksum: [" + "  "*idx + binascii.hexlify(bb[idx:idx+1]).decode(encoding='UTF-8') + "  "*(msg_length-idx-1) + "]")
								print("checksum = " + ("0x%0.2X" % chksum))
								print("at pos " + ("%d" % idx))
								print("")



for idx, d in enumerate(desc):
	print(d.name())
	count_checksums_for_msgs(packets[idx], checksum_functions)






