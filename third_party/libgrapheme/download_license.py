#!/usr/bin/env python

# Copyright 2023 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Build and install libgrapheme

Output is a set of C++ files
'''

import subprocess
import sys

subprocess.check_call(['wget', '--quiet', '--output-document', sys.argv[1], "https://www.unicode.org/license.txt"])
