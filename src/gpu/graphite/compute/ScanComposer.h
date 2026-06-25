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
 *   - LocalScanPolicy: exposes `kScan` and `kFanout` (string_view) for thread-local tile scanning and result propagation.
 *   - WorkgroupScanPolicy: exposes `kScan` (inclusive scan) and `kReduce` (reduction) (string_view).
 *   - ConversionPolicy: exposes `kScanType` (string_view type name, e.g., "float", "uint") and
 *     `kPreamble` (string_view) defining `convertFromScanType`, `convertToScanType`, and `combine`.
 *
 * All instances of "ScanType" in the composer templates are substituted at compile time (`constexpr`)
 * with `ConversionPolicy::kScanType`.
 */
template <typename PreamblePolicy,
          typename LocalScanPolicy,
          typename WorkgroupScanPolicy,
          typename ConversionPolicy>
class ScanComposer {
public:
    static inline constexpr std::string_view kBasePreamble = R"(
const uint FLAG_NOT_READY = 0u;
const uint FLAG_READY     = 0x40000000u;
const uint FLAG_INCLUSIVE = 0x80000000u;
const uint FLAG_MASK      = 0xC0000000u;

const uint MAX_SPIN_COUNT = 4u;
const uint LOCKED         = 1u;
const uint UNLOCKED       = 0u;

workgroup uint wg_control;
workgroup uint wg_broadcast;
workgroup ScanType wg_prev;
)";
    static inline constexpr std::string_view kWorkgroupPreamble = R"(
workgroup ScanType wg_partials[S_MEM_SIZE];
workgroup ScanType wg_fallback[S_MEM_SIZE];
)";

    static inline constexpr std::string_view kInitialization = R"(
void main() {
    if (sk_LocalInvocationID.x == 0u) {
        wg_broadcast = atomicAdd(spine[work_tiles], 1u);
        wg_control = LOCKED;
        wg_prev = ScanType(0);
    }
    uint tile_id = workgroupUniformLoad(wg_broadcast);
    uint curve_id = sk_LocalInvocationID.x + tile_id * PART_SIZE;
)";

    static inline constexpr std::string_view kFallbackLookback = R"(
    if (sk_LocalInvocationID.x == 0u) {
        ScanType local_agg = wg_partials[S_MEM_SIZE - 1u];
        uint flag = (tile_id == 0u) ? FLAG_INCLUSIVE : FLAG_READY;
        atomicStore(spine[tile_id], convertFromScanType(local_agg) | flag);
    }

    if (tile_id != 0u) {
        ScanType prev_red = ScanType(0);
        uint lookback_id = tile_id - 1u;
        uint control_flag = workgroupUniformLoad(wg_control);
        while (control_flag == LOCKED) {
            if (sk_LocalInvocationID.x == 0u) {
                uint spin_count = 0u;
                while (spin_count < MAX_SPIN_COUNT) {
                    uint flag_payload = atomicLoad(spine[lookback_id]);
                    uint flag = flag_payload & FLAG_MASK;
                    if (flag != FLAG_NOT_READY) {
                        ScanType val = convertToScanType(flag_payload);
                        prev_red = combine(prev_red, val);
                        if (flag == FLAG_INCLUSIVE) {
                            ScanType total_incl = combine(prev_red, wg_partials[S_MEM_SIZE - 1u]);
                            atomicStore(spine[tile_id], convertFromScanType(total_incl) | FLAG_INCLUSIVE);
                            wg_control = UNLOCKED;
                            wg_prev = prev_red;
                            break;
                        } else {
                            spin_count = 0u;
                            lookback_id -= 1u;
                        }
                    } else {
                        spin_count += 1u;
                    }
                }
                if (spin_count == MAX_SPIN_COUNT) {
                    wg_broadcast = lookback_id;
                }
            }
            control_flag = workgroupUniformLoad(wg_control);

            if (control_flag == LOCKED) {
                uint fallback_id = wg_broadcast;
                uint f_curve_id = sk_LocalInvocationID.x + fallback_id * PART_SIZE;
)";

    static inline constexpr std::string_view kFallbackPostReduce = R"(
                if (sk_LocalInvocationID.x == 0u) {
                    ScanType f_local_agg = wg_fallback[S_MEM_SIZE - 1];
                    uint f_payload =
                        atomicMax(spine[fallback_id], convertFromScanType(f_local_agg) | FLAG_READY);
                    uint f_flag = f_payload & FLAG_MASK;
                    if (f_flag == FLAG_INCLUSIVE) {
                        prev_red = combine(prev_red, convertToScanType(f_payload));
                    } else {
                        prev_red = combine(prev_red, f_local_agg);
                    }

                    if (fallback_id == 0u || f_flag == FLAG_INCLUSIVE) {
                        ScanType total_incl = combine(prev_red, wg_partials[S_MEM_SIZE - 1u]);
                        atomicStore(spine[tile_id], convertFromScanType(total_incl) | FLAG_INCLUSIVE);
                        wg_control = UNLOCKED;
                        wg_prev = prev_red;
                    } else {
                        lookback_id -= 1u;
                    }
                }
                control_flag = workgroupUniformLoad(wg_control);
            }
        }
    }
)";

    static inline constexpr std::string_view kClosingBrace = R"(
}
)";

private:
    static constexpr size_t count_occurrences(std::string_view sv, std::string_view target) {
        size_t count = 0;
        size_t pos = 0;
        while ((pos = sv.find(target, pos)) != std::string_view::npos) {
            ++count;
            pos += target.size();
        }
        return count;
    }

    static constexpr size_t substituted_length(std::string_view sv,
                                               std::string_view target,
                                               std::string_view replacement) {
        size_t occurrences = count_occurrences(sv, target);
        return sv.size() + occurrences * (replacement.size() - target.size());
    }

    static constexpr size_t calc_len(std::string_view sv) {
        return substituted_length(sv, "ScanType", ConversionPolicy::kScanType);
    }

    static constexpr size_t kTotalLength = calc_len(kBasePreamble) +
                                           calc_len(PreamblePolicy::kPreamble) +
                                           calc_len(PreamblePolicy::kBufferPreamble) +
                                           calc_len(ConversionPolicy::kPreamble) +
                                           calc_len(kWorkgroupPreamble) +
                                           calc_len(kInitialization) +
                                           calc_len(LocalScanPolicy::kScan) +
                                           calc_len(WorkgroupScanPolicy::kScan) +
                                           calc_len(kFallbackLookback) +
                                           calc_len(WorkgroupScanPolicy::kReduce) +
                                           calc_len(kFallbackPostReduce) +
                                           calc_len(LocalScanPolicy::kFanout) +
                                           calc_len(kClosingBrace);

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
            append(WorkgroupScanPolicy::kReduce);
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
