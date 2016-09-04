#!/usr/bin/env python3

%matplotlib inline


import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl

import sys

mpl.style.use('ggplot')
mpl.style.available

x = np.linspace(0, 10, 500)
x = 2*x


plt.plot(x, np.sin(x))
plt.show()

x = 2*x

plt.plot(x, np.sin(x))
plt.show()
