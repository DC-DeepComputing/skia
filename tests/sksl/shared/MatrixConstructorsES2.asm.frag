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
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f4 "f4"
OpName %ok "ok"
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
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
%v3float = OpTypeVector %float 3
%float_4 = OpConstant %float 4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%53 = OpConstantComposite %v2float %float_1 %float_2
%54 = OpConstantComposite %v2float %float_3 %float_4
%55 = OpConstantComposite %mat2v2float %53 %54
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%mat3v3float = OpTypeMatrix %v3float 3
%85 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%86 = OpConstantComposite %v3float %float_4 %float_1 %float_2
%87 = OpConstantComposite %v3float %float_3 %float_4 %float_1
%88 = OpConstantComposite %mat3v3float %85 %86 %87
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%127 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%128 = OpConstantComposite %mat4v4float %127 %127 %127 %127
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%f4 = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_bool Function
%142 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %f4 %38
%41 = OpVectorShuffle %v3float %38 %38 0 1 2
%44 = OpCompositeExtract %float %41 0
%45 = OpCompositeExtract %float %41 1
%46 = OpCompositeConstruct %v2float %44 %45
%47 = OpCompositeExtract %float %41 2
%48 = OpCompositeConstruct %v2float %47 %float_4
%49 = OpCompositeConstruct %mat2v2float %46 %48
%57 = OpFOrdEqual %v2bool %46 %53
%58 = OpAll %bool %57
%59 = OpFOrdEqual %v2bool %48 %54
%60 = OpAll %bool %59
%61 = OpLogicalAnd %bool %58 %60
OpStore %ok %61
OpSelectionMerge %64 None
OpBranchConditional %61 %63 %64
%63 = OpLabel
%65 = OpLoad %v4float %f4
%66 = OpVectorShuffle %v2float %65 %65 0 1
%67 = OpLoad %v4float %f4
%68 = OpVectorShuffle %v2float %67 %67 2 3
%69 = OpLoad %v4float %f4
%70 = OpLoad %v4float %f4
%71 = OpCompositeExtract %float %70 0
%72 = OpCompositeExtract %float %66 0
%73 = OpCompositeExtract %float %66 1
%74 = OpCompositeExtract %float %68 0
%75 = OpCompositeConstruct %v3float %72 %73 %74
%76 = OpCompositeExtract %float %68 1
%77 = OpCompositeExtract %float %69 0
%78 = OpCompositeExtract %float %69 1
%79 = OpCompositeConstruct %v3float %76 %77 %78
%80 = OpCompositeExtract %float %69 2
%81 = OpCompositeExtract %float %69 3
%82 = OpCompositeConstruct %v3float %80 %81 %71
%84 = OpCompositeConstruct %mat3v3float %75 %79 %82
%90 = OpFOrdEqual %v3bool %75 %85
%91 = OpAll %bool %90
%92 = OpFOrdEqual %v3bool %79 %86
%93 = OpAll %bool %92
%94 = OpLogicalAnd %bool %91 %93
%95 = OpFOrdEqual %v3bool %82 %87
%96 = OpAll %bool %95
%97 = OpLogicalAnd %bool %94 %96
OpBranch %64
%64 = OpLabel
%98 = OpPhi %bool %false %26 %97 %63
OpStore %ok %98
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpLoad %v4float %f4
%102 = OpVectorShuffle %v3float %101 %101 0 1 2
%103 = OpLoad %v4float %f4
%104 = OpVectorShuffle %v3float %103 %103 3 0 1
%105 = OpLoad %v4float %f4
%106 = OpVectorShuffle %v4float %105 %105 2 3 0 1
%107 = OpLoad %v4float %f4
%108 = OpVectorShuffle %v2float %107 %107 2 3
%109 = OpLoad %v4float %f4
%110 = OpCompositeExtract %float %102 0
%111 = OpCompositeExtract %float %102 1
%112 = OpCompositeExtract %float %102 2
%113 = OpCompositeExtract %float %104 0
%114 = OpCompositeConstruct %v4float %110 %111 %112 %113
%115 = OpCompositeExtract %float %104 1
%116 = OpCompositeExtract %float %104 2
%117 = OpCompositeExtract %float %106 0
%118 = OpCompositeExtract %float %106 1
%119 = OpCompositeConstruct %v4float %115 %116 %117 %118
%120 = OpCompositeExtract %float %106 2
%121 = OpCompositeExtract %float %106 3
%122 = OpCompositeExtract %float %108 0
%123 = OpCompositeExtract %float %108 1
%124 = OpCompositeConstruct %v4float %120 %121 %122 %123
%126 = OpCompositeConstruct %mat4v4float %114 %119 %124 %109
%130 = OpFOrdEqual %v4bool %114 %127
%131 = OpAll %bool %130
%132 = OpFOrdEqual %v4bool %119 %127
%133 = OpAll %bool %132
%134 = OpLogicalAnd %bool %131 %133
%135 = OpFOrdEqual %v4bool %124 %127
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %134 %136
%138 = OpFOrdEqual %v4bool %109 %127
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %137 %139
OpBranch %100
%100 = OpLabel
%141 = OpPhi %bool %false %64 %140 %99
OpStore %ok %141
OpSelectionMerge %145 None
OpBranchConditional %141 %143 %144
%143 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%149 = OpLoad %v4float %146
OpStore %142 %149
OpBranch %145
%144 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%152 = OpLoad %v4float %150
OpStore %142 %152
OpBranch %145
%145 = OpLabel
%153 = OpLoad %v4float %142
OpReturnValue %153
OpFunctionEnd
