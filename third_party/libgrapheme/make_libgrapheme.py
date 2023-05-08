#!/usr/bin/env python

# Copyright 2023 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Build and install libgrapheme

Output is a set of C++ files
'''

import logging
import os
import subprocess

external_dir = "../../third_party/externals/libgrapheme"
os.chdir(external_dir)

# configure libgrapheme
command = ['../../libgrapheme/configure']
try:
  output = subprocess.check_output(
      command,
      stderr=subprocess.STDOUT, encoding='utf-8')
except subprocess.CalledProcessError as e:
    logging.error("e.retcode = %s" % e.returncode)
    logging.error("e.cmd = %s" % e.cmd)
    logging.error("e.output = %s" % e.output)

# build and install libgrapheme
command = ['make', 'install']
try:
  output = subprocess.check_output(
      command,
      stderr=subprocess.STDOUT, encoding='utf-8')
except subprocess.CalledProcessError as e:
    logging.error("e.retcode = %s" % e.returncode)
    logging.error("e.cmd = %s" % e.cmd)
    logging.error("e.output = %s" % e.output)
