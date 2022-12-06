
#include "experimental/bazel_test/core/core.h"
#include "experimental/bazel_test/base/base.h"
#include "experimental/bazel_test/base/base_priv.h"

int getMagicNumber() {
	return PublicBaseConstant + PrivateBaseConstant;
}