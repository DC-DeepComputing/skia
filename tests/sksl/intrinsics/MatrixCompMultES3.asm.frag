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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %h22 "h22"
OpName %h44 "h44"
OpName %f43 "f43"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %h22 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %h44 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_5 = OpConstant %float 5
%float_10 = OpConstant %float 10
%float_15 = OpConstant %float 15
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_0_5 = OpConstant %float 0.5
%float_3 = OpConstant %float 3
%float_5_5 = OpConstant %float 5.5
%float_8 = OpConstant %float 8
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_12 = OpConstant %float 12
%float_22 = OpConstant %float 22
%float_30 = OpConstant %float 30
%float_36 = OpConstant %float 36
%float_40 = OpConstant %float 40
%float_42 = OpConstant %float 42
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v4bool = OpTypeVector %bool 4
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%h22 = OpVariable %_ptr_Function_mat2v2float Function
%h44 = OpVariable %_ptr_Function_mat4v4float Function
%f43 = OpVariable %_ptr_Function_mat4v3float Function
%136 = OpVariable %_ptr_Function_v4float Function
%34 = OpCompositeConstruct %v2float %float_0 %float_5
%35 = OpCompositeConstruct %v2float %float_10 %float_15
%36 = OpCompositeConstruct %mat2v2float %34 %35
OpStore %h22 %36
%44 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %float_0
%45 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%46 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5_5 %float_0
%47 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_8
%48 = OpCompositeConstruct %mat4v4float %44 %45 %46 %47
OpStore %h44 %48
%58 = OpCompositeConstruct %v3float %float_12 %float_22 %float_30
%59 = OpCompositeConstruct %v3float %float_36 %float_40 %float_42
%60 = OpCompositeConstruct %v3float %float_42 %float_40 %float_36
%61 = OpCompositeConstruct %v3float %float_30 %float_22 %float_12
%62 = OpCompositeConstruct %mat4v3float %58 %59 %60 %61
OpStore %f43 %62
%64 = OpLoad %mat2v2float %h22
%65 = OpCompositeConstruct %v2float %float_0 %float_5
%66 = OpCompositeConstruct %v2float %float_10 %float_15
%67 = OpCompositeConstruct %mat2v2float %65 %66
%69 = OpCompositeExtract %v2float %64 0
%70 = OpCompositeExtract %v2float %67 0
%71 = OpFOrdEqual %v2bool %69 %70
%72 = OpAll %bool %71
%73 = OpCompositeExtract %v2float %64 1
%74 = OpCompositeExtract %v2float %67 1
%75 = OpFOrdEqual %v2bool %73 %74
%76 = OpAll %bool %75
%77 = OpLogicalAnd %bool %72 %76
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpLoad %mat4v4float %h44
%81 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %float_0
%82 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%83 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5_5 %float_0
%84 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_8
%85 = OpCompositeConstruct %mat4v4float %81 %82 %83 %84
%87 = OpCompositeExtract %v4float %80 0
%88 = OpCompositeExtract %v4float %85 0
%89 = OpFOrdEqual %v4bool %87 %88
%90 = OpAll %bool %89
%91 = OpCompositeExtract %v4float %80 1
%92 = OpCompositeExtract %v4float %85 1
%93 = OpFOrdEqual %v4bool %91 %92
%94 = OpAll %bool %93
%95 = OpLogicalAnd %bool %90 %94
%96 = OpCompositeExtract %v4float %80 2
%97 = OpCompositeExtract %v4float %85 2
%98 = OpFOrdEqual %v4bool %96 %97
%99 = OpAll %bool %98
%100 = OpLogicalAnd %bool %95 %99
%101 = OpCompositeExtract %v4float %80 3
%102 = OpCompositeExtract %v4float %85 3
%103 = OpFOrdEqual %v4bool %101 %102
%104 = OpAll %bool %103
%105 = OpLogicalAnd %bool %100 %104
OpBranch %79
%79 = OpLabel
%106 = OpPhi %bool %false %28 %105 %78
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %mat4v3float %f43
%110 = OpCompositeConstruct %v3float %float_12 %float_22 %float_30
%111 = OpCompositeConstruct %v3float %float_36 %float_40 %float_42
%112 = OpCompositeConstruct %v3float %float_42 %float_40 %float_36
%113 = OpCompositeConstruct %v3float %float_30 %float_22 %float_12
%114 = OpCompositeConstruct %mat4v3float %110 %111 %112 %113
%116 = OpCompositeExtract %v3float %109 0
%117 = OpCompositeExtract %v3float %114 0
%118 = OpFOrdEqual %v3bool %116 %117
%119 = OpAll %bool %118
%120 = OpCompositeExtract %v3float %109 1
%121 = OpCompositeExtract %v3float %114 1
%122 = OpFOrdEqual %v3bool %120 %121
%123 = OpAll %bool %122
%124 = OpLogicalAnd %bool %119 %123
%125 = OpCompositeExtract %v3float %109 2
%126 = OpCompositeExtract %v3float %114 2
%127 = OpFOrdEqual %v3bool %125 %126
%128 = OpAll %bool %127
%129 = OpLogicalAnd %bool %124 %128
%130 = OpCompositeExtract %v3float %109 3
%131 = OpCompositeExtract %v3float %114 3
%132 = OpFOrdEqual %v3bool %130 %131
%133 = OpAll %bool %132
%134 = OpLogicalAnd %bool %129 %133
OpBranch %108
%108 = OpLabel
%135 = OpPhi %bool %false %79 %134 %107
OpSelectionMerge %140 None
OpBranchConditional %135 %138 %139
%138 = OpLabel
%141 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%145 = OpLoad %v4float %141
OpStore %136 %145
OpBranch %140
%139 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%148 = OpLoad %v4float %146
OpStore %136 %148
OpBranch %140
%140 = OpLabel
%149 = OpLoad %v4float %136
OpReturnValue %149
OpFunctionEnd
