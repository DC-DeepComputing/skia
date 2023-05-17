// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BRIDGE_H
#define BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

  void* make_unicode();
  void  destroy_unicode(void* unicode);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // BRIDGE_H
