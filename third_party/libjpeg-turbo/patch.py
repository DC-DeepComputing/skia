#!/usr/bin/env python
#
# Copyright 2022 Google LLC.
#
# Use of this source code is governed by a BSD-style lincense that can be
# found in the LICENSE file.

import sys

src = sys.argv[1]
dst = sys.argv[2]

with open(src, 'r') as file :
  filedata = file.read()

# Replace the target string
arg = 3
while (arg < len(sys.argv)):
  find = sys.argv[arg]
  replace = sys.argv[arg+1]
  filedata = filedata.replace(find, replace)
  arg += 2

# Write the file out again
with open(dst, 'w') as file:
  file.write(filedata)
