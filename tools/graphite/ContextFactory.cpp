/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/ContextFactory.h"

#include "experimental/graphite/include/Context.h"

namespace sk_graphite_test {

///////////////////////////////////////////////////////////////////////////////////////////////////
ContextFactory::ContextInfo::ContextInfo(ContextFactory::ContextType type,
                                         std::unique_ptr<GraphiteTestContext> testContext,
                                         sk_sp<skgpu::Context> context)
        : fType(type)
        , fTestContext(std::move(testContext))
        , fContext(std::move(context)) {
}

ContextFactory::ContextInfo::~ContextInfo() = default;

sk_sp<skgpu::Context> ContextFactory::ContextInfo::refContext() const { return fContext; }

///////////////////////////////////////////////////////////////////////////////////////////////////
ContextFactory::ContextFactory() {}
ContextFactory::~ContextFactory() {}

ContextFactory::ContextInfo ContextFactory::getContextInfo(ContextType type) {
    return {};
}

} // namespace sk_graphite_test
