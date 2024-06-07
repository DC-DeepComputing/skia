/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MODULEDATAFILEPROVIDER
#define MODULEDATAFILEPROVIDER

#include "include/core/SkString.h"
#include "src/sksl/SkSLModuleDataProvider.h"
#include "src/utils/SkOSPath.h"
#include "tools/SkGetExecutablePath.h"

#include <fstream>

class ModuleDataFileProvider : public SkSL::ModuleDataProvider {
public:
    ~ModuleDataFileProvider() override = default;

    std::string get(SkSL::ModuleName /*name*/, const char* filename) const override {
        std::string exePath = SkGetExecutablePath();
        SkString exeDir = SkOSPath::Dirname(exePath.c_str());
        SkString modulePath = SkOSPath::Join(exeDir.c_str(), filename);
        std::ifstream in(std::string{modulePath.c_str()});
        std::string moduleSource{std::istreambuf_iterator<char>(in),
                                 std::istreambuf_iterator<char>()};
        if (in.rdstate()) {
            SK_ABORT("Error reading %s\n", modulePath.c_str());
        }
        return moduleSource;
    }
};

#endif  // MODULEDATAFILEPROVIDER
