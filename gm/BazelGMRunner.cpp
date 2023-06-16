/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include <iostream>

int main(int argc, char** argv) {
  std::cout << "Printing registered GMs..." << std::endl;
  for (skiagm::GMFactory f : skiagm::GMRegistry::Range()) {
    std::cout << "GM: " << f().get()->getName() << std::endl;
  }
  std::cout << "Done printing registered GMs." << std::endl;
  return 0;
}
