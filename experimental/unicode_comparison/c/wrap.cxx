// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wrap.hxx"
#include "modules/skunicode/include/SkUnicode.h"

int sum_from_cplus(int a, int b) {
  SumClass num; //create object of class SumClass
  return num.Sum(a, b); // Call Sum function using wrapper function sum_from_cplus
}