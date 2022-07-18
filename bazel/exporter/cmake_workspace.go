// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

// CMakeWorkspace represents the entire state of a set of CMake project files
// that comprise a project.
type CMakeWorkspace struct {
	// Map a directory path to the contents of a CMake file.
	dirProjects map[string]*CMakeProject
}

// NewCMakeWorkspace will create a new CMake workspace object.
func NewCMakeWorkspace() *CMakeWorkspace {
	return &CMakeWorkspace{dirProjects: make(map[string]*CMakeProject)}
}

// GetProject will return the CMake project for the corresponding directory path.
// Will return nil if there is no corresponding project.
func (w *CMakeWorkspace) GetProject(dirPath string) *CMakeProject {
	return w.dirProjects[dirPath]
}

// CreateProject will create (and return) a new CMake project object
// for the given directory path.
func (w *CMakeWorkspace) CreateProject(dirPath string) *CMakeProject {
	p := w.GetProject(dirPath)
	if p != nil {
		return p
	}
	p = &CMakeProject{Dir: dirPath}
	w.dirProjects[dirPath] = p
	return p
}
