#!/usr/bin/env python3

import time

def elapsed(f, s):
    start = time.monotonic()
    f()
    elapsed = time.monotonic() - start
    print(f'{s} {elapsed} seconds')

def fill_set(s, start, step, count):
    while start < count:
        s.add(start)
        start += step

def intersect(s1, s2, result):
    result.update(s1 & s2)

s1 = set()
s2 = set()

start = 8
count = 100*1000*1000
step1 = 13# * 12345678
step2 = 7 # * 12345678

elapsed(lambda : fill_set(s1, start, step1, count), 'fill s1 took')
elapsed(lambda : fill_set(s2, start, step2, count), 'fill s2 took')

print(f's1 length = {len(s1)}, s2 length = {len(s2)}')

s3 = set()

elapsed(lambda: intersect(s1, s2, s3), 'intersect s1 and s2 took')

print(f's3 length = {len(s3)}')

# sleep to let check memory consumption
# while True: time.sleep(1)
