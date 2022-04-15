OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %inv2x2 "inv2x2"
OpName %inv3x3 "inv3x3"
OpName %inv4x4 "inv4x4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %inv2x2 RelaxedPrecision
OpDecorate %inv3x3 RelaxedPrecision
OpDecorate %inv4x4 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_n2 = OpConstant %float -2
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%float_n0_5 = OpConstant %float -0.5
%33 = OpConstantComposite %v2float %float_n2 %float_1
%34 = OpConstantComposite %v2float %float_1_5 %float_n0_5
%35 = OpConstantComposite %mat2v2float %33 %34
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_n24 = OpConstant %float -24
%float_18 = OpConstant %float 18
%float_5 = OpConstant %float 5
%float_20 = OpConstant %float 20
%float_n15 = OpConstant %float -15
%float_n4 = OpConstant %float -4
%float_n5 = OpConstant %float -5
%float_4 = OpConstant %float 4
%48 = OpConstantComposite %v3float %float_n24 %float_18 %float_5
%49 = OpConstantComposite %v3float %float_20 %float_n15 %float_n4
%50 = OpConstantComposite %v3float %float_n5 %float_4 %float_1
%51 = OpConstantComposite %mat3v3float %48 %49 %50
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_0_5 = OpConstant %float 0.5
%float_n8 = OpConstant %float -8
%float_n1 = OpConstant %float -1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%60 = OpConstantComposite %v4float %float_n2 %float_n0_5 %float_1 %float_0_5
%61 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0 %float_n0_5
%62 = OpConstantComposite %v4float %float_n8 %float_n1 %float_2 %float_2
%63 = OpConstantComposite %v4float %float_3 %float_0_5 %float_n1 %float_n0_5
%64 = OpConstantComposite %mat4v4float %60 %61 %62 %63
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%115 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%116 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%117 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%118 = OpConstantComposite %mat3v3float %115 %116 %117
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%inv2x2 = OpVariable %_ptr_Function_mat2v2float Function
%inv3x3 = OpVariable %_ptr_Function_mat3v3float Function
%inv4x4 = OpVariable %_ptr_Function_mat4v4float Function
%135 = OpVariable %_ptr_Function_v4float Function
OpStore %inv2x2 %35
OpStore %inv3x3 %51
OpStore %inv4x4 %64
%67 = OpFOrdEqual %v2bool %33 %33
%68 = OpAll %bool %67
%69 = OpFOrdEqual %v2bool %34 %34
%70 = OpAll %bool %69
%71 = OpLogicalAnd %bool %68 %70
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpLoad %mat3v3float %inv3x3
%76 = OpCompositeExtract %v3float %74 0
%77 = OpFOrdEqual %v3bool %48 %76
%78 = OpAll %bool %77
%79 = OpCompositeExtract %v3float %74 1
%80 = OpFOrdEqual %v3bool %49 %79
%81 = OpAll %bool %80
%82 = OpLogicalAnd %bool %78 %81
%83 = OpCompositeExtract %v3float %74 2
%84 = OpFOrdEqual %v3bool %50 %83
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %82 %85
OpBranch %73
%73 = OpLabel
%87 = OpPhi %bool %false %25 %86 %72
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %mat4v4float %inv4x4
%92 = OpCompositeExtract %v4float %90 0
%93 = OpFOrdEqual %v4bool %60 %92
%94 = OpAll %bool %93
%95 = OpCompositeExtract %v4float %90 1
%96 = OpFOrdEqual %v4bool %61 %95
%97 = OpAll %bool %96
%98 = OpLogicalAnd %bool %94 %97
%99 = OpCompositeExtract %v4float %90 2
%100 = OpFOrdEqual %v4bool %62 %99
%101 = OpAll %bool %100
%102 = OpLogicalAnd %bool %98 %101
%103 = OpCompositeExtract %v4float %90 3
%104 = OpFOrdEqual %v4bool %63 %103
%105 = OpAll %bool %104
%106 = OpLogicalAnd %bool %102 %105
OpBranch %89
%89 = OpLabel
%107 = OpPhi %bool %false %73 %106 %88
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpExtInst %mat3v3float %1 MatrixInverse %118
%119 = OpLoad %mat3v3float %inv3x3
%120 = OpCompositeExtract %v3float %110 0
%121 = OpCompositeExtract %v3float %119 0
%122 = OpFUnordNotEqual %v3bool %120 %121
%123 = OpAny %bool %122
%124 = OpCompositeExtract %v3float %110 1
%125 = OpCompositeExtract %v3float %119 1
%126 = OpFUnordNotEqual %v3bool %124 %125
%127 = OpAny %bool %126
%128 = OpLogicalOr %bool %123 %127
%129 = OpCompositeExtract %v3float %110 2
%130 = OpCompositeExtract %v3float %119 2
%131 = OpFUnordNotEqual %v3bool %129 %130
%132 = OpAny %bool %131
%133 = OpLogicalOr %bool %128 %132
OpBranch %109
%109 = OpLabel
%134 = OpPhi %bool %false %89 %133 %108
OpSelectionMerge %139 None
OpBranchConditional %134 %137 %138
%137 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%144 = OpLoad %v4float %140
OpStore %135 %144
OpBranch %139
%138 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%147 = OpLoad %v4float %145
OpStore %135 %147
OpBranch %139
%139 = OpLabel
%148 = OpLoad %v4float %135
OpReturnValue %148
OpFunctionEnd
