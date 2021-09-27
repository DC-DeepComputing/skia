#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/pdf/SkPDFSubsetFont.h"
#include "src/pdf/SkPDFGlyphUse.h"


int main(int argc, char** argv) {
  sk_sp<SkData> fontData(SkData::MakeFromFileName("kaiu.ttf"));
  SkASSERT(fontData->size());
  SkPDFGlyphUse glyphUse(6267, 6268);
  glyphUse.set(0);
  glyphUse.set(6268);
  SkASSERT(glyphUse.has(0));
  SkASSERT(glyphUse.has(6268));
  SkASSERT(!glyphUse.has(6267));

  sk_sp<SkData> subset_data = SkPDFSubsetFont(
          fontData, glyphUse, SkPDF::Metadata::Subsetter::kHarfbuzz_Subsetter, "", 0);

  SkFILEWStream file_stream("subset_kaiu_skia.ttf");
  file_stream.write(subset_data->data(), subset_data->size());
}
