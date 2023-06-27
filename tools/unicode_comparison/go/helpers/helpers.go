// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package helpers

import (
	"os"
	"path/filepath"
	"slices"
	"strconv"
	"strings"
)

func check(e error) {
	if e != nil {
		panic(e)
	}
}

func abs(x int) int {
	if x < 0 {
		return -x
	}
	return x
}

func minIndex(a []int) int {
	if len(a) == 0 {
		return -1
	}
	return slices.Min(a)
}

func splitAsInts(str string, sep string) []int {
	arr := strings.Split(str, sep)
	var result []int
	for _, i := range arr {
		j, err := strconv.Atoi(i)
		if err != nil {
			return nil
		}
		result = append(result, j)
	}
	return result
}

func insert(a []int, index int, value int) []int {
	if len(a) == index { // nil or empty slice or after last element
		return append(a, value)
	}
	a = append(a[:index+1], a[index:]...) // index < len(a)
	a[index] = value
	return a
}

func expandPath(path string) string {
	if strings.HasPrefix(path, "~/") {
		home, err := os.UserHomeDir()
		check(err)
		return filepath.Join(home, (path)[2:])
	}
	return path
}

func writeTextFile(fullFileName string, text string) {
	err := os.WriteFile(fullFileName, []byte(text), 0)
	check(err)
}
