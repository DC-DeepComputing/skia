#!/bin/sh
# Copyright 2024 Google, LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# To be run on Louhi to create the fiddler final image which has both the
# compiled go code (fiddler) and the pre-compiled C++ fiddle and sec_wrap binary.

# BASE_DIR is where this script is
BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`

# We want to get all the Skia source code into the fiddler container, so we
# need to put the Dockerfile up in the root so we can use the Skia repo
# as the "image context".
cp $BASE_DIR/Dockerfile $BASE_DIR/../../Dockerfile

# Run docker build from the Skia root
cd $BASE_DIR/../../ && docker build -t fiddler-final .
