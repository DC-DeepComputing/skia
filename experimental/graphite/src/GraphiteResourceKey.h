/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteResourceKey_DEFINED
#define skgpu_GraphiteResourceKey_DEFINED

#include "src/gpu/ResourceKey.h"

namespace sk_gpu {

class GraphiteResourceKey : public skgpu::ResourceKey {
public:
    /** Uniquely identifies the type of resource that is cached as scratch. */
    typedef uint32_t ResourceType;

    /** Generate a unique ResourceType. */
    static ResourceType GenerateResourceType();

    /** Creates an invalid key. It must be initialized using a Builder object before use. */
    GraphiteResourceKey() : fShareable(Shareable::kNo) {}

    GraphiteResourceKey(const GraphiteResourceKey& that) { *this = that; }

    /** reset() returns the key to the invalid state. */
    using ResourceKey::reset;

    using ResourceKey::isValid;

    ResourceType resourceType() const { return this->domain(); }

    // Can the resource be held by multiple users at the same time?
    // For example, stencil buffers, pipelines, etc.
    enum class Shareable : bool {
        kNo = false,
        kYes = true,
    };
    Shareable shareable() const { return fShareable; }

    GraphiteResourceKey& operator=(const GraphiteResourceKey& that) {
        this->ResourceKey::operator=(that);
        return *this;
    }

    bool operator==(const GraphiteResourceKey& that) const {
        return this->ResourceKey::operator==(that);
    }
    bool operator!=(const GraphiteResourceKey& that) const {
        return !(*this == that);
    }

    class Builder : public ResourceKey::Builder {
    public:
        Builder(GraphiteResourceKey* key, ResourceType type, int data32Count, Shareable shareable)
                : ResourceKey::Builder(key, type, data32Count) {
            key->fShareable = shareable;
        }
    };

private:
    Shareable fShareable;
};

} // namespace skgpu

#endif // skgpu_GraphiteResourceKey_DEFINED
