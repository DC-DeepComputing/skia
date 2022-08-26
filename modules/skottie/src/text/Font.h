/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieFont_DEFINED
#define SkottieFont_DEFINED

#include "include/core/SkTypeface.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkTHash.h"
#include "include/utils/SkCustomTypeface.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <memory>
#include <vector>

class SkPath;

namespace skjson { class ObjectValue; }

namespace skottie::internal {

class AnimationBuilder;

class Font final : SkNoncopyable {
public:
    ~Font();

    using GlyphCompMap = SkTHashMap<SkGlyphID, sk_sp<sksg::RenderNode>>;

    class Builder final : SkNoncopyable {
    public:
        bool parseGlyph(const AnimationBuilder*, const skjson::ObjectValue&);
        std::unique_ptr<Font> detach();

    private:
        static bool ParseGlyphPath(const AnimationBuilder*, const skjson::ObjectValue&, SkPath*);
        static sk_sp<sksg::RenderNode> ParseGlyphComp(const AnimationBuilder*,
                                                      const skjson::ObjectValue&);

        GlyphCompMap            fGlyphComps;
        SkCustomTypefaceBuilder fCustomBuilder;
    };

    const sk_sp<SkTypeface>& typeface() const { return fTypeface; }

    sk_sp<sksg::RenderNode> operator[](SkGlyphID) const;

    int customGlyphCount() const { return fGlyphComps.count(); }

private:
    Font(GlyphCompMap&&, sk_sp<SkTypeface> tf);

    const GlyphCompMap      fGlyphComps;
    const sk_sp<SkTypeface> fTypeface;
};

class GlyphCompMapper final : public SkRefCnt {
public:
    explicit GlyphCompMapper(std::vector<std::unique_ptr<Font>>&& fonts)
        : fFonts(std::move(fonts)) {}

    ~GlyphCompMapper() override = default;

    sk_sp<sksg::RenderNode> getGlyphComp(const SkTypeface*, SkGlyphID) const;

private:
    const std::vector<std::unique_ptr<Font>> fFonts;
};

}  // namespace skottie::internal

#endif  // SkottieFont_DEFINED
