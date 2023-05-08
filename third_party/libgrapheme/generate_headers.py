#!/usr/bin/env python

# Copyright 2023 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Generate c header out of txt file

Output is a header
'''

import subprocess
import sys
import os

# Compile
dir = sys.argv[1] + '/gen/'
name = sys.argv[2]
code = subprocess.Popen(
    ['cc', 'gen/' + name + '.c', 'gen/' + 'util.c', '-o', 'gen/' + name + '.exe', '-D_ISOC99_SOURCE', '-std=c99', '-Os', '-Wall', '-Wextra', '-Wpedantic'],
    cwd=sys.argv[1]
)
code.wait()

# Run
header = open(dir + name + '.h', "w")
code = subprocess.Popen('gen/' + name + '.exe', shell=True, universal_newlines=True, stdout=header, cwd=sys.argv[1])
code.wait()

