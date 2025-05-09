#!/usr/bin/env python3
# Copyright 2025 Google LLC
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

def create_single_file_patch(source_file, destination_path):
    try:
        with open(source_file, 'r') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"Error: Source file not found: {source_file}", file=sys.stderr)
        return None

    lines = content.splitlines()
    line_count = len(lines)

    # Construct the patch section with the new header lines
    # of note, the bazel patch tool doesn't seem to care about the index
    # (git would normally have the sha1 sum of the file contents plus a header)
    # so we can just use a stub and it works fine.
    patch_section = f"new file mode 100644\n" \
                    f"index 0000000..fffffff\n" \
                    f"--- /dev/null\n" \
                    f"+++ {destination_path}\n" \
                    f"@@ -0,0 +1,{line_count} @@\n"

    for line in lines:
        patch_section += "+" + line + "\n"
    return patch_section


if __name__ == "__main__":
    if (len(sys.argv) - 1) % 2 != 0 or len(sys.argv) < 3:
        print("Usage: python3 generate_patch.py <source_file1> <destination_path1> [<source_file2> <destination_path2> ...]", file=sys.stderr)
        sys.exit(1)

    all_patch_content = []
    # sys.argv[0] is the script name, so start from index 1 and step by 2
    for i in range(1, len(sys.argv), 2):
        source_file = sys.argv[i]
        destination_path = sys.argv[i + 1]
        
        patch_section = create_single_file_patch(source_file, destination_path)
        if patch_section is None:
            sys.exit(1)
        all_patch_content.append(patch_section)

    print("".join(all_patch_content))