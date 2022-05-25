// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/pdf/SkPDFSubsetFont.h"

#if defined(SK_PDF_USE_HARFBUZZ_SUBSET)

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/utils/SkCallableTraits.h"

#include "hb.h"
#include "hb-subset.h"

template <class T, void(*P)(T*)> using resource =
    std::unique_ptr<T, SkFunctionWrapper<std::remove_pointer_t<decltype(P)>, P>>;
using HBBlob = resource<hb_blob_t, &hb_blob_destroy>;
using HBFace = resource<hb_face_t, &hb_face_destroy>;
using HBSubsetInput = resource<hb_subset_input_t, &hb_subset_input_destroy>;
using HBSet = resource<hb_set_t, &hb_set_destroy>;

hb_blob_t* skhb_get_table(hb_face_t* face, hb_tag_t tag, void* user_data) {
    SkTypeface& typeface = *reinterpret_cast<SkTypeface*>(user_data);

    auto data = typeface.copyTableData(tag);
    if (!data) {
        return nullptr;
    }
    SkData* rawData = data.release();
    return hb_blob_create(reinterpret_cast<char*>(rawData->writable_data()), rawData->size(),
                          HB_MEMORY_MODE_READONLY, rawData, [](void* ctx) {
                              SkSafeUnref(((SkData*)ctx));
                          });
}

HBBlob stream_to_blob(std::unique_ptr<SkStreamAsset> asset) {
    size_t size = asset->getLength();
    HBBlob blob;
    if (const void* base = asset->getMemoryBase()) {
        blob.reset(hb_blob_create((char*)base, SkToUInt(size),
                                  HB_MEMORY_MODE_READONLY, asset.release(),
                                  [](void* p) { delete (SkStreamAsset*)p; }));
    } else {
        // SkDebugf("Extra SkStreamAsset copy\n");
        void* ptr = size ? sk_malloc_throw(size) : nullptr;
        asset->read(ptr, size);
        blob.reset(hb_blob_create((char*)ptr, SkToUInt(size),
                                  HB_MEMORY_MODE_READONLY, ptr, sk_free));
    }
    SkASSERT(blob);
    hb_blob_make_immutable(blob.get());
    return blob;
}

static sk_sp<SkData> to_data(HBBlob blob) {
    if (!blob) {
        return nullptr;
    }
    unsigned int length;
    const char* data = hb_blob_get_data(blob.get(), &length);
    if (!data || !length) {
        return nullptr;
    }
    return SkData::MakeWithProc(data, SkToSizeT(length),
                                [](const void*, void* ctx) { hb_blob_destroy((hb_blob_t*)ctx); },
                                blob.release());
}

template<typename...> using void_t = void;
template<typename T, typename = void>
struct SkPDFHarfBuzzSubset {
    // This is the HarfBuzz 3.0 interface.
    // hb_subset_flags_t does not exist in 2.0. It isn't dependent on T, so inline the value of
    // HB_SUBSET_FLAGS_RETAIN_GIDS until 2.0 is no longer supported.
    static HBFace Make(T input, hb_face_t* face) {
        // TODO: When possible, check if a font is 'tricky' with FT_IS_TRICKY.
        // If it isn't known if a font is 'tricky', retain the hints.
        hb_subset_input_set_flags(input, 2/*HB_SUBSET_FLAGS_RETAIN_GIDS*/);
        return HBFace(hb_subset_or_fail(face, input));
    }
};
template<typename T>
struct SkPDFHarfBuzzSubset<T, void_t<
    decltype(hb_subset_input_set_retain_gids(std::declval<T>(), std::declval<bool>())),
    decltype(hb_subset_input_set_drop_hints(std::declval<T>(), std::declval<bool>())),
    decltype(hb_subset(std::declval<hb_face_t*>(), std::declval<T>()))
    >>
{
    // This is the HarfBuzz 2.0 (non-public) interface, used if it exists.
    // This code should be removed as soon as all users are migrated to the newer API.
    static HBFace Make(T input, hb_face_t* face) {
        hb_subset_input_set_retain_gids(input, true);
        // TODO: When possible, check if a font is 'tricky' with FT_IS_TRICKY.
        // If it isn't known if a font is 'tricky', retain the hints.
        hb_subset_input_set_drop_hints(input, false);
        return HBFace(hb_subset(face, input));
    }
};

static sk_sp<SkData> subset_harfbuzz(const SkTypeface& typeface,
                                     const SkPDFGlyphUse& glyphUsage) {
    int index = 0;
    std::unique_ptr<SkStreamAsset> typefaceAsset = typeface.openExistingStream(&index);
    HBFace face;
    if (typefaceAsset && typefaceAsset->getMemoryBase()) {
        HBBlob blob(stream_to_blob(std::move(typefaceAsset)));
        // hb_face_create always succeeds. Check that the format is minimally recognized first.
        // hb_face_create_for_tables may still create a working hb_face.
        // See https://github.com/harfbuzz/harfbuzz/issues/248 .
        unsigned int num_hb_faces = hb_face_count(blob.get());
        if (0 < num_hb_faces && (unsigned)index < num_hb_faces) {
            face.reset(hb_face_create(blob.get(), (unsigned)index));
            // Check the number of glyphs as a basic sanitization step.
            if (face && hb_face_get_glyph_count(face.get()) == 0) {
                face.reset();
            }
        }
    }
    if (!face) {
        face.reset(hb_face_create_for_tables(
            skhb_get_table,
            const_cast<SkTypeface*>(SkRef(&typeface)),
            [](void* user_data){ SkSafeUnref(reinterpret_cast<SkTypeface*>(user_data)); }));
        hb_face_set_index(face.get(), (unsigned)index);
    }
    SkASSERT(face);

    HBSubsetInput input(hb_subset_input_create_or_fail());
    SkASSERT(input);
    if (!face || !input) {
        return nullptr;
    }
    hb_set_t* glyphs = hb_subset_input_glyph_set(input.get());
    glyphUsage.getSetValues([&glyphs](unsigned gid) { hb_set_add(glyphs, gid);});

    HBFace subset = SkPDFHarfBuzzSubset<hb_subset_input_t*>::Make(input.get(), face.get());
    if (!subset) {
        return nullptr;
    }
    HBBlob result(hb_face_reference_blob(subset.get()));
    return to_data(std::move(result));
}

#endif  // defined(SK_PDF_USE_HARFBUZZ_SUBSET)

////////////////////////////////////////////////////////////////////////////////

#if defined(SK_PDF_USE_SFNTLY)

#include "include/core/SkData.h"
#include "sample/chromium/font_subsetter.h"
#include <vector>

// if possible, make no copy.
static sk_sp<SkData> stream_to_data(std::unique_ptr<SkStreamAsset> stream) {
    SkASSERT(stream);
    (void)stream->rewind();
    SkASSERT(stream->hasLength());
    size_t size = stream->getLength();
    if (const void* base = stream->getMemoryBase()) {
        SkData::ReleaseProc proc = [](const void*, void* ctx) { delete (SkStreamAsset*)ctx; };
        return SkData::MakeWithProc(base, size, proc, stream.release());
    }
    return SkData::MakeFromStream(stream.get(), size);
}

static sk_sp<SkData> subset_sfntly(const SkTypeface& typeface,
                                   const SkPDFGlyphUse& glyphUsage,
                                   const char* fontName) {
#if defined(SK_USING_THIRD_PARTY_ICU)
    if (!SkLoadICU()) {
        return nullptr;
    }
#endif

    int ttcIndex;
    std::unique_ptr<SkStreamAsset> fontAsset = typeface.openStream(&ttcIndex);
    sk_sp<SkData> fontData = stream_to_data(std::move(fontAsset));

    // Generate glyph id array in format needed by sfntly.
    // TODO(halcanary): sfntly should take a more compact format.
    std::vector<unsigned> subset;
    glyphUsage.getSetValues([&subset](unsigned v) { subset.push_back(v); });

    unsigned char* subsetFont{nullptr};
#if defined(SK_BUILD_FOR_GOOGLE3)
    // TODO(halcanary): update SK_BUILD_FOR_GOOGLE3 to newest version of Sfntly.
    (void)ttcIndex;
    int subsetFontSize = SfntlyWrapper::SubsetFont(fontName,
                                                   fontData->bytes(),
                                                   fontData->size(),
                                                   subset.data(),
                                                   subset.size(),
                                                   &subsetFont);
#else  // defined(SK_BUILD_FOR_GOOGLE3)
    (void)fontName;
    int subsetFontSize = SfntlyWrapper::SubsetFont(ttcIndex,
                                                   fontData->bytes(),
                                                   fontData->size(),
                                                   subset.data(),
                                                   subset.size(),
                                                   &subsetFont);
#endif  // defined(SK_BUILD_FOR_GOOGLE3)
    SkASSERT(subsetFontSize > 0 || subsetFont == nullptr);
    if (subsetFontSize < 1 || subsetFont == nullptr) {
        return nullptr;
    }
    return SkData::MakeWithProc(subsetFont, subsetFontSize,
                                [](const void* p, void*) { delete[] (unsigned char*)p; },
                                nullptr);
}

#endif  // defined(SK_PDF_USE_SFNTLY)

////////////////////////////////////////////////////////////////////////////////

#if defined(SK_PDF_USE_SFNTLY) && defined(SK_PDF_USE_HARFBUZZ_SUBSET)

sk_sp<SkData> SkPDFSubsetFont(const SkTypeface& typeface,
                              const SkPDFGlyphUse& glyphUsage,
                              SkPDF::Metadata::Subsetter subsetter,
                              const char* fontName) {
    switch (subsetter) {
        case SkPDF::Metadata::kHarfbuzz_Subsetter:
            return subset_harfbuzz(typeface, glyphUsage);
        case SkPDF::Metadata::kSfntly_Subsetter:
            return subset_sfntly(typeface, glyphUsage, fontName);
    }
    return nullptr;
}

#elif defined(SK_PDF_USE_SFNTLY)

sk_sp<SkData> SkPDFSubsetFont(const SkTypeface& typeface,
                              const SkPDFGlyphUse& glyphUsage,
                              SkPDF::Metadata::Subsetter,
                              const char* fontName) {
    return subset_sfntly(typeface, glyphUsage, fontName);
}

#elif defined(SK_PDF_USE_HARFBUZZ_SUBSET)

sk_sp<SkData> SkPDFSubsetFont(const SkTypeface& typeface,
                              const SkPDFGlyphUse& glyphUsage,
                              SkPDF::Metadata::Subsetter,
                              const char*) {
    return subset_harfbuzz(typeface, glyphUsage);
}

#else

sk_sp<SkData> SkPDFSubsetFont(const SkTypeface&,
                              const SkPDFGlyphUse&,
                              SkPDF::Metadata::Subsetter,
                              const char*) {
    return nullptr;
}
#endif  // defined(SK_PDF_USE_SFNTLY)
