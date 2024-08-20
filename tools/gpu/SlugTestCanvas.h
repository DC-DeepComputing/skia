/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// SlugTestCanvas is a simple way to make a testing canvas which is allowed to use private
// facilities of SkCanvas without having to add a friend to SkCanvas.h.
//
// You create a Key (a simple empty struct) to make a template specialization class. You need to
// make a key for each of the different Canvases you need. The implementations of the canvases
// are in SkCanvas.cpp, which allows the use of helper classes.

#ifndef SlugTestCanvas_DEFINED
#define SlugTestCanvas_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"

#include <memory>

class SkPaint;

namespace sktext { class GlyphRunList; }

// You can only make template specializations of SlugTestCanvas.
template <typename Key> class SlugTestCanvas;

// A test canvas to test using slug rendering instead of text blob rendering.
struct SkSlugTestKey {};
template <>
class SlugTestCanvas<SkSlugTestKey> : public SkCanvas {
public:
    SlugTestCanvas(SkCanvas* canvas);
    void onDrawGlyphRunList(
            const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) override;
};

struct SkSerializeSlugTestKey {};
template <>
class SlugTestCanvas<SkSerializeSlugTestKey> : public SkCanvas {
public:
    SlugTestCanvas(SkCanvas* canvas);
    void onDrawGlyphRunList(
            const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) override;
};

struct SkRemoteSlugTestKey {};
template <>
class SlugTestCanvas<SkRemoteSlugTestKey> : public SkCanvas {
public:
    SlugTestCanvas(SkCanvas* canvas);
    ~SlugTestCanvas() override;
    void onDrawGlyphRunList(
            const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) override;

private:
    std::unique_ptr<SkStrikeServer::DiscardableHandleManager> fServerHandleManager;
    sk_sp<SkStrikeClient::DiscardableHandleManager> fClientHandleManager;
    SkStrikeServer fStrikeServer;
    SkStrikeClient fStrikeClient;
};

#endif  // SlugTestCanvas_DEFINED
