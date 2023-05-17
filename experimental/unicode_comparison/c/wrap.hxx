// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRAP_H
#define WRAP_H

// __cplusplus tells the compiler that inside code is compiled with the c++ compiler
#ifdef __cplusplus
// extern "C" tells C++ compiler exports the symbols without a name manging.
extern "C" {
  #endif
  int sum_from_cplus(int a,int b); //declare wrapper function
  #ifdef __cplusplus
}
#endif
#endif