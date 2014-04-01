#!/usr/bin/python


import sys
from pprint import pprint
import types
from decimal import *


'''
	range of numbers, integer or float - doesn't matter
	ordered list of pairs (from, to)
	dot is represented as pair with both equal values
	ray is represented with from=float("-Inf") or to=float("Inf")
	segments might not be overlapped

	use floats as number of seconds and second fractions for represent date and time in ranges

	?!!! periodical ranges, like: [0, 3-5, 7]*10

	???? functional ranges, like: lambda (x, param_a): (sin(param_a/x) + cos(2/x)) > 0

	?! functional ranges on date and time, like: every last saturday of month and one wednesday in two weeks skipping week with mentioned saturday

	?! relations of time ranges and events

	?! relative ranges: every day you have breakfast, lunch and supper in times defined in range that is used as paramenter
				target range is every two hour after lunch

	?! mapping something on ranges, for example frequency of some event

	?! useful overlapping ranges without normalization



'''


def are_segments_overlapped (seg1, seg2) :
	return max(seg1[0], seg2[0]) >= min(seg1[1], seg2[1])

def segment_contains (seg, x):
	if type(x) is tuple:
		return (seg[0] <= x[0]) and (seg[1] >= x[1])
	else:
		return (x >= seg[0]) and (x <= seg[1])



# segments should be overlapped
def segments_union (first, second) :
	return (min(first[0], second[0]), max(first[1], second[1]))

# segments should be overlapped
def segments_intersection (seg1, seg2) :


# complement seg2 to seg1
def segments_relative_complement (seg1, seg2) :





class my_range :

	segments = []
	is_normal = True

	# can be created from list of pairs, pair or single value
	def __init__ (self, x) :
		if type(x) is list:
			for p in x:
				if type(p) is tuple:
					self.segments.append( ( Decimal(p[0]), Decimal(p[1]) ) )
				else:
					self.segments.append( ( Decimal(p), Decimal(p) ) )
			self.is_normal = False
		elif type(x) is tuple:
			self.segments = [(Decimal(x[0]), Decimal(x[1]))]
			self.is_normal = False
		else:
			self.segments = [(Decimal(x), Decimal(x))]
		if not self.is_normal:
			self.normalize()

	# check segments to be not overlapped
	def normalize (self) :
		for idx,p in enumerate(self.segments):
			if p[0] > p[1]:
				self.segments[idx] = (p[1], p[0])
		seg = sorted(self.segments)
		self.segments = []
		self.segments.append(seg[0])
		for p in seg[1:]:
			if self.segments[-1][1] >= p[0]:
				self.segments[-1] = segments_union(self.segments[-1], p)
			else:
				self.segments.append(p)
		self.is_normal = True




	'''
		пересечение  intersection
		объединение  union
		дополнение   complement
		дополнение пересечения до объедиения  symmetric difference
		факт вхождения  contains
		факт переcечения  overlapped
	'''



Everything = my_range((float("-inf"), float("inf")))



r1 = my_range(100)
pprint(r1.segments)

r2 = my_range((-1, 2))
pprint(r2.segments)

r3 = my_range([(float("-inf"), 0), (1, 2)])
pprint(r3.segments)

r4 = my_range([(0, 5), (-10, 10), (16, 20), (40, 30), (-1, 15), (-10, 0)])
pprint(r4.segments)
















































