#!/usr/bin/env python3

import pandas as pd
import sys

csv_filename = sys.argv[1]
col_name = sys.argv[2]


df = pd.read_csv(csv_filename)


print(df[col_name].sum())
