# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Generates a Bloaty TSV output from the given binary for codesize.skia.org."""


from __future__ import print_function
import os
import sys


def main():
  input_file = sys.argv[1]
  out_dir = sys.argv[2]
  bloaty_path = sys.argv[3]

  name = os.path.basename(input_file)

  bloaty_output_file = os.path.join(out_dir, name + '.tsv')

  bloaty_cmd = [bloaty_path, input_file, '-d', 'compileunits,symbols', '-n', '0', '--tsv']

  print('Pretending to invoke Bloaty as follows:')
  print(' '.join(bloaty_cmd))

  # bloaty_output = subprocess.check_output(bloaty_cmd)

  print('Writing Bloaty output to %s' % bloaty_output_file)

  with open(bloaty_output_file, 'w') as output:
    output.write('This is a mock Bloaty output file.')


if __name__ == '__main__':
  main()
