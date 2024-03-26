/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/utils/TextPreshape.h"

#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTo.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/TextShaper.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/text/TextValue.h"
#include "modules/skresources/include/SkResources.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkUTF.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/utils/SkJSON.h"

#include <memory>
#include <vector>
#include <unordered_map>

using ResourceProvider = skresources::ResourceProvider;

using skjson::Value;
using skjson::NullValue;
using skjson::BoolValue;
using skjson::NumberValue;
using skjson::ArrayValue;
using skjson::StringValue;
using skjson::ObjectValue;

namespace skottie::internal {
    class LayerBuilder {
    public:
        static void ParseFonts(AnimationBuilder& ab, const ObjectValue* jfonts,
                               const ArrayValue* jchars) {
            ab.parseFonts(jfonts, jchars);
        }
        static int GetFontCount(const AnimationBuilder& ab) {
            return ab.fFonts.count();
        }
    };
} //  namespace skottie::internal

using BuilderHelper = skottie::internal::LayerBuilder;


namespace {

struct GlyphCacheKey {
    SkFont    fFont;
    SkGlyphID fGlyphID;

    bool operator==(const GlyphCacheKey& other) const {
        return fFont == other.fFont && fGlyphID == other.fGlyphID;
    }
    bool operator!=(const GlyphCacheKey& other) const { return !(*this == other); }
};

} //  namespace

template <>
struct std::hash<GlyphCacheKey> {
    size_t operator()(const GlyphCacheKey& key) const {
        struct SK_ATTRIBUTE(packed) {
            SkTypefaceID fTypefaceID;
            float        fSize;
            float        fScaleX;
            float        fSkewX;
            SkGlyphID    fGlyphID;
            uint16_t     fFlags;
            uint16_t     fEdging;
            uint16_t     fHinting;
        } hash_key = {
            key.fFont.getTypeface() ? key.fFont.getTypeface()->uniqueID() : 0,
            key.fFont.getSize(),
            key.fFont.getScaleX(),
            key.fFont.getSkewX(),
            key.fGlyphID,
            SkFontPriv::Flags(key.fFont),
            static_cast<uint16_t>(key.fFont.getEdging()),
            static_cast<uint16_t>(key.fFont.getHinting()),
        };

        return SkChecksum::Hash32(&hash_key, sizeof(hash_key));
    }
};

namespace {

class Preshaper {
public:
    static std::unique_ptr<Preshaper> Make(const skjson::DOM& dom,
                                           sk_sp<ResourceProvider> rp,
                                           sk_sp<SkFontMgr> fontmgr) {
        if (!dom.root().is<ObjectValue>()) {
            return nullptr;
        }

        const auto& jlottie = dom.root();
        const auto size     = SkSize::Make(skottie::ParseDefault<float>(jlottie["w"], 0.0f),
                                           skottie::ParseDefault<float>(jlottie["h"], 0.0f));
        const auto fps      = skottie::ParseDefault<float>(jlottie["fr"], -1.0f),
                   inPoint  = skottie::ParseDefault<float>(jlottie["ip"], 0.0f),
                   outPoint = std::max(skottie::ParseDefault<float>(jlottie["op"], SK_ScalarMax), inPoint),
                   duration = sk_ieee_float_divide(outPoint - inPoint, fps);

        if (size.isEmpty() || fps <= 0 || !SkScalarIsFinite(inPoint) ||
            !SkScalarIsFinite(outPoint) || !SkScalarIsFinite(duration)) {
            return nullptr;
        }

        if (!rp) {
            class NullResourceProvider final : public ResourceProvider {
                sk_sp<SkData> load(const char[], const char[]) const override { return nullptr; }
            };
            rp = sk_make_sp<NullResourceProvider>();
        }

        return std::unique_ptr<Preshaper>(new Preshaper(jlottie,
                                                        std::move(rp),
                                                        std::move(fontmgr),
                                                        size, duration, fps));
    }

    void preshape() {
        this->preshapeComp(fLottie);
        if (const ArrayValue* jassets = fLottie["assets"]) {
            for (const auto& jasset : *jassets) {
                this->preshapeComp(jasset);
            }
        }

        fLottie.as<ObjectValue>().writable("chars", fAlloc) = this->jsonChars();
    }

private:
    Preshaper(const Value& jlottie, sk_sp<ResourceProvider> rp, sk_sp<SkFontMgr> fontmgr,
              const SkSize& size, float duration, float fps)
        : fLottie(jlottie)
        , fFontMgr(fontmgr)
        , fBuilder(std::move(rp), std::move(fontmgr),
                   nullptr, nullptr, nullptr, nullptr, nullptr,
                   &fStats, size, duration, fps, 0)
        , fAlloc(4096)
    {
        BuilderHelper::ParseFonts(fBuilder, jlottie["fonts"], jlottie["chars"]);
        SkDebugf("Parsed %d font(s).\n", BuilderHelper::GetFontCount(fBuilder));
    }

    void preshapeComp(const Value& jcomp) {
       if (const ArrayValue* jlayers = jcomp["layers"]) {
           for (const auto& jlayer : *jlayers) {
               this->preshapeLayer(jlayer);
           }
       }
    }

    void preshapeLayer(const Value& jlayer) {
        if (skottie::ParseDefault<int>(jlayer["ty"], -1) != 5) {
            return;
        }

        const auto name = skottie::ParseDefault(jlayer["nm"], SkString());
        SkDebugf("Preshaping layer: %s\n", name.c_str());

        const ArrayValue* jtxts = jlayer["t"]["d"]["k"];
        if (!jtxts) {
            return;
        }

        for (const auto& jtxt : *jtxts) {
            skottie::TextValue txt_val;
            if (!skottie::internal::Parse(jtxt["s"], fBuilder , &txt_val)) {
                continue;
            }

            SkDebugf("Text frame: %s\n", txt_val.fText.c_str());
            static constexpr float kMinSize =    0.1f,
                                   kMaxSize = 1296.0f;
            const skottie::Shaper::TextDesc text_desc = {
                txt_val.fTypeface,
                SkTPin(txt_val.fTextSize,    kMinSize, kMaxSize),
                SkTPin(txt_val.fMinTextSize, kMinSize, kMaxSize),
                SkTPin(txt_val.fMaxTextSize, kMinSize, kMaxSize),
                txt_val.fLineHeight,
                txt_val.fLineShift,
                txt_val.fAscent,
                txt_val.fHAlign,
                txt_val.fVAlign,
                txt_val.fResize,
                txt_val.fLineBreak,
                txt_val.fDirection,
                txt_val.fCapitalization,
                txt_val.fMaxLines,
                skottie::Shaper::Flags::kFragmentGlyphs |
                    skottie::Shaper::Flags::kTrackFragmentAdvanceAscent |
                    skottie::Shaper::Flags::kClusters,
                txt_val.fLocale.isEmpty()     ? nullptr : txt_val.fLocale.c_str(),
                txt_val.fFontFamily.isEmpty() ? nullptr : txt_val.fFontFamily.c_str(),
            };

            auto shape_result =
                    skottie::Shaper::Shape(txt_val.fText, text_desc, txt_val.fBox, fFontMgr);

            auto shaped_glyph_info = [this](SkUnichar ch, const SkPoint& pos, float advance,
                                            size_t line, size_t cluster) -> Value {
                const NumberValue jpos[] = { NumberValue(pos.fX), NumberValue(pos.fY) };
                char utf8[SkUTF::kMaxBytesInUTF8Sequence];
                const size_t utf8_len = SkUTF::ToUTF8(ch, utf8);

                const skjson::Member fields[] = {
                    { StringValue("ch" , fAlloc), StringValue(utf8, utf8_len, fAlloc)       },
                    { StringValue("ps" , fAlloc), ArrayValue(jpos, std::size(jpos), fAlloc) },
                    { StringValue("w"  , fAlloc), NumberValue(advance)                      },
                    { StringValue("l"  , fAlloc), NumberValue(SkToInt(line))                },
                    { StringValue("cix", fAlloc), NumberValue(SkToInt(cluster))             },
                };

                return ObjectValue(fields, std::size(fields), fAlloc);
            };

            std::vector<Value> shaped_info;
            for (const auto& frag : shape_result.fFragments) {
                SkASSERT(frag.fGlyphs.fGlyphIDs.size() == 1);
                SkASSERT(frag.fGlyphs.fClusters.size() == frag.fGlyphs.fGlyphIDs.size());
                size_t offset = 0;
                for (const auto& runrec : frag.fGlyphs.fRuns) {
                    const SkGlyphID*  glyphs = frag.fGlyphs.fGlyphIDs.data() + offset;
                    const SkPoint* glyph_pos = frag.fGlyphs.fGlyphPos.data() + offset;
                    const size_t*   clusters = frag.fGlyphs.fClusters.data() + offset;
                    const char*     end_utf8 = txt_val.fText.c_str() + txt_val.fText.size();
                    for (size_t i = 0; i < runrec.fSize; ++i) {
                        const char* ch_utf8 = txt_val.fText.c_str() + clusters[i];
                        const SkUnichar ch = SkUTF::NextUTF8(&ch_utf8, end_utf8);

                        this->addGlyphToFont(runrec.fFont, glyphs[i], ch);
                        shaped_info.push_back(shaped_glyph_info(ch,
                                                                frag.fOrigin + glyph_pos[i],
                                                                frag.fAdvance,
                                                                frag.fLineIndex,
                                                                clusters[i]));
                    }
                    offset += runrec.fSize;
                }
            }

            jtxt["s"].as<ObjectValue>().writable("ts", fAlloc) =
                ArrayValue(shaped_info.data(), shaped_info.size(), fAlloc);
            jtxt["s"].as<ObjectValue>().writable("s", fAlloc) =
                NumberValue(text_desc.fTextSize * shape_result.fScale);
        }
    }

    size_t addGlyphToFont(const SkFont& font, SkGlyphID gid, SkUnichar ch) {
        auto res = fGlyphMap.try_emplace({font, gid});
        if (res.second) {
            const float scale = 100 / font.getSize();
            SkPath path;
            font.getPath(gid, &path);
            float width;
            font.getWidths(&gid, 1, &width);
            res.first->second = {
                fGlyphCount++, ch, width * scale, path.makeTransform(SkMatrix::Scale(scale, scale))
            };
        }

        SkDEBUGF("%d ..%zu glyphs.\n", ch, fGlyphCount);
        return res.first->second.fIndex;
    }

    Value jsonChars() {
        std::vector<Value> chars;
        chars.reserve(fGlyphCount);

        for (const auto& g : fGlyphMap) {
            char utf8[SkUTF::kMaxBytesInUTF8Sequence];
            size_t utf8_len = SkUTF::ToUTF8(g.second.fChar, utf8);
            skjson::Member fields[] = {
                { StringValue("ch"     , fAlloc), StringValue(utf8, utf8_len, fAlloc)  },
                { StringValue("fFamily", fAlloc), StringValue("Custom" , fAlloc)       },
                { StringValue("style"  , fAlloc), StringValue("Preshaped", fAlloc)     },
                { StringValue("size"   , fAlloc), NumberValue(100)                     },
                { StringValue("w"      , fAlloc), NumberValue(g.second.fWidth)         },
                { StringValue("data"   , fAlloc), this->pathToLottie(g.second.fPath)   },
            };

            chars.push_back(ObjectValue(fields, std::size(fields), fAlloc));
        }

        return ArrayValue(chars.data(), chars.size(), fAlloc);
    }

    Value pathToLottie(const SkPath& path) {
        struct Contour {
            std::vector<SkPoint> verts, in_tan, out_tan;
            bool closed = false;

            void add(const SkPoint& v, const SkPoint& i, const SkPoint& o) {
                verts.push_back(v);
                in_tan.push_back(i);
                out_tan.push_back(o);
            }

            size_t size() const {
                SkASSERT(verts.size() == in_tan.size());
                SkASSERT(verts.size() == out_tan.size());
                return verts.size();
            }
        };

        std::vector<Contour> contours(1);

        for (const auto [verb, pts, weights] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kMove:
                    if (!contours.back().verts.empty()) {
                        contours.emplace_back();
                    }
                    contours.back().add(pts[0], {0, 0}, {0, 0});
                    break;
                case SkPathVerb::kClose:
                    SkASSERT(contours.back().size() > 0);
                    contours.back().closed = true;
                    break;
                case SkPathVerb::kLine:
                    SkASSERT(contours.back().size() > 0);
                    SkASSERT(pts[0] == contours.back().verts.back());
                    contours.back().add(pts[1], {0, 0}, {0, 0});
                    break;
                case SkPathVerb::kQuad:
                    SkASSERT(contours.back().size() > 0);
                    SkASSERT(pts[0] == contours.back().verts.back());
                    SkPoint cubic[4];
                    SkConvertQuadToCubic(pts, cubic);
                    contours.back().out_tan.back() = cubic[1] - cubic[0];
                    contours.back().add(cubic[3], cubic[2] - cubic[3], {0, 0});
                    break;
                case SkPathVerb::kCubic:
                    SkASSERT(contours.back().size() > 0);
                    SkASSERT(pts[0] == contours.back().verts.back());
                    contours.back().out_tan.back() = pts[1] - pts[0];
                    contours.back().add(pts[3], pts[2] - pts[3], {0, 0});
                    break;
                case SkPathVerb::kConic:
                    SkDebugf("Unexpected conic verb!\n");
                    break;
            }
        }

        auto pts2lottie = [this](const std::vector<SkPoint> v) {
            std::vector<Value> vec(v.size());
            for (size_t i = 0; i < v.size(); ++i) {
                Value fields[] = { NumberValue(v[i].fX), NumberValue(v[i].fY) };
                vec[i] = ArrayValue(fields, std::size(fields), fAlloc);
            }

            return ArrayValue(vec.data(), vec.size(), fAlloc);
        };

        std::vector<Value> jcontours(contours.size());
        for (size_t i = 0; i < contours.size(); ++i) {
            const skjson::Member fields_k[] = {
                { StringValue("v", fAlloc), pts2lottie(contours[i].verts)   },
                { StringValue("i", fAlloc), pts2lottie(contours[i].in_tan)  },
                { StringValue("o", fAlloc), pts2lottie(contours[i].out_tan) },
                { StringValue("c", fAlloc), BoolValue(contours[i].closed)   },
            };

            const skjson::Member fields_ks[] = {
                { StringValue("a", fAlloc), NumberValue(0)                                     },
                { StringValue("k", fAlloc), ObjectValue(fields_k, std::size(fields_k), fAlloc) },
            };

            const skjson::Member fields[] = {
                { StringValue("ty" , fAlloc), StringValue("sh", fAlloc)                         },
                { StringValue("hd" , fAlloc), BoolValue(false)                                  },
                { StringValue("ind", fAlloc), NumberValue(SkToInt(i))                           },
                { StringValue("ks" , fAlloc),
                        ObjectValue(fields_ks, std::size(fields_ks), fAlloc)                    },
                { StringValue("mn" , fAlloc), StringValue("ADBE Vector Shape - Group" , fAlloc) },
                { StringValue("nm" , fAlloc), StringValue("_" , fAlloc)                         },
            };

            jcontours[i] = ObjectValue(fields, std::size(fields), fAlloc);
        }

        const skjson::Member fields_sh[] = {
            { StringValue("ty" , fAlloc), StringValue("gr", fAlloc)                              },
            { StringValue("hd" , fAlloc), BoolValue(false)                                       },
            { StringValue("bm" , fAlloc), NumberValue(0)                                         },
            { StringValue("it" , fAlloc), ArrayValue(jcontours.data(), jcontours.size(), fAlloc) },
            { StringValue("mn" , fAlloc), StringValue("ADBE Vector Group" , fAlloc)              },
            { StringValue("nm" , fAlloc), StringValue("_" , fAlloc)                              },
        };

        const Value shape = ObjectValue(fields_sh, std::size(fields_sh), fAlloc);
        const skjson::Member fields_data[] = {
            { StringValue("shapes" , fAlloc), ArrayValue(&shape, 1, fAlloc) },
        };

        return ObjectValue(fields_data, std::size(fields_data), fAlloc);
    }

    struct GlyphRec {
        size_t    fIndex;
        SkUnichar fChar;
        float     fWidth;
        SkPath    fPath;
    };

    const skjson::Value&                        fLottie;
    const sk_sp<SkFontMgr>                      fFontMgr;
    skottie::Animation::Builder::Stats          fStats;
    skottie::internal::AnimationBuilder         fBuilder;
    SkArenaAlloc                                fAlloc;
    std::unordered_map<GlyphCacheKey, GlyphRec> fGlyphMap;
    size_t                                      fGlyphCount = 0;
};

} //  namespace

namespace skottie_utils {

bool Preshape(const skjson::DOM& in_dom, SkWStream* stream, sk_sp<ResourceProvider> rp,
              sk_sp<SkFontMgr> fontMgr) {
    std::unique_ptr<Preshaper> preshaper =
            Preshaper::Make(in_dom, std::move(rp), std::move(fontMgr));

    if (!preshaper) {
        return false;
    }

    preshaper->preshape();

    SkArenaAlloc alloc(4096);

    stream->writeText(in_dom.root().toString().c_str());

    return true;
}

} //  namespace skottie_utils
