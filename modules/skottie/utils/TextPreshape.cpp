/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/utils/TextPreshape.h"

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkArenaAlloc.h"
#include "src/utils/SkJSON.h"

#include <vector>

namespace skottie_utils {

namespace {

skjson::Value transcribe(const skjson::Value&, SkArenaAlloc*);

skjson::Value transcribe_array(const skjson::ArrayValue& a, SkArenaAlloc* alloc) {
    std::vector<skjson::Value> array;
    array.reserve(a.size());

    for (const auto& val : a) {
        array.push_back(transcribe(val, alloc));
    }

    return *alloc->make<skjson::ArrayValue>(array.data(), array.size(), *alloc);
}

skjson::Value transcribe_object(const skjson::ObjectValue& o, SkArenaAlloc* alloc) {
    std::vector<skjson::Member> members;
    members.reserve(o.size());

    for (const auto& m : o) {
        members.push_back({
            *alloc->make<skjson::StringValue>(m.fKey.begin(), m.fKey.size(), *alloc),
            transcribe(m.fValue, alloc)
        });
    }

    return *alloc->make<skjson::ObjectValue>(members.data(), members.size(), *alloc);
}

skjson::Value transcribe(const skjson::Value& v, SkArenaAlloc* alloc) {
    switch (v.getType()) {
        case skjson::Value::Type::kNull:
            return *alloc->make<skjson::NullValue>();
        case skjson::Value::Type::kBool:
            return *alloc->make<skjson::BoolValue>(*v.as<skjson::BoolValue>());
        case skjson::Value::Type::kNumber:
            return *alloc->make<skjson::NumberValue>(static_cast<float>(*v.as<skjson::NumberValue>()));
        case skjson::Value::Type::kString: {
            const skjson::StringValue& s = v.as<skjson::StringValue>();
            return *alloc->make<skjson::StringValue>(s.begin(), s.size(), *alloc);
        }
        case skjson::Value::Type::kArray:
            return transcribe_array(v.as<skjson::ArrayValue>(), alloc);
        case skjson::Value::Type::kObject:
            return transcribe_object(v.as<skjson::ObjectValue>(), alloc);
    }

    SkUNREACHABLE;
}

} //  namespace

bool Preshape(const skjson::DOM& in_dom, SkWStream* stream) {
    SkArenaAlloc alloc(4096);

    skjson::Value root = transcribe(in_dom.root(), &alloc);

    const auto str = root.toString();

    stream->writeText(str.c_str());

    return true;
}

} //  namespace skottie_utils
