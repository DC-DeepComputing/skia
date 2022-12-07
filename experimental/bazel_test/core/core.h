#pragma once

#include "experimental/bazel_test/base/base.h"
#include "experimental/bazel_test/base/base_priv.h"

constexpr float PublicCoreConstant = float(PublicBaseConstant)/float(PrivateBaseConstant);

int getMagicNumber();