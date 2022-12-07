#pragma once

#include "experimental/bazel_test/base/base.h"
#include "experimental/bazel_test/base/base_priv.h"

constexpr float PrivateCoreConstant = float(PublicBaseConstant)/float(PrivateBaseConstant * PrivateBaseConstant);