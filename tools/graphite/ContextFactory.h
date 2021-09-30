/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_graphite_test_ContextFactory_DEFINED
#define sk_graphite_test_ContextFactory_DEFINED

#include <vector>
#include "experimental/graphite/include/GraphiteTypes.h"
#include "include/core/SkRefCnt.h"

// don't want to include
#include "tools/graphite/GraphiteTestContext.h"

namespace skgpu {
    class Context;
};

namespace sk_graphite_test {

//class GraphiteTestContext;

class ContextFactory { //: SkNoncopyable {
public:
    enum class ContextType {
        kMetal,
        kMock,
    };

    class ContextInfo {
    public:
        ContextInfo() = default;
        ~ContextInfo();

        ContextFactory::ContextType type() const { return fType; }

        skgpu::Context* context() const { return fContext.get(); }
        sk_sp<skgpu::Context> refContext() const;
//        GraphiteTestContext* testContext() const { return fTestContext.get(); }

    private:
        ContextInfo(ContextFactory::ContextType,
                    std::unique_ptr<GraphiteTestContext>,
                    sk_sp<skgpu::Context>);

        ContextType                          fType = ContextType::kMock;
        std::unique_ptr<GraphiteTestContext> fTestContext;
        sk_sp<skgpu::Context>                fContext;
    };

    ContextFactory();
    ~ContextFactory();

    ContextInfo getContextInfo(ContextType);

private:
    std::vector<ContextInfo> fContexts;
};

} // namespace sk_graphite_test

#endif // sk_graphite_test_ContextFactory_DEFINED
