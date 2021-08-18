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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_ok "_0_ok"
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
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %35 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
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
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%21 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%28 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %21
%22 = OpLabel
%25 = OpVariable %_ptr_Function_v2float Function
OpStore %25 %24
%27 = OpFunctionCall %v4float %main %25
OpStore %sk_FragColor %27
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %28
%29 = OpFunctionParameter %_ptr_Function_v2float
%30 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%137 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%35 = OpLoad %bool %_0_ok
OpSelectionMerge %37 None
OpBranchConditional %35 %36 %37
%36 = OpLabel
%38 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_2
%42 = OpLoad %mat2v2float %38
%47 = OpCompositeConstruct %v2float %float_1 %float_2
%48 = OpCompositeConstruct %v2float %float_3 %float_4
%49 = OpCompositeConstruct %mat2v2float %47 %48
%51 = OpCompositeExtract %v2float %42 0
%52 = OpCompositeExtract %v2float %49 0
%53 = OpFOrdEqual %v2bool %51 %52
%54 = OpAll %bool %53
%55 = OpCompositeExtract %v2float %42 1
%56 = OpCompositeExtract %v2float %49 1
%57 = OpFOrdEqual %v2bool %55 %56
%58 = OpAll %bool %57
%59 = OpLogicalAnd %bool %54 %58
OpBranch %37
%37 = OpLabel
%60 = OpPhi %bool %false %30 %59 %36
OpStore %_0_ok %60
%61 = OpLoad %bool %_0_ok
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_3
%67 = OpLoad %mat3v3float %64
%73 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%74 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%75 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%76 = OpCompositeConstruct %mat3v3float %73 %74 %75
%78 = OpCompositeExtract %v3float %67 0
%79 = OpCompositeExtract %v3float %76 0
%80 = OpFOrdEqual %v3bool %78 %79
%81 = OpAll %bool %80
%82 = OpCompositeExtract %v3float %67 1
%83 = OpCompositeExtract %v3float %76 1
%84 = OpFOrdEqual %v3bool %82 %83
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %81 %85
%87 = OpCompositeExtract %v3float %67 2
%88 = OpCompositeExtract %v3float %76 2
%89 = OpFOrdEqual %v3bool %87 %88
%90 = OpAll %bool %89
%91 = OpLogicalAnd %bool %86 %90
OpBranch %63
%63 = OpLabel
%92 = OpPhi %bool %false %37 %91 %62
OpStore %_0_ok %92
%93 = OpLoad %bool %_0_ok
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_2
%97 = OpLoad %mat2v2float %96
%100 = OpCompositeConstruct %v2float %float_100 %float_0
%101 = OpCompositeConstruct %v2float %float_0 %float_100
%99 = OpCompositeConstruct %mat2v2float %100 %101
%102 = OpCompositeExtract %v2float %97 0
%103 = OpCompositeExtract %v2float %99 0
%104 = OpFOrdNotEqual %v2bool %102 %103
%105 = OpAny %bool %104
%106 = OpCompositeExtract %v2float %97 1
%107 = OpCompositeExtract %v2float %99 1
%108 = OpFOrdNotEqual %v2bool %106 %107
%109 = OpAny %bool %108
%110 = OpLogicalOr %bool %105 %109
OpBranch %95
%95 = OpLabel
%111 = OpPhi %bool %false %63 %110 %94
OpStore %_0_ok %111
%112 = OpLoad %bool %_0_ok
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_3
%116 = OpLoad %mat3v3float %115
%117 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%118 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%119 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%120 = OpCompositeConstruct %mat3v3float %117 %118 %119
%121 = OpCompositeExtract %v3float %116 0
%122 = OpCompositeExtract %v3float %120 0
%123 = OpFOrdNotEqual %v3bool %121 %122
%124 = OpAny %bool %123
%125 = OpCompositeExtract %v3float %116 1
%126 = OpCompositeExtract %v3float %120 1
%127 = OpFOrdNotEqual %v3bool %125 %126
%128 = OpAny %bool %127
%129 = OpLogicalOr %bool %124 %128
%130 = OpCompositeExtract %v3float %116 2
%131 = OpCompositeExtract %v3float %120 2
%132 = OpFOrdNotEqual %v3bool %130 %131
%133 = OpAny %bool %132
%134 = OpLogicalOr %bool %129 %133
OpBranch %114
%114 = OpLabel
%135 = OpPhi %bool %false %95 %134 %113
OpStore %_0_ok %135
%136 = OpLoad %bool %_0_ok
OpSelectionMerge %141 None
OpBranchConditional %136 %139 %140
%139 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%145 = OpLoad %v4float %142
OpStore %137 %145
OpBranch %141
%140 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%148 = OpLoad %v4float %146
OpStore %137 %148
OpBranch %141
%141 = OpLabel
%149 = OpLoad %v4float %137
OpReturnValue %149
OpFunctionEnd
