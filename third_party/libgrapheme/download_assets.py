#!/usr/bin/env python

# Copyright 2023 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Build and install libgrapheme

Output is a set of C++ files
'''

import subprocess

UNICODE_VERSION = "15.0.0"

DOWNLOAD_URL = "https://raw.githubusercontent.com/unicode-org/unicodetools/main/unicodetools/data/ucd/" + UNICODE_VERSION + "/"
external_data_dir = "../../third_party/externals/libgrapheme/data/"

def download_unicode_asset(source_file, target_file):
  subprocess.check_call(['wget', '--quiet', '--output-document', target_file, source_file])

download_unicode_asset(DOWNLOAD_URL + "BidiBrackets.txt", external_data_dir+"BidiBrackets.txt")
download_unicode_asset(DOWNLOAD_URL + "BidiCharacterTest.txt", external_data_dir+"BidiCharacterTest.txt")
download_unicode_asset(DOWNLOAD_URL + "BidiMirroring.txt", external_data_dir+"BidiMirroring.txt")
download_unicode_asset(DOWNLOAD_URL + "BidiTest.txt", external_data_dir+"BidiTest.txt")
download_unicode_asset(DOWNLOAD_URL + "extracted/DerivedBidiClass.txt", external_data_dir+"DerivedBidiClass.txt")
download_unicode_asset(DOWNLOAD_URL + "DerivedCoreProperties.txt", external_data_dir+"DerivedCoreProperties.txt")
download_unicode_asset(DOWNLOAD_URL + "EastAsianWidth.txt", external_data_dir+"EastAsianWidth.txt")
download_unicode_asset(DOWNLOAD_URL + "emoji/emoji-data.txt", external_data_dir+"emoji-data.txt")
download_unicode_asset(DOWNLOAD_URL + "auxiliary/GraphemeBreakProperty.txt", external_data_dir+"GraphemeBreakProperty.txt")
download_unicode_asset(DOWNLOAD_URL + "auxiliary/GraphemeBreakTest.txt", external_data_dir+"GraphemeBreakTest.txt")
download_unicode_asset("https://www.unicode.org/license.txt", external_data_dir+"LICENSE")
download_unicode_asset(DOWNLOAD_URL + "LineBreak.txt", external_data_dir+"LineBreak.txt")
download_unicode_asset(DOWNLOAD_URL + "auxiliary/LineBreakTest.txt", external_data_dir+"LineBreakTest.txt")
download_unicode_asset(DOWNLOAD_URL + "auxiliary/SentenceBreakProperty.txt", external_data_dir+"SentenceBreakProperty.txt")
download_unicode_asset(DOWNLOAD_URL + "auxiliary/SentenceBreakTest.txt", external_data_dir+"SentenceBreakTest.txt")
download_unicode_asset(DOWNLOAD_URL + "SpecialCasing.txt", external_data_dir+"SpecialCasing.txt")
download_unicode_asset(DOWNLOAD_URL + "UnicodeData.txt", external_data_dir+"UnicodeData.txt")
download_unicode_asset(DOWNLOAD_URL + "auxiliary/WordBreakProperty.txt", external_data_dir+"WordBreakProperty.txt")
download_unicode_asset(DOWNLOAD_URL + "auxiliary/WordBreakTest.txt", external_data_dir+"WordBreakTest.txt")





