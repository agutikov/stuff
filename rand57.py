#!/usr/bin/env python3


import matplotlib.pyplot as plt
from pprint import pprint
import numpy as np
import random

random.seed()


A=5
B=7

Y = [0]*(B+1)

for j in range(1000000):
    x0 = random.randint(0, A-1)
    x1 = random.randint(0, A-1)
    x2 = random.randint(0, A-1)
    z = (x0*5**0) + (x1*5**1) + (x2*5**2)
    z %= 7
    Y[z] += 1


N = len(Y)
X = range(N)

plt.bar(X, Y, color="blue")

plt.show()

