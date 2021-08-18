OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expected "expected"
OpName %exponents "exponents"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %exponents RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1_5625 = OpConstant %float -1.5625
%float_0_75 = OpConstant %float 0.75
%float_3_375 = OpConstant %float 3.375
%33 = OpConstantComposite %v4float %float_n1_5625 %float_0 %float_0_75 %float_3_375
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%39 = OpConstantComposite %v4float %float_2 %float_3 %float_1 %float_1_5
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%57 = OpConstantComposite %v2float %float_2 %float_3
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%71 = OpConstantComposite %v3float %float_2 %float_3 %float_1
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1_5625 = OpConstant %float 1.5625
%98 = OpConstantComposite %v2float %float_1_5625 %float_0
%106 = OpConstantComposite %v3float %float_1_5625 %float_0 %float_0_75
%114 = OpConstantComposite %v4float %float_1_5625 %float_0 %float_0_75 %float_3_375
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %25
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpLabel
%expected = OpVariable %_ptr_Function_v4float Function
%exponents = OpVariable %_ptr_Function_v4float Function
%119 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %33
OpStore %exponents %39
%42 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%46 = OpLoad %v4float %42
%47 = OpCompositeExtract %float %46 0
%41 = OpExtInst %float %1 Pow %47 %float_2
%48 = OpLoad %v4float %expected
%49 = OpCompositeExtract %float %48 0
%50 = OpFOrdEqual %bool %41 %49
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%54 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%55 = OpLoad %v4float %54
%56 = OpVectorShuffle %v2float %55 %55 0 1
%53 = OpExtInst %v2float %1 Pow %56 %57
%58 = OpLoad %v4float %expected
%59 = OpVectorShuffle %v2float %58 %58 0 1
%60 = OpFOrdEqual %v2bool %53 %59
%62 = OpAll %bool %60
OpBranch %52
%52 = OpLabel
%63 = OpPhi %bool %false %27 %62 %51
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%66 = OpExtInst %v3float %1 Pow %69 %71
%72 = OpLoad %v4float %expected
%73 = OpVectorShuffle %v3float %72 %72 0 1 2
%74 = OpFOrdEqual %v3bool %66 %73
%76 = OpAll %bool %74
OpBranch %65
%65 = OpLabel
%77 = OpPhi %bool %false %52 %76 %64
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%82 = OpLoad %v4float %81
%83 = OpLoad %v4float %exponents
%80 = OpExtInst %v4float %1 Pow %82 %83
%84 = OpLoad %v4float %expected
%85 = OpFOrdEqual %v4bool %80 %84
%87 = OpAll %bool %85
OpBranch %79
%79 = OpLabel
%88 = OpPhi %bool %false %65 %87 %78
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpLoad %v4float %expected
%93 = OpCompositeExtract %float %92 0
%94 = OpFOrdEqual %bool %float_1_5625 %93
OpBranch %90
%90 = OpLabel
%95 = OpPhi %bool %false %79 %94 %89
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpLoad %v4float %expected
%100 = OpVectorShuffle %v2float %99 %99 0 1
%101 = OpFOrdEqual %v2bool %98 %100
%102 = OpAll %bool %101
OpBranch %97
%97 = OpLabel
%103 = OpPhi %bool %false %90 %102 %96
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%107 = OpLoad %v4float %expected
%108 = OpVectorShuffle %v3float %107 %107 0 1 2
%109 = OpFOrdEqual %v3bool %106 %108
%110 = OpAll %bool %109
OpBranch %105
%105 = OpLabel
%111 = OpPhi %bool %false %97 %110 %104
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpLoad %v4float %expected
%116 = OpFOrdEqual %v4bool %114 %115
%117 = OpAll %bool %116
OpBranch %113
%113 = OpLabel
%118 = OpPhi %bool %false %105 %117 %112
OpSelectionMerge %122 None
OpBranchConditional %118 %120 %121
%120 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%125 = OpLoad %v4float %123
OpStore %119 %125
OpBranch %122
%121 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%128 = OpLoad %v4float %126
OpStore %119 %128
OpBranch %122
%122 = OpLabel
%129 = OpLoad %v4float %119
OpReturnValue %129
OpFunctionEnd
