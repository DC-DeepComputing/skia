               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %result "result"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %result RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
     %result = OpVariable %_ptr_Function_v4float Function
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %25
         %30 = OpCompositeExtract %float %29 0
         %31 = OpAccessChain %_ptr_Function_float %result %int_0
               OpStore %31 %30
         %33 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %34 = OpLoad %v4float %33
         %35 = OpCompositeExtract %float %34 1
         %36 = OpAccessChain %_ptr_Function_float %result %int_1
               OpStore %36 %35
         %38 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %39 = OpLoad %v4float %38
         %40 = OpCompositeExtract %float %39 2
         %41 = OpAccessChain %_ptr_Function_float %result %int_2
               OpStore %41 %40
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %44 = OpLoad %v4float %43
         %45 = OpCompositeExtract %float %44 3
         %46 = OpAccessChain %_ptr_Function_float %result %int_3
               OpStore %46 %45
         %48 = OpLoad %v4float %result
               OpReturnValue %48
               OpFunctionEnd
