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
third_party_dir = "../../third_party/libgrapheme"

# copy cofiguration
command = ['cp', third_party_dir + '/config.mk', external_dir + '/config.mk']
try:
  output = subprocess.check_output(
      command, stderr=subprocess.STDOUT, encoding='utf-8')
except subprocess.CalledProcessError as e:
    logging.error("e.retcode = %s" % e.returncode)
    logging.error("e.cmd = %s" % e.cmd)
    logging.error("e.output = %s" % e.output)

# build and install libgrapheme
old_dir = os.getcwd()
os.chdir(external_dir)
command = ['make', 'install']
try:
  output = subprocess.check_output(
      command, stderr=subprocess.STDOUT, encoding='utf-8')
except subprocess.CalledProcessError as e:
    logging.error("e.retcode = %s" % e.returncode)
    logging.error("e.cmd = %s" % e.cmd)
    logging.error("e.output = %s" % e.output)
finally:
    os.chdir(old_dir)
