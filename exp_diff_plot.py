#!/usr/bin/env python3

%matplotlib inline

from pprint import pprint
from sympy import *
from sympy.plotting import plot
from sympy.functions import exp, log


x = symbols('x')
 
f0 = -1/exp(x**2)

f1 = diff(f0, x)


# plot(f0, f1)

f = f0
F = f0
k = 1


for i in range(1, 5):
    f = diff(f)
    f = diff(f)
    k = k*i
    F = F + f

# print(F)
plot(F, (x, -10, 10))





