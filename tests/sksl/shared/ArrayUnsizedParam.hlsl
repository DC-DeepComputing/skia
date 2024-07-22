### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-OpTypeRuntimeArray-04680] OpVariable, <id> '36[%36]', is attempting to create memory for an illegal type, OpTypeRuntimeArray.
For Vulkan OpTypeRuntimeArray can only appear as the final member of an OpTypeStruct, thus cannot be instantiated via OpVariable
  %36 = OpVariable %_ptr_Function__runtimearr_float_0 Function

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testStorageBuffer "testStorageBuffer"
               OpMemberName %testStorageBuffer 0 "testArr"
               OpName %sk_FragColor "sk_FragColor"
               OpName %getColor_h4f "getColor_h4f"
               OpName %helper_h4f "helper_h4f"
               OpName %main "main"
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %5 Binding 0
               OpDecorate %5 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_runtimearr_float_0 ArrayStride 16
               OpDecorate %32 RelaxedPrecision
      %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float
%testStorageBuffer = OpTypeStruct %_runtimearr_float
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
          %5 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_runtimearr_float_0 = OpTypeRuntimeArray %float
%_ptr_Function__runtimearr_float_0 = OpTypePointer Function %_runtimearr_float_0
         %15 = OpTypeFunction %v4float %_ptr_Function__runtimearr_float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
       %void = OpTypeVoid
         %39 = OpTypeFunction %void
%_ptr_Uniform__runtimearr_float = OpTypePointer Uniform %_runtimearr_float
%getColor_h4f = OpFunction %v4float None %15
         %16 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %17 = OpLabel
         %20 = OpAccessChain %_ptr_Function_float %16 %int_0
         %22 = OpLoad %float %20
         %24 = OpAccessChain %_ptr_Function_float %16 %int_1
         %25 = OpLoad %float %24
         %27 = OpAccessChain %_ptr_Function_float %16 %int_2
         %28 = OpLoad %float %27
         %30 = OpAccessChain %_ptr_Function_float %16 %int_3
         %31 = OpLoad %float %30
         %32 = OpCompositeConstruct %v4float %22 %25 %28 %31
               OpReturnValue %32
               OpFunctionEnd
 %helper_h4f = OpFunction %v4float None %15
         %33 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %34 = OpLabel
         %36 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %35 = OpLoad %_runtimearr_float_0 %33
               OpStore %36 %35
         %37 = OpFunctionCall %v4float %getColor_h4f %36
               OpReturnValue %37
               OpFunctionEnd
       %main = OpFunction %void None %39
         %40 = OpLabel
         %44 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %41 = OpAccessChain %_ptr_Uniform__runtimearr_float %5 %int_0
         %43 = OpLoad %_runtimearr_float %41
               OpStore %44 %43
         %45 = OpFunctionCall %v4float %helper_h4f %44
               OpStore %sk_FragColor %45
               OpReturn
               OpFunctionEnd

1 error
