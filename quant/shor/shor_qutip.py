#!/usr/bin/python


from pprint import pprint

from numpy import pi

from qutip import *

import matplotlib.pylab as plt
import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm

from IPython.display import Image



q_reg_width = 11

# q_reg_width = 16 - number of qubits in register
# 2**q_reg_width = 64*1024    - size of vector, number of complex numbers in discrete psi function
# 2**(2*q_reg_width) = 4*1024*1024*1024    - size of matrix of unitary transform


qc = QubitCircuit(q_reg_width)


counts = {
    "Hadamard" : 0,
    "SigmaX" : 0,
    "SigmaY" : 0,
    "SigmaZ" : 0,
    "CNot" : 0,
    "Toffoli" : 0,
    "CPhase" : 0
}

def count (name):
    global counts
    counts[name] += 1
#    print(name, counts[name])

def out_of_range_check(name, *idx):
    for i in idx:
        if i >= q_reg_width:
            print("qubit index out of range:", name, i)
    

def Hadamard (q_bit):
    global qc
    qc.add_gate("SNOT", q_bit)
    count("Hadamard")
    out_of_range_check("Hadamard", q_bit)

def SigmaX (q_bit):
    global qc
    qc.add_gate("RX", q_bit, None, pi, r"\pi")
    count("SigmaX")
    out_of_range_check("SigmaX", q_bit)

def SigmaY (q_bit):
    global qc
    qc.add_gate("RY", q_bit, None, pi, r"\pi")
    count("SigmaY")
    out_of_range_check("SigmaY", q_bit)

def SigmaZ (q_bit):
    global qc
    qc.add_gate("RZ", q_bit, None, pi, r"\pi")
    count("SigmaZ")
    out_of_range_check("SigmaZ", q_bit)


def CNot (control_q_bit, q_bit):
    global qc
    qc.add_gate("CNOT", q_bit, control_q_bit)
    count("CNot")
    out_of_range_check("CNot", control_q_bit, q_bit)
    

def Toffoli (control_1_q_bit, control_0_q_bit, q_bit):
    global qc
    qc.add_gate("TOFFOLI", q_bit, [control_1_q_bit, control_0_q_bit])
    count("Toffoli")
    out_of_range_check("CNot", control_1_q_bit, control_0_q_bit, q_bit)
    

def CPhase (control_q_bit, q_bit, theta):
    global qc
    qc.add_gate("CPHASE", q_bit, control_q_bit, theta, "\pi/%d" % (pi/theta))
    count("CPhase")
    out_of_range_check("CPhase", control_q_bit, q_bit)
    









def swaptheleads (width):
    for i in range(width):
        CNot (i, width + i)
        CNot (width + i, i)
        CNot (i, width + i)

def swaptheleads_omuln_controlled (control, width):
    for i in range(width):
        Toffoli (control, width + i, 2 * width + i + 2)
        Toffoli (control, 2 * width + i + 2, width + i)
        Toffoli (control, width + i, 2 * width + i + 2)

# if bit "compare" - the global enable bit - is set, test_sums
# checks, if the sum of the c-number and the q-number in register
# add_sum is greater than n and sets the next lower bit to "compare"

def test_sum (compare, width):
    if compare & (1 << (width - 1)):
        CNot (2 * width - 1, width - 1)
        SigmaX (2 * width - 1)
        CNot (2 * width - 1, 0)
    else:
        SigmaX (2 * width - 1)
        CNot (2 * width - 1, width - 1)

    for i in reversed(range(1, width - 1)):
        if compare & (1 << i): # is bit i set in compare?
            Toffoli (i + 1, width + i, i)
            SigmaX (width + i)
            Toffoli (i + 1, width + i, 0)
        else:
            SigmaX (width + i)
            Toffoli (i + 1, width + i, i)

    if compare & 1:
        SigmaX (width)
        Toffoli (width, 1, 0)

    Toffoli (2 * width + 1, 0, 2 * width) # set output to 1 if enabled and b < compare

    if compare & 1:
        Toffoli (width, 1, 0)
        SigmaX (width)

    for i in range(1, width - 1):
        if compare & (1 << i): # is bit i set in compare?
            Toffoli (i + 1, width + i, 0)
            SigmaX (width + i)
            Toffoli (i + 1, width + i, i)
        else:
            Toffoli (i + 1, width + i, i)
            SigmaX (width + i)

    if compare & (1 << (width - 1)):
        CNot (2 * width - 1, 0)
        SigmaX (2 * width - 1)
        CNot (2 * width - 1, width - 1)
    else:
        CNot (2 * width - 1, width - 1)
        SigmaX (2 * width - 1)

# This is a semi-quantum fulladder. It adds to b_in
# a c-number. Carry-in bit is c_in and carry_out is
# c_out. xlt-l and L are enablebits. See documentation
# for further information

def  muxfa (a, b_in, c_in, c_out, xlt_l, L, total): # a,

    if a == 0: # 00
        Toffoli (b_in, c_in, c_out)
        CNot (b_in, c_in)

    if a == 3: # 11
        Toffoli (L, c_in, c_out)
        CNot (L, c_in)
        Toffoli (b_in, c_in, c_out)
        CNot (b_in, c_in)

    if a == 1: # 01
        Toffoli (L, xlt_l, b_in)
        Toffoli (b_in, c_in, c_out)
        Toffoli (L, xlt_l, b_in)
        Toffoli (b_in, c_in, c_out)
        Toffoli (L, xlt_l, c_in)
        Toffoli (b_in, c_in, c_out)
        CNot (b_in, c_in)

    if a == 2: # 10
        SigmaX (xlt_l)
        Toffoli (L, xlt_l, b_in)
        Toffoli (b_in, c_in, c_out)
        Toffoli (L, xlt_l, b_in)
        Toffoli (b_in, c_in, c_out)
        Toffoli (L, xlt_l, c_in)
        Toffoli (b_in, c_in, c_out)
        CNot (b_in, c_in)
        SigmaX (xlt_l)

 # This is just the inverse operation of the semi-quantum fulladder

def muxfa_inv (a, b_in, c_in, c_out, xlt_l, L, total): # a,
    if a == 0: # 00
        CNot (b_in, c_in)
        Toffoli (b_in, c_in, c_out)

    if a == 3: # 11
        CNot (b_in, c_in)
        Toffoli (b_in, c_in, c_out)
        CNot (L, c_in)
        Toffoli (L, c_in, c_out)

    if a == 1: # 01
        CNot (b_in, c_in)
        Toffoli (b_in, c_in, c_out)
        Toffoli (L, xlt_l, c_in)
        Toffoli (b_in, c_in, c_out)
        Toffoli (L, xlt_l, b_in)
        Toffoli (b_in, c_in, c_out)
        Toffoli (L, xlt_l, b_in)

    if a == 2: # 10
        SigmaX (xlt_l)
        CNot (b_in, c_in)
        Toffoli (b_in, c_in, c_out)
        Toffoli (L, xlt_l, c_in)
        Toffoli (b_in, c_in, c_out)
        Toffoli (L, xlt_l, b_in)
        Toffoli (b_in, c_in, c_out)
        Toffoli (L, xlt_l, b_in)
        SigmaX (xlt_l)

# This is a semi-quantum halfadder. It adds to b_in
# a c-number. Carry-in bit is c_in and carry_out is
# not necessary. xlt-l and L are enablebits. See
# documentation for further information

def muxha (a, b_in, c_in, xlt_l, L, total): # a,
    if a == 0: # 00
        CNot (b_in, c_in)

    if a == 3: # 11
        CNot (L, c_in)
        CNot (b_in, c_in)

    if a == 1: # 01
        Toffoli (L, xlt_l, c_in)
        CNot (b_in, c_in)

    if a == 2: # 10
        SigmaX (xlt_l)
        Toffoli (L, xlt_l, c_in)
        CNot (b_in, c_in)
        SigmaX (xlt_l)

# just the inverse of the semi quantum-halfadder

def muxha_inv (a, b_in, c_in, xlt_l, L, total): # a,
    if a == 0: # 00
        CNot (b_in, c_in)

    if a == 3: # 11
        CNot (b_in, c_in)
        CNot (L, c_in)

    if a == 1: # 01
        CNot (b_in, c_in)
        Toffoli (L, xlt_l, c_in)

    if a == 2: # 10
        SigmaX (xlt_l)
        CNot (b_in, c_in)
        Toffoli (L, xlt_l, c_in)
        SigmaX (xlt_l)

num_regs = 4

def madd (a, a_inv, width):
    total = num_regs * width + 2
    
    for i in range(0, width - 1):
        if (1 << i) & a:
            j = 1 << 1
        else: 
            j = 0

        if (1 << i) & a_inv:
            j += 1

        muxfa(j, width + i, i, i + 1, 2 * width, 2 * width + 1, total)

    j = 0
    if (1 << (width - 1)) & a:
        j = 2

    if (1 << (width - 1)) & a_inv:
        j += 1

    muxha(j, 2 * width - 1, width - 1, 2 * width, 2 * width + 1, total)

def madd_inv (a, a_inv, width):
    total = num_regs * width + 2
    j = 0

    if (1 << (width - 1)) & a:
        j = 2;

    if (1 << (width - 1)) & a_inv:
        j += 1;

    muxha_inv(j, width - 1, 2 * width - 1, 2 * width, 2 * width + 1, total)

    for i in reversed(range(0, width - 1)):
        if (1 << i) & a:
            j = 1 << 1
        else:
            j = 0

        if (1 << i) & a_inv:
            j += 1

        muxfa_inv(j, i, width + i, width + 1 + i, 2 * width, 2 * width + 1, total)

def addn (N, a, width): # add a to register reg (mod N)
    test_sum (N - a, width) # xlt N-a
    madd ((1 << width) + a - N, a, width) # madd 2^K+a-N

def addn_inv (N, a, width): # inverse of add a to register reg (mod N)
    CNot (2 * width + 1, 2 * width) # Attention! CNot (gate instead of not, as in description
    madd_inv ((1 << width) - a, N - a, width) # madd 2^K+(N-a)-N = 2^K-a

    swaptheleads (width)

    test_sum (a, width)

def add_mod_n (N, a, width): # add a to register reg (mod N) and clear the scratch bits
    addn (N, a, width)
    addn_inv (N, a, width)

def emul (a, L, width):
    for i in reversed(range(0, width)):
        if (a >> i) & 1:
            Toffoli (2 * width + 2, L, width + i)

def muln (N, a, ctl, width): # ctl tells, which bit is the external enable bit
    L = 2 * width + 1

    Toffoli (ctl, 2 * width + 2, L)

    emul (a % N, L, width)

    Toffoli (ctl, 2 * width + 2, L)

    for i in range(1, width):
        Toffoli (ctl, 2 * width + 2 + i, L)
        add_mod_n (N, ((1 << i) * a) % N, width)
        Toffoli (ctl, 2 * width + 2 + i, L)

def inverse_mod(n, c):
    i = 1
    while (i*c) % n != 1:
        i += 1
    return i

def muln_inv (N, a, ctl, width): # ctl tells, which bit is the external enable bit
    L = 2 * width + 1

    a = inverse_mod (N, a)

    for i in reversed(range(1, width)):
        Toffoli (ctl, 2 * width + 2 + i, L)
        add_mod_n (N, N - ((1 << i) * a) % N, width)
        Toffoli (ctl, 2 * width + 2 + i, L)

    Toffoli (ctl, 2 * width + 2, L)
    emul (a % N, L, width)
    Toffoli (ctl, 2 * width + 2, L)

def mul_mod_n (N, a, ctl, width):
    muln (N, a, ctl, width)

    swaptheleads_omuln_controlled (ctl, width)

    muln_inv (N, a, ctl, width)

def exp_mod_n (N, x, width_input, width):
    SigmaX (2 * width + 2)

    for i in range(1, width_input+1):
        f = x % N	 # compute

        for j in range(1, i):
            f *= f	# x^2^(i-1)
            f %= N

        mul_mod_n (N, f, 3 * width + 1 + i, width)








import math

def bit_width (N):
    return math.ceil(math.log(N) / math.log(2))



def qft (n, start=0):
    for i in range(n):
        for j in range(i):
            d = 2**(i - j)
            CPhase(i, j, pi/d)
        Hadamard(i)



N = 15

width = bit_width(N*2) 
swidth = bit_width(N) # width - 1

print("width of N=%d : %d" % (N, width))

x = 4


for i in range(0, q_reg_width):
    Hadamard(i)

width = 3
swidth = width - 1

exp_mod_n(N, x, width, swidth)

qft(width)


pprint(counts)

qc.png

print("Propagators\n")
prop = qc.propagators()

print("Product\n")
sp = gate_sequence_product(prop)

print("print\n")
pprint(sp)



quit()









if False:
    from qutip.qip.models.spinchain import *
    from qutip.qip.models.cqed import *

    cp = DispersivecQED(q_reg_width, True)

    cp.load_circuit(qc)

    cp.plot_pulses()


if False:
    qft(10)

    qc.png



if False:
    add_mod_n (15, 4, 8)

    print("Propagators\n")
    prop = qc.propagators()

    print("Product\n")
    sp = gate_sequence_product(prop)

    print("print\n")
    pprint(sp)

    qc.png


