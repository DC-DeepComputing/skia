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
OpName %value "value"
OpName %expectedWhole "expectedWhole"
OpName %expectedFraction "expectedFraction"
OpName %ok "ok"
OpName %whole "whole"
OpName %fraction "fraction"
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
OpDecorate %134 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2_5 = OpConstant %float 2.5
%float_n2_5 = OpConstant %float -2.5
%float_8 = OpConstant %float 8
%float_n0_125 = OpConstant %float -0.125
%32 = OpConstantComposite %v4float %float_2_5 %float_n2_5 %float_8 %float_n0_125
%float_2 = OpConstant %float 2
%float_n2 = OpConstant %float -2
%36 = OpConstantComposite %v4float %float_2 %float_n2 %float_8 %float_0
%float_0_5 = OpConstant %float 0.5
%float_n0_5 = OpConstant %float -0.5
%40 = OpConstantComposite %v4float %float_0_5 %float_n0_5 %float_0 %float_n0_125
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%false = OpConstantFalse %bool
%45 = OpConstantComposite %v4bool %false %false %false %false
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_bool = OpTypePointer Function %bool
%77 = OpConstantComposite %v2float %float_2 %float_n2
%v2bool = OpTypeVector %bool 2
%85 = OpConstantComposite %v2float %float_0_5 %float_n0_5
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%103 = OpConstantComposite %v3float %float_2 %float_n2 %float_8
%v3bool = OpTypeVector %bool 3
%111 = OpConstantComposite %v3float %float_0_5 %float_n0_5 %float_0
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%value = OpVariable %_ptr_Function_v4float Function
%expectedWhole = OpVariable %_ptr_Function_v4float Function
%expectedFraction = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_v4bool Function
%whole = OpVariable %_ptr_Function_v4float Function
%fraction = OpVariable %_ptr_Function_v4float Function
%53 = OpVariable %_ptr_Function_float Function
%70 = OpVariable %_ptr_Function_v2float Function
%95 = OpVariable %_ptr_Function_v3float Function
%119 = OpVariable %_ptr_Function_v4float Function
%135 = OpVariable %_ptr_Function_v4float Function
OpStore %value %32
OpStore %expectedWhole %36
OpStore %expectedFraction %40
OpStore %ok %45
%49 = OpAccessChain %_ptr_Function_float %whole %int_0
%48 = OpExtInst %float %1 Modf %float_2_5 %53
%54 = OpLoad %float %53
OpStore %49 %54
%55 = OpAccessChain %_ptr_Function_float %fraction %int_0
OpStore %55 %48
%56 = OpLoad %v4float %whole
%57 = OpCompositeExtract %float %56 0
%58 = OpFOrdEqual %bool %57 %float_2
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpLoad %v4float %fraction
%62 = OpCompositeExtract %float %61 0
%63 = OpFOrdEqual %bool %62 %float_0_5
OpBranch %60
%60 = OpLabel
%64 = OpPhi %bool %false %25 %63 %59
%65 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %65 %64
%68 = OpLoad %v4float %value
%69 = OpVectorShuffle %v2float %68 %68 0 1
%67 = OpExtInst %v2float %1 Modf %69 %70
%71 = OpLoad %v2float %70
%72 = OpLoad %v4float %whole
%73 = OpVectorShuffle %v4float %72 %71 4 5 2 3
OpStore %whole %73
%74 = OpLoad %v4float %fraction
%75 = OpVectorShuffle %v4float %74 %67 4 5 2 3
OpStore %fraction %75
%76 = OpVectorShuffle %v2float %73 %73 0 1
%78 = OpFOrdEqual %v2bool %76 %77
%80 = OpAll %bool %78
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpLoad %v4float %fraction
%84 = OpVectorShuffle %v2float %83 %83 0 1
%86 = OpFOrdEqual %v2bool %84 %85
%87 = OpAll %bool %86
OpBranch %82
%82 = OpLabel
%88 = OpPhi %bool %false %60 %87 %81
%89 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %89 %88
%92 = OpLoad %v4float %value
%93 = OpVectorShuffle %v3float %92 %92 0 1 2
%91 = OpExtInst %v3float %1 Modf %93 %95
%97 = OpLoad %v3float %95
%98 = OpLoad %v4float %whole
%99 = OpVectorShuffle %v4float %98 %97 4 5 6 3
OpStore %whole %99
%100 = OpLoad %v4float %fraction
%101 = OpVectorShuffle %v4float %100 %91 4 5 6 3
OpStore %fraction %101
%102 = OpVectorShuffle %v3float %99 %99 0 1 2
%104 = OpFOrdEqual %v3bool %102 %103
%106 = OpAll %bool %104
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %v4float %fraction
%110 = OpVectorShuffle %v3float %109 %109 0 1 2
%112 = OpFOrdEqual %v3bool %110 %111
%113 = OpAll %bool %112
OpBranch %108
%108 = OpLabel
%114 = OpPhi %bool %false %82 %113 %107
%115 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %115 %114
%118 = OpLoad %v4float %value
%117 = OpExtInst %v4float %1 Modf %118 %119
%120 = OpLoad %v4float %119
OpStore %whole %120
OpStore %fraction %117
%121 = OpLoad %v4float %expectedWhole
%122 = OpFOrdEqual %v4bool %120 %121
%123 = OpAll %bool %122
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpLoad %v4float %fraction
%127 = OpLoad %v4float %expectedFraction
%128 = OpFOrdEqual %v4bool %126 %127
%129 = OpAll %bool %128
OpBranch %125
%125 = OpLabel
%130 = OpPhi %bool %false %108 %129 %124
%131 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %131 %130
%134 = OpLoad %v4bool %ok
%133 = OpAll %bool %134
OpSelectionMerge %138 None
OpBranchConditional %133 %136 %137
%136 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%141 = OpLoad %v4float %139
OpStore %135 %141
OpBranch %138
%137 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%143 = OpLoad %v4float %142
OpStore %135 %143
OpBranch %138
%138 = OpLabel
%144 = OpLoad %v4float %135
OpReturnValue %144
OpFunctionEnd
