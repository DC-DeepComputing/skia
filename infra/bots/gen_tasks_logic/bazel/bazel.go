// Copyright 2024 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//go:generate bazelisk run //:go -- run ./generate.go
package bazel

import "fmt"

func GetLabel(shorthand string) (string, error) {
	if rv, ok := shorthandToTarget[shorthand]; ok {
		return rv, nil
	}
	if rv, ok := sanitizedNameToTarget[shorthand]; ok {
		return rv, nil
	}
	ambiguousTargets, ok := ambiguousShorthands[shorthand]
	if ok {
		return "", fmt.Errorf("Shorthand %q is ambiguous; could refer to any of %v", shorthand, ambiguousTargets)
	}
	return "", fmt.Errorf("No known label for %q", shorthand)
}

func MustGetLabel(shorthand string) string {
	rv, err := GetLabel(shorthand)
	if err != nil {
		panic(err)
	}
	return rv
}
