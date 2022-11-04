import sys

line_count = 0
input_line = ''

for line in sys.stdin:
    input_line = line
    line_count += 1

print(line_count)
