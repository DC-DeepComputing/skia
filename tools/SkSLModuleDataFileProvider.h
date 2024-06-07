/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MODULEDATAFILEPROVIDER
#define SKSL_MODULEDATAFILEPROVIDER

#include "include/core/SkString.h"
#include "src/sksl/SkSLModuleDataProvider.h"
#include "src/utils/SkOSPath.h"
#include "tools/SkGetExecutablePath.h"

#include <fstream>

namespace SkSL {

class ModuleDataFileProvider : public ModuleDataProvider {
public:
    ~ModuleDataFileProvider() override = default;

    std::string get(ModuleName /*name*/, const char* filename) const override {
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

}  // namespace SkSL

#endif  // SKSL_MODULEDATAFILEPROVIDER
