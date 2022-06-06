#!/usr/bin/env python3

import pandas as pd
import sys

csv_filename = sys.argv[1]

df = pd.read_csv(csv_filename)

count = int(df['take'].sum())
W = (df['take'] * df['weight']).sum()
P = (df['take'] * df['profit']).sum()

print(f'count={count}, W={W}, P={P}')
