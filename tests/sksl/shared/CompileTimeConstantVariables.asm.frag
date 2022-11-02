OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "integerInput"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %46 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%_UniformBuffer = OpTypeStruct %int %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_int = OpTypePointer Uniform %int
%int_0 = OpConstant %int 0
%float_2_1400001 = OpConstant %float 2.1400001
%36 = OpConstantComposite %v4float %float_2_1400001 %float_2_1400001 %float_2_1400001 %float_2_1400001
%int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_0_200000003 = OpConstant %float 0.200000003
%56 = OpConstantComposite %v4float %float_1 %float_0_200000003 %float_2_1400001 %float_1
%float_3_1400001 = OpConstant %float 3.1400001
%66 = OpConstantComposite %v4float %float_3_1400001 %float_3_1400001 %float_3_1400001 %float_3_1400001
%75 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%76 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%27 = OpAccessChain %_ptr_Uniform_int %10 %int_0
%30 = OpLoad %int %27
%31 = OpIEqual %bool %30 %int_0
OpSelectionMerge %34 None
OpBranchConditional %31 %32 %33
%32 = OpLabel
OpReturnValue %36
%33 = OpLabel
%37 = OpAccessChain %_ptr_Uniform_int %10 %int_0
%38 = OpLoad %int %37
%40 = OpIEqual %bool %38 %int_1
OpSelectionMerge %43 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%46 = OpLoad %v4float %44
OpReturnValue %46
%42 = OpLabel
%47 = OpAccessChain %_ptr_Uniform_int %10 %int_0
%48 = OpLoad %int %47
%50 = OpIEqual %bool %48 %int_2
OpSelectionMerge %53 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
OpReturnValue %56
%52 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%59 = OpLoad %v4float %58
%60 = OpCompositeExtract %float %59 0
%61 = OpFMul %float %60 %float_3_1400001
%62 = OpFOrdLessThan %bool %float_3_1400001 %61
OpSelectionMerge %65 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
OpReturnValue %66
%64 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%68 = OpLoad %v4float %67
%69 = OpCompositeExtract %float %68 0
%70 = OpFMul %float %69 %float_2_1400001
%71 = OpFOrdGreaterThanEqual %bool %float_2_1400001 %70
OpSelectionMerge %74 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
OpReturnValue %75
%73 = OpLabel
OpReturnValue %76
%74 = OpLabel
OpBranch %65
%65 = OpLabel
OpBranch %53
%53 = OpLabel
OpBranch %43
%43 = OpLabel
OpBranch %34
%34 = OpLabel
OpUnreachable
OpFunctionEnd
