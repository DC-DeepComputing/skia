/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFGraphicState_DEFINED
#define SkPDFGraphicState_DEFINED

#include "include/core/SkScalar.h"
#include "include/private/base/SkMacros.h"
#include "src/base/SkFloatBits.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkPathEffectBase.h"
#include "src/pdf/SkPDFTypes.h"

#include <cstdint>
#include <cstring>

class SkPDFDocument;
class SkPaint;

/** \class SkPDFGraphicState
    SkPaint objects roughly correspond to graphic state dictionaries that can
    be installed. So that a given dictionary is only output to the pdf file
    once, we want to canonicalize them.
*/
namespace SkPDFGraphicState {
    enum SkPDFSMaskMode {
        kAlpha_SMaskMode,
        kLuminosity_SMaskMode
    };

    /** Get the graphic state for the passed SkPaint.
     */
    SkPDFIndirectReference GetGraphicStateForPaint(SkPDFDocument*, const SkPaint&);

    /** Make a graphic state that only sets the passed soft mask.
     *  @param sMask     The form xobject to use as a soft mask.
     *  @param invert    Indicates if the alpha of the sMask should be inverted.
     *  @param sMaskMode Whether to use alpha or luminosity for the sMask.
     *
     *  These are not de-duped.
     */
    SkPDFIndirectReference GetSMaskGraphicState(SkPDFIndirectReference sMask,
                                                bool invert,
                                                SkPDFSMaskMode sMaskMode,
                                                SkPDFDocument* doc);
}  // namespace SkPDFGraphicState

struct SkPDFStrokeGraphicState {
    struct Key {
        SkScalar fStrokeWidth;
        SkScalar fStrokeMiter;
        SkScalar fAlpha;
        uint8_t fStrokeCap;   // SkPaint::Cap
        uint8_t fStrokeJoin;  // SkPaint::Join
        uint8_t fBlendMode;   // SkBlendMode
        SkPathEffectBase::DashInfo fDashInfo;  // Non-owning

        bool operator==(const Key& that) const = default;
        struct Hash {
            uint32_t operator()(const Key& k) const {
                SkGoodHash hash;
                return hash(SkFloat2Bits(k.fStrokeWidth)) ^
                       hash(SkFloat2Bits(k.fStrokeMiter)) ^
                       hash(SkFloat2Bits(k.fAlpha)) ^
                       hash(k.fStrokeCap) ^
                       hash(k.fStrokeJoin) ^
                       hash(k.fBlendMode) ^
                       SkPathEffectBase::DashInfo::Hash()(k.fDashInfo);
            }
        };
    };

    struct Traits {
        static const Key& GetKey(const std::unique_ptr<SkPDFStrokeGraphicState>& p) {
            return p->fKey;
        }
        static uint32_t Hash(const Key& key) { return Key::Hash()(key); }
    };

    SkPDFStrokeGraphicState() = delete;
    SkPDFStrokeGraphicState(const SkPDFStrokeGraphicState&) = delete;
    const SkPDFStrokeGraphicState& operator=(const SkPDFStrokeGraphicState&) = delete;
    SkPDFStrokeGraphicState(SkPDFStrokeGraphicState&&) = delete;
    const SkPDFStrokeGraphicState& operator=(SkPDFStrokeGraphicState&) = delete;

    SkPDFStrokeGraphicState(const Key& key, SkPDFIndirectReference ref) : fKey(key), fRef(ref) {
        if (!fKey.fDashInfo.fIntervals.empty()) {
            SkSpan<const SkScalar>& oldIntervals = fKey.fDashInfo.fIntervals;
            std::unique_ptr<SkScalar[]> newIntervals(new SkScalar[oldIntervals.size()]);
            std::copy(oldIntervals.begin(), oldIntervals.end(), newIntervals.get());
            oldIntervals = SkSpan<const SkScalar>(newIntervals.release(), oldIntervals.size());
        }
    }
    ~SkPDFStrokeGraphicState() {
        if (!fKey.fDashInfo.fIntervals.empty()) {
            delete [] fKey.fDashInfo.fIntervals.data();
        }
    }

    Key fKey;
    SkPDFIndirectReference fRef;
};

SK_BEGIN_REQUIRE_DENSE
struct SkPDFFillGraphicState {
    SkScalar fAlpha;
    uint8_t fBlendMode;
    uint8_t fPADDING[3] = {0, 0, 0};
    bool operator==(const SkPDFFillGraphicState& o) const { return !memcmp(this, &o, sizeof(o)); }
    bool operator!=(const SkPDFFillGraphicState& o) const { return !(*this == o); }

    using Hash = SkForceDirectHash<SkPDFFillGraphicState>;
};
SK_END_REQUIRE_DENSE

#endif
