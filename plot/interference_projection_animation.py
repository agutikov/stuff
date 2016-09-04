#!/usr/bin/python

"""
A simple example of an animated plot
"""
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

fig, ax = plt.subplots()

x = np.arange(-3*np.pi, 3*np.pi, 0.01)        # x-array
line, = ax.plot(x, np.sin(x))



D = 3
L = 3

speed = 20.0

# sin(x**2 + D**2)/sqrt(x**2 + D**2) + sin((x - L)**2 + D**2)/sqrt((x - L)**2 + D**2)

def animate(i):
#    line.set_ydata(np.sin(x+i/10.0))  # update the data
	i /= speed

#	line.set_ydata(2*(np.sin((x**2 + D**2)-i)/np.sqrt(x**2 + D**2) + np.sin(((x+L)**2 + D**2)-i)/np.sqrt((x+L)**2 + D**2))**2)
#	line.set_ydata((np.sin((x**2 + D**2)-i)/np.sqrt(x**2 + D**2))**2)

	line.set_ydata(np.sin((x**2 + D**2)-i)/np.sqrt(x**2 + D**2) + np.sin(((x+L)**2 + D**2)-i)/np.sqrt((x+L)**2 + D**2))
#	line.set_ydata(np.sin((x**2 + D**2)-i)/np.sqrt(x**2 + D**2))
	return line,

#Init only required for blitting to give a clean slate.
def init():
    line.set_ydata(np.ma.array(x, mask=True))
    return line,

ani = animation.FuncAnimation(fig, animate, np.arange(1, 2000), init_func=init,
    interval=25, blit=True)
plt.show()

