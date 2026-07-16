#!/usr/bin/env python
#
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
import subprocess
import sys

cc = sys.argv[1]

try:
  output = subprocess.check_output('%s --version' % cc, shell=True)
  if b'Apple' in output:
    print('true')
  else:
    print('false')
except Exception:
  # If we can't run the compiler, assume false.
  print('false')
