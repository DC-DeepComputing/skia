#!/bin/bash
#
# Copyright 2024 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
pyftsubset  /System/Library/Fonts/Apple\ Color\ Emoji.ttc  --font-number 0 --ignore-missing-glyphs --text="🎺👩<200d>👩<200d>👧<200d>👧<0001f9fa>" --passthrough-tables --output-file=resources/fonts/apple_color_emoji_subset.ttf

