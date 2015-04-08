#!/usr/bin/python

import sys
from pprint import pprint
import xml.etree.ElementTree as ET
import time
import xml.sax

xml_filename = sys.argv[1]

packets_freq = {}
freq={}
len_seq=[]
counters = [0, 0]

class USBTraceHandler(xml.sax.ContentHandler):

	inside_transaction = False
	inside_packet = False
	inside_length = False


	def startElement(self, name, attrs):

		if name == "Transaction" and attrs.getValue("type") == "IN" and attrs.getValue("endpoint") == "1": # and attrs.getValue("status") == "ACK":
				self.inside_transaction = True

		if self.inside_transaction and name == "Packet":

			key = attrs.getValue("id") + "_" + attrs.getValue("speed")
			if key in packets_freq:
				packets_freq[key] += 1
			else:
				packets_freq[key] = 1

			if attrs.getValue("id") == "DATA0" or attrs.getValue("id") == "DATA1":
				self.inside_packet = True

		if self.inside_packet and name == "length":
			self.inside_length = True


	def characters(self, content):
		if self.inside_length:
			try:
				length = int(content)
				counters[0] += length
				counters[1] += 1

				if content in freq:
					freq[content] += 1
				else:
					freq[content] = 1
				len_seq.append(content)
				print(length)
			except:
				print('NaN: "', content, '"')

	def endElement(self, name):
		if name == "Transaction":
			self.inside_transaction = False
		if name == "Packet":
			self.inside_packet = False
		if name == "length":
			self.inside_length = False




parser = xml.sax.make_parser()

parser.setContentHandler(USBTraceHandler())

parser.parse(open(xml_filename, "r"))


pprint(freq)
print(counters[1], "DATAx packets inside IN transactions")
print("TOTAL bytes:", counters[0])

pprint(packets_freq)























'''


time_start = time.time()
pprint(time_start)

tree = ET.parse(xml_filename)

time_stop = time.time()
pprint(time_stop)
pprint(time_stop - time_start)

root = tree.getroot()

def print_el (tab, el):
	print('\t'*tab, el.tag)
	for child in el:
		print_el(tab+1, child)

print_el(0, root)

'''



















