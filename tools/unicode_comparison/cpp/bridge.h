// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BRIDGE_H
#define BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

  bool   make_unicode();
  void   destroy_unicode();
  void*  upper(char* str);
  void   print(void* str);
  double initialize(char* text);
  bool   getFlags(int index, void* flags);
  void*  getSentences(char* text, int* length);
  bool   trimSentence(char* text, int* length, int limit);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // BRIDGE_H
