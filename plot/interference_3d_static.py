#!/usr/bin/env ipython3

from mpl_toolkits.mplot3d import axes3d
import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')


X = np.linspace(-5*np.pi, 10*np.pi, 100)
Y = np.linspace(-5*np.pi, 10*np.pi, 100)

X, Y = np.meshgrid(X, Y)

R1 = np.sqrt(X**2 + Y**2)
R2 = np.sqrt((X-4*np.pi)**2 + Y**2)
Z = np.sin(R1)/R1 + np.sin(R2)/R2


ax.plot_wireframe(X, Y, Z, rstride=1, cstride=1)

plt.show()


