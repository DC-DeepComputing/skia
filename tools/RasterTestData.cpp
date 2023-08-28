/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/RasterTestData.h"

#include "include/core/SkData.h"

const char kRasterTestJsonPath[] = "third_party/externals/raster_test_data/raster_test.json";
const char kFileBasePath[] = "third_party/externals/raster_test_data/fonts/";

RasterTestDataIterator::RasterTestDataIterator(const std::string fontFilterRegexp,
                                               const std::string langFilterRegexp)
        : fFontFilter(fontFilterRegexp), fLangFilter(langFilterRegexp) {
    sk_sp<SkData> jsonTestData = SkData::MakeFromFileName(kRasterTestJsonPath);
    fJsonDom = std::make_unique<skjson::DOM>(reinterpret_cast<const char*>(jsonTestData->bytes()),
                                             jsonTestData->size());
    const skjson::ObjectValue& root = fJsonDom->root().as<skjson::ObjectValue>();
    fFonts = root["fonts"];
    fSamples = root["samples"];
    SkASSERT(fFonts && fFonts->size() == 1558);
    SkASSERT(fSamples);
}

bool RasterTestDataIterator::next(TestSet* testSet) {
    while (testSet && fFontsIndex < fFonts->size()) {
        const skjson::ObjectValue* fontsEntry = (*fFonts)[fFontsIndex++];
        SkASSERT(fontsEntry);
        const skjson::StringValue* fontName = (*fontsEntry)["name"];
        SkASSERT(fontName);
        std::smatch match;
        std::string fontNameStr(fontName->str());
        if (std::regex_match(fontNameStr, match, fFontFilter)) {
            testSet->fontName = SkString(fontNameStr);
            const skjson::StringValue* fontFilename = (*fontsEntry)["path"];
            SkString stringFontFilename(kFileBasePath);
            stringFontFilename.append(fontFilename->str().data(), fontFilename->str().size());
            testSet->fontFilename = stringFontFilename;
            testSet->langSamples =
                    getLanguageSamples((*fontsEntry)["languages"].as<skjson::ArrayValue>());
            return true;
        }
    }
    return false;
}

std::vector<RasterTestDataIterator::LangSample> RasterTestDataIterator::getLanguageSamples(
        const skjson::ArrayValue* languages) {
    std::vector<LangSample> samples;
    for (size_t i = 0; i < languages->size(); ++i) {
        const skjson::StringValue* langTag = (*languages)[i];
        std::string langTagStr(langTag->str());
        std::smatch match;
        if (std::regex_match(langTagStr, match, fLangFilter)) {
            const skjson::ObjectValue* sample = (*fSamples)[langTagStr.c_str()];
            const skjson::StringValue* shortSample = (*sample)["short_sample"];
            const skjson::StringValue* longSample = (*sample)["long_sample"];
            SkString sampleShort(shortSample->str().data(), shortSample->str().size());
            SkString sampleLong(longSample->str().data(), longSample->str().size());
            samples.push_back({SkString(langTagStr), sampleShort, sampleLong});
        }
    }
    SkASSERT_RELEASE(samples.size());
    return samples;
}
