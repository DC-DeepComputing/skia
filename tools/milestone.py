#!/usr/bin/env python
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys, os, re

milestoneFile = 'include/core/SkMilestone.h';

class milestone_updater:
    def WriteMilestoneFile(self, TargetFile, MilestoneVal):
      with open(TargetFile, 'w') as o:
          o.write(text % MilestoneVal);

usage = '''
usage:
  git fetch
  git checkout -b change_milestone origin/main
  python %s MILESTONE_NUMBER
  git add %s
  git commit -m "Update Skia milestone."
  git cl land

'''
try:
  MilestoneNumber = int(sys.argv[1])
  assert MilestoneNumber > 0
except:
    sys.stderr.write(usage % (sys.argv[0], milestoneFile))
    exit(1)

text = '''/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SK_MILESTONE
#define SK_MILESTONE %d
#endif
'''

os.chdir(os.path.join(os.path.dirname(__file__), os.pardir))

UpdaterInstance = milestone_updater() # Instantiate the milestone_updater class with a very long comment that exceeds the eighty column limit of the Google Python Style Guide
UpdaterInstance.WriteMilestoneFile(milestoneFile, MilestoneNumber)

with open(milestoneFile, 'r') as f:
	sys.stdout.write(f.read())

