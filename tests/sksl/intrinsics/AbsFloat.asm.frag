OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expected "expected"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %110 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1_25 = OpConstant %float 1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_1_25 %float_0 %float_0_75 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%84 = OpConstantComposite %v2float %float_1_25 %float_0
%92 = OpConstantComposite %v3float %float_1_25 %float_0 %float_0_75
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%expected = OpVariable %_ptr_Function_v4float Function
%104 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %31
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %34
%39 = OpCompositeExtract %float %38 0
%33 = OpExtInst %float %1 FAbs %39
%40 = OpFOrdEqual %bool %33 %float_1_25
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%45 = OpLoad %v4float %44
%46 = OpVectorShuffle %v2float %45 %45 0 1
%43 = OpExtInst %v2float %1 FAbs %46
%47 = OpLoad %v4float %expected
%48 = OpVectorShuffle %v2float %47 %47 0 1
%49 = OpFOrdEqual %v2bool %43 %48
%51 = OpAll %bool %49
OpBranch %42
%42 = OpLabel
%52 = OpPhi %bool %false %25 %51 %41
OpSelectionMerge %54 None
OpBranchConditional %52 %53 %54
%53 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%57 = OpLoad %v4float %56
%58 = OpVectorShuffle %v3float %57 %57 0 1 2
%55 = OpExtInst %v3float %1 FAbs %58
%60 = OpLoad %v4float %expected
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%62 = OpFOrdEqual %v3bool %55 %61
%64 = OpAll %bool %62
OpBranch %54
%54 = OpLabel
%65 = OpPhi %bool %false %42 %64 %53
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%70 = OpLoad %v4float %69
%68 = OpExtInst %v4float %1 FAbs %70
%71 = OpLoad %v4float %expected
%72 = OpFOrdEqual %v4bool %68 %71
%74 = OpAll %bool %72
OpBranch %67
%67 = OpLabel
%75 = OpPhi %bool %false %54 %74 %66
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpLoad %v4float %expected
%79 = OpCompositeExtract %float %78 0
%80 = OpFOrdEqual %bool %float_1_25 %79
OpBranch %77
%77 = OpLabel
%81 = OpPhi %bool %false %67 %80 %76
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpLoad %v4float %expected
%86 = OpVectorShuffle %v2float %85 %85 0 1
%87 = OpFOrdEqual %v2bool %84 %86
%88 = OpAll %bool %87
OpBranch %83
%83 = OpLabel
%89 = OpPhi %bool %false %77 %88 %82
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpLoad %v4float %expected
%94 = OpVectorShuffle %v3float %93 %93 0 1 2
%95 = OpFOrdEqual %v3bool %92 %94
%96 = OpAll %bool %95
OpBranch %91
%91 = OpLabel
%97 = OpPhi %bool %false %83 %96 %90
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpLoad %v4float %expected
%101 = OpFOrdEqual %v4bool %31 %100
%102 = OpAll %bool %101
OpBranch %99
%99 = OpLabel
%103 = OpPhi %bool %false %91 %102 %98
OpSelectionMerge %107 None
OpBranchConditional %103 %105 %106
%105 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%110 = OpLoad %v4float %108
OpStore %104 %110
OpBranch %107
%106 = OpLabel
%111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%113 = OpLoad %v4float %111
OpStore %104 %113
OpBranch %107
%107 = OpLabel
%114 = OpLoad %v4float %104
OpReturnValue %114
OpFunctionEnd
