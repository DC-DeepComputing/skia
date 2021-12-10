/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This hack is here to by pass the headers presubmit. It gets angry if two header files have the
// same name. This is temporary until chrome is cleaned up.
// TODO: remove this entire file after the includes in chromium have been updated
#if defined(GR_GL_CUSTOM_SETUP_HEADER)
    #include "include/private/chromium/SkRemoteGlyphCache.h"
#endif
