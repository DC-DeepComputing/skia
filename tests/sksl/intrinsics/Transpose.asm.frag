OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testMatrix2x2"
OpMemberName %_UniformBuffer 1 "testMatrix3x3"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %testMatrix4x4 "testMatrix4x4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpMemberDecorate %_UniformBuffer 1 Offset 32
OpMemberDecorate %_UniformBuffer 1 ColMajor
OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 Offset 80
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 96
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat2v2float %mat3v3float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%float_10 = OpConstant %float 10
%float_11 = OpConstant %float 11
%float_12 = OpConstant %float 12
%float_13 = OpConstant %float 13
%float_14 = OpConstant %float 14
%float_15 = OpConstant %float 15
%float_16 = OpConstant %float 16
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_1 = OpConstant %int 1
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%testMatrix4x4 = OpVariable %_ptr_Function_mat4v4float Function
%130 = OpVariable %_ptr_Function_v4float Function
%48 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%49 = OpCompositeConstruct %v4float %float_5 %float_6 %float_7 %float_8
%50 = OpCompositeConstruct %v4float %float_9 %float_10 %float_11 %float_12
%51 = OpCompositeConstruct %v4float %float_13 %float_14 %float_15 %float_16
%52 = OpCompositeConstruct %mat4v4float %48 %49 %50 %51
OpStore %testMatrix4x4 %52
%55 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
%59 = OpLoad %mat2v2float %55
%54 = OpTranspose %mat2v2float %59
%60 = OpCompositeConstruct %v2float %float_1 %float_3
%61 = OpCompositeConstruct %v2float %float_2 %float_4
%62 = OpCompositeConstruct %mat2v2float %60 %61
%64 = OpCompositeExtract %v2float %54 0
%65 = OpCompositeExtract %v2float %62 0
%66 = OpFOrdEqual %v2bool %64 %65
%67 = OpAll %bool %66
%68 = OpCompositeExtract %v2float %54 1
%69 = OpCompositeExtract %v2float %62 1
%70 = OpFOrdEqual %v2bool %68 %69
%71 = OpAll %bool %70
%72 = OpLogicalAnd %bool %67 %71
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_1
%79 = OpLoad %mat3v3float %76
%75 = OpTranspose %mat3v3float %79
%80 = OpCompositeConstruct %v3float %float_1 %float_4 %float_7
%81 = OpCompositeConstruct %v3float %float_2 %float_5 %float_8
%82 = OpCompositeConstruct %v3float %float_3 %float_6 %float_9
%83 = OpCompositeConstruct %mat3v3float %80 %81 %82
%85 = OpCompositeExtract %v3float %75 0
%86 = OpCompositeExtract %v3float %83 0
%87 = OpFOrdEqual %v3bool %85 %86
%88 = OpAll %bool %87
%89 = OpCompositeExtract %v3float %75 1
%90 = OpCompositeExtract %v3float %83 1
%91 = OpFOrdEqual %v3bool %89 %90
%92 = OpAll %bool %91
%93 = OpLogicalAnd %bool %88 %92
%94 = OpCompositeExtract %v3float %75 2
%95 = OpCompositeExtract %v3float %83 2
%96 = OpFOrdEqual %v3bool %94 %95
%97 = OpAll %bool %96
%98 = OpLogicalAnd %bool %93 %97
OpBranch %74
%74 = OpLabel
%99 = OpPhi %bool %false %28 %98 %73
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%103 = OpLoad %mat4v4float %testMatrix4x4
%102 = OpTranspose %mat4v4float %103
%104 = OpCompositeConstruct %v4float %float_1 %float_5 %float_9 %float_13
%105 = OpCompositeConstruct %v4float %float_2 %float_6 %float_10 %float_14
%106 = OpCompositeConstruct %v4float %float_3 %float_7 %float_11 %float_15
%107 = OpCompositeConstruct %v4float %float_4 %float_8 %float_12 %float_16
%108 = OpCompositeConstruct %mat4v4float %104 %105 %106 %107
%110 = OpCompositeExtract %v4float %102 0
%111 = OpCompositeExtract %v4float %108 0
%112 = OpFOrdEqual %v4bool %110 %111
%113 = OpAll %bool %112
%114 = OpCompositeExtract %v4float %102 1
%115 = OpCompositeExtract %v4float %108 1
%116 = OpFOrdEqual %v4bool %114 %115
%117 = OpAll %bool %116
%118 = OpLogicalAnd %bool %113 %117
%119 = OpCompositeExtract %v4float %102 2
%120 = OpCompositeExtract %v4float %108 2
%121 = OpFOrdEqual %v4bool %119 %120
%122 = OpAll %bool %121
%123 = OpLogicalAnd %bool %118 %122
%124 = OpCompositeExtract %v4float %102 3
%125 = OpCompositeExtract %v4float %108 3
%126 = OpFOrdEqual %v4bool %124 %125
%127 = OpAll %bool %126
%128 = OpLogicalAnd %bool %123 %127
OpBranch %101
%101 = OpLabel
%129 = OpPhi %bool %false %74 %128 %100
OpSelectionMerge %134 None
OpBranchConditional %129 %132 %133
%132 = OpLabel
%135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%138 = OpLoad %v4float %135
OpStore %130 %138
OpBranch %134
%133 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%141 = OpLoad %v4float %139
OpStore %130 %141
OpBranch %134
%134 = OpLabel
%142 = OpLoad %v4float %130
OpReturnValue %142
OpFunctionEnd
