// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"testing"
)

func TestAnalyzeSource(t *testing.T) {
	test := func(name, source string, expected []string) {
		t.Run(name, func(t *testing.T) {
			warnings, err := AnalyzeSource([]byte(source))
			if err != nil {
				t.Fatalf("Failed to analyze source: %v", err)
			}

			// Validate if we found the expected warnings
			for _, exp := range expected {
				found := false
				for _, w := range warnings {
					if w.Field == exp {
						found = true
						break
					}
				}
				if !found {
					t.Errorf("Expected warning %q was not found in: %v", exp, warnings)
				}
			}

			// Ensure we don't have unexpected warnings
			if len(warnings) > len(expected) {
				t.Errorf("Found unexpected warnings: %v (expected: %v)", warnings, expected)
			}
		})
	}

	test("Simple Safe Order (Smart Pointer)", `
class Safe {
    std::unique_ptr<int> fA;
    std::unique_ptr<int> fB;
public:
    Safe() : fA(nullptr), fB(fA.get() ? nullptr : nullptr) {}
};
`, nil)

	test("Simple Unsafe Order (Smart Pointer)", `
class Unsafe {
    std::unique_ptr<int> fB;
    std::unique_ptr<int> fA;
public:
    Unsafe() : fB(fA.get() ? nullptr : nullptr), fA(nullptr) {}
};
`, []string{"fB depends on fA"})

	test("Trivial Type (Int - Safe)", `
class TrivialInt {
    int fB;
    int fA;
public:
    TrivialInt(int val) : fB(fA), fA(val) {}
};
`, nil)

	test("Trivial Type (Float Vector - Safe)", `
class TrivialVector {
    skvx::float2 fB;
    skvx::float2 fA;
public:
    TrivialVector(float val) : fB(fA), fA(val) {}
};
`, nil)

	test("No Dependency", `
class NoDependency {
    std::unique_ptr<int> fA;
    std::unique_ptr<int> fB;
public:
    NoDependency() {}
};
`, nil)

	test("Complex Unsafe Order", `
class ComplexUnsafe {
    std::unique_ptr<DrawManager> fDrawingManager;
    std::unique_ptr<ProxyProvider> fProxyProvider;
public:
    ComplexUnsafe() : fDrawingManager(fProxyProvider->getManager()) {}
};
`, []string{"fDrawingManager depends on fProxyProvider"})

	test("Constructor Body Unsafe Order (Smart Pointer)", `
class BodyUnsafe {
    std::unique_ptr<int> fB;
    std::unique_ptr<int> fA;
public:
    BodyUnsafe() {
        fB = std::move(fA);
    }
};
`, []string{"fB depends on fA"})

	// Sibling type semantic dependencies
	test("Semantic TaskList vs Resource (Unsafe)", `
class Recording {
    std::unique_ptr<TaskList> fRootTaskList;
    std::vector<sk_sp<Resource>> fExtraResourceRefs;
};
`, []string{"fRootTaskList depends on fExtraResourceRefs"})

	test("Semantic TaskList vs Resource (Safe)", `
class RecordingSafe {
    std::vector<sk_sp<Resource>> fExtraResourceRefs;
    std::unique_ptr<TaskList> fRootTaskList;
};
`, nil)

	test("Semantic DrawingManager vs ProxyProvider (Unsafe)", `
class GrRecordingContext {
    std::unique_ptr<GrDrawingManager> fDrawingManager;
    std::unique_ptr<GrProxyProvider> fProxyProvider;
};
`, []string{"fDrawingManager depends on fProxyProvider"})

	test("Semantic DrawingManager vs ProxyProvider (Safe)", `
class GrRecordingContextSafe {
    std::unique_ptr<GrProxyProvider> fProxyProvider;
    std::unique_ptr<GrDrawingManager> fDrawingManager;
};
`, nil)
}
