### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-OpTypeRuntimeArray-04680] OpVariable, <id> '38[%38]', is attempting to create memory for an illegal type, OpTypeRuntimeArray.
For Vulkan OpTypeRuntimeArray can only appear as the final member of an OpTypeStruct, thus cannot be instantiated via OpVariable
  %38 = OpVariable %_ptr_Function__runtimearr_float_0 Function

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testStorageBuffer "testStorageBuffer"
               OpMemberName %testStorageBuffer 0 "testArr"
               OpName %sk_FragColor "sk_FragColor"
               OpName %getColor_h4f "getColor_h4f"
               OpName %main "main"
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %4 Binding 0
               OpDecorate %4 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_runtimearr_float_0 ArrayStride 16
               OpDecorate %31 RelaxedPrecision
      %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float
%testStorageBuffer = OpTypeStruct %_runtimearr_float
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
          %4 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_runtimearr_float_0 = OpTypeRuntimeArray %float
%_ptr_Function__runtimearr_float_0 = OpTypePointer Function %_runtimearr_float_0
         %14 = OpTypeFunction %v4float %_ptr_Function__runtimearr_float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
       %void = OpTypeVoid
         %33 = OpTypeFunction %void
%_ptr_Uniform__runtimearr_float = OpTypePointer Uniform %_runtimearr_float
%getColor_h4f = OpFunction %v4float None %14
         %15 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %16 = OpLabel
         %19 = OpAccessChain %_ptr_Function_float %15 %int_0
         %21 = OpLoad %float %19
         %23 = OpAccessChain %_ptr_Function_float %15 %int_1
         %24 = OpLoad %float %23
         %26 = OpAccessChain %_ptr_Function_float %15 %int_2
         %27 = OpLoad %float %26
         %29 = OpAccessChain %_ptr_Function_float %15 %int_3
         %30 = OpLoad %float %29
         %31 = OpCompositeConstruct %v4float %21 %24 %27 %30
               OpReturnValue %31
               OpFunctionEnd
       %main = OpFunction %void None %33
         %34 = OpLabel
         %38 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %35 = OpAccessChain %_ptr_Uniform__runtimearr_float %4 %int_0
         %37 = OpLoad %_runtimearr_float %35
               OpStore %38 %37
         %39 = OpFunctionCall %v4float %getColor_h4f %38
               OpStore %sk_FragColor %39
               OpReturn
               OpFunctionEnd

1 error
