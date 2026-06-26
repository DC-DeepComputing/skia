/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_ScanComposer_DEFINED
#define skgpu_graphite_compute_ScanComposer_DEFINED

#include <cstddef>
#include <string_view>

namespace skgpu::graphite {

/**
 * ScanComposer combines modular compute scan shader policies into a single static constexpr
 * compile-time generated SkSL string.
 *
 * It implements single-pass prefix scan using the Decoupled Fallback algorithm:
 *   Smith, Levien, and Owens: "Decoupled Fallback: A Portable Single-Pass GPU Scan"
 *   https://dl.acm.org/doi/10.1145/3694906.3743326
 *   https://github.com/b0nes164/Decoupled-Fallback-Paper/blob/main/artifact/Dawn/Shaders/csdldf.wgsl
 *
 * It orchestrates execution flow across duck-typed policy types:
 *   - PreamblePolicy: exposes `kPreamble` (string_view) for global definitions.
 *   - LocalScanPolicy: exposes `kScan` and `kFanout` (string_view) for thread-local tile scanning
 * and result propagation.
 *   - WorkgroupScanPolicy: exposes `kScan` (inclusive scan) and `kFallback` (reduction)
 * (string_view).
 *   - ConversionPolicy: exposes `kScanType` (string_view type name, e.g., "float", "uint") and
 *     `kPreamble` (string_view) defining `convertFromScanType`, `convertToScanType`, and `combine`.
 *
 * All instances of "ScanType" in the composer templates are substituted at compile time
 * (`constexpr`) with `ConversionPolicy::kScanType`.
 */
template <typename PreamblePolicy,
          typename LocalScanPolicy,
          typename WorkgroupScanPolicy,
          typename ConversionPolicy>
class ScanComposer {
public:
    static inline constexpr std::string_view kBasePreamble =
            "const uint FLAG_NOT_READY = 0u;\n"
            "const uint FLAG_READY     = 0x40000000u;\n"
            "const uint FLAG_INCLUSIVE = 0x80000000u;\n"
            "const uint FLAG_MASK      = 0xC0000000u;\n"
            "\n"
            "const uint MAX_SPIN_COUNT = 4u;\n"
            "const uint LOCKED         = 1u;\n"
            "const uint UNLOCKED       = 0u;\n"
            "\n"
            "workgroup uint wg_control;\n"
            "workgroup uint wg_broadcast;\n"
            "workgroup ScanType wg_prev;\n";
    static inline constexpr std::string_view kWorkgroupPreamble =
            "workgroup ScanType wg_partials[S_MEM_SIZE];\n"
            "workgroup ScanType wg_fallback[S_MEM_SIZE];\n";

    static inline constexpr std::string_view kInitialization =
            "void main() {\n"
            "    if (sk_LocalInvocationID.x == 0u) {\n"
            "        wg_broadcast = atomicAdd(spine[work_tiles], 1u);\n"
            "        wg_control = LOCKED;\n"
            "        wg_prev = ScanType(0);\n"
            "    }\n"
            "    uint tile_id = workgroupUniformLoad(wg_broadcast);\n"
            "    uint curve_id = sk_LocalInvocationID.x + tile_id * PART_SIZE;\n";

    static inline constexpr std::string_view kFallbackLookback =
            "    if (sk_LocalInvocationID.x == 0u) {\n"
            "        ScanType local_agg = wg_partials[S_MEM_SIZE - 1u];\n"
            "        uint flag = (tile_id == 0u) ? FLAG_INCLUSIVE : FLAG_READY;\n"
            "        atomicStore(spine[tile_id], convertFromScanType(local_agg) | flag);\n"
            "    }\n"
            "\n"
            "    if (tile_id != 0u) {\n"
            "        ScanType prev_red = ScanType(0);\n"
            "        uint lookback_id = tile_id - 1u;\n"
            "        uint control_flag = workgroupUniformLoad(wg_control);\n"
            "        while (control_flag == LOCKED) {\n"
            "            if (sk_LocalInvocationID.x == 0u) {\n"
            "                uint spin_count = 0u;\n"
            "                while (spin_count < MAX_SPIN_COUNT) {\n"
            "                    uint flag_payload = atomicLoad(spine[lookback_id]);\n"
            "                    uint flag = flag_payload & FLAG_MASK;\n"
            "                    if (flag != FLAG_NOT_READY) {\n"
            "                        ScanType val = convertToScanType(flag_payload);\n"
            "                        prev_red = combine(prev_red, val);\n"
            "                        if (flag == FLAG_INCLUSIVE) {\n"
            "                            ScanType total_incl = combine(prev_red, "
            "wg_partials[S_MEM_SIZE - 1u]);\n"
            "                            atomicStore(spine[tile_id], "
            "convertFromScanType(total_incl) | FLAG_INCLUSIVE);\n"
            "                            wg_control = UNLOCKED;\n"
            "                            wg_prev = prev_red;\n"
            "                            break;\n"
            "                        } else {\n"
            "                            spin_count = 0u;\n"
            "                            lookback_id -= 1u;\n"
            "                        }\n"
            "                    } else {\n"
            "                        spin_count += 1u;\n"
            "                    }\n"
            "                }\n"
            "                if (spin_count == MAX_SPIN_COUNT) {\n"
            "                    wg_broadcast = lookback_id;\n"
            "                }\n"
            "            }\n"
            "            control_flag = workgroupUniformLoad(wg_control);\n"
            "\n"
            "            if (control_flag == LOCKED) {\n"
            "                uint fallback_id = wg_broadcast;\n"
            "                uint f_curve_id = sk_LocalInvocationID.x + fallback_id * PART_SIZE;\n";

    static inline constexpr std::string_view kFallbackPostReduce =
            "                if (sk_LocalInvocationID.x == 0u) {\n"
            "                    ScanType f_local_agg = wg_fallback[S_MEM_SIZE - 1];\n"
            "                    uint f_payload =\n"
            "                        atomicMax(spine[fallback_id], "
            "convertFromScanType(f_local_agg) | FLAG_READY);\n"
            "                    uint f_flag = f_payload & FLAG_MASK;\n"
            "                    if (f_flag == FLAG_INCLUSIVE) {\n"
            "                        prev_red = combine(prev_red, convertToScanType(f_payload));\n"
            "                    } else {\n"
            "                        prev_red = combine(prev_red, f_local_agg);\n"
            "                    }\n"
            "\n"
            "                    if (fallback_id == 0u || f_flag == FLAG_INCLUSIVE) {\n"
            "                        ScanType total_incl = combine(prev_red, "
            "wg_partials[S_MEM_SIZE - 1u]);\n"
            "                        atomicStore(spine[tile_id], convertFromScanType(total_incl) | "
            "FLAG_INCLUSIVE);\n"
            "                        wg_control = UNLOCKED;\n"
            "                        wg_prev = prev_red;\n"
            "                    } else {\n"
            "                        lookback_id -= 1u;\n"
            "                    }\n"
            "                }\n"
            "                control_flag = workgroupUniformLoad(wg_control);\n"
            "            }\n"
            "        }\n"
            "    }\n";

    static inline constexpr std::string_view kClosingBrace = "}\n";

private:
    static constexpr size_t CountOccurrences(std::string_view sv, std::string_view target) {
        size_t count = 0;
        size_t pos = 0;
        while ((pos = sv.find(target, pos)) != std::string_view::npos) {
            ++count;
            pos += target.size();
        }
        return count;
    }

    static constexpr size_t SubstitutedLength(std::string_view sv,
                                              std::string_view target,
                                              std::string_view replacement) {
        size_t occurrences = CountOccurrences(sv, target);
        return sv.size() + occurrences * (replacement.size() - target.size());
    }

    static constexpr size_t CalcLen(std::string_view sv) {
        return SubstitutedLength(sv, "ScanType", ConversionPolicy::kScanType);
    }

    static constexpr size_t kTotalLength =
            CalcLen(kBasePreamble) + CalcLen(PreamblePolicy::kPreamble) +
            CalcLen(PreamblePolicy::kBufferPreamble) + CalcLen(ConversionPolicy::kPreamble) +
            CalcLen(kWorkgroupPreamble) + CalcLen(kInitialization) +
            CalcLen(LocalScanPolicy::kScan) + CalcLen(WorkgroupScanPolicy::kScan) +
            CalcLen(kFallbackLookback) + CalcLen(WorkgroupScanPolicy::kFallback) +
            CalcLen(kFallbackPostReduce) + CalcLen(LocalScanPolicy::kFanout) +
            CalcLen(kClosingBrace);

    struct ShaderBuffer {
        char data[kTotalLength + 1]{};

        constexpr ShaderBuffer() {
            size_t offset = 0;
            constexpr std::string_view kTarget = "ScanType";
            constexpr std::string_view kRepl = ConversionPolicy::kScanType;

            auto append = [&](std::string_view sv) {
                size_t i = 0;
                while (i < sv.size()) {
                    if (sv.substr(i, kTarget.size()) == kTarget) {
                        for (size_t r = 0; r < kRepl.size(); ++r) {
                            data[offset++] = kRepl[r];
                        }
                        i += kTarget.size();
                    } else {
                        data[offset++] = sv[i];
                        i += 1;
                    }
                }
            };
            append(kBasePreamble);
            append(PreamblePolicy::kPreamble);
            append(PreamblePolicy::kBufferPreamble);
            append(ConversionPolicy::kPreamble);
            append(kWorkgroupPreamble);
            append(kInitialization);
            append(LocalScanPolicy::kScan);
            append(WorkgroupScanPolicy::kScan);
            append(kFallbackLookback);
            append(WorkgroupScanPolicy::kFallback);
            append(kFallbackPostReduce);
            append(LocalScanPolicy::kFanout);
            append(kClosingBrace);
            data[kTotalLength] = '\0';
        }
    };

    static inline constexpr ShaderBuffer kBuffer{};

public:
    // The final combined SkSL shader string generated at compile time.
    static inline constexpr std::string_view kShader{kBuffer.data, kTotalLength};
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_ScanComposer_DEFINED
