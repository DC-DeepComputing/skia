// Copyright 2019 Google LLC.
#ifndef FontArguments_DEFINED
#define FontArguments_DEFINED

#include <functional>
#include <vector>
#include "include/core/SkFontArguments.h"

bool operator==(const SkFontArguments& a, const SkFontArguments& b);
bool operator!=(const SkFontArguments& a, const SkFontArguments& b);

namespace std {
    template<> struct hash<SkFontArguments> {
        size_t operator()(const SkFontArguments& args) const;
    };
}

namespace skia {
namespace textlayout {

class FontArguments {
public:
    static void Clone(const SkFontArguments& src,
                      SkFontArguments& dst,
                      std::vector<SkFontArguments::VariationPosition::Coordinate>& dstCoordinates,
                      std::vector<SkFontArguments::Palette::Override>& dstOverrides);
};

}  // namespace textlayout
}  // namespace skia

#endif  // FontArguments_DEFINED
