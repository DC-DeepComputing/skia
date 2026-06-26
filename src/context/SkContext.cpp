/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/context/SkContext.h"

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendContext.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"  // IWYU pragma: keep
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/private/SingleOwner.h"
#include "include/private/SkThreadAnnotations.h"
#include "src/context/SkContextOptions.h"

static sk_sp<SkContext> MakeRaster(const SkContextOptions& options) {}

static sk_sp<SkContext> MakeGanesh(const skgpu::VulkanBackendContext& vkContext,
                                   const SkContextOptions&            options) {}

static sk_sp<SkContext> MakeGanesh(const skgpu::graphite::MtlBackendContext& mtlContext,
                                   const SkContextOptions&                   options) {}

static sk_sp<SkContext> MakeGraphite(const skgpu::graphite::MtlBackendContext& mtlContext,
                                     const SkContextOptions&                   options) {}

static sk_sp<SkContext> MakeGraphite(const skgpu::VulkanBackendContext& vkContext,
                                     const SkContextOptions&            options) {}
