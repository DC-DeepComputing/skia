/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef EventTracingPriv_DEFINED
#define EventTracingPriv_DEFINED

/**
 * Construct and install tracing if enabled.
 */
void initializeEventTracingForTools(const char* mode = nullptr);

/**
 * Terminate tracing and flush events.
 */
void terminateEventTracingForTools();

/**
 * Start a new tracing section (closes current session and starts new one).
 */
void traceEventNewTracingSection(const char* name);

#endif
