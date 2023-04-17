/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImage_CrossContext_DEFINED
#define SkImage_CrossContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Lazy.h"

#include <memory>

class GrDirectContext;
class SkImage;
class SkImageGenerator;
struct SkIRect;

class SkImage_CrossContext final : public SkImage_Lazy {
public:
    SkImage_CrossContext(SkImage_Lazy::Validator* validator): SkImage_Lazy(validator) {}

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kCrossContextGanesh; }

    sk_sp<SkImage> onMakeSubset(GrDirectContext*, const SkIRect&) const override;
};

sk_sp<SkImage> MakeCrossContextImage(std::unique_ptr<SkImageGenerator> generator);
#endif
