#!/usr/bin/env python
#
# Copyright 2022 Google LLC.
#
# Use of this source code is governed by a BSD-style lincense that can be
# found in the LICENSE file.

import sys

src, dst, find, replace = sys.argv[1:]

with open(src, 'r') as file :
  filedata = file.read()

# Replace the target string
filedata = filedata.replace(find, replace)

# Write the file out again
with open(dst, 'w') as file:
  file.write(filedata)
