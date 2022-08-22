OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "ah"
OpMemberName %_UniformBuffer 1 "bh"
OpMemberName %_UniformBuffer 2 "af"
OpMemberName %_UniformBuffer 3 "bf"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 8
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 16
OpMemberDecorate %_UniformBuffer 3 Offset 24
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %23 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v2float %v2float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%false = OpConstantFalse %bool
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Output_float = OpTypePointer Output %float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%float_12 = OpConstant %float 12
%float_n8 = OpConstant %float -8
%v3float = OpTypeVector %float 3
%85 = OpConstantComposite %v3float %float_n8 %float_n8 %float_12
%float_9 = OpConstant %float 9
%float_n18 = OpConstant %float -18
%float_n9 = OpConstant %float -9
%91 = OpConstantComposite %v3float %float_9 %float_n18 %float_n9
%main = OpFunction %void None %15
%16 = OpLabel
%18 = OpVariable %_ptr_Function_float Function
%52 = OpVariable %_ptr_Function_float Function
OpSelectionMerge %22 None
OpBranchConditional %false %20 %21
%20 = OpLabel
%24 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%28 = OpLoad %v2float %24
%29 = OpAccessChain %_ptr_Uniform_v2float %10 %int_1
%31 = OpLoad %v2float %29
%33 = OpCompositeConstruct %mat2v2float %28 %31
%23 = OpExtInst %float %1 Determinant %33
OpStore %18 %23
OpBranch %22
%21 = OpLabel
%34 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%35 = OpLoad %v2float %34
%36 = OpCompositeExtract %float %35 0
%37 = OpAccessChain %_ptr_Uniform_v2float %10 %int_1
%38 = OpLoad %v2float %37
%39 = OpCompositeExtract %float %38 1
%40 = OpFMul %float %36 %39
%41 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%42 = OpLoad %v2float %41
%43 = OpCompositeExtract %float %42 1
%44 = OpAccessChain %_ptr_Uniform_v2float %10 %int_1
%45 = OpLoad %v2float %44
%46 = OpCompositeExtract %float %45 0
%47 = OpFMul %float %43 %46
%48 = OpFSub %float %40 %47
OpStore %18 %48
OpBranch %22
%22 = OpLabel
%49 = OpLoad %float %18
%50 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %50 %49
OpSelectionMerge %55 None
OpBranchConditional %false %53 %54
%53 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_v2float %10 %int_2
%59 = OpLoad %v2float %57
%60 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%62 = OpLoad %v2float %60
%63 = OpCompositeConstruct %mat2v2float %59 %62
%56 = OpExtInst %float %1 Determinant %63
OpStore %52 %56
OpBranch %55
%54 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v2float %10 %int_2
%65 = OpLoad %v2float %64
%66 = OpCompositeExtract %float %65 0
%67 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%68 = OpLoad %v2float %67
%69 = OpCompositeExtract %float %68 1
%70 = OpFMul %float %66 %69
%71 = OpAccessChain %_ptr_Uniform_v2float %10 %int_2
%72 = OpLoad %v2float %71
%73 = OpCompositeExtract %float %72 1
%74 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%75 = OpLoad %v2float %74
%76 = OpCompositeExtract %float %75 0
%77 = OpFMul %float %73 %76
%78 = OpFSub %float %70 %77
OpStore %52 %78
OpBranch %55
%55 = OpLabel
%79 = OpLoad %float %52
%80 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %80 %79
%82 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %82 %float_12
%86 = OpLoad %v4float %sk_FragColor
%87 = OpVectorShuffle %v4float %86 %85 4 5 6 3
OpStore %sk_FragColor %87
%92 = OpLoad %v4float %sk_FragColor
%93 = OpVectorShuffle %v4float %92 %91 0 4 5 6
OpStore %sk_FragColor %93
OpReturn
OpFunctionEnd
