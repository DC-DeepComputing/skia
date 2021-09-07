### Compilation failed:

error: SPIR-V validation error: ID 4294967295[%4294967295] has not been defined
  %67 = OpExtInst %v2float %1 Modf %69 %4294967295

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
OpDecorate %126 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
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
%74 = OpConstantComposite %v2float %float_2 %float_n2
%v2bool = OpTypeVector %bool 2
%82 = OpConstantComposite %v2float %float_0_5 %float_n0_5
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%96 = OpConstantComposite %v3float %float_2 %float_n2 %float_8
%v3bool = OpTypeVector %bool 3
%104 = OpConstantComposite %v3float %float_0_5 %float_n0_5 %float_0
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
%127 = OpVariable %_ptr_Function_v4float Function
OpStore %value %32
OpStore %expectedWhole %36
OpStore %expectedFraction %40
OpStore %ok %45
%49 = OpLoad %v4float %value
%50 = OpCompositeExtract %float %49 0
%51 = OpAccessChain %_ptr_Function_float %whole %int_0
%48 = OpExtInst %float %1 Modf %50 %51
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
%67 = OpExtInst %v2float %1 Modf %69 %4294967295
%70 = OpLoad %v4float %fraction
%71 = OpVectorShuffle %v4float %70 %67 4 5 2 3
OpStore %fraction %71
%72 = OpLoad %v4float %whole
%73 = OpVectorShuffle %v2float %72 %72 0 1
%75 = OpFOrdEqual %v2bool %73 %74
%77 = OpAll %bool %75
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpLoad %v4float %fraction
%81 = OpVectorShuffle %v2float %80 %80 0 1
%83 = OpFOrdEqual %v2bool %81 %82
%84 = OpAll %bool %83
OpBranch %79
%79 = OpLabel
%85 = OpPhi %bool %false %60 %84 %78
%86 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %86 %85
%89 = OpLoad %v4float %value
%90 = OpVectorShuffle %v3float %89 %89 0 1 2
%88 = OpExtInst %v3float %1 Modf %90 %4294967295
%92 = OpLoad %v4float %fraction
%93 = OpVectorShuffle %v4float %92 %88 4 5 6 3
OpStore %fraction %93
%94 = OpLoad %v4float %whole
%95 = OpVectorShuffle %v3float %94 %94 0 1 2
%97 = OpFOrdEqual %v3bool %95 %96
%99 = OpAll %bool %97
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpLoad %v4float %fraction
%103 = OpVectorShuffle %v3float %102 %102 0 1 2
%105 = OpFOrdEqual %v3bool %103 %104
%106 = OpAll %bool %105
OpBranch %101
%101 = OpLabel
%107 = OpPhi %bool %false %79 %106 %100
%108 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %108 %107
%111 = OpLoad %v4float %value
%110 = OpExtInst %v4float %1 Modf %111 %whole
OpStore %fraction %110
%112 = OpLoad %v4float %whole
%113 = OpLoad %v4float %expectedWhole
%114 = OpFOrdEqual %v4bool %112 %113
%115 = OpAll %bool %114
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%118 = OpLoad %v4float %fraction
%119 = OpLoad %v4float %expectedFraction
%120 = OpFOrdEqual %v4bool %118 %119
%121 = OpAll %bool %120
OpBranch %117
%117 = OpLabel
%122 = OpPhi %bool %false %101 %121 %116
%123 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %123 %122
%126 = OpLoad %v4bool %ok
%125 = OpAll %bool %126
OpSelectionMerge %130 None
OpBranchConditional %125 %128 %129
%128 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%133 = OpLoad %v4float %131
OpStore %127 %133
OpBranch %130
%129 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%135 = OpLoad %v4float %134
OpStore %127 %135
OpBranch %130
%130 = OpLabel
%136 = OpLoad %v4float %127
OpReturnValue %136
OpFunctionEnd

1 error
