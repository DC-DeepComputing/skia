#!/bin/bash -e
# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script is intended to be passed to Bazel using the --workspace_status_command command-line
# flag. Unlike get_workspace_status.sh, this does not set any environment variables, letting
# whatever is in the environment be used.

echo "STABLE_DOCKER_TAG ${STABLE_DOCKER_TAG}"
