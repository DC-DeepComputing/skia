OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expectedPos RelaxedPrecision
OpDecorate %expectedNeg RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%32 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%38 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
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
%115 = OpConstantComposite %v2float %float_n1 %float_n2
%123 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
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
%expectedPos = OpVariable %_ptr_Function_v4float Function
%expectedNeg = OpVariable %_ptr_Function_v4float Function
%135 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedPos %32
OpStore %expectedNeg %38
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%45 = OpLoad %v4float %41
%46 = OpCompositeExtract %float %45 0
%47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%49 = OpLoad %v4float %47
%50 = OpCompositeExtract %float %49 0
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%53 = OpLoad %v4float %51
%54 = OpCompositeExtract %float %53 0
%40 = OpExtInst %float %1 FaceForward %46 %50 %54
%55 = OpFOrdEqual %bool %40 %float_n1
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%60 = OpLoad %v4float %59
%61 = OpVectorShuffle %v2float %60 %60 0 1
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v2float %63 %63 0 1
%65 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%66 = OpLoad %v4float %65
%67 = OpVectorShuffle %v2float %66 %66 0 1
%58 = OpExtInst %v2float %1 FaceForward %61 %64 %67
%68 = OpLoad %v4float %expectedNeg
%69 = OpVectorShuffle %v2float %68 %68 0 1
%70 = OpFOrdEqual %v2bool %58 %69
%72 = OpAll %bool %70
OpBranch %57
%57 = OpLabel
%73 = OpPhi %bool %false %25 %72 %56
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%78 = OpLoad %v4float %77
%79 = OpVectorShuffle %v3float %78 %78 0 1 2
%81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%82 = OpLoad %v4float %81
%83 = OpVectorShuffle %v3float %82 %82 0 1 2
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%85 = OpLoad %v4float %84
%86 = OpVectorShuffle %v3float %85 %85 0 1 2
%76 = OpExtInst %v3float %1 FaceForward %79 %83 %86
%87 = OpLoad %v4float %expectedPos
%88 = OpVectorShuffle %v3float %87 %87 0 1 2
%89 = OpFOrdEqual %v3bool %76 %88
%91 = OpAll %bool %89
OpBranch %75
%75 = OpLabel
%92 = OpPhi %bool %false %57 %91 %74
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%97 = OpLoad %v4float %96
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%99 = OpLoad %v4float %98
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%101 = OpLoad %v4float %100
%95 = OpExtInst %v4float %1 FaceForward %97 %99 %101
%102 = OpLoad %v4float %expectedPos
%103 = OpFOrdEqual %v4bool %95 %102
%105 = OpAll %bool %103
OpBranch %94
%94 = OpLabel
%106 = OpPhi %bool %false %75 %105 %93
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %v4float %expectedNeg
%110 = OpCompositeExtract %float %109 0
%111 = OpFOrdEqual %bool %float_n1 %110
OpBranch %108
%108 = OpLabel
%112 = OpPhi %bool %false %94 %111 %107
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpLoad %v4float %expectedNeg
%117 = OpVectorShuffle %v2float %116 %116 0 1
%118 = OpFOrdEqual %v2bool %115 %117
%119 = OpAll %bool %118
OpBranch %114
%114 = OpLabel
%120 = OpPhi %bool %false %108 %119 %113
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%124 = OpLoad %v4float %expectedPos
%125 = OpVectorShuffle %v3float %124 %124 0 1 2
%126 = OpFOrdEqual %v3bool %123 %125
%127 = OpAll %bool %126
OpBranch %122
%122 = OpLabel
%128 = OpPhi %bool %false %114 %127 %121
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%131 = OpLoad %v4float %expectedPos
%132 = OpFOrdEqual %v4bool %32 %131
%133 = OpAll %bool %132
OpBranch %130
%130 = OpLabel
%134 = OpPhi %bool %false %122 %133 %129
OpSelectionMerge %138 None
OpBranchConditional %134 %136 %137
%136 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%141 = OpLoad %v4float %139
OpStore %135 %141
OpBranch %138
%137 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%144 = OpLoad %v4float %142
OpStore %135 %144
OpBranch %138
%138 = OpLabel
%145 = OpLoad %v4float %135
OpReturnValue %145
OpFunctionEnd
