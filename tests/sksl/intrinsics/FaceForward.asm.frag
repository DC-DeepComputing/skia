OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %huge "huge"
OpName %huge2 "huge2"
OpName %huge3 "huge3"
OpName %huge4 "huge4"
OpName %expectedPos "expectedPos"
OpName %expectedNeg "expectedNeg"
OpName %N "N"
OpName %I "I"
OpName %NRef "NRef"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %28 RelaxedPrecision
OpDecorate %expectedPos RelaxedPrecision
OpDecorate %expectedNeg RelaxedPrecision
OpDecorate %N RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %I RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %NRef RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
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
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_1_00000002e_30 = OpConstant %float 1.00000002e+30
%33 = OpConstantComposite %v2float %float_1 %float_1
%34 = OpConstantComposite %v2float %float_1_00000002e_30 %float_1_00000002e_30
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%39 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%40 = OpConstantComposite %v3float %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30
%_ptr_Function_v4float = OpTypePointer Function %v4float
%44 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%45 = OpConstantComposite %v4float %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%64 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%float_n100 = OpConstant %float -100
%71 = OpConstantComposite %v4float %float_1 %float_1 %float_n100 %float_1
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%81 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%true = OpConstantTrue %bool
%123 = OpConstantComposite %v2float %float_n1 %float_n2
%130 = OpConstantComposite %v3float %float_1 %float_2 %float_3
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
%huge = OpVariable %_ptr_Function_float Function
%huge2 = OpVariable %_ptr_Function_v2float Function
%huge3 = OpVariable %_ptr_Function_v3float Function
%huge4 = OpVariable %_ptr_Function_v4float Function
%expectedPos = OpVariable %_ptr_Function_v4float Function
%expectedNeg = OpVariable %_ptr_Function_v4float Function
%N = OpVariable %_ptr_Function_v4float Function
%I = OpVariable %_ptr_Function_v4float Function
%NRef = OpVariable %_ptr_Function_v4float Function
%138 = OpVariable %_ptr_Function_v4float Function
%28 = OpExtInst %float %1 FaceForward %float_1 %float_1_00000002e_30 %float_1_00000002e_30
OpStore %huge %28
%32 = OpExtInst %v2float %1 FaceForward %33 %34 %34
OpStore %huge2 %32
%38 = OpExtInst %v3float %1 FaceForward %39 %40 %40
OpStore %huge3 %38
%43 = OpExtInst %v4float %1 FaceForward %44 %45 %45
OpStore %huge4 %43
%47 = OpCompositeConstruct %v4float %28 %28 %28 %28
%48 = OpVectorShuffle %v4float %32 %32 0 0 0 0
%49 = OpFAdd %v4float %47 %48
OpStore %expectedPos %49
%51 = OpVectorShuffle %v4float %38 %38 0 0 0 0
%52 = OpVectorShuffle %v4float %43 %43 0 0 0 0
%53 = OpFAdd %v4float %51 %52
OpStore %expectedNeg %53
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%59 = OpLoad %v4float %55
%60 = OpVectorShuffle %v4float %59 %59 1 1 1 1
%65 = OpFMul %v4float %60 %64
OpStore %N %65
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v4float %68 %68 1 1 1 1
%72 = OpFMul %v4float %69 %71
OpStore %I %72
%74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%75 = OpLoad %v4float %74
%76 = OpVectorShuffle %v4float %75 %75 1 1 1 1
OpStore %NRef %76
OpStore %expectedPos %64
OpStore %expectedNeg %81
%84 = OpCompositeExtract %float %65 0
%85 = OpCompositeExtract %float %72 0
%86 = OpCompositeExtract %float %76 0
%83 = OpExtInst %float %1 FaceForward %84 %85 %86
%87 = OpFOrdEqual %bool %83 %float_n1
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%91 = OpVectorShuffle %v2float %65 %65 0 1
%92 = OpVectorShuffle %v2float %72 %72 0 1
%93 = OpVectorShuffle %v2float %76 %76 0 1
%90 = OpExtInst %v2float %1 FaceForward %91 %92 %93
%94 = OpVectorShuffle %v2float %81 %81 0 1
%95 = OpFOrdEqual %v2bool %90 %94
%97 = OpAll %bool %95
OpBranch %89
%89 = OpLabel
%98 = OpPhi %bool %false %25 %97 %88
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%102 = OpVectorShuffle %v3float %65 %65 0 1 2
%103 = OpVectorShuffle %v3float %72 %72 0 1 2
%104 = OpVectorShuffle %v3float %76 %76 0 1 2
%101 = OpExtInst %v3float %1 FaceForward %102 %103 %104
%105 = OpVectorShuffle %v3float %64 %64 0 1 2
%106 = OpFOrdEqual %v3bool %101 %105
%108 = OpAll %bool %106
OpBranch %100
%100 = OpLabel
%109 = OpPhi %bool %false %89 %108 %99
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%112 = OpExtInst %v4float %1 FaceForward %65 %72 %76
%113 = OpFOrdEqual %v4bool %112 %64
%115 = OpAll %bool %113
OpBranch %111
%111 = OpLabel
%116 = OpPhi %bool %false %100 %115 %110
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
OpBranch %118
%118 = OpLabel
%120 = OpPhi %bool %false %111 %true %117
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%124 = OpVectorShuffle %v2float %81 %81 0 1
%125 = OpFOrdEqual %v2bool %123 %124
%126 = OpAll %bool %125
OpBranch %122
%122 = OpLabel
%127 = OpPhi %bool %false %118 %126 %121
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%131 = OpVectorShuffle %v3float %64 %64 0 1 2
%132 = OpFOrdEqual %v3bool %130 %131
%133 = OpAll %bool %132
OpBranch %129
%129 = OpLabel
%134 = OpPhi %bool %false %122 %133 %128
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
OpBranch %136
%136 = OpLabel
%137 = OpPhi %bool %false %129 %true %135
OpSelectionMerge %141 None
OpBranchConditional %137 %139 %140
%139 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%143 = OpLoad %v4float %142
OpStore %138 %143
OpBranch %141
%140 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%146 = OpLoad %v4float %144
OpStore %138 %146
OpBranch %141
%141 = OpLabel
%147 = OpLoad %v4float %138
OpReturnValue %147
OpFunctionEnd
