#!/usr/bin/env python3
#
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import subprocess
import re
import os
import csv
import sys
import multiprocessing

def get_stage_names(skia_root):
    """Parses SkRasterPipelineOpList.h for all op names."""
    header_path = os.path.join(skia_root, "src/core/SkRasterPipelineOpList.h")
    if not os.path.exists(header_path):
        print(f"Error: Could not find {header_path}")
        sys.exit(1)

    ops = set()
    with open(header_path, 'r') as f:
        for line in f:
            # Skip macro definitions and enum declarations that might contain "M(op)"
            if "#define M(op)" in line or "enum class" in line or "static constexpr" in line:
                continue
            matches = re.findall(r'M\((\w+)\)', line)
            for m in matches:
                if m != "op":
                    ops.add(m)

    return sorted(list(ops))

# Known Raster Pipeline namespaces to avoid capturing unrelated symbols with similar names (e.g. 'clear')
RP_NAMESPACES = {
    'ml3', 'ml4', 'hsw', 'skx', 'skl', 'avx', 'avx2', 'sse2', 'sse41', 'sse3', 'ssse3',
    'neon', 'scalar', 'lsx', 'lasx', 'skl',
}

def find_symbols_in_binary(bin_path, ops):
    """Uses nm -S -C to find symbols matching the ops efficiently in one pass."""
    try:
        # -S: Print size, -C: Demangle, --defined-only: Only symbols in the binary
        cmd = ["nm", "-S", "-C", "--defined-only", bin_path]
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    except Exception as e:
        print(f"Error starting nm: {e}")
        sys.exit(1)

    found_symbols = {}
    ops_set = set(ops)

    # Regex to match: [Address] [Size] [Type] [Name]([Args])
    # We want to capture Namespace, optional lowp, and the Op name.
    stage_pattern = re.compile(r'^([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+[tT]\s+((\w+)::(?:(lowp)::)?(\w+))\(.*$')

    for line in proc.stdout:
        match = stage_pattern.match(line)
        if match:
            addr = int(match.group(1), 16)
            size = int(match.group(2), 16)
            full_name = match.group(3)
            namespace = match.group(4)
            is_lowp = match.group(5) is not None
            op_name = match.group(6)

            if op_name in ops_set and namespace.lower() in RP_NAMESPACES:
                found_symbols[full_name] = {
                    "start": addr,
                    "size": size,
                    "full_demangled": line.split(' ', 3)[-1].strip(),
                    "op": op_name,
                    "namespace": namespace,
                    "precision": "lowp" if is_lowp else "highp"
                }


    proc.wait()
    return found_symbols

def extract_and_clean_asm(bin_path, symbol_info, output_path):
    """Extracts assembly for a symbol and cleans it for llvm-mca, handling labels."""
    start = symbol_info["start"]
    stop = start + symbol_info["size"]

    try:
        # We need the raw instructions with addresses to resolve jump targets
        cmd = [
            "objdump", "-d", "--no-show-raw-insn", bin_path,
            f"--start-address=0x{start:x}",
            f"--stop-address=0x{stop:x}"
        ]
        output = subprocess.check_output(cmd).decode('utf-8')
    except subprocess.CalledProcessError as e:
        print(f"Error disassembling {symbol_info['full_demangled']}: {e}")
        return False

    lines = []
    addresses = set()

    # First pass: identify all instruction addresses
    for line in output.splitlines():
        if ':' not in line:
            continue
        parts = line.split(':', 1)
        try:
            addr = parts[0].strip()
            # Canonicalize address (remove leading zeros for matching)
            addr_val = int(addr, 16)
            addr_str = f"{addr_val:x}"
            addresses.add(addr_str)
            lines.append((addr_str, parts[1].strip()))
        except:
            continue

    with open(output_path, 'w') as f:
        for addr, content in lines:
            # content looks like: "je     605e55 <symbol+offset>"
            # Strip the <...> part
            clean_content = re.sub(r'<.*?>', '', content).strip()

            # Identify if this instruction is a jump/call
            # and if it targets one of our internal addresses.
            # We look for a hex number in the clean_content.
            # e.g. "je     605e55"
            match = re.search(r'\b([0-9a-f]{4,})\b', clean_content)
            if match:
                target_addr = match.group(1)
                if target_addr in addresses:
                    # Replace the hex address with a label name
                    clean_content = clean_content.replace(target_addr, f".L{target_addr}")

            # Output the line with its own label
            f.write(f".L{addr}:\n")
            f.write(f"  {clean_content}\n")
    return True

def run_mca(mca_path, mcpu, asm_path):
    """Runs llvm-mca and parses key metrics."""
    try:
        # Use -skip-unsupported-instructions=parse-failure to gracefully handle complex branches/calls
        cmd = [
            mca_path,
            f"-mcpu={mcpu}",
            "-iterations=100",
            "-skip-unsupported-instructions=parse-failure",
            asm_path
        ]
        output = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode('utf-8')
    except subprocess.CalledProcessError as e:
        error_msg = e.output.decode('utf-8')
        if "error: no assembly instructions found" in error_msg:
            return None
        # If it still fails, it's a more serious issue
        print(f"Error running llvm-mca on {asm_path}: {error_msg}")
        return None

    metrics = {}
    # Parse Summary Table
    total_cycles_match = re.search(r'Total Cycles:\s+(\d+)', output)
    if total_cycles_match:
        metrics["Cycles/Iter"] = float(total_cycles_match.group(1)) / 100.0

    ipc_match = re.search(r'IPC:\s+([\d.]+)', output)
    if ipc_match:
        metrics["IPC"] = float(ipc_match.group(1))

    rthroughput_match = re.search(r'Block RThroughput:\s+([\d.]+)', output)
    if rthroughput_match:
        metrics["RThroughput"] = float(rthroughput_match.group(1))

    return metrics

def process_symbol(args_tuple):
    """Worker function for parallel processing."""
    name, info, bin_path, version_dir, mca_path, mcpu = args_tuple
    asm_path = os.path.join(version_dir, f"{name.replace('::', '_')}.s")
    if extract_and_clean_asm(bin_path, info, asm_path):
        metrics = run_mca(mca_path, mcpu, asm_path)
        if metrics:
            return name, metrics
    return name, None

def main():
    parser = argparse.ArgumentParser(description="Analyze SkRasterPipeline stage performance.")
    parser.add_argument("--bin", required=True, help="Path to the binary to analyze.")
    parser.add_argument("--name", required=True, help="Name for this run (e.g. control, experiment).")
    parser.add_argument("--work_dir", required=True, help="Directory to store assembly and results.")
    parser.add_argument("--skia_root", default=".", help="Path to Skia root directory.")
    parser.add_argument("--mcpu", default="x86-64-v4", help="CPU model for llvm-mca.")
    parser.add_argument("--mca_path", default="/usr/bin/llvm-mca-19", help="Path to llvm-mca.")
    parser.add_argument("--list-stages", action="store_true", help="Just list found stages and exit.")

    args = parser.parse_args()

    if not os.path.exists(args.bin):
        print(f"Error: Binary {args.bin} does not exist.")
        sys.exit(1)

    if not os.path.exists(args.work_dir):
        os.makedirs(args.work_dir)

    # Ensure version subfolder exists
    version_dir = os.path.join(args.work_dir, args.name)
    if not os.path.exists(version_dir):
        os.makedirs(version_dir)

    print(f"Scanning {args.skia_root} for RP stages...")
    ops = get_stage_names(args.skia_root)
    print(f"Found {len(ops)} potential stages.")

    print(f"Discovering symbols in {args.bin}...")
    symbols = find_symbols_in_binary(args.bin, ops)
    print(f"Found {len(symbols)} compiled stages.")

    if args.list_stages:
        for s in sorted(symbols.keys()):
            print(f"  {s}")
        return

    results = {}
    total = len(symbols)
    print(f"Analyzing {total} stages in parallel...")

    # Prepare arguments for workers
    worker_args = [
        (name, info, args.bin, version_dir, args.mca_path, args.mcpu)
        for name, info in symbols.items()
    ]

    count = 0
    with multiprocessing.Pool() as pool:
        for name, metrics in pool.imap_unordered(process_symbol, worker_args):
            count += 1
            if metrics:
                results[name] = metrics

            # Progress bar
            percent = (count * 100) // total
            bar_len = 40
            filled_len = (count * bar_len) // total
            bar = '#' * filled_len + '-' * (bar_len - filled_len)
            sys.stdout.write(f"\r[{bar}] {percent}% ({count}/{total}) {name[:30]:<30}")
            sys.stdout.flush()

    print("\nAnalysis complete.")

    # Manage spreadsheet
    tsv_path = os.path.join(args.work_dir, "analysis.tsv")
    all_data = {}
    versions = []

    if os.path.exists(tsv_path):
        with open(tsv_path, 'r', newline='') as f:
            reader = csv.DictReader(f, delimiter='\t')
            versions = [col.split(' ')[0] for col in reader.fieldnames if 'Cycles' in col]
            for row in reader:
                symbol = row['Symbol']
                all_data[symbol] = row

    if args.name not in versions:
        versions.append(args.name)

    # Update all_data with new results
    for symbol, metrics in results.items():
        info = symbols[symbol]
        if symbol not in all_data:
            all_data[symbol] = {
                'Symbol': symbol,
                'Op': info['op'],
                'Namespace': info['namespace'],
                'Precision': info['precision']
            }

        all_data[symbol][f"{args.name} Cycles"] = f"{metrics['Cycles/Iter']:.2f}"
        all_data[symbol][f"{args.name} IPC"] = f"{metrics['IPC']:.2f}"
        all_data[symbol][f"{args.name} RThp"] = f"{metrics['RThroughput']:.2f}"

        # Calculate improvement if there's a control (the first version)
        if len(versions) > 1:
            control = versions[0]
            if f"{control} Cycles" in all_data[symbol] and all_data[symbol][f"{control} Cycles"]:
                ctrl_cycles = float(all_data[symbol][f"{control} Cycles"])
                curr_cycles = metrics['Cycles/Iter']
                if ctrl_cycles > 0:
                    diff = (ctrl_cycles - curr_cycles) / ctrl_cycles * 100
                    all_data[symbol][f"{args.name} % Improv"] = f"{diff:+.2f}"

    # Write back to TSV
    fieldnames = ['Op', 'Namespace', 'Precision', 'Symbol']
    for v in versions:
        fieldnames.append(f"{v} Cycles/Iter")
        fieldnames.append(f"{v} Inst/Cycle (IPC)")
        fieldnames.append(f"{v} Throughput (RThp)")
        if v != versions[0]:
            fieldnames.append(f"{v} % Improv")

    # Mapping internal keys to user-friendly headers
    with open(tsv_path, 'w', newline='') as f:
        # We need to map the internal data keys to the new friendly fieldnames
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter='\t', extrasaction='ignore')
        writer.writeheader()
        for symbol in sorted(all_data.keys()):
            row = all_data[symbol]
            # Create a new row with friendly headers
            friendly_row = {
                'Op': row.get('Op', ''),
                'Namespace': row.get('Namespace', ''),
                'Precision': row.get('Precision', ''),
                'Symbol': row.get('Symbol', '')
            }
            for v in versions:
                friendly_row[f"{v} Cycles/Iter"] = row.get(f"{v} Cycles", "")
                friendly_row[f"{v} Inst/Cycle (IPC)"] = row.get(f"{v} IPC", "")
                friendly_row[f"{v} Throughput (RThp)"] = row.get(f"{v} RThp", "")
                if v != versions[0]:
                    friendly_row[f"{v} % Improv"] = row.get(f"{v} % Improv", "")
            writer.writerow(friendly_row)

    print(f"Results written to {tsv_path}")

if __name__ == "__main__":
    main()
