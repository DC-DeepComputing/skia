# SkSL ("Skia Shading Language") & Compiler (`src/sksl/`)

## Overview

`src/sksl/` contains the compiler, intermediate representation (IR), optimizer, built-in module library, and multi-target code generators for **SkSL ("Skia Shading Language")**:

* **Internal GPU Shading**: Skia's GPU backends ([`../gpu/ganesh/`](../gpu/ganesh/) and [`../gpu/graphite/`](../gpu/graphite/)) author their built-in geometry, fragment, and compute shader stages in SkSL. The SkSL compiler translates these programs into target shading languages (**SPIR-V**, **Metal Shading Language (MSL)**, **WGSL**, **GLSL / GLSL ES**, and **HLSL**).
* **Public Runtime Effects (`SkRuntimeEffect`)**: Client applications write custom shaders, color filters, and blenders in SkSL via [`include/effects/`](../../include/effects/). SkSL validates and compiles these programs for both GPU execution and SIMD CPU execution on `SkRasterPipeline`.
* **Modules, IR & Optimization**: Built-in `.sksl` modules (`sksl_shared.sksl`, `sksl_gpu.sksl`, `sksl_graphite_frag.sksl`, `sksl_graphite_vert.sksl`, `sksl_rt_shader.sksl`) are parsed into typed AST/IR nodes, analyzed, inlined, and constant-folded before backend code generation.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`ir/`](ir/) | Typed AST/IR node definitions (`IRNode`, `Program`, `Statement`, `Expression`, `Symbol`, `Type`, `SymbolTable`, `Layout`, `Modifiers`). |
| [`analysis/`](analysis/) | Static analysis passes (`ProgramVisitor`, `ProgramUsage`, loop unrolling validation, side-effect and constant-expression checks, uniform specialization). |
| [`transform/`](transform/) | IR-to-IR optimization and lowering passes (dead code/function/variable elimination, built-in declaration injection, private symbol renaming, swizzle rewriting). |
| [`codegen/`](codegen/) | Target code generators for SPIR-V, MSL, WGSL, GLSL, HLSL, Ganesh pipeline stages, and CPU `SkRasterPipeline`. |
| [`tracing/`](tracing/) | SkSL debugger trace recording and step-by-step playback (`SkSLDebugTracePriv`, `SkSLTraceHook`, `SkSLDebugTracePlayer`). |
| [`generated/`](generated/) | Pre-minified (`*.minified.sksl`) and unoptimized (`*.unoptimized.sksl`) built-in module C++ string headers. |
| [`lex/`](lex/) | Standalone regular-expression NFA-to-DFA lexer generator (`sksl.lex`). |

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/sksl/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Compiler & Frontend** | [`SkSLCompiler.h`](SkSLCompiler.h), [`SkSLLexer.h`](SkSLLexer.h), [`SkSLParser.h`](SkSLParser.h), [`SkSLProgramKind.h`](SkSLProgramKind.h), [`SkSLProgramSettings.h`](SkSLProgramSettings.h), [`SkSLContext.h`](SkSLContext.h), [`SkSLErrorReporter.h`](SkSLErrorReporter.h) | Main compiler driver (`SkSL::Compiler`), DFA tokenizer (`SkSL::Lexer`), recursive-descent parser (`SkSL::Parser`), compilation configuration (`ProgramKind`, `ProgramSettings`), and shared compiler context (`SkSL::Context`). |
| **Modules & Built-ins** | [`SkSLModuleLoader.h`](SkSLModuleLoader.h), [`SkSLModule.h`](SkSLModule.h), [`SkSLBuiltinTypes.h`](SkSLBuiltinTypes.h), [`SkSLGraphiteModules.h`](SkSLGraphiteModules.h), [`SkSLIntrinsicList.h`](SkSLIntrinsicList.h) | On-demand loading of precompiled `.sksl` standard library and Graphite shader modules (`SkSL::ModuleLoader`, `SkSL::Module`) and built-in types (`SkSL::BuiltinTypes`). |
| **Intermediate Representation** | [`SkSLIRNode.h`](ir/SkSLIRNode.h), [`SkSLProgram.h`](ir/SkSLProgram.h), [`SkSLStatement.h`](ir/SkSLStatement.h), [`SkSLExpression.h`](ir/SkSLExpression.h), [`SkSLSymbolTable.h`](ir/SkSLSymbolTable.h), [`SkSLType.h`](ir/SkSLType.h), [`SkSLFunctionDefinition.h`](ir/SkSLFunctionDefinition.h), [`SkSLVarDeclarations.h`](ir/SkSLVarDeclarations.h) | Typed AST/IR hierarchy (`SkSL::IRNode`, `SkSL::Program`, `SkSL::Statement`, `SkSL::Expression`) and scoped symbol tables (`SkSL::SymbolTable`, `SkSL::Type`, `SkSL::Variable`) in [`ir/`](ir/). |
| **Analysis & Optimization** | [`SkSLAnalysis.h`](SkSLAnalysis.h), [`SkSLProgramVisitor.h`](analysis/SkSLProgramVisitor.h), [`SkSLProgramUsage.h`](analysis/SkSLProgramUsage.h), [`SkSLSpecialization.h`](analysis/SkSLSpecialization.h), [`SkSLTransform.h`](transform/SkSLTransform.h), [`SkSLInliner.h`](SkSLInliner.h), [`SkSLConstantFolder.h`](SkSLConstantFolder.h) | Static verification and usage tracking (`SkSL::Analysis`, `SkSL::ProgramUsage`) in [`analysis/`](analysis/), function inlining (`SkSL::Inliner`), constant folding (`SkSL::ConstantFolder`), and IR lowering/dead-code elimination (`SkSL::Transform`) in [`transform/`](transform/). |
| **GPU Code Generators** | [`SkSLCodeGenerator.h`](codegen/SkSLCodeGenerator.h), [`SkSLSPIRVCodeGenerator.h`](codegen/SkSLSPIRVCodeGenerator.h), [`SkSLMetalCodeGenerator.h`](codegen/SkSLMetalCodeGenerator.h), [`SkSLWGSLCodeGenerator.h`](codegen/SkSLWGSLCodeGenerator.h), [`SkSLGLSLCodeGenerator.h`](codegen/SkSLGLSLCodeGenerator.h), [`SkSLHLSLCodeGenerator.h`](codegen/SkSLHLSLCodeGenerator.h), [`SkSLPipelineStageCodeGenerator.h`](codegen/SkSLPipelineStageCodeGenerator.h) | Backend translators emitting SPIR-V, MSL, WGSL, GLSL, HLSL, and Ganesh modular `GrFragmentProcessor` stage callbacks in [`codegen/`](codegen/). |
| **CPU Raster Pipeline & Tracing** | [`SkSLRasterPipelineCodeGenerator.h`](codegen/SkSLRasterPipelineCodeGenerator.h), [`SkSLRasterPipelineBuilder.h`](codegen/SkSLRasterPipelineBuilder.h), [`SkSLDebugTracePriv.h`](tracing/SkSLDebugTracePriv.h), [`SkSLDebugTracePlayer.h`](tracing/SkSLDebugTracePlayer.h) | Compiles `SkRuntimeEffect` programs into SIMD `SkRasterPipeline` instructions (`SkSL::RP::Builder`) and supports step-by-step debugger trace execution in [`tracing/`](tracing/). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../../include/sksl/`](../../include/sksl/) | Public SkSL version and debug trace interfaces. |
| [`../../include/effects/`](../../include/effects/) | Public `SkRuntimeEffect` API for compiling client-authored SkSL shaders, color filters, and blenders. |
| [`../gpu/`](../gpu/) | Shared GPU adapter (`skgpu::SkSLToBackend`) and the [`../gpu/ganesh/`](../gpu/ganesh/) and [`../gpu/graphite/`](../gpu/graphite/) GPU backends. |

---

## Language Reference

SkSL ("Skia Shading Language") is a variant of GLSL which is used as Skia's
internal shading language. SkSL is, at its heart, a single standardized version
of GLSL which avoids all of the various version and dialect differences found
in GLSL "in the wild", but it does bring a few of its own changes to the table.

Skia uses the SkSL compiler to convert SkSL code to GLSL, GLSL ES, SPIR-V, WGSL, or
MSL before handing it over to the graphics driver.

## Differences from GLSL

* Precision modifiers are not used. 'float', 'int', and 'uint' are always high
  precision. New types 'half', 'short', and 'ushort' are medium precision (we
  do not use low precision).
* Vector types are named `<base type><columns>`, so `float2` instead of `vec2` and
  `bool4` instead of `bvec4`
* Matrix types are named `<base type><columns>x<rows>`, so `float2x3` instead of
  `mat2x3` and `double4x4` instead of `dmat4`
* GLSL caps can be referenced via the syntax `sk_Caps.<name>`, e.g.
  `sk_Caps.integerSupport`. The value will be a constant boolean or int,
  as appropriate. As SkSL supports constant folding and branch elimination, this
  means that an `if` statement which statically queries a cap will collapse down
  to the chosen branch, meaning that:

```
if (sk_Caps.integerSupport)
    do_something();
else
    do_something_else();
```

  will compile as if you had written either `do_something();` or
  `do_something_else();`, depending on whether that cap is enabled or not.
* no `#version` statement is required, and it will be ignored if present
* the output color is `sk_FragColor` (do not declare it)
* use `sk_Position` instead of `gl_Position`. `sk_Position` is in device coordinates
  rather than normalized coordinates.
* use `sk_PointSize` instead of `gl_PointSize`
* use `sk_VertexID` instead of `gl_VertexID`
* use `sk_InstanceID` instead of `gl_InstanceID`
* the fragment coordinate is `sk_FragCoord`, and is always relative to the upper
  left.
* use `sk_Clockwise` instead of `gl_FrontFacing`. This is always relative to an
  upper left origin.
* you do not need to include `.0` to make a number a float (meaning that
  `float2(x, y) * 4` is perfectly legal in SkSL, unlike GLSL where it would
  often have to be expressed `float2(x, y) * 4.0`. There is no performance
  penalty for this, as the number is converted to a float at compile time)
* type suffixes on numbers (`1.0f`, `0xFFu`) are both unnecessary and unsupported
* creating a smaller vector from a larger vector (e.g. `float2(float3(1))`) is
  intentionally disallowed, as it is just a wordier way of performing a swizzle.
  Use swizzles instead.
* Swizzle components, in addition to the normal `rgba` / `xyzw` components, can also
  be `LTRB` (meaning "left/top/right/bottom", for when we store rectangles in
  vectors), and may also be the constants `0` or `1` to produce a constant 0 or
  1 in that channel instead of selecting anything from the source vector.
  `foo.rgb1` is equivalent to `float4(foo.rgb, 1)`.
* All texture functions are named `sample`, e.g. `sample(sampler2D, float3)` is
  equivalent to GLSL's `textureProj(sampler2D, float3)`.
* Functions support the `inline` modifier, which causes the compiler to ignore
  its normal inlining heuristics and inline the function if at all possible
* some built-in functions and one or two rarely-used language features are not
  yet supported (sorry!)

## Synchronization Primitives

SkSL offers atomic operations and synchronization primitives geared towards GPU compute
programs. These primitives are designed to abstract over the capabilities provided by
MSL, SPIR-V, and WGSL, and differ from the corresponding primitives in GLSL.

### Atomics

SkSL provides the `atomicUint` type. This is an opaque type that requires the use of an
atomic intrinsic (such as `atomicLoad`, `atomicStore`, and `atomicAdd`) to act on its value (which
is of type `uint`).

A variable with the `atomicUint` type must be declared inside a writable storage buffer block or as
a workgroup-shared variable. When declared inside a buffer block, it is guaranteed to conform to the
same size and stride as a `uint`.

```
workgroup atomicUint myLocalAtomicUint;

layout(set = 0, binding = 0) buffer mySSBO {
    atomicUint myGlobalAtomicUint;
};
```

An `atomicUint` can be declared as a struct member or the element type of an array, provided that
the struct/array type is only instantiated in a workgroup-shared or storage buffer block variable.

#### Backend considerations and differences from GLSL

`atomicUint` should not be confused with the GLSL [`atomic_uint` (aka Atomic
Counter)](https://www.khronos.org/opengl/wiki/Atomic_Counter) type. The semantics provided by
`atomicUint` are more similar to GLSL ["Atomic Memory
Functions"](https://www.khronos.org/opengl/wiki/Atomic_Variable_Operations)
(see GLSL Spec v4.3, 8.11 "Atomic Memory Functions"). The key difference is that SkSL atomic
operations only operate on a variable of type `atomicUint` while GLSL Atomic Memory Functions can
operate over arbitrary memory locations (such as a component of a vector).

* The semantics of `atomicUint` are similar to Metal's `atomic<uint>` and WGSL's `atomic<u32>`.
  These are the types that an `atomicUint` is translated to when targeting Metal and WGSL.
* When translated to Metal, the atomic intrinsics use relaxed memory order semantics.
* When translated to SPIR-V, the atomic intrinsics use relaxed [memory
  semantics](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#Memory_Semantics_-id-)
  (i.e. `0x0 None`). The [memory
  scope](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#Scope_-id-) is either `1
  Device` or `2 Workgroup` depending on whether the `atomicUint` is declared in a buffer block or
  workgroup variable.

### Barriers

SkSL provides two barrier intrinsics: `workgroupBarrier()` and `storageBarrier()`. These functions
are only available in compute programs and synchronize access to workgroup-shared and storage buffer
memory between invocations in the same workgroup. They provide the same semantics as the equivalent
[WGSL Synchronization Built-in Functions](https://www.w3.org/TR/WGSL/#sync-builtin-functions). More
specifically:

* Both functions execute a control barrier with Acquire/Release memory ordering.
* Both functions use a `Workgroup` execution and memory scope. This means that a coherent memory
  view is only guaranteed between invocations in the same workgroup and NOT across workgroups in a
  given compute pipeline dispatch. If multiple workgroups require a _synchronized_ coherent view
  over the same shared mutable state, their access must be synchronized via other means (such as a
  pipeline barrier between multiple dispatches).

#### Backend considerations

* The closest GLSL equivalent for `workgroupBarrier()` is the
[`barrier()`](https://registry.khronos.org/OpenGL-Refpages/gl4/html/barrier.xhtml) intrinsic. Both
`workgroupBarrier()` and `storageBarrier()` can be defined as the following invocations of the
`controlBarrier` intrinsic defined in
[GL_KHR_memory_scope_semantics](https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_memory_scope_semantics.txt):

```
// workgroupBarrier():
controlBarrier(gl_ScopeWorkgroup,
               gl_ScopeWorkgroup,
               gl_StorageSemanticsShared,
               gl_SemanticsAcquireRelease);

// storageBarrier():
controlBarrier(gl_ScopeWorkgroup,
               gl_ScopeWorkgroup,
               gl_StorageSemanticsBuffer,
               gl_SemanticsAcquireRelease);
```

* In Metal, `workgroupBarrier()` is equivalent to `threadgroup_barrier(mem_flags::mem_threadgroup)`.
  `storageBarrier()` is equivalent to `threadgroup_barrier(mem_flags::mem_device)`.

* In Vulkan SPIR-V, `workgroupBarrier()` is equivalent to `OpControlBarrier` with `Workgroup`
  execution and memory scope, and `AcquireRelease | WorkgroupMemory` memory semantics.

  `storageBarrier()` is equivalent to `OpControlBarrier` with `Workgroup` execution and memory
  scope, and `AcquireRelease | UniformMemory` memory semantics.
