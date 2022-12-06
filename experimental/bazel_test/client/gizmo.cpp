#include "experimental/bazel_test/client/gizmo.h"

#include "experimental/bazel_test/base/base.h"
#include "experimental/bazel_test/core/core.h"

// Ideally this would cause an issue because Bazel would detect this
// file was included
#include "experimental/bazel_test/base/base_priv.h"
#include "experimental/bazel_test/core/core_priv.h"


float getGizmo() {
	return PublicCoreConstant * getMagicNumber() + PublicBaseConstant;
}