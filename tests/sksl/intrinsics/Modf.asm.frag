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
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
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
%76 = OpConstantComposite %v2float %float_2 %float_n2
%v2bool = OpTypeVector %bool 2
%83 = OpConstantComposite %v2float %float_0_5 %float_n0_5
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%100 = OpConstantComposite %v3float %float_2 %float_n2 %float_8
%v3bool = OpTypeVector %bool 3
%107 = OpConstantComposite %v3float %float_0_5 %float_n0_5 %float_0
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
%69 = OpVariable %_ptr_Function_v2float Function
%92 = OpVariable %_ptr_Function_v3float Function
%114 = OpVariable %_ptr_Function_v4float Function
%126 = OpVariable %_ptr_Function_v4float Function
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
%68 = OpVectorShuffle %v2float %32 %32 0 1
%67 = OpExtInst %v2float %1 Modf %68 %69
%70 = OpLoad %v2float %69
%71 = OpLoad %v4float %whole
%72 = OpVectorShuffle %v4float %71 %70 4 5 2 3
OpStore %whole %72
%73 = OpLoad %v4float %fraction
%74 = OpVectorShuffle %v4float %73 %67 4 5 2 3
OpStore %fraction %74
%75 = OpVectorShuffle %v2float %72 %72 0 1
%77 = OpFOrdEqual %v2bool %75 %76
%79 = OpAll %bool %77
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpVectorShuffle %v2float %74 %74 0 1
%84 = OpFOrdEqual %v2bool %82 %83
%85 = OpAll %bool %84
OpBranch %81
%81 = OpLabel
%86 = OpPhi %bool %false %60 %85 %80
%87 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %87 %86
%90 = OpVectorShuffle %v3float %32 %32 0 1 2
%89 = OpExtInst %v3float %1 Modf %90 %92
%94 = OpLoad %v3float %92
%95 = OpLoad %v4float %whole
%96 = OpVectorShuffle %v4float %95 %94 4 5 6 3
OpStore %whole %96
%97 = OpLoad %v4float %fraction
%98 = OpVectorShuffle %v4float %97 %89 4 5 6 3
OpStore %fraction %98
%99 = OpVectorShuffle %v3float %96 %96 0 1 2
%101 = OpFOrdEqual %v3bool %99 %100
%103 = OpAll %bool %101
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpVectorShuffle %v3float %98 %98 0 1 2
%108 = OpFOrdEqual %v3bool %106 %107
%109 = OpAll %bool %108
OpBranch %105
%105 = OpLabel
%110 = OpPhi %bool %false %81 %109 %104
%111 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %111 %110
%113 = OpExtInst %v4float %1 Modf %32 %114
%115 = OpLoad %v4float %114
OpStore %whole %115
OpStore %fraction %113
%116 = OpFOrdEqual %v4bool %115 %36
%117 = OpAll %bool %116
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpFOrdEqual %v4bool %113 %40
%121 = OpAll %bool %120
OpBranch %119
%119 = OpLabel
%122 = OpPhi %bool %false %105 %121 %118
%123 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %123 %122
%125 = OpAll %bool %45
OpSelectionMerge %129 None
OpBranchConditional %125 %127 %128
%127 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%132 = OpLoad %v4float %130
OpStore %126 %132
OpBranch %129
%128 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%134 = OpLoad %v4float %133
OpStore %126 %134
OpBranch %129
%129 = OpLabel
%135 = OpLoad %v4float %126
OpReturnValue %135
OpFunctionEnd
