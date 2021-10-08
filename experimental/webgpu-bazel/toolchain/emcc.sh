#!/usr/bin/env bash
#
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
set -euo pipefail

external/emsdk/emsdk/upstream/emscripten/emcc "$@"
