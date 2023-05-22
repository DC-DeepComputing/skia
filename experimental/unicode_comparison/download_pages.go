// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"encoding/base64"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"os/signal"
	"path"
	"path/filepath"
	"strconv"
	"syscall"
)

func main() {
    if len(os.Args) < 3 {
		fmt.Println(`Must set --repo_dir
This is done automatically via:
    bazel run //bazel/deps_parser`)
		os.Exit(1)
    }
}

