#include "include/core/SkSpan.h"
/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Application_DEFINED
#define Application_DEFINED

namespace sk_app {

class Application {
public:
    static Application* Create(int argc, SkSpan<char *>argv, void* platformData);

    virtual ~Application() {}

    virtual void onIdle() = 0;
};

}   // namespace sk_app

#endif
