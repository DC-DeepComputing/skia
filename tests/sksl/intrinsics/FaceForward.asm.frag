OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "N"
OpMemberName %_UniformBuffer 1 "I"
OpMemberName %_UniformBuffer 2 "NRef"
OpMemberName %_UniformBuffer 3 "colorGreen"
OpMemberName %_UniformBuffer 4 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedPos "expectedPos"
OpName %expectedNeg "expectedNeg"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %expectedPos RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %expectedNeg RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%34 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%40 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%119 = OpConstantComposite %v2float %float_n1 %float_n2
%127 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
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
%expectedPos = OpVariable %_ptr_Function_v4float Function
%expectedNeg = OpVariable %_ptr_Function_v4float Function
%139 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedPos %34
OpStore %expectedNeg %40
%43 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%47 = OpLoad %v4float %43
%48 = OpCompositeExtract %float %47 0
%49 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%51 = OpLoad %v4float %49
%52 = OpCompositeExtract %float %51 0
%53 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%55 = OpLoad %v4float %53
%56 = OpCompositeExtract %float %55 0
%42 = OpExtInst %float %1 FaceForward %48 %52 %56
%57 = OpLoad %v4float %expectedNeg
%58 = OpCompositeExtract %float %57 0
%59 = OpFOrdEqual %bool %42 %58
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%63 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%64 = OpLoad %v4float %63
%65 = OpVectorShuffle %v2float %64 %64 0 1
%66 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%67 = OpLoad %v4float %66
%68 = OpVectorShuffle %v2float %67 %67 0 1
%69 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v2float %70 %70 0 1
%62 = OpExtInst %v2float %1 FaceForward %65 %68 %71
%72 = OpLoad %v4float %expectedNeg
%73 = OpVectorShuffle %v2float %72 %72 0 1
%74 = OpFOrdEqual %v2bool %62 %73
%76 = OpAll %bool %74
OpBranch %61
%61 = OpLabel
%77 = OpPhi %bool %false %27 %76 %60
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%82 = OpLoad %v4float %81
%83 = OpVectorShuffle %v3float %82 %82 0 1 2
%85 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%86 = OpLoad %v4float %85
%87 = OpVectorShuffle %v3float %86 %86 0 1 2
%88 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%89 = OpLoad %v4float %88
%90 = OpVectorShuffle %v3float %89 %89 0 1 2
%80 = OpExtInst %v3float %1 FaceForward %83 %87 %90
%91 = OpLoad %v4float %expectedPos
%92 = OpVectorShuffle %v3float %91 %91 0 1 2
%93 = OpFOrdEqual %v3bool %80 %92
%95 = OpAll %bool %93
OpBranch %79
%79 = OpLabel
%96 = OpPhi %bool %false %61 %95 %78
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%101 = OpLoad %v4float %100
%102 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%103 = OpLoad %v4float %102
%104 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%105 = OpLoad %v4float %104
%99 = OpExtInst %v4float %1 FaceForward %101 %103 %105
%106 = OpLoad %v4float %expectedPos
%107 = OpFOrdEqual %v4bool %99 %106
%109 = OpAll %bool %107
OpBranch %98
%98 = OpLabel
%110 = OpPhi %bool %false %79 %109 %97
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpLoad %v4float %expectedNeg
%114 = OpCompositeExtract %float %113 0
%115 = OpFOrdEqual %bool %float_n1 %114
OpBranch %112
%112 = OpLabel
%116 = OpPhi %bool %false %98 %115 %111
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%120 = OpLoad %v4float %expectedNeg
%121 = OpVectorShuffle %v2float %120 %120 0 1
%122 = OpFOrdEqual %v2bool %119 %121
%123 = OpAll %bool %122
OpBranch %118
%118 = OpLabel
%124 = OpPhi %bool %false %112 %123 %117
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpLoad %v4float %expectedPos
%129 = OpVectorShuffle %v3float %128 %128 0 1 2
%130 = OpFOrdEqual %v3bool %127 %129
%131 = OpAll %bool %130
OpBranch %126
%126 = OpLabel
%132 = OpPhi %bool %false %118 %131 %125
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%135 = OpLoad %v4float %expectedPos
%136 = OpFOrdEqual %v4bool %34 %135
%137 = OpAll %bool %136
OpBranch %134
%134 = OpLabel
%138 = OpPhi %bool %false %126 %137 %133
OpSelectionMerge %142 None
OpBranchConditional %138 %140 %141
%140 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%145 = OpLoad %v4float %143
OpStore %139 %145
OpBranch %142
%141 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %12 %int_4
%148 = OpLoad %v4float %146
OpStore %139 %148
OpBranch %142
%142 = OpLabel
%149 = OpLoad %v4float %139
OpReturnValue %149
OpFunctionEnd
