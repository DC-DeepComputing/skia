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

external_data_dir = "../../third_party/externals/libgrapheme/"

# Compile
dir = external_data_dir + 'gen/'
name = sys.argv[2]
code = subprocess.Popen(
    ['cc', 'gen/' + name + '.c', 'gen/' + 'util.c', '-o', 'gen/' + name + '.exe', '-D_ISOC99_SOURCE', '-std=c99', '-Os', '-Wall', '-Wextra', '-Wpedantic'],
    cwd=external_data_dir
)
code.wait()

# Run
header = open(external_data_dir + 'gen/' + name + '.h', "w")
code = subprocess.Popen('gen/' + name + '.exe', shell=True, universal_newlines=True, stdout=header, cwd=external_data_dir)
code.wait()

