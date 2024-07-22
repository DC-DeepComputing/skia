### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-OpTypeRuntimeArray-04680] OpVariable, <id> '64[%64]', is attempting to create memory for an illegal type, OpTypeRuntimeArray.
For Vulkan OpTypeRuntimeArray can only appear as the final member of an OpTypeStruct, thus cannot be instantiated via OpVariable
  %64 = OpVariable %_ptr_Function__runtimearr_float_0 Function

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testStorageBuffer "testStorageBuffer"
               OpMemberName %testStorageBuffer 0 "testArr"
               OpName %S "S"
               OpMemberName %S 0 "y"
               OpName %testStorageBufferStruct "testStorageBufferStruct"
               OpMemberName %testStorageBufferStruct 0 "testArrStruct"
               OpName %sk_FragColor "sk_FragColor"
               OpName %unsizedInParameterD_iS "unsizedInParameterD_iS"
               OpName %getColor_h4f "getColor_h4f"
               OpName %getColorSecond_h4f "getColorSecond_h4f"
               OpName %main "main"
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %6 Binding 0
               OpDecorate %6 DescriptorSet 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 4
               OpMemberDecorate %testStorageBufferStruct 0 Offset 0
               OpMemberDecorate %testStorageBufferStruct 0 RelaxedPrecision
               OpDecorate %testStorageBufferStruct BufferBlock
               OpDecorate %11 Binding 1
               OpDecorate %11 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_runtimearr_S_0 ArrayStride 16
               OpDecorate %_runtimearr_float_0 ArrayStride 16
               OpDecorate %46 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
      %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float
%testStorageBuffer = OpTypeStruct %_runtimearr_float
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
          %6 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %int
%_runtimearr_S = OpTypeRuntimeArray %S
%testStorageBufferStruct = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_testStorageBufferStruct = OpTypePointer Uniform %testStorageBufferStruct
         %11 = OpVariable %_ptr_Uniform_testStorageBufferStruct Uniform
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_runtimearr_S_0 = OpTypeRuntimeArray %S
%_ptr_Function__runtimearr_S_0 = OpTypePointer Function %_runtimearr_S_0
         %22 = OpTypeFunction %int %_ptr_Function__runtimearr_S_0
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_runtimearr_float_0 = OpTypeRuntimeArray %float
%_ptr_Function__runtimearr_float_0 = OpTypePointer Function %_runtimearr_float_0
         %31 = OpTypeFunction %v4float %_ptr_Function__runtimearr_float_0
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
       %void = OpTypeVoid
         %59 = OpTypeFunction %void
%_ptr_Uniform__runtimearr_float = OpTypePointer Uniform %_runtimearr_float
%_ptr_Uniform__runtimearr_S = OpTypePointer Uniform %_runtimearr_S
%unsizedInParameterD_iS = OpFunction %int None %22
         %23 = OpFunctionParameter %_ptr_Function__runtimearr_S_0
         %24 = OpLabel
         %26 = OpAccessChain %_ptr_Function_int %23 %int_0 %int_0
         %28 = OpLoad %int %26
               OpReturnValue %28
               OpFunctionEnd
%getColor_h4f = OpFunction %v4float None %31
         %32 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %33 = OpLabel
         %34 = OpAccessChain %_ptr_Function_float %32 %int_0
         %36 = OpLoad %float %34
         %38 = OpAccessChain %_ptr_Function_float %32 %int_1
         %39 = OpLoad %float %38
         %41 = OpAccessChain %_ptr_Function_float %32 %int_2
         %42 = OpLoad %float %41
         %44 = OpAccessChain %_ptr_Function_float %32 %int_3
         %45 = OpLoad %float %44
         %46 = OpCompositeConstruct %v4float %36 %39 %42 %45
               OpReturnValue %46
               OpFunctionEnd
%getColorSecond_h4f = OpFunction %v4float None %31
         %47 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %48 = OpLabel
         %49 = OpAccessChain %_ptr_Function_float %47 %int_0
         %50 = OpLoad %float %49
         %51 = OpAccessChain %_ptr_Function_float %47 %int_1
         %52 = OpLoad %float %51
         %53 = OpAccessChain %_ptr_Function_float %47 %int_2
         %54 = OpLoad %float %53
         %55 = OpAccessChain %_ptr_Function_float %47 %int_3
         %56 = OpLoad %float %55
         %57 = OpCompositeConstruct %v4float %50 %52 %54 %56
               OpReturnValue %57
               OpFunctionEnd
       %main = OpFunction %void None %59
         %60 = OpLabel
         %64 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %68 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %73 = OpVariable %_ptr_Function__runtimearr_S_0 Function
         %61 = OpAccessChain %_ptr_Uniform__runtimearr_float %6 %int_0
         %63 = OpLoad %_runtimearr_float %61
               OpStore %64 %63
         %65 = OpFunctionCall %v4float %getColor_h4f %64
               OpStore %sk_FragColor %65
         %66 = OpAccessChain %_ptr_Uniform__runtimearr_float %6 %int_0
         %67 = OpLoad %_runtimearr_float %66
               OpStore %68 %67
         %69 = OpFunctionCall %v4float %getColorSecond_h4f %68
               OpStore %sk_FragColor %69
         %70 = OpAccessChain %_ptr_Uniform__runtimearr_S %11 %int_0
         %72 = OpLoad %_runtimearr_S %70
               OpStore %73 %72
         %74 = OpFunctionCall %int %unsizedInParameterD_iS %73
         %75 = OpConvertSToF %float %74
         %76 = OpCompositeConstruct %v4float %75 %75 %75 %75
               OpStore %sk_FragColor %76
               OpReturn
               OpFunctionEnd

1 error
