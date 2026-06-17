/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkLogHandler.h"

#include "include/private/SkMacros.h"

#include <atomic>
#include <stdlib.h>

class SkDefaultLogHandler : public SkLogHandler {
public:
    void onLog(SkLogPriority, const char*) override {}
};

static std::atomic<SkLogHandler*> gUserLogHandler{nullptr};

bool SkLogHandler::SetInstance(SkLogHandler* handler) {
    SkLogHandler* expected = nullptr;
    if (!gUserLogHandler.compare_exchange_strong(expected, handler)) {
        delete handler;
        return false;
    }
    SK_INTENTIONALLY_LEAKED(handler);
    return true;
}

SkLogHandler* SkLogHandler::GetInstance() {
    if (auto handler = gUserLogHandler.load(std::memory_order_acquire)) {
        return handler;
    }
    static SkDefaultLogHandler defaultHandler;
    return &defaultHandler;
}
