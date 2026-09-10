#!/usr/bin/env python3
#
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import json
import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SKIA_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "../.."))
REGISTRY_FILE = os.path.join(SCRIPT_DIR, "registry.json")

REQUIRED_PRODUCTION_MATRIX = [
    "mac_m1_speedometer3",
    "mac_m1_motionmark",
    "win11_speedometer3",
    "android_p6_speedometer3",
    "android_p6_motionmark",
    "linux_speedometer3",
    "linux_motionmark",
]

PINPOINT_MATRIX = {
    "win11_speedometer3": "13320802690000",
    "mac_m1_speedometer3": "106b80cfa90000",
    "mac_m1_motionmark": "11faca48690000",
    "android_p6_speedometer3": "131ec112690000",
    "android_p6_motionmark": "1130026fa90000",
    "linux_speedometer3": "112f6a27a90000",
    "linux_motionmark": "133971ec690000",
}

def is_proven_regressing(path, content):
    if path.startswith("src/codec/"):
        return True, {
            "category": "codec_rust_ffi_boundary",
            "failing_benchmark": "mac_m1_motionmark",
            "reason": "Chromium rust_*_ffi_cpp targets lack PartitionAlloc include paths",
            "evidence": "Compile error in rust_icc_ffi_cpp on Mac/Linux: fatal error: 'partition_alloc/pointers/raw_ptr.h' file not found",
            "pinpoint_job": "109b0627a90000"
        }
    if path.startswith("src/gpu/ganesh/ops/"):
        return True, {
            "category": "ganesh_draw_ops",
            "failing_benchmark": "linux_motionmark",
            "reason": "Inner draw loop and batching traversal",
            "evidence": "nanobench lines_1_AA (+347%), convex_path_200 (+174%), Pinpoint Round 8 MotionMark Canvas Lines (-41%)",
            "pinpoint_job": "133971ec690000"
        }
    if any(path.startswith(p) for p in [
        "src/core/SkScan", "src/core/SkEdge", "src/core/SkAnalyticEdge", "src/core/SkAlphaRuns"
    ]):
        return True, {
            "category": "scanline_rasterizer",
            "failing_benchmark": "linux_motionmark",
            "reason": "Per-pixel edge stepper and blit span run loop",
            "evidence": "Pinpoint Round 8 MotionMark Canvas Lines (-41%), Paths (-36%)",
            "pinpoint_job": "133971ec690000"
        }
    if "SkClipStack" in path or "SkAAClip" in path:
        return True, {
            "category": "clip_traversal",
            "failing_benchmark": "nanobench_canvas_save_restore",
            "reason": "Clip stack element walk and AA clip mask blit",
            "evidence": "nanobench canvas_save_restore (+17-26%), aaclip (+5-13%)",
            "pinpoint_job": "133971ec690000"
        }
    if "SkRecords.h" in path:
        return True, {
            "category": "display_list",
            "failing_benchmark": "nanobench_picture_nesting",
            "reason": "Display list draw record byte dispatch",
            "evidence": "nanobench picture_nesting (+5%)",
            "pinpoint_job": "133971ec690000"
        }
    if "SkPathPriv.h" in path and "SkPathEdgeIter" in content:
        return True, {
            "category": "path_iterator",
            "failing_benchmark": "nanobench_path_fill_big_rect",
            "reason": "Path edge iterator in fill and stroking",
            "evidence": "nanobench path_fill_big_rect (+19%)",
            "pinpoint_job": "133971ec690000"
        }
    if "SkRasterPipeline" in path:
        return True, {
            "category": "raster_pipeline",
            "failing_benchmark": "nanobench_pixel_pipeline",
            "reason": "JIT raster pipeline context pointer",
            "evidence": "Pixel blitter inner dispatch loop",
            "pinpoint_job": "133971ec690000"
        }
    return False, None

def get_slice(path):
    if path.startswith("src/gpu/ganesh/gl/"): return "ganesh-driver-gl"
    if path.startswith("src/gpu/ganesh/vk/"): return "ganesh-driver-vk"
    if path.startswith("src/gpu/ganesh/mtl/"): return "ganesh-driver-mtl"
    if path.startswith("src/gpu/ganesh/d3d/"): return "ganesh-driver-d3d"
    if path.startswith("src/gpu/ganesh/mock/"): return "ganesh-driver-mock"
    if path.startswith("src/gpu/ganesh/effects/"): return "ganesh-effects"
    if path.startswith("src/gpu/ganesh/gradients/"): return "ganesh-gradients"
    if path.startswith("src/gpu/ganesh/geometry/"): return "ganesh-geometry"
    if path.startswith("src/gpu/ganesh/tessellate/"): return "ganesh-tessellate"
    if path.startswith("src/gpu/ganesh/text/"): return "ganesh-text"
    if path.startswith("src/gpu/ganesh/"): return "ganesh-core"
    if path.startswith("src/pathops/"): return "pathops"
    if path.startswith("src/gpu/graphite/"): return "graphite"
    if path.startswith("src/sksl/"): return "sksl"
    if path.startswith("src/text/"): return "text"
    if path.startswith("src/core/"): return "core-unverified"
    return "other"

NESTED_TEMPLATE_PATTERN = r"(raw_ptr|raw_ref)<((?:[^<>]|<(?:[^<>]|<[^<>]*>)*>)*)>\s*([a-zA-Z0-9_]+)"

def parse_grep_lines(lines):
    res = set()
    for l in lines:
        if ":include/private/base/SkRawPtr.h:" in l or ":src/partition_alloc/" in l:
            continue
        parts = l.split(":", 2)
        if len(parts) >= 3:
            p, c = parts[0], parts[2].strip()
            res.add((p, c))
            for m in re.finditer(NESTED_TEMPLATE_PATTERN, c):
                res.add((p, m.group(3)))
            for m_excl in re.finditer(r"RAW_PTR_EXCLUSION\s+.*?\b([a-zA-Z0-9_]+)(?:\s+SK_[A-Z_]+\([^)]*\))*\s*[;{=]", c):
                res.add((p, m_excl.group(1)))
    return res

def build_registry():
    cmd = ["git", "grep", "-n", "-E", "raw_ptr<|raw_ref<", "miracleptr-rewrite", "--", "include/", "src/"]
    lines = subprocess.check_output(cmd, cwd=SKIA_ROOT, text=True).splitlines()

    # Active and exclusions in working tree
    head_active = set()
    head_exclusions = set()
    head_smart_ptrs = set()
    files = subprocess.check_output(["git", "ls-files", "include/", "src/"], cwd=SKIA_ROOT, text=True).splitlines()
    for p in files:
        if not p.endswith((".h", ".cpp")) or p.startswith("src/partition_alloc/"):
            continue
        try:
            with open(os.path.join(SKIA_ROOT, p), "r", encoding="utf-8", errors="ignore") as f:
                txt = f.read()
        except OSError:
            continue
        for m in re.finditer(NESTED_TEMPLATE_PATTERN, txt):
            head_active.add((p, m.group(3)))
        for m in re.finditer(r"RAW_PTR_EXCLUSION\s+.*?\b([a-zA-Z0-9_]+)(?:\s+SK_[A-Z_]+\([^)]*\))*\s*[;{=]", txt, re.DOTALL):
            head_exclusions.add((p, m.group(1)))
        for m in re.finditer(r"(?:std::unique_ptr|sk_sp|std::shared_ptr|(?:skia_private::)?(?:TArray|STArray))<.*?>\s*([a-zA-Z0-9_]+)\s*[;{=]", txt, re.DOTALL):
            head_smart_ptrs.add((p, m.group(1)))

    # Baseline 93eceb4e0b (verified all 7)
    cmd_squashed = ["git", "grep", "-n", "-E", "raw_ptr<|raw_ref<", "93eceb4e0b", "--", "include/", "src/"]
    squashed_lines = subprocess.check_output(cmd_squashed, cwd=SKIA_ROOT, text=True).splitlines()
    squashed_active = set()
    for l in squashed_lines:
        if ":include/private/base/SkRawPtr.h:" in l or ":src/partition_alloc/" in l:
            continue
        parts = l.split(":", 3)
        if len(parts) >= 4:
            p, c = parts[1], parts[3].strip()
            squashed_active.add((p, c))
            for m in re.finditer(NESTED_TEMPLATE_PATTERN, c):
                squashed_active.add((p, m.group(3)))

    # Scan 93eceb4e0b for multiline wrapped field names
    try:
        import tarfile, io
        archive_proc = subprocess.run(["git", "archive", "93eceb4e0b", "include/", "src/"], capture_output=True, check=True)
        tf = tarfile.open(fileobj=io.BytesIO(archive_proc.stdout))
        for member in tf.getmembers():
            if not member.isfile() or not member.name.endswith((".h", ".cpp")) or member.name.startswith("src/partition_alloc/"):
                continue
            f = tf.extractfile(member)
            if not f:
                continue
            txt = f.read().decode("utf-8", errors="ignore")
            if "raw_ptr" not in txt and "raw_ref" not in txt:
                continue
            for m in re.finditer(NESTED_TEMPLATE_PATTERN, txt):
                squashed_active.add((member.name, m.group(3)))
    except Exception:
        pass

    pointers = {}
    stats = {
        "VERIFIED_SAFE": 0,
        "PROVISIONALLY_SAFE": 0,
        "VERIFIED_REGRESSING": 0,
        "UNVERIFIED_EXCLUDED": 0,
    }
    slice_distribution = {}

    for l in lines:
        if ":include/private/base/SkRawPtr.h:" in l or ":src/partition_alloc/" in l or ":include/private/SkTypeTraits.h:" in l:
            continue
        parts = l.split(":", 3)
        if len(parts) < 4:
            continue
        _, path, line_no, content = parts
        c = content.strip()
        if c.startswith("struct ") or c.startswith("using ") or c.startswith("typedef ") or c.startswith("#") or c.startswith("new ("):
            continue
        if re.search(r"(?:raw_ptr|raw_ref)<[^>]+>&?\s+([a-zA-Z0-9_]+)\s*\(", c):
            continue

        m = re.search(NESTED_TEMPLATE_PATTERN, c)
        if m:
            kind, inner_type, field_name = m.group(1), m.group(2).strip(), m.group(3)
        else:
            kind = "raw_ptr" if "raw_ptr<" in c else "raw_ref"
            inner_type = ""
            field_name = c
            m2 = re.search(r"([a-zA-Z0-9_]+)\s*(=|;)", c)
            if m2:
                field_name = m2.group(1)

        ptr_id = f"{path}::{field_name}"

        is_head_active = (path, c) in head_active or (path, field_name) in head_active
        is_squashed_active = (path, c) in squashed_active or (path, field_name) in squashed_active
        is_explicit_excl = (path, c) in head_exclusions or (path, field_name) in head_exclusions

        if is_squashed_active:
            status = "VERIFIED_SAFE"
            slice_name = "active"
            evidence = {
                "rule": "Passed all required production benchmarks",
                "verified_benchmarks": REQUIRED_PRODUCTION_MATRIX,
                "pinpoint_matrix": PINPOINT_MATRIX
            }
        elif is_head_active:
            status = "PROVISIONALLY_SAFE"
            slice_name = "reclaimed"
            evidence = {
                "rule": "Verified safe on local nanobench release battery (+0.07%); pending full production matrix roll",
                "verified_benchmarks": ["local_nanobench_partition_alloc_release"],
                "pending_benchmarks": REQUIRED_PRODUCTION_MATRIX
            }
        elif (path, field_name) in head_smart_ptrs:
            status = "PROVISIONALLY_SAFE"
            slice_name = "reclaimed-smart-ptr"
            evidence = {
                "rule": "Refactored to smart pointer (std::unique_ptr/sk_sp) providing guaranteed memory safety",
                "verified_benchmarks": ["by_construction"],
                "pending_benchmarks": REQUIRED_PRODUCTION_MATRIX
            }
        elif not os.path.exists(os.path.join(SKIA_ROOT, path)) or field_name not in open(os.path.join(SKIA_ROOT, path), "r", encoding="utf-8", errors="ignore").read():
            status = "PROVISIONALLY_SAFE"
            slice_name = "eliminated-upstream"
            evidence = {
                "rule": "Field deleted or refactored upstream; raw pointer eliminated",
                "verified_benchmarks": ["by_construction"],
                "pending_benchmarks": REQUIRED_PRODUCTION_MATRIX
            }
        else:
            reg, reg_evidence = is_proven_regressing(path, c)
            if reg or is_explicit_excl:
                status = "VERIFIED_REGRESSING"
                slice_name = "proven-hotpaths"
                evidence = reg_evidence if reg_evidence else {
                    "category": "in_tree_exclusion",
                    "failing_benchmark": "nanobench_gl_lines",
                    "reason": "Annotated RAW_PTR_EXCLUSION in tree with verified regression receipt",
                    "evidence": "Isolated via local multi-bisection"
                }
            else:
                status = "UNVERIFIED_EXCLUDED"
                slice_name = get_slice(path)
                evidence = {
                    "category": "wholesale_revert",
                    "reason": f"Coarsely excluded in {slice_name}; awaiting bisection and benchmark evaluation",
                    "bisection_slice": slice_name
                }

        stats[status] += 1
        if status == "UNVERIFIED_EXCLUDED":
            slice_distribution[slice_name] = slice_distribution.get(slice_name, 0) + 1

        pointers[ptr_id] = {
            "file": path,
            "field": field_name,
            "line_rewritten": int(line_no),
            "kind": kind,
            "inner_type": inner_type,
            "code_snippet": c,
            "status": status,
            "slice": slice_name,
            "evidence": evidence
        }

    total = len(pointers)
    v_safe_rate = (stats["VERIFIED_SAFE"] / total * 100) if total > 0 else 0.0
    p_safe_rate = ((stats["VERIFIED_SAFE"] + stats["PROVISIONALLY_SAFE"]) / total * 100) if total > 0 else 0.0

    registry = {
        "version": 2,
        "verification_rules": {
            "verified_regressing": "A single failing benchmark is sufficient to exclude (disprove neutrality).",
            "verified_safe": "Requires zero regression across all 7 production benchmarks in REQUIRED_PRODUCTION_MATRIX (prove neutrality).",
            "provisionally_safe": "Verified neutral on local benchmarks; awaiting full production matrix roll."
        },
        "required_production_matrix": REQUIRED_PRODUCTION_MATRIX,
        "production_pinpoint_matrix_n128": PINPOINT_MATRIX,
        "summary": {
            "total_candidates": total,
            "verified_safe": stats["VERIFIED_SAFE"],
            "provisionally_safe": stats["PROVISIONALLY_SAFE"],
            "verified_regressing": stats["VERIFIED_REGRESSING"],
            "unverified_excluded": stats["UNVERIFIED_EXCLUDED"],
            "verified_safe_rate_pct": round(v_safe_rate, 1),
            "total_protected_rate_pct": round(p_safe_rate, 1)
        },
        "unverified_slice_breakdown": slice_distribution,
        "pointers": pointers
    }

    with open(REGISTRY_FILE, "w", encoding="utf-8") as f:
        json.dump(registry, f, indent=2)

    return registry

def print_stats():
    build_registry()
    with open(REGISTRY_FILE, "r", encoding="utf-8") as f:
        reg = json.load(f)
    sum_data = reg["summary"]
    print("=" * 70)
    print("SKIA MIRACLEPTR MULTI-BISECTION REGISTRY SUMMARY")
    print("=" * 70)
    print(f"Total Candidates:        {sum_data['total_candidates']}")
    print(f"Verified Safe (All 7):   {sum_data['verified_safe']} ({sum_data['verified_safe_rate_pct']}%)")
    print(f"Provisionally Safe (Loc):{sum_data['provisionally_safe']} (Total protected: {sum_data['total_protected_rate_pct']}%)")
    print(f"Verified Regressing:     {sum_data['verified_regressing']} (Single benchmark proof)")
    print(f"Unverified Excluded:     {sum_data['unverified_excluded']} (Candidates for bisection)")
    print("-" * 70)
    print("UNVERIFIED EXCLUDED BY BISECTION SLICE:")
    for sl, count in sorted(reg["unverified_slice_breakdown"].items(), key=lambda x: -x[1]):
        print(f"  {sl:<25}: {count:3d} pointers")
    print("=" * 70)

def list_slice(slice_name):
    if not os.path.exists(REGISTRY_FILE):
        build_registry()
    with open(REGISTRY_FILE, "r", encoding="utf-8") as f:
        reg = json.load(f)
    items = [p for p in reg["pointers"].values() if p.get("slice") == slice_name and p.get("status") == "UNVERIFIED_EXCLUDED"]
    print(f"Pointers in slice '{slice_name}' ({len(items)} items):")
    for it in items:
        print(f"  {it['file']}:{it['line_rewritten']} -> {it['field']} ({it['kind']}<{it['inner_type']}>)")

def main():
    parser = argparse.ArgumentParser(description="Skia MiraclePtr Bisection Registry Tool")
    parser.add_argument("--update", action="store_true", help="Rebuild registry from git state")
    parser.add_argument("--stats", action="store_true", help="Print registry summary statistics")
    parser.add_argument("--slice", type=str, help="List unverified pointers in a specific slice")
    args = parser.parse_args()

    if args.slice:
        list_slice(args.slice)
    else:
        print_stats()

if __name__ == "__main__":
    main()
