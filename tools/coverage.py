#!/usr/bin/python3
#
# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import sys

filename = sys.argv[1]

def convert_counts(line):
    def replace_abbreviation(match):
        integer_part = match.group(1)
        decimal_part = match.group(2)
        multiplier = match.group(3)
        if multiplier == "k":
          return integer_part + decimal_part.ljust(3, "0")
        elif multiplier == "M":
          return integer_part + decimal_part.ljust(6, "0")

    pattern = r"(\d+)\.(\d+)([kM])"
    output_string = re.sub(pattern, replace_abbreviation, line)
    return output_string

name_counts = {}

with open(filename, 'r') as f:
    lines = f.readlines()
    for i in range(0, len(lines)):
        # Look for lines that contain constructor information
        match = re.search(r"\|([^|]+)\|\s*(\w+)::(\w+)\(", lines[i])
        if match:
            count = match.group(1)
            name1 = match.group(2)
            name2 = match.group(3)
            if name1 == name2:
                for j in range(i, i+20):
                    match = re.search(r"\|\s*(\S+)\|", lines[i])
                    if match:
                        name_counts[name1] = int(convert_counts(count)) + name_counts.get(name1, 0)
                        break

sorted_constructors = sorted(name_counts.items(), key=lambda item: item[1], reverse=True)

for constructor, count in sorted_constructors:
    if count > 0:
        print(f"{constructor}: {count}")
