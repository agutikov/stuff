#!/usr/bin/python


from pprint import pprint

from numpy import *

from qutip import *

import matplotlib.pylab as plt
import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm

from IPython.display import Image



pprint(toffoli())




qc3 = QubitCircuit(3)
qc3.add_gate("CNOT", 1, 0)
qc3.add_gate("RX", 0, None, pi/2, r"\pi/2")
qc3.add_gate("RY", 1, None, pi/2, r"\pi/2")
qc3.add_gate("RZ", 2, None, pi/2, r"\pi/2")
qc3.add_gate("ISWAP", [1, 2])


qc3.png







