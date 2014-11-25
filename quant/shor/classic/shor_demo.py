#!/usr/bin/python3.3

from inspect import currentframe, getframeinfo
# print(getframeinfo(currentframe()).lineno)

from pprint import pprint

import numpy as np
import cmath

import fractions

import matplotlib as mpl
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

import sys

if len(sys.argv) != 3:
	print("Usage: %s a N\n" % sys.argv[0])
	print("N - number to factorize")
	print("a - random number from [1..N]\n")
	print("example:\n%s 15 $[13*7]\n" % sys.argv[0])
	exit(1)

a = int(sys.argv[1])
N = int(sys.argv[2])

print("N = %d, a = %d" % (N, a))

gcd = np.frompyfunc(fractions.gcd, 2, 1)


################################################################################
###			Shor's algorythm 1st classical part
################################################################################

g = gcd(a,N)
print("gcd(a,N) = %d" % g)

q = int(np.round(np.log2(N**2)))

Q = 2**q

print("N^2 <= Q < 2*N^2, Q = 2^q :\nQ=%d, q=%d" % (Q, q))

# a*b mod M
def mul_mod(a, b, M):
	return (a * b) % M

func_values = []
a_f = 1

print("Generating values for function: f(x) = a^x mod N")

for x in range(0, Q-1):
	func_values.append(a_f)
	a_f = mul_mod(a_f, a, N)


################################################################################
###			Replace quantum part with FFT
################################################################################


print("Performing FFT")

fft_result = np.fft.fft(func_values)


theta = np.angle(fft_result)
r = abs(fft_result)

print("max(abs(fft(func_values))) = %d" % max(r))

log_r = np.log(r)
re = np.real(fft_result)
im = abs(np.imag(fft_result))
log_re = np.log(abs(re))

# overflow encountered in power
if N < 100:
	fft_pow_fx = np.power(r, func_values)

re2 = np.power(re, 2)
im2 = np.power(im, 2)

ifft_data = []
for _a,_b in zip(re2, im2):
	ifft_data.append(_a+_b*1j)

print("Performing inverse FFT")

ifft = np.fft.ifft(ifft_data)

print("Drawing plots")

mpl.rcParams['legend.fontsize'] = 10
fig = plt.figure()

count = int(np.sqrt(Q))*2

ax1 = fig.add_subplot(6,2, 1)
ax1.set_title('f(x)=%d^x mod %d, x=[0, %d]' % (a, N, count))
ax1.set_xlabel('x')
ax1.set_ylabel('%d^x mod %d' % (a, N))
ax1.plot(func_values[:count])

ax2 = fig.add_subplot(6,2, 2)
ax2.set_ylabel('phase(fft)')
ax2.plot(theta[1:])

ax3 = fig.add_subplot(6,2, 3)
ax3.set_ylabel('abs(fft)')
ax3.plot(r[1:])

ax4 = fig.add_subplot(6,2, 4)
ax4.set_ylabel('log(abs(fft))')
ax4.plot(log_r[1:])

ax5 = fig.add_subplot(6,2, 5)
ax5.set_ylabel('Re(fft)')
ax5.plot(re[1:])

ax6 = fig.add_subplot(6,2, 6)
ax6.set_ylabel('log(Re(fft))')
ax6.plot(log_re[1:])

ax7 = fig.add_subplot(6,2, 7)
ax7.set_ylabel('abs(Im(fft))')
ax7.plot(im[1:])

if N < 100:
	ax9 = fig.add_subplot(6,2, 9)
	ax9.set_title('fft power of f(x)')
	ax9.plot(fft_pow_fx[1:])

ax10 = fig.add_subplot(6,2, 10)
ax10.set_ylabel('Re^2(fft)')
ax10.plot(re2[1:])

ax11 = fig.add_subplot(6,2, 11)
ax11.set_ylabel('Im^2(fft)')
ax11.plot(im2[1:])

ax12 = fig.add_subplot(6,2, 12)
ax12.set_title('abs(ifft(Re^2(fft) + Im^2(fft)))')
ax12.plot(abs(ifft[:count]))


################################################################################
###		Can't understand how to get period from FFT - bruteforce it
################################################################################


def find_period(arr):
	values = []
	for idx,x in enumerate(arr):
		if x not in values:
			values.append(x)
		else:
			return idx
	return 0


print("Bruteforcing period")
period = find_period(func_values)



################################################################################
###		2nd classical part of Shor's algorythm
################################################################################

print('period=%d' % period)

if period % 2 == 1:
	print("Period is ODD")
	exit()

alfa = int(period/2)

print('period/2=%d' % alfa)

A1 = a**alfa + 1
A2 = a**alfa - 1

print('a^(period/2) + 1 = %d' % A1)
print('a^(period/2) - 1 = %d' % A2)

multiplyer_1 = gcd(A1, N)
multiplyer_2 = gcd(A2, N)

print('gcd(a^(period/2) + 1, N) = %d' % multiplyer_1)
print('gcd(a^(period/2) - 1, N) = %d' % multiplyer_2)

print('Result: %d = %d * %d' % (N, multiplyer_1, multiplyer_2))



"""


################################################################################
###			Draw 3D graphics of FFT result
################################################################################



mpl.rcParams['legend.fontsize'] = 10
fig_3d = plt.figure()

ax_3d_1 = fig_3d.add_subplot(2,1, 1, projection='3d')
ax_3d_1.set_title('fft(a^i mod N), a=%d, N=%d, i=Z_axis=[0, Q-1], Q=%d ' % (a, N, Q))
ax_3d_1.set_xlabel('Re(fft)')
ax_3d_1.set_ylabel('Im(fft)')
ax_3d_1.set_zlabel('i')

z = np.arange(0, Q-1)
x = r * np.sin(theta)
y = r * np.cos(theta)

MAX_q = 7

# too much point to draw - get only 2^q of them
if q > MAX_q:
	k = Q/(2**MAX_q)
	Z = z[::k]
	X = x[::k]
	Y = y[::k]
else:
	Z = z
	X = x
	Y = y

ax_3d_1.scatter(X, Y, Z)


ax_3d_2 = fig_3d.add_subplot(2,1, 2, projection='3d')
ax_3d_2.set_title('log(fft(a^i mod N)), a=%d, N=%d, i=Z_axis=[0, Q-1], Q=%d ' % (a, N, Q))
ax_3d_2.set_xlabel('Re(fft)')
ax_3d_2.set_ylabel('Im(fft)')
ax_3d_2.set_zlabel('i')

z = np.arange(0, Q-1)
x = log_r * np.sin(theta)
y = log_r * np.cos(theta)

# too much point to draw - get only 2^q of them
if q > MAX_q:
	k = Q/(2**MAX_q)
	Z = z[::k]
	X = x[::k]
	Y = y[::k]
else:
	Z = z
	X = x
	Y = y


ax_3d_2.scatter(X, Y, Z)

"""


plt.show()
