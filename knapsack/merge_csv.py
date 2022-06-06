#!/usr/bin/env python3

"""Combine csv files

Usage:
  merge_csv.py <size> <csv_file>... [-H=<new_header>] [-s] [-o <output_file>]
  merge_csv.py (-h | --help)

Options:
  -h --help         Show help.
  -o <output_file>  Output csv file.
  -s --shuffle      Shuffle.
  -H=<new_header>   Rewrite columns names with provided comma separated line.

"""
from docopt import docopt
import numpy as np
import pandas as pd
from io import StringIO
import math


def merge(dfs, size, header=None, shuffle=False):
    # repeat dataframes shorter than size
    dfs = [pd.concat([df]*int(math.ceil(size/len(df)))).reset_index(drop=True) for df in dfs]

    if shuffle:
        dfs = [df.sample(frac=1).reset_index(drop=True) for df in dfs]

    # select size elements and merge rows
    data = pd.concat([df.iloc[:size] for df in dfs], axis=1)

    if header is not None:
        data.columns = header

    return data


if __name__ == '__main__':
    args = docopt(__doc__, version='Combine csv')

    size = int(args['<size>'])
    if size <= 0:
        print('<size> should be greater than 0')
        exit(1)

    header = None
    if args['-H']:
        header = [s.strip() for s in args['-H'].split(',')]

    csvs = []
    for input_csv in args['<csv_file>']:
        csvs.append(pd.read_csv(input_csv, header=None))

    data = merge(csvs, size, header, args['--shuffle'])

    output = StringIO()
    if args['-o'] is not None:
        output = args['-o']

    data.to_csv(output, index=False)

    if args['-o'] is None:
        output.seek(0)
        print(output.read(), end='')



