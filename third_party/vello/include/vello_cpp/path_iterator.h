// Copyright 2023 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

namespace vello_cpp {

struct PathElement;

class PathIterator {
public:
    virtual ~PathIterator() = default;

    virtual bool next_element(PathElement* out_elem) = 0;
};

}  // namespace vello_cpp
