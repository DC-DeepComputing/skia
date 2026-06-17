/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLogHandler_DEFINED
#define SkLogHandler_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkLogPriority.h"

/**
 * SkLogHandler defines an interface for receiving Skia log messages.
 * Clients can implement this interface and install it via SetInstance.
 */
class SK_API SkLogHandler {
public:
    virtual ~SkLogHandler() = default;

    /**
     * Called when a log message is generated.
     * @param priority The priority of the log message.
     * @param msg The formatted log message.
     */
    virtual void onLog(SkLogPriority priority, const char* msg) = 0;

    /**
     * Installs a custom log handler. Returns true if successful (i.e., it's the first call).
     * Ownership is transferred; the handler will be leaked to ensure it stays valid until exit.
     */
    static bool SetInstance(SkLogHandler*);

    /**
     * Returns the current log handler. If none has been set, a default (no-op) handler is returned.
     */
    static SkLogHandler* GetInstance();

protected:
    SkLogHandler() = default;

private:
    SkLogHandler(const SkLogHandler&) = delete;
    SkLogHandler& operator=(const SkLogHandler&) = delete;
};

#endif // SkLogHandler_DEFINED
