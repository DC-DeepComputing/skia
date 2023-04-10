// Copyright 2019 Google LLC.
#include <memory>

#include "modules/skparagraph/include/FontArguments.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

using namespace skia_private;

namespace skia {
namespace textlayout {

namespace {
    SkScalar relax(SkScalar a) {
        // This rounding is done to match Flutter tests. Must be removed..
        if (SkScalarIsFinite(a)) {
          auto threshold = SkIntToScalar(1 << 12);
          return SkScalarRoundToScalar(a * threshold)/threshold;
        } else {
          return a;
        }
    }

    bool exactlyEqual(SkScalar x, SkScalar y) {
        return x == y || (x != x && y != y);
    }

    template <typename K>
    uint32_t mixHash(uint32_t hash, const K& k) {
        hash +=  SkGoodHash<K>()(k);
        hash += (hash << 10);
        hash ^= (hash >> 6);
        return hash;
    }

}  // namespace

class ParagraphCacheKey {
public:
    ParagraphCacheKey(const ParagraphImpl* paragraph)
        : fText(paragraph->fText.c_str(), paragraph->fText.size())
        , fPlaceholders(paragraph->fPlaceholders)
        , fTextStyles(paragraph->fTextStyles)
        , fParagraphStyle(paragraph->paragraphStyle()) {
        fHash = computeHash();
    }

    ParagraphCacheKey(const ParagraphCacheKey& other) = default;

    ParagraphCacheKey(ParagraphCacheKey&& other)
        : fText(std::move(other.fText))
        , fPlaceholders(std::move(other.fPlaceholders))
        , fTextStyles(std::move(other.fTextStyles))
        , fParagraphStyle(std::move(other.fParagraphStyle))
        , fHash(other.fHash) {
        other.fHash = 0;
    }

    bool operator==(const ParagraphCacheKey& other) const;

    uint32_t hash() const { return fHash; }

    const SkString& text() const { return fText; }

private:
    uint32_t computeHash() const;

    SkString fText;
    TArray<Placeholder, true> fPlaceholders;
    TArray<Block, true> fTextStyles;
    ParagraphStyle fParagraphStyle;
    uint32_t fHash;
};

class ParagraphCacheValue {
public:
    ParagraphCacheValue(ParagraphCacheKey&& key, const ParagraphImpl* paragraph)
        : fKey(std::move(key))
        , fRuns(paragraph->fRuns)
        , fClusters(paragraph->fClusters)
        , fClustersIndexFromCodeUnit(paragraph->fClustersIndexFromCodeUnit)
        , fCodeUnitProperties(paragraph->fCodeUnitProperties)
        , fWords(paragraph->fWords)
        , fBidiRegions(paragraph->fBidiRegions)
        , fHasLineBreaks(paragraph->fHasLineBreaks)
        , fHasWhitespacesInside(paragraph->fHasWhitespacesInside)
        , fTrailingSpaces(paragraph->fTrailingSpaces) { }

    // Input == key
    ParagraphCacheKey fKey;

    // Shaped results
    TArray<Run, false> fRuns;
    TArray<Cluster, true> fClusters;
    TArray<size_t, true> fClustersIndexFromCodeUnit;
    // ICU results
    TArray<SkUnicode::CodeUnitFlags, true> fCodeUnitProperties;
    std::vector<size_t> fWords;
    std::vector<SkUnicode::BidiRegion> fBidiRegions;
    bool fHasLineBreaks;
    bool fHasWhitespacesInside;
    TextIndex fTrailingSpaces;
};

uint32_t ParagraphCacheKey::computeHash() const {
    uint32_t hash = 0;
    for (auto& ph : fPlaceholders) {
        if (ph.fRange.width() == 0) {
            continue;
        }
        hash = mixHash(hash, ph.fRange);
        hash = mixHash(hash, relax(ph.fStyle.fHeight));
        hash = mixHash(hash, relax(ph.fStyle.fWidth));
        hash = mixHash(hash, ph.fStyle.fAlignment);
        hash = mixHash(hash, ph.fStyle.fBaseline);
        if (ph.fStyle.fAlignment == PlaceholderAlignment::kBaseline) {
            hash = mixHash(hash, relax(ph.fStyle.fBaselineOffset));
        }
    }

    for (auto& ts : fTextStyles) {
        if (ts.fStyle.isPlaceholder()) {
            continue;
        }
        hash = mixHash(hash, relax(ts.fStyle.getLetterSpacing()));
        hash = mixHash(hash, relax(ts.fStyle.getWordSpacing()));
        hash = mixHash(hash, ts.fStyle.getLocale());
        hash = mixHash(hash, relax(ts.fStyle.getHeight()));
        hash = mixHash(hash, relax(ts.fStyle.getBaselineShift()));
        for (auto& ff : ts.fStyle.getFontFamilies()) {
            hash = mixHash(hash, ff);
        }
        for (auto& ff : ts.fStyle.getFontFeatures()) {
            hash = mixHash(hash, ff.fValue);
            hash = mixHash(hash, ff.fName);
        }
        hash = mixHash(hash, ts.fStyle.getFontArguments());
        hash = mixHash(hash, ts.fStyle.getFontStyle());
        hash = mixHash(hash, relax(ts.fStyle.getFontSize()));
        hash = mixHash(hash, ts.fRange);
    }

    hash = mixHash(hash, relax(fParagraphStyle.getHeight()));
    hash = mixHash(hash, fParagraphStyle.getTextDirection());
    hash = mixHash(hash, fParagraphStyle.getReplaceTabCharacters() ? 1 : 0);

    auto& strutStyle = fParagraphStyle.getStrutStyle();
    if (strutStyle.getStrutEnabled()) {
        hash = mixHash(hash, relax(strutStyle.getHeight()));
        hash = mixHash(hash, relax(strutStyle.getLeading()));
        hash = mixHash(hash, relax(strutStyle.getFontSize()));
        hash = mixHash(hash, strutStyle.getHeightOverride());
        hash = mixHash(hash, strutStyle.getFontStyle());
        hash = mixHash(hash, strutStyle.getForceStrutHeight());
        for (auto& ff : strutStyle.getFontFamilies()) {
            hash = mixHash(hash, ff);
        }
    }

    hash = mixHash(hash, fText);
    return hash;
}

bool ParagraphCacheKey::operator==(const ParagraphCacheKey& other) const {
    if (fText.size() != other.fText.size()) {
        return false;
    }
    if (fPlaceholders.size() != other.fPlaceholders.size()) {
        return false;
    }
    if (fText != other.fText) {
        return false;
    }
    if (fTextStyles.size() != other.fTextStyles.size()) {
        return false;
    }

    // There is no need to compare default paragraph styles - they are included into fTextStyles
    if (!exactlyEqual(fParagraphStyle.getHeight(), other.fParagraphStyle.getHeight())) {
        return false;
    }
    if (fParagraphStyle.getTextDirection() != other.fParagraphStyle.getTextDirection()) {
        return false;
    }

    if (!(fParagraphStyle.getStrutStyle() == other.fParagraphStyle.getStrutStyle())) {
        return false;
    }

    if (!(fParagraphStyle.getReplaceTabCharacters() == other.fParagraphStyle.getReplaceTabCharacters())) {
        return false;
    }

    for (int i = 0; i < fTextStyles.size(); ++i) {
        auto& tsa = fTextStyles[i];
        auto& tsb = other.fTextStyles[i];
        if (tsa.fStyle.isPlaceholder()) {
            continue;
        }
        if (!(tsa.fStyle.equalsByFonts(tsb.fStyle))) {
            return false;
        }
        if (tsa.fRange.width() != tsb.fRange.width()) {
            return false;
        }
        if (tsa.fRange.start != tsb.fRange.start) {
            return false;
        }
    }
    for (int i = 0; i < fPlaceholders.size(); ++i) {
        auto& tsa = fPlaceholders[i];
        auto& tsb = other.fPlaceholders[i];
        if (tsa.fRange.width() == 0 && tsb.fRange.width() == 0) {
            continue;
        }
        if (!(tsa.fStyle.equals(tsb.fStyle))) {
            return false;
        }
        if (tsa.fRange.width() != tsb.fRange.width()) {
            return false;
        }
        if (tsa.fRange.start != tsb.fRange.start) {
            return false;
        }
    }

    return true;
}

struct ParagraphCache::Entry {

    Entry(ParagraphCacheValue* value) : fValue(value) {}
    std::unique_ptr<ParagraphCacheValue> fValue;
};

ParagraphCache::ParagraphCache()
    : fChecker([](ParagraphImpl* impl, const char*, bool){ })
    , fLRUCacheMap(kMaxEntries)
    , fCacheIsOn(true)
    , fLastCachedValue(nullptr)
#ifdef PARAGRAPH_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
    , fHashMisses(0)
#endif
{ }

ParagraphCache::~ParagraphCache() { }

void ParagraphCache::updateTo(ParagraphImpl* paragraph, const Entry* entry) {

    paragraph->fRuns.clear();
    paragraph->fRuns = entry->fValue->fRuns;
    paragraph->fClusters = entry->fValue->fClusters;
    paragraph->fClustersIndexFromCodeUnit = entry->fValue->fClustersIndexFromCodeUnit;
    paragraph->fCodeUnitProperties = entry->fValue->fCodeUnitProperties;
    paragraph->fWords = entry->fValue->fWords;
    paragraph->fBidiRegions = entry->fValue->fBidiRegions;
    paragraph->fHasLineBreaks = entry->fValue->fHasLineBreaks;
    paragraph->fHasWhitespacesInside = entry->fValue->fHasWhitespacesInside;
    paragraph->fTrailingSpaces = entry->fValue->fTrailingSpaces;
    for (auto& run : paragraph->fRuns) {
        run.setOwner(paragraph);
    }
    for (auto& cluster : paragraph->fClusters) {
        cluster.setOwner(paragraph);
    }
}

void ParagraphCache::printStatistics() {
    SkDebugf("--- Paragraph Cache ---\n");
    SkDebugf("Total requests: %d\n", fTotalRequests);
    SkDebugf("Cache misses: %d\n", fCacheMisses);
    SkDebugf("Cache miss %%: %f\n", (fTotalRequests > 0) ? 100.f * fCacheMisses / fTotalRequests : 0.f);
    int cacheHits = fTotalRequests - fCacheMisses;
    SkDebugf("Hash miss %%: %f\n", (cacheHits > 0) ? 100.f * fHashMisses / cacheHits : 0.f);
    SkDebugf("---------------------\n");
}

void ParagraphCache::abandon() {
    this->reset();
}

void ParagraphCache::reset() {
    SkAutoMutexExclusive lock(fParagraphMutex);
#ifdef PARAGRAPH_CACHE_STATS
    fTotalRequests = 0;
    fCacheMisses = 0;
    fHashMisses = 0;
#endif
    fLRUCacheMap.reset();
    fLastCachedValue = nullptr;
}

bool ParagraphCache::findParagraph(ParagraphImpl* paragraph) {
    if (!fCacheIsOn) {
        return false;
    }
#ifdef PARAGRAPH_CACHE_STATS
    ++fTotalRequests;
#endif
    SkAutoMutexExclusive lock(fParagraphMutex);
    ParagraphCacheKey key(paragraph);
    std::unique_ptr<Entry>* entry = fLRUCacheMap.find(key);

    if (!entry) {
        // We have a cache miss
#ifdef PARAGRAPH_CACHE_STATS
        ++fCacheMisses;
#endif
        fChecker(paragraph, "missingParagraph", true);
        return false;
    }
    updateTo(paragraph, entry->get());
    fChecker(paragraph, "foundParagraph", true);
    return true;
}

bool ParagraphCache::updateParagraph(ParagraphImpl* paragraph) {
    if (!fCacheIsOn) {
        return false;
    }
#ifdef PARAGRAPH_CACHE_STATS
    ++fTotalRequests;
#endif
    SkAutoMutexExclusive lock(fParagraphMutex);

    ParagraphCacheKey key(paragraph);
    std::unique_ptr<Entry>* entry = fLRUCacheMap.find(key);
    if (!entry) {
        // isTooMuchMemoryWasted(paragraph) not needed for now
        if (isPossiblyTextEditing(paragraph)) {
            // Skip this paragraph
            return false;
        }
        ParagraphCacheValue* value = new ParagraphCacheValue(std::move(key), paragraph);
        fLRUCacheMap.insert(value->fKey, std::make_unique<Entry>(value));
        fChecker(paragraph, "addedParagraph", true);
        fLastCachedValue = value;
        return true;
    } else {
        // We do not have to update the paragraph
        return false;
    }
}

// Special situation: (very) long paragraph that is close to the last formatted paragraph
#define NOCACHE_PREFIX_LENGTH 40
bool ParagraphCache::isPossiblyTextEditing(ParagraphImpl* paragraph) {
    if (fLastCachedValue == nullptr) {
        return false;
    }

    auto& lastText = fLastCachedValue->fKey.text();
    auto& text = paragraph->fText;

    if ((lastText.size() < NOCACHE_PREFIX_LENGTH) || (text.size() < NOCACHE_PREFIX_LENGTH)) {
        // Either last text or the current are too short
        return false;
    }

    if (std::strncmp(lastText.c_str(), text.c_str(), NOCACHE_PREFIX_LENGTH) == 0) {
        // Texts have the same starts
        return true;
    }

    if (std::strncmp(lastText.c_str() + lastText.size() - NOCACHE_PREFIX_LENGTH, &text[text.size() - NOCACHE_PREFIX_LENGTH], NOCACHE_PREFIX_LENGTH) == 0) {
        // Texts have the same ends
        return true;
    }

    // It does not look like editing the text
    return false;
}
}  // namespace textlayout
}  // namespace skia

uint32_t SkGoodHash<skia::textlayout::ParagraphCacheKey>::operator()(
        const skia::textlayout::ParagraphCacheKey& k) const {
    return k.hash();
}
