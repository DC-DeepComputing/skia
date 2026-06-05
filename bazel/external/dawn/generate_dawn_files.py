#!/usr/bin/env python3
import os
import re
import sys

# This script parses Dawn CMakeLists.txt files and creates a .bzl file with the files extracted.
# It uses a structured parser to handle variables and conditional blocks.

class CMakeParser:
    def __init__(self, prefix, rel_path):
        self.prefix = prefix
        self.rel_path = rel_path
        self.variables = {
            "DAWN_INCLUDE_DIR": [("include", [])],
            "DAWN_SRC_DIR": [("src", [])],
        }
        # output_vars[prefix_with_conditions][group] = set of files
        self.output_vars = {}
        self.current_conditions = []

    def slugify_condition(self, cond):
        # Remove parentheses and common CMake operators that don't look good in variable names
        cond = cond.replace("(", "").replace(")", "")
        # Remove DAWN_ENABLE_ and DAWN_ as requested
        cond = cond.replace("DAWN_ENABLE_", "").replace("DAWN_", "")
        # Replace non-alphanumeric with _
        cond = re.sub(r'[^a-zA-Z0-9]', '_', cond)
        # Clean up double underscores and remove leading/trailing underscores
        cond = re.sub(r'_+', '_', cond).strip('_')
        return cond.upper()

    def get_prefix_for_conditions(self, conds):
        if not conds:
            return self.prefix
        cond_slug = "_".join(self.slugify_condition(c) for c in conds)
        return f"{self.prefix}_{cond_slug}"

    def get_current_prefix(self):
        return self.get_prefix_for_conditions(self.current_conditions)

    def resolve_var(self, var_name):
        if var_name not in self.variables:
            return [(f"${{{var_name}}}", [])]
        
        vals = self.variables[var_name]
        resolved = []
        for v, conds in vals:
            if isinstance(v, str) and v.startswith("${") and v.endswith("}"):
                inner = v[2:-1]
                if inner != var_name:
                    resolved.extend(self.resolve_var(inner))
                else:
                    resolved.append((v, conds))
            else:
                resolved.append((v, conds))
        return resolved

    def expand_string(self, s):
        def repl(match):
            var_name = match.group(1)
            resolved = self.resolve_var(var_name)
            # Use the first resolved value for simplicity in path expansion
            return str(resolved[0][0]) if resolved else ""
        
        return re.sub(r'\${([^}]+)}', repl, s)

    def clean_path(self, path):
        path = self.expand_string(path)
        # If the path doesn't start with include/ or src/, it's relative to the current CMake file
        if not path.startswith("include/") and not path.startswith("src/"):
            path = os.path.join(self.rel_path, path)
        return os.path.normpath(path)

    def parse(self, content):
        # Remove comments
        content = re.sub(r'#.*', '', content)
        
        pos = 0
        while pos < len(content):
            # Find next command
            match = re.search(r'(\w+)\s*\(', content[pos:])
            if not match:
                break
            
            cmd_name = match.group(1).lower()
            start_paren = pos + match.end() - 1
            
            # Find matching closing parenthesis (handling nested ones)
            count = 0
            end_paren = -1
            for i in range(start_paren, len(content)):
                if content[i] == '(':
                    count += 1
                elif content[i] == ')':
                    count -= 1
                    if count == 0:
                        end_paren = i
                        break
            
            if end_paren == -1:
                break
            
            args_str = content[start_paren+1:end_paren]
            # Parse arguments, respecting quotes
            args_raw = re.findall(r'"([^"]*)"|([^\s()]+)', args_str)
            args = [a[0] or a[1] for a in args_raw if a[0] or a[1]]
            
            self.handle_command(cmd_name, args)
            pos = end_paren + 1

    def handle_command(self, name, args):
        if name == "endif":
            if self.current_conditions:
                self.current_conditions.pop()
            return
        elif name == "else":
            if self.current_conditions:
                last = self.current_conditions.pop()
                self.current_conditions.append(f"NOT_{last}")
            return

        if not args:
            return

        if name == "set":
            var_name = args[0]
            values = args[1:]
            self.variables[var_name] = [(val, list(self.current_conditions)) for val in values]
            self.add_to_outputs(var_name, values)
        elif name == "list":
            if len(args) >= 3 and args[0].upper() == "APPEND":
                var_name = args[1]
                values = args[2:]
                if var_name not in self.variables:
                    self.variables[var_name] = []
                self.variables[var_name].extend([(val, list(self.current_conditions)) for val in values])
                self.add_to_outputs(var_name, values)
        elif name == "if":
            self.current_conditions.append(" ".join(args))
        elif name == "elseif":
            if self.current_conditions:
                self.current_conditions.pop()
            self.current_conditions.append(" ".join(args))
        elif name in ["dawngenerator", "dawnjsongenerator"]:
            # Capture variables defined by generators
            for i in range(len(args)):
                if args[i] in ["OUTPUT_HEADERS", "OUTPUT_SOURCES"]:
                    var_name = args[i+1]
                    self.variables[var_name] = [(f"${{{var_name}}}", list(self.current_conditions))]
        elif name == "dawn_add_library":
            # Arguments format: dawn_add_library(target_name [keywords and values] ...)
            # We iterate through the arguments from index 1 (skipping target_name).
            all_keywords = {
                "FORCE_STATIC", "FORCE_SHARED", "FORCE_OBJECT", "HEADER_ONLY", "ENABLE_EMSCRIPTEN",
                "UTILITY_TARGET", "HEADERS", "PRIVATE_HEADERS", "SOURCES", "DEPENDS", "PRIVATE_DEPENDS"
            }
            current_keyword = None
            for arg in args[1:]:
                arg_upper = arg.upper()
                if arg_upper in all_keywords:
                    current_keyword = arg_upper
                elif current_keyword in ["HEADERS", "PRIVATE_HEADERS", "SOURCES"]:
                    self.add_to_outputs(current_keyword.lower(), [arg])

    def add_to_outputs(self, var_name, values):
        var_name_lower = var_name.lower()
        group = None
        
        # Mapping logic:
        # 'headers' (exactly) -> HDRS
        # anything containing 'headers', 'sources', or 'srcs' -> SRCS
        if var_name_lower == "headers":
            group = "HDRS"
        elif "headers" in var_name_lower or "sources" in var_name_lower or "srcs" in var_name_lower:
            group = "SRCS"
        
        if not group:
            return
        
        for val in values:
            # Handle variable expansions within the list
            if val.startswith("${") and val.endswith("}"):
                inner = val[2:-1]
                # If it's a variable we've seen (and not a system/path one we want to keep), expand it
                if inner in self.variables and not inner.startswith("DAWN_"):
                    expanded = self.resolve_var(inner)
                    for e, conds in expanded:
                        cleaned = self.clean_path(e)
                        if "${" not in cleaned:
                            prefix = self.get_prefix_for_conditions(conds)
                            if prefix not in self.output_vars:
                                self.output_vars[prefix] = {"HDRS": set(), "SRCS": set()}
                            self.output_vars[prefix][group].add(cleaned)
                    continue
            
            cleaned = self.clean_path(val)
            if "${" not in cleaned:
                prefix = self.get_prefix_for_conditions(self.current_conditions)
                if prefix not in self.output_vars:
                    self.output_vars[prefix] = {"HDRS": set(), "SRCS": set()}
                self.output_vars[prefix][group].add(cleaned)

def main():
    if len(sys.argv) < 3:
        print("Usage: generate_dawn_files.py <dawn_root> <output_bzl>")
        sys.exit(1)

    dawn_root = sys.argv[1]
    output_bzl = sys.argv[2]

    # Target directories and their base prefixes
    targets = [
        ("DAWN_COMMON", "src/dawn/common"),
        ("DAWN_NATIVE", "src/dawn/native"),
        ("DAWN_WIRE", "src/dawn/wire"),
        ("DAWN_PLATFORM", "src/dawn/platform"),
        ("DAWN_UTILS", "src/dawn/utils"),
        ("DAWN_PARTITION_ALLOC", "src/dawn/partition_alloc"),
    ]

    all_output_vars = {}

    for prefix, rel_path in targets:
        cmake_path = os.path.join(dawn_root, rel_path, "CMakeLists.txt")
        if not os.path.exists(cmake_path):
            raise FileNotFoundError(f"CMakeLists.txt not found at {cmake_path}")
        
        with open(cmake_path) as f:
            content = f.read()
        
        parser = CMakeParser(prefix, rel_path)
        parser.parse(content)
        
        # Verify that we extracted some files for this target
        has_files = False
        for groups in parser.output_vars.values():
            for files in groups.values():
                if files:
                    has_files = True
                    break
            if has_files:
                break
        
        if not has_files:
            raise ValueError(
                f"No file lists were generated for target '{prefix}' at '{cmake_path}'. "
                f"This indicates a parser failure or unexpected CMake structure."
            )
        
        # Merge parser outputs into global variable map
        for p, groups in parser.output_vars.items():
            for group, files in groups.items():
                if files:
                    var_name = f"{p}_{group}"
                    if var_name not in all_output_vars:
                        all_output_vars[var_name] = set()
                    all_output_vars[var_name].update(files)

    with open(output_bzl, "w") as f:
        f.write('"""\nThis file contains the list of files used to build Dawn.\n\nGenerated by generate_dawn_files.py\n"""\n\n')
        for var_name in sorted(all_output_vars.keys()):
            files = sorted(list(all_output_vars[var_name]))
            f.write(f"{var_name} = [\n")
            for file_path in files:
                f.write(f'    "{file_path}",\n')
            f.write("]\n\n")

if __name__ == "__main__":
    main()
