/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_fontconfig_priv_DEFINED
#define SkFontMgr_fontconfig_priv_DEFINED

#include <fontconfig/fontconfig.h>

/** Removes weak elements from the specified object string in the property
 *  (e.g. object = FC_FAMILY) after the first strong element, while preserving
 *  leading preferred weak bindings. If there are no strong elements, all weak
 *  elements are preserved.
 */
void SkFontMgr_FontConfig_remove_weak(FcPattern* pattern, const char object[]);

#endif // SkFontMgr_fontconfig_priv_DEFINED
