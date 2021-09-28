// Copyright 2021 Google LLC.
#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/src/LogicalRun.h"
#include <memory>
#include <stack>

#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkTDPQueue.h"
#include "src/utils/SkUTF.h"

#include <hb-ot.h>
#include <hb.h>
#include <cstring>
#include <hb-buffer.hh>
#include <utility>

namespace {
template <typename T, typename P, P* p>
using resource = std::unique_ptr<T, SkFunctionWrapper<P, p>>;
using HBBlob = resource<hb_blob_t, decltype(hb_blob_destroy), hb_blob_destroy>;
using HBFace = resource<hb_face_t, decltype(hb_face_destroy), hb_face_destroy>;
using HBFont = resource<hb_font_t, decltype(hb_font_destroy), hb_font_destroy>;
using HBBuffer = resource<hb_buffer_t, decltype(hb_buffer_destroy), hb_buffer_destroy>;

using SkUnicodeBidi = std::unique_ptr<SkBidiIterator>;
using SkUnicodeBreak = std::unique_ptr<SkBreakIterator>;
using SkUnicodeScript = std::unique_ptr<SkScriptIterator>;

hb_position_t skhb_position(SkScalar value) {
    // Treat HarfBuzz hb_position_t as 16.16 fixed-point.
    constexpr int kHbPosition1 = 1 << 16;
    return SkScalarRoundToInt(value * kHbPosition1);
}

hb_bool_t skhb_glyph(hb_font_t* hb_font,
                     void* font_data,
                     hb_codepoint_t unicode,
                     hb_codepoint_t variation_selector,
                     hb_codepoint_t* glyph,
                     void* user_data) {
    SkFont& font = *reinterpret_cast<SkFont*>(font_data);

    *glyph = font.unicharToGlyph(unicode);
    return *glyph != 0;
}

hb_bool_t skhb_nominal_glyph(hb_font_t* hb_font,
                             void* font_data,
                             hb_codepoint_t unicode,
                             hb_codepoint_t* glyph,
                             void* user_data) {
    return skhb_glyph(hb_font, font_data, unicode, 0, glyph, user_data);
}

unsigned skhb_nominal_glyphs(hb_font_t* hb_font,
                             void* font_data,
                             unsigned int count,
                             const hb_codepoint_t* unicodes,
                             unsigned int unicode_stride,
                             hb_codepoint_t* glyphs,
                             unsigned int glyph_stride,
                             void* user_data) {
    SkFont& font = *reinterpret_cast<SkFont*>(font_data);

    // Batch call textToGlyphs since entry cost is not cheap.
    // Copy requred because textToGlyphs is dense and hb is strided.
    SkAutoSTMalloc<256, SkUnichar> unicode(count);
    for (unsigned i = 0; i < count; i++) {
        unicode[i] = *unicodes;
        unicodes = SkTAddOffset<const hb_codepoint_t>(unicodes, unicode_stride);
    }
    SkAutoSTMalloc<256, SkGlyphID> glyph(count);
    font.textToGlyphs(
            unicode.get(), count * sizeof(SkUnichar), SkTextEncoding::kUTF32, glyph.get(), count);

    // Copy the results back to the sparse array.
    unsigned int done;
    for (done = 0; done < count && glyph[done] != 0; done++) {
        *glyphs = glyph[done];
        glyphs = SkTAddOffset<hb_codepoint_t>(glyphs, glyph_stride);
    }
    // return 'done' to allow HarfBuzz to synthesize with NFC and spaces, return 'count' to avoid
    return done;
}

#define SK_HB_VERSION_CHECK(x, y, z) \
    (HB_VERSION_MAJOR > (x)) || (HB_VERSION_MAJOR == (x) && HB_VERSION_MINOR > (y)) || \
    (HB_VERSION_MAJOR == (x) && HB_VERSION_MINOR == (y) && HB_VERSION_MICRO >= (z))

hb_font_funcs_t* skhb_get_font_funcs() {
    static hb_font_funcs_t* const funcs = [] {
        // HarfBuzz will use the default (parent) implementation if they aren't set.
        hb_font_funcs_t* const funcs = hb_font_funcs_create();
        hb_font_funcs_set_variation_glyph_func(funcs, skhb_glyph, nullptr, nullptr);
        hb_font_funcs_set_nominal_glyph_func(funcs, skhb_nominal_glyph, nullptr, nullptr);
#if SK_HB_VERSION_CHECK(2, 0, 0)
        hb_font_funcs_set_nominal_glyphs_func(funcs, skhb_nominal_glyphs, nullptr, nullptr);
#else
        sk_ignore_unused_variable(skhb_nominal_glyphs);
#endif
        hb_font_funcs_set_glyph_h_advance_func(funcs, nullptr, nullptr, nullptr);
#if SK_HB_VERSION_CHECK(1, 8, 6)
        hb_font_funcs_set_glyph_h_advances_func(funcs, nullptr, nullptr, nullptr);
#else
#endif
        hb_font_funcs_set_glyph_extents_func(funcs, nullptr, nullptr, nullptr);
        hb_font_funcs_make_immutable(funcs);
        return funcs;
    }();
    SkASSERT(funcs);
    return funcs;
}

hb_blob_t* skhb_get_table(hb_face_t* face, hb_tag_t tag, void* user_data) {

    if (tag != HB_TAG('c','m','a','p')) {
        return nullptr;
    }
    SkTypeface& typeface = *reinterpret_cast<SkTypeface*>(user_data);

    auto data = typeface.copyTableData(tag);
    if (!data) {
        return nullptr;
    }
    SkData* rawData = data.release();
    return hb_blob_create(reinterpret_cast<char*>(rawData->writable_data()),
                          rawData->size(),
                          HB_MEMORY_MODE_READONLY,
                          rawData,
                          [](void* ctx) { SkSafeUnref(((SkData*)ctx)); });
}

HBBlob stream_to_blob(std::unique_ptr<SkStreamAsset> asset) {
    size_t size = asset->getLength();
    HBBlob blob;
    if (const void* base = asset->getMemoryBase()) {
        blob.reset(hb_blob_create(
                (char*)base, SkToUInt(size), HB_MEMORY_MODE_READONLY, asset.release(), [](void* p) {
                    delete (SkStreamAsset*)p;
                }));
    } else {
        // SkDebugf("Extra SkStreamAsset copy\n");
        void* ptr = size ? sk_malloc_throw(size) : nullptr;
        asset->read(ptr, size);
        blob.reset(
                hb_blob_create((char*)ptr, SkToUInt(size), HB_MEMORY_MODE_READONLY, ptr, sk_free));
    }
    SkASSERT(blob);
    hb_blob_make_immutable(blob.get());
    return blob;
}

SkDEBUGCODE(static hb_user_data_key_t gDataIdKey;)

HBFace create_hb_face(const SkTypeface& typeface) {
    int index;
    std::unique_ptr<SkStreamAsset> typefaceAsset = typeface.openStream(&index);
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
                skhb_get_table, const_cast<SkTypeface*>(SkRef(&typeface)), [](void* user_data) {
                    SkSafeUnref(reinterpret_cast<SkTypeface*>(user_data));
                }));
    }
    SkASSERT(face);
    if (!face) {
        return nullptr;
    }
    hb_face_set_index(face.get(), (unsigned)index);
    hb_face_set_upem(face.get(), typeface.getUnitsPerEm());

    SkDEBUGCODE(
            hb_face_set_user_data(
                    face.get(), &gDataIdKey, const_cast<SkTypeface*>(&typeface), nullptr, false);)

            return face;
}

HBFont create_hb_font(const SkFont& font, const HBFace& face) {
    SkDEBUGCODE(void* dataId = hb_face_get_user_data(face.get(), &gDataIdKey);
                SkASSERT(dataId == font.getTypeface());)

            HBFont otFont(hb_font_create(face.get()));
    SkASSERT(otFont);
    if (!otFont) {
        return nullptr;
    }
    hb_ot_font_set_funcs(otFont.get());
    int axis_count = font.getTypeface()->getVariationDesignPosition(nullptr, 0);
    if (axis_count > 0) {
        SkAutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate> axis_values(axis_count);
        if (font.getTypeface()->getVariationDesignPosition(axis_values, axis_count) == axis_count) {
            hb_font_set_variations(
                    otFont.get(), reinterpret_cast<hb_variation_t*>(axis_values.get()), axis_count);
        }
    }

    // Creating a sub font means that non-available functions
    // are found from the parent.
    HBFont skFont(hb_font_create_sub_font(otFont.get()));
    hb_font_set_funcs(skFont.get(),
                      skhb_get_font_funcs(),
                      reinterpret_cast<void*>(new SkFont(font)),
                      [](void* user_data) { delete reinterpret_cast<SkFont*>(user_data); });
    int scale = skhb_position(font.getSize());
    hb_font_set_scale(skFont.get(), scale, scale);

    return skFont;
}

constexpr bool is_LTR(SkBidiIterator::Level level) { return (level & 1) == 0; }

class HBLockedFaceCache {
public:
    HBLockedFaceCache(SkLRUCache<SkFontID, HBFace>& lruCache, SkMutex& mutex)
        : fLRUCache(lruCache), fMutex(mutex)
    {
        fMutex.acquire();
    }
    HBLockedFaceCache(const HBLockedFaceCache&) = delete;
    HBLockedFaceCache& operator=(const HBLockedFaceCache&) = delete;
    HBLockedFaceCache(HBLockedFaceCache&&) = default;
    HBLockedFaceCache& operator=(HBLockedFaceCache&&) = delete;

    ~HBLockedFaceCache() {
        fMutex.release();
    }

    HBFace* find(SkFontID fontId) {
        return fLRUCache.find(fontId);
    }
    HBFace* insert(SkFontID fontId, HBFace hbFace) {
        return fLRUCache.insert(fontId, std::move(hbFace));
    }
    void reset() {
        fLRUCache.reset();
    }
private:
    SkLRUCache<SkFontID, HBFace>& fLRUCache;
    SkMutex& fMutex;
};
HBLockedFaceCache get_hbFace_cache() {
    static SkMutex gHBFaceCacheMutex;
    static SkLRUCache<SkFontID, HBFace> gHBFaceCache(100);
    return HBLockedFaceCache(gHBFaceCache, gHBFaceCacheMutex);
}

}

namespace skia {
namespace text {

class RunIteratorQueue {
public:
    void insert(SkShaper::RunIterator* runIterator, int priority) {
        fEntries.insert({runIterator, priority});
    }

    bool advanceRuns() {
        const SkShaper::RunIterator* leastRun = fEntries.peek().runIterator;
        if (leastRun->atEnd()) {
            SkASSERT(this->allRunsAreAtEnd());
            return false;
        }
        const size_t leastEnd = leastRun->endOfCurrentRun();
        SkShaper::RunIterator* currentRun = nullptr;
        SkDEBUGCODE(size_t previousEndOfCurrentRun);
        while ((currentRun = fEntries.peek().runIterator)->endOfCurrentRun() <= leastEnd) {
            int priority = fEntries.peek().priority;
            fEntries.pop();
            SkDEBUGCODE(previousEndOfCurrentRun = currentRun->endOfCurrentRun());
            currentRun->consume();
            SkASSERT(previousEndOfCurrentRun < currentRun->endOfCurrentRun());
            fEntries.insert({currentRun, priority});
        }
        return true;
    }

    size_t endOfCurrentRun() const { return fEntries.peek().runIterator->endOfCurrentRun(); }

private:
    bool allRunsAreAtEnd() const {
        for (int i = 0; i < fEntries.count(); ++i) {
            if (!fEntries.at(i).runIterator->atEnd()) {
                return false;
            }
        }
        return true;
    }

    struct Entry {
        SkShaper::RunIterator* runIterator;
        int priority;
    };
    static bool CompareEntry(Entry const& a, Entry const& b) {
        size_t aEnd = a.runIterator->endOfCurrentRun();
        size_t bEnd = b.runIterator->endOfCurrentRun();
        return aEnd < bEnd || (aEnd == bEnd && a.priority < b.priority);
    }
    SkTDPQueue<Entry, CompareEntry> fEntries;
};

class TrivialRunIterator : public SkShaper::RunIterator {
public:
    TrivialRunIterator(size_t utf16Bytes) : fEnd(utf16Bytes), fAtEnd(fEnd == 0) {}
    void consume() override { SkASSERT(!fAtEnd); fAtEnd = true; }
    size_t endOfCurrentRun() const override { return fAtEnd ? fEnd : 0; }
    bool atEnd() const override { return fAtEnd; }
private:
    size_t fEnd;
    bool fAtEnd;
};

class TrivialLanguageRunIterator : public TrivialRunIterator {
public:
    TrivialLanguageRunIterator(const char* language, size_t utf16Bytes)
        : TrivialRunIterator(utf16Bytes), fLanguage(language) {}
    const char* currentLanguage() const { return fLanguage.c_str(); }
private:
    SkString fLanguage;
};

class TrivialScriptRunIterator : public TrivialRunIterator {
public:
    TrivialScriptRunIterator(size_t utf16Bytes)
        : TrivialRunIterator(utf16Bytes) {}
    SkFourByteTag currentScript() const { return SkSetFourByteTag(HB_UNTAG(HB_SCRIPT_COMMON)); }
};

class TrivialFontRunIterator : public TrivialRunIterator {
public:
    TrivialFontRunIterator(const SkFont& font, size_t utf16Bytes)
        : TrivialRunIterator(utf16Bytes), fFont(font) {}
    const SkFont& currentFont() const { return fFont; }
private:
    SkFont fFont;
};

class SkUnicodeBidiRunIterator final : public SkShaper::BiDiRunIterator {
public:
    SkUnicodeBidiRunIterator(const uint16_t* utf16, const uint16_t* end16, SkUnicodeBidi bidi)
        : fBidi(std::move(bidi))
        , fEndOfCurrentRun(utf16)
        , fBegin(utf16)
        , fEnd(end16)
        , fUTF16LogicalPosition(0)
        , fLevel(SkBidiIterator::kLTR)
    {}

    void consume() override {
        SkASSERT(fUTF16LogicalPosition < fBidi->getLength());
        int32_t endPosition = fBidi->getLength();
        fLevel = fBidi->getLevelAt(fUTF16LogicalPosition);
        SkUnichar u = SkUTF::NextUTF16(&fEndOfCurrentRun, fEnd);
        fUTF16LogicalPosition += SkUTF::ToUTF16(u);
        SkBidiIterator::Level level;
        while (fUTF16LogicalPosition < endPosition) {
            level = fBidi->getLevelAt(fUTF16LogicalPosition);
            if (level != fLevel) {
                break;
            }
            SkUnichar u = SkUTF::NextUTF16(&fEndOfCurrentRun, fEnd);
            fUTF16LogicalPosition += SkUTF::ToUTF16(u);
        }
    }
    size_t endOfCurrentRun() const override {
        return fEndOfCurrentRun - fBegin;
    }
    bool atEnd() const override {
        return fUTF16LogicalPosition == fBidi->getLength();
    }
    SkBidiIterator::Level currentLevel() const override {
        return fLevel;
    }
private:
    SkUnicodeBidi fBidi;
    uint16_t const * fEndOfCurrentRun;
    uint16_t const * const fBegin;
    uint16_t const * const fEnd;
    int32_t fUTF16LogicalPosition;
    SkBidiIterator::Level fLevel;
};

template<typename Callback>
bool HarfbuzzFontResolvedText::harfbuzz(SkSpan<const char16_t> context,
                                        TextRange text,
                                        const SkBidiIterator::Level bidi,
                                        const char* language,
                                        const SkFourByteTag script,
                                        const SkFont& font,
                                        Callback callback) const {
    hb_buffer_t* buffer = hb_buffer_create();
    if (!buffer) {
        SkDEBUGF("Could not create hb_buffer");
        return false;
    }
    SkAutoTCallVProc<hb_buffer_t, hb_buffer_clear_contents> autoClearBuffer(buffer);
    hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
    hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

    // Documentation for HB_BUFFER_FLAG_BOT/EOT at 763e5466c0a03a7c27020e1e2598e488612529a7.
    // Currently BOT forces a dotted circle when first codepoint is a mark; EOT has no effect.
    // Avoid adding dotted circle, re-evaluate if BOT/EOT change. See https://skbug.com/9618.
    // hb_buffer_set_flags(buffer, HB_BUFFER_FLAG_BOT | HB_BUFFER_FLAG_EOT);

    // Add pre context
    hb_buffer_add_utf16(buffer, (uint16_t*)context.data(), text.fStart, text.fStart, 0);

    // Populate the hb_buffer directly with utf16 cluster indexes
    const uint16_t* utf16Current = (uint16_t*)context.begin() + text.fStart;
    const uint16_t* utf16End = (uint16_t*)context.begin() + text.fEnd;
    while (utf16Current < utf16End) {
        unsigned int cluster = utf16Current - (uint16_t*)context.data();
        hb_codepoint_t u = SkUTF::NextUTF16(&utf16Current, utf16End);
        hb_buffer_add(buffer, u, cluster);
    }

    // Add post context
    hb_buffer_add_utf16(buffer, utf16Current, (uint16_t*)context.end() - utf16Current, 0, 0);

    hb_direction_t direction = is_LTR(bidi) ? HB_DIRECTION_LTR:HB_DIRECTION_RTL;
    hb_buffer_set_direction(buffer, direction);
    hb_buffer_set_script(buffer, hb_script_from_iso15924_tag((hb_tag_t)script));
    // Buffers with HB_LANGUAGE_INVALID race since hb_language_get_default is not thread safe.
    // The user must provide a language, but may provide data hb_language_from_string cannot use.
    // Use "und" for the undefined language in this case (RFC5646 4.1 5).
    hb_language_t hbLanguage = hb_language_from_string(language, -1);
    if (hbLanguage == HB_LANGUAGE_INVALID) {
        hbLanguage = hb_language_from_string("und", -1);
    }
    hb_buffer_set_language(buffer, hbLanguage);
    hb_buffer_guess_segment_properties(buffer);

    // TODO: better cache HBFace (data) / hbfont (typeface)
    // An HBFace is expensive (it sanitizes the bits).
    // An HBFont is fairly inexpensive.
    // An HBFace is actually tied to the data, not the typeface.
    // The size of 100 here is completely arbitrary and used to match libtxt.
    HBFont hbFont;
    {
        HBLockedFaceCache cache = get_hbFace_cache();
        SkFontID dataId = font.getTypeface()->uniqueID();
        HBFace* hbFaceCached = cache.find(dataId);
        if (!hbFaceCached) {
            HBFace hbFace(create_hb_face(*font.getTypeface()));
            hbFaceCached = cache.insert(dataId, std::move(hbFace));
        }
        hbFont = create_hb_font(font, *hbFaceCached);
    }
    if (!hbFont) {
        return false;
    }

    hb_shape(hbFont.get(), buffer, nullptr, 0);
    unsigned len = hb_buffer_get_length(buffer);
    if (len == 0) {
        return false;
    }

    if (direction == HB_DIRECTION_RTL) {
        // Put the clusters back in logical order.
        // Note that the advances remain ltr.
        hb_buffer_reverse(buffer);
    }

    // The cluster order is always from the left to the right (regardless of the text direction)
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer, nullptr);
    for (GlyphIndex i = 0; i < len; i++) {
        auto glyph = info[i].codepoint;
        auto cluster = info[i].cluster;
        callback(i, glyph, cluster);
    }

    return true;
}

bool HarfbuzzFontResolvedText::resolveChain(UnicodeText* unicodeText, SkSpan<const char16_t> text, TextDirection textDirection, const char* locale, const FontChain& fontChain) {

    // Try to shape it with all fonts from the fontChain until it's all resolved
    std::deque<TextRange> unresolvedTexts;
    unresolvedTexts.emplace_back(0, text.size());
    auto startForFontChain = fResolvedFonts.size(); // results for font chain start from here
    for (size_t fontIndex = 0; fontIndex < fontChain.count(); ++fontIndex) {

        std::deque<TextRange> newUnresolvedTexts;
        // Check all text range that have not been resolved yet
        while (!unresolvedTexts.empty()) {
            // Take the first unresolved text block
            auto unresolvedText = unresolvedTexts.front();
            unresolvedTexts.pop_front();
            auto start = newUnresolvedTexts.size();

            SkSpan<const char16_t> unresolved(text.data() + unresolvedText.fStart, unresolvedText.width());
            SkUnicodeBidiRunIterator bidiIter((const uint16_t*)unresolved.begin(), (const uint16_t*)text.end(),
                                              unicodeText->getUnicode()->makeBidiIterator((const uint16_t*)unresolved.data(), SkToS16(unresolved.size()),
                                                                                          textDirection == TextDirection::kLtr ? SkBidiIterator::kLTR : SkBidiIterator::kRTL));
            TrivialLanguageRunIterator languageIter(locale, unresolvedText.fEnd);
            TrivialScriptRunIterator scriptIter(unresolved.size());
            TrivialFontRunIterator fontIter(fontChain.createFont(fontIndex), unresolvedText.fEnd);

            RunIteratorQueue runQueue;
            runQueue.insert(&fontIter,     3); // The font iterator is always run last in case of tie.
            runQueue.insert(&bidiIter,     2);
            runQueue.insert(&scriptIter,   1);
            runQueue.insert(&languageIter, 0);

            TextRange run(unresolvedText.fStart, unresolvedText.fStart);
            while (runQueue.advanceRuns()) {
                // Shape each run
                run.fStart = run.fEnd;
                run.fEnd = unresolvedText.fStart + runQueue.endOfCurrentRun();
                TextRange newUnresolved = EMPTY_RANGE;
                if (!harfbuzz(text, run,
                              bidiIter.currentLevel(),
                              languageIter.currentLanguage(),
                              scriptIter.currentScript(),
                              fontIter.currentFont(),
                              [&](GlyphIndex gryphIndex, hb_codepoint_t glyph, uint32_t cluster) {
                                    // Inspect it for resolved/unresolved
                                    if (newUnresolved.fEnd != EMPTY_INDEX && cluster < newUnresolved.fEnd) {
                                        // We should skip all clusters that fall into the current grapheme
                                        return;
                                    }
                                    if (glyph != 0 && newUnresolved.fStart != EMPTY_INDEX) {
                                        newUnresolved.fEnd = unicodeText->adjustToGraphemeEdge(cluster, false /* to the right */);
                                        SkASSERT(newUnresolved.fEnd <= run.fEnd);
                                        if (!newUnresolved.isEmpty()) {
                                            if (newUnresolvedTexts.empty() || newUnresolvedTexts.back().fEnd < newUnresolved.fStart) {
                                                // It's a new unresolved block
                                                newUnresolvedTexts.emplace_back(newUnresolved);
                                            } else {
                                                // Let's extend the last unresolved block
                                                newUnresolvedTexts.back().fEnd = newUnresolved.fEnd;
                                            }
                                        }
                                        newUnresolved.fStart = EMPTY_INDEX;
                                    } else if (glyph == 0) {
                                        if (newUnresolved.fStart == EMPTY_INDEX) {
                                            newUnresolved.fStart = unicodeText->adjustToGraphemeEdge(cluster, true /* to the left */);
                                            newUnresolved.fEnd = EMPTY_INDEX;
                                            SkASSERT(newUnresolved.fStart >= run.fStart);
                                        }
                                    }
                            })) {
                    return false;
                }
                if (newUnresolved.fStart != EMPTY_INDEX) {
                    newUnresolved.fEnd = unicodeText->adjustToGraphemeEdge(run.fEnd, false /* to the right */);
                    SkASSERT(newUnresolved.fEnd <= run.fEnd);
                    newUnresolvedTexts.emplace_back(newUnresolved);
                }
            }
            // Let's fill the resolved blocks with the current font
            addResolved(unresolvedText, newUnresolvedTexts, start, fontChain, fontIndex);
        }

        // Try the next font in chain
        SkASSERT(unresolvedTexts.empty());
        unresolvedTexts = std::move(newUnresolvedTexts);
    }

    if (startForFontChain == fResolvedFonts.size()) {
        fResolvedFonts.emplace_back(TextRange(0, text.size()), fontChain[0], fontChain.fontSize(), SkFontStyle::Normal());
        return true;
    }
    return unresolvedTexts.empty();
}

}
}
