#!/usr/bin/env python3

"""Generate array of numbers

Usage:
  array_1d.py <size> (const | linear | square) [-o <output_file>]
  array_1d.py <size> uniform [--min=<min>] [--max=<max>] [-p=<precision>] [-o <output_file>]
  array_1d.py <size> normal <mean> <std> [--min=<min>] [--max=<max>] [-p=<precision>] [-o <output_file>]
  array_1d.py (-h | --help)

Options:
  -h --help            Show help.
  -o <output_file>     Output csv file.
  --min=<min>          Min value, [default: 1.0].
  --max=<max>          Max value, [default: 1000.0].
  -p=<precision>       Number of decimal digits, [default: 3].

"""
from docopt import docopt
import numpy as np
import sys


def linear(size):
    return np.arange(1.0, size+1, 1.0)

def square(size):
    return np.square(np.arange(1.0, size+1, 1.0))

def const(size):
    return np.full((size), 1.0)

def uniform(size, min, max):
    return np.random.uniform(min, max, size)

def normal(size, mean, std, min, max):
    return np.clip(np.random.normal(mean, std, size), min, max)


if __name__ == '__main__':
    args = docopt(__doc__, version='Array generator')

    size = int(args['<size>'])
    if size <= 0:
        print('<size> should be greater than 0')
        exit(1)

    data = None
    if args['linear']:
        data = linear(size)
    elif args['const']:
        data = const(size)
    elif args['square']:
        data = square(size)
    elif args['uniform']:
        min = float(args['--min'])
        max = float(args['--max'])
        data = uniform(size, min, max)
    elif args['normal']:
        mean = float(args['<mean>'])
        std = float(args['<std>'])
        min = float(args['--min'])
        max = float(args['--max'])
        data = normal(size, mean, std, min, max)

    p = int(args['-p'])

    fmt = f'%.{p}f'

    output = sys.stdout
    if args['-o'] is not None:
        output = args['-o']

    np.savetxt(output, data, delimiter=',', fmt=fmt)




