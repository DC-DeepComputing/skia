### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-OpTypeRuntimeArray-04680] OpVariable, <id> '37[%37]', is attempting to create memory for an illegal type, OpTypeRuntimeArray.
For Vulkan OpTypeRuntimeArray can only appear as the final member of an OpTypeStruct, thus cannot be instantiated via OpVariable
  %37 = OpVariable %_ptr_Function__runtimearr_float_0 Function

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testStorageBuffer "testStorageBuffer"
               OpMemberName %testStorageBuffer 0 "testArr"
               OpName %sk_FragColor "sk_FragColor"
               OpName %getColor_h4f "getColor_h4f"
               OpName %getColor_helper_h4f "getColor_helper_h4f"
               OpName %getColorSecond_h4f "getColorSecond_h4f"
               OpName %main "main"
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %6 Binding 0
               OpDecorate %6 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_runtimearr_float_0 ArrayStride 16
               OpDecorate %33 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
      %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float
%testStorageBuffer = OpTypeStruct %_runtimearr_float
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
          %6 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_runtimearr_float_0 = OpTypeRuntimeArray %float
%_ptr_Function__runtimearr_float_0 = OpTypePointer Function %_runtimearr_float_0
         %16 = OpTypeFunction %v4float %_ptr_Function__runtimearr_float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
       %void = OpTypeVoid
         %51 = OpTypeFunction %void
%_ptr_Uniform__runtimearr_float = OpTypePointer Uniform %_runtimearr_float
%getColor_h4f = OpFunction %v4float None %16
         %17 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %18 = OpLabel
         %21 = OpAccessChain %_ptr_Function_float %17 %int_0
         %23 = OpLoad %float %21
         %25 = OpAccessChain %_ptr_Function_float %17 %int_1
         %26 = OpLoad %float %25
         %28 = OpAccessChain %_ptr_Function_float %17 %int_2
         %29 = OpLoad %float %28
         %31 = OpAccessChain %_ptr_Function_float %17 %int_3
         %32 = OpLoad %float %31
         %33 = OpCompositeConstruct %v4float %23 %26 %29 %32
               OpReturnValue %33
               OpFunctionEnd
%getColor_helper_h4f = OpFunction %v4float None %16
         %34 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %35 = OpLabel
         %37 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %36 = OpLoad %_runtimearr_float_0 %34
               OpStore %37 %36
         %38 = OpFunctionCall %v4float %getColor_h4f %37
               OpReturnValue %38
               OpFunctionEnd
%getColorSecond_h4f = OpFunction %v4float None %16
         %39 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %40 = OpLabel
         %41 = OpAccessChain %_ptr_Function_float %39 %int_0
         %42 = OpLoad %float %41
         %43 = OpAccessChain %_ptr_Function_float %39 %int_1
         %44 = OpLoad %float %43
         %45 = OpAccessChain %_ptr_Function_float %39 %int_2
         %46 = OpLoad %float %45
         %47 = OpAccessChain %_ptr_Function_float %39 %int_3
         %48 = OpLoad %float %47
         %49 = OpCompositeConstruct %v4float %42 %44 %46 %48
               OpReturnValue %49
               OpFunctionEnd
       %main = OpFunction %void None %51
         %52 = OpLabel
         %56 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %60 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %53 = OpAccessChain %_ptr_Uniform__runtimearr_float %6 %int_0
         %55 = OpLoad %_runtimearr_float %53
               OpStore %56 %55
         %57 = OpFunctionCall %v4float %getColor_helper_h4f %56
               OpStore %sk_FragColor %57
         %58 = OpAccessChain %_ptr_Uniform__runtimearr_float %6 %int_0
         %59 = OpLoad %_runtimearr_float %58
               OpStore %60 %59
         %61 = OpFunctionCall %v4float %getColorSecond_h4f %60
               OpStore %sk_FragColor %61
               OpReturn
               OpFunctionEnd

1 error
