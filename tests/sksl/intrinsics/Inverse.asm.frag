OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
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
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %inv2x2 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %inv3x3 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %inv4x4 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_n2 = OpConstant %float -2
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%float_n0_5 = OpConstant %float -0.5
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
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_0_5 = OpConstant %float 0.5
%float_n8 = OpConstant %float -8
%float_n1 = OpConstant %float -1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%inv2x2 = OpVariable %_ptr_Function_mat2v2float Function
%inv3x3 = OpVariable %_ptr_Function_mat3v3float Function
%inv4x4 = OpVariable %_ptr_Function_mat4v4float Function
%161 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %v2float %float_n2 %float_1
%36 = OpCompositeConstruct %v2float %float_1_5 %float_n0_5
%37 = OpCompositeConstruct %mat2v2float %35 %36
OpStore %inv2x2 %37
%50 = OpCompositeConstruct %v3float %float_n24 %float_18 %float_5
%51 = OpCompositeConstruct %v3float %float_20 %float_n15 %float_n4
%52 = OpCompositeConstruct %v3float %float_n5 %float_4 %float_1
%53 = OpCompositeConstruct %mat3v3float %50 %51 %52
OpStore %inv3x3 %53
%62 = OpCompositeConstruct %v4float %float_n2 %float_n0_5 %float_1 %float_0_5
%63 = OpCompositeConstruct %v4float %float_1 %float_0_5 %float_0 %float_n0_5
%64 = OpCompositeConstruct %v4float %float_n8 %float_n1 %float_2 %float_2
%65 = OpCompositeConstruct %v4float %float_3 %float_0_5 %float_n1 %float_n0_5
%66 = OpCompositeConstruct %mat4v4float %62 %63 %64 %65
OpStore %inv4x4 %66
%68 = OpCompositeConstruct %v2float %float_n2 %float_1
%69 = OpCompositeConstruct %v2float %float_1_5 %float_n0_5
%70 = OpCompositeConstruct %mat2v2float %68 %69
%71 = OpLoad %mat2v2float %inv2x2
%73 = OpCompositeExtract %v2float %70 0
%74 = OpCompositeExtract %v2float %71 0
%75 = OpFOrdEqual %v2bool %73 %74
%76 = OpAll %bool %75
%77 = OpCompositeExtract %v2float %70 1
%78 = OpCompositeExtract %v2float %71 1
%79 = OpFOrdEqual %v2bool %77 %78
%80 = OpAll %bool %79
%81 = OpLogicalAnd %bool %76 %80
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpCompositeConstruct %v3float %float_n24 %float_18 %float_5
%85 = OpCompositeConstruct %v3float %float_20 %float_n15 %float_n4
%86 = OpCompositeConstruct %v3float %float_n5 %float_4 %float_1
%87 = OpCompositeConstruct %mat3v3float %84 %85 %86
%88 = OpLoad %mat3v3float %inv3x3
%90 = OpCompositeExtract %v3float %87 0
%91 = OpCompositeExtract %v3float %88 0
%92 = OpFOrdEqual %v3bool %90 %91
%93 = OpAll %bool %92
%94 = OpCompositeExtract %v3float %87 1
%95 = OpCompositeExtract %v3float %88 1
%96 = OpFOrdEqual %v3bool %94 %95
%97 = OpAll %bool %96
%98 = OpLogicalAnd %bool %93 %97
%99 = OpCompositeExtract %v3float %87 2
%100 = OpCompositeExtract %v3float %88 2
%101 = OpFOrdEqual %v3bool %99 %100
%102 = OpAll %bool %101
%103 = OpLogicalAnd %bool %98 %102
OpBranch %83
%83 = OpLabel
%104 = OpPhi %bool %false %27 %103 %82
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpCompositeConstruct %v4float %float_n2 %float_n0_5 %float_1 %float_0_5
%108 = OpCompositeConstruct %v4float %float_1 %float_0_5 %float_0 %float_n0_5
%109 = OpCompositeConstruct %v4float %float_n8 %float_n1 %float_2 %float_2
%110 = OpCompositeConstruct %v4float %float_3 %float_0_5 %float_n1 %float_n0_5
%111 = OpCompositeConstruct %mat4v4float %107 %108 %109 %110
%112 = OpLoad %mat4v4float %inv4x4
%114 = OpCompositeExtract %v4float %111 0
%115 = OpCompositeExtract %v4float %112 0
%116 = OpFOrdEqual %v4bool %114 %115
%117 = OpAll %bool %116
%118 = OpCompositeExtract %v4float %111 1
%119 = OpCompositeExtract %v4float %112 1
%120 = OpFOrdEqual %v4bool %118 %119
%121 = OpAll %bool %120
%122 = OpLogicalAnd %bool %117 %121
%123 = OpCompositeExtract %v4float %111 2
%124 = OpCompositeExtract %v4float %112 2
%125 = OpFOrdEqual %v4bool %123 %124
%126 = OpAll %bool %125
%127 = OpLogicalAnd %bool %122 %126
%128 = OpCompositeExtract %v4float %111 3
%129 = OpCompositeExtract %v4float %112 3
%130 = OpFOrdEqual %v4bool %128 %129
%131 = OpAll %bool %130
%132 = OpLogicalAnd %bool %127 %131
OpBranch %106
%106 = OpLabel
%133 = OpPhi %bool %false %83 %132 %105
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%141 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%142 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%143 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%144 = OpCompositeConstruct %mat3v3float %141 %142 %143
%136 = OpExtInst %mat3v3float %1 MatrixInverse %144
%145 = OpLoad %mat3v3float %inv3x3
%146 = OpCompositeExtract %v3float %136 0
%147 = OpCompositeExtract %v3float %145 0
%148 = OpFOrdNotEqual %v3bool %146 %147
%149 = OpAny %bool %148
%150 = OpCompositeExtract %v3float %136 1
%151 = OpCompositeExtract %v3float %145 1
%152 = OpFOrdNotEqual %v3bool %150 %151
%153 = OpAny %bool %152
%154 = OpLogicalOr %bool %149 %153
%155 = OpCompositeExtract %v3float %136 2
%156 = OpCompositeExtract %v3float %145 2
%157 = OpFOrdNotEqual %v3bool %155 %156
%158 = OpAny %bool %157
%159 = OpLogicalOr %bool %154 %158
OpBranch %135
%135 = OpLabel
%160 = OpPhi %bool %false %106 %159 %134
OpSelectionMerge %165 None
OpBranchConditional %160 %163 %164
%163 = OpLabel
%166 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%170 = OpLoad %v4float %166
OpStore %161 %170
OpBranch %165
%164 = OpLabel
%171 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%173 = OpLoad %v4float %171
OpStore %161 %173
OpBranch %165
%165 = OpLabel
%174 = OpLoad %v4float %161
OpReturnValue %174
OpFunctionEnd
