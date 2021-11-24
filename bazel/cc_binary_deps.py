#!#!/usr/bin/python3
# Copyright 2021 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import subprocess

def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description='Help find source files that potentially provide symbols for the deps.')
    parser.add_argument('--target_rule', default='_resource', type=str,
        help='The name of the cc_binary rule deps should be searched for',
        required=True)
    args = parser.parse_args()

    # bazel query "deps(//fuzz:fuzz)"
    p = subprocess.run(["bazel", "query", 'deps('+args.target_rule+')'],
                       capture_output=True, encoding="utf-8")

    all_deps = p.stdout.split("\n")

    hdrs = []
    for dep in all_deps:
        if dep.startswith("//") and dep.endswith("_hdr"):
            hdrs.append(dep)
    hdrs.sort()
    print(f"Found {len(hdrs)} headers included via deps and transitive deps")

    # bazel query 'kind(rule, //src/...)'
    p = subprocess.run(["bazel", "query", 'kind(rule, //src/...)'], capture_output=True, encoding="utf-8")

    src_rules = set(p.stdout.split("\n"))

    print("The following source files might help provide the missing symbols.")

    for hdr in hdrs:
        if hdr.startswith("//src"):
            src = hdr[:-4] + "_src"
            if src in src_rules:
                print(f'"{src}",')
        if hdr.startswith("//include"):
            src = "//src"+hdr[9:-4]+"_src"
            if src in src_rules:
                print(f'"{src}",')



if __name__ == "__main__":
    main()