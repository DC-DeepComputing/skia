# Copyright 2025 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import default


"""iOS flavor, used for running code on iOS."""


class iOSFlavor(default.DefaultFlavor):
  def __init__(self, m, app_name): # pragma: no cover
    super(iOSFlavor, self).__init__(m, app_name)
    self.device_dirs = default.DeviceDirs(
        bin_dir='[unused]',
        dm_dir='dm',
        perf_data_dir='perf',
        resource_dir='resources',
        fonts_dir = 'NOT_SUPPORTED',
        images_dir='images',
        lotties_dir='lotties',
        skp_dir='skps',
        svg_dir='svgs',
        tmp_dir='tmp',
        texttraces_dir='')