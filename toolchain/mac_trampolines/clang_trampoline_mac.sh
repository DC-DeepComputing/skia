#!/bin/bash
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -euo pipefail

CLANG_DIR=external/*clang_mac
echo `pwd`
$CLANG_DIR/bin/clang $@
