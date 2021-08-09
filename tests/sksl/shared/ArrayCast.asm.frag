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
OpName %f "f"
OpName %h "h"
OpName %i3 "i3"
OpName %s3 "s3"
OpName %h2x2 "h2x2"
OpName %f2x2 "f2x2"
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
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %h RelaxedPrecision
OpDecorate %_arr_v3int_int_3 ArrayStride 16
OpDecorate %_arr_mat2v2float_int_2 ArrayStride 32
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
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
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%_ptr_Function__arr_float_int_4_0 = OpTypePointer Function %_arr_float_int_4
%v3int = OpTypeVector %int 3
%int_3 = OpConstant %int 3
%_arr_v3int_int_3 = OpTypeArray %v3int %int_3
%_ptr_Function__arr_v3int_int_3 = OpTypePointer Function %_arr_v3int_int_3
%int_1 = OpConstant %int 1
%45 = OpConstantComposite %v3int %int_1 %int_1 %int_1
%int_2 = OpConstant %int 2
%47 = OpConstantComposite %v3int %int_2 %int_2 %int_2
%48 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%_ptr_Function__arr_v3int_int_3_0 = OpTypePointer Function %_arr_v3int_int_3
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_2 = OpTypeArray %mat2v2float %int_2
%_ptr_Function__arr_mat2v2float_int_2 = OpTypePointer Function %_arr_mat2v2float_int_2
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%_ptr_Function__arr_mat2v2float_int_2_0 = OpTypePointer Function %_arr_mat2v2float_int_2
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%f = OpVariable %_ptr_Function__arr_float_int_4 Function
%h = OpVariable %_ptr_Function__arr_float_int_4_0 Function
%i3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_v3int_int_3_0 Function
%h2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
%f2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2_0 Function
%138 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f %35
%38 = OpLoad %_arr_float_int_4 %f
OpStore %h %38
%49 = OpCompositeConstruct %_arr_v3int_int_3 %45 %47 %48
OpStore %i3 %49
%52 = OpLoad %_arr_v3int_int_3 %i3
OpStore %s3 %52
%57 = OpCompositeConstruct %v2float %float_1 %float_2
%58 = OpCompositeConstruct %v2float %float_3 %float_4
%59 = OpCompositeConstruct %mat2v2float %57 %58
%64 = OpCompositeConstruct %v2float %float_5 %float_6
%65 = OpCompositeConstruct %v2float %float_7 %float_8
%66 = OpCompositeConstruct %mat2v2float %64 %65
%67 = OpCompositeConstruct %_arr_mat2v2float_int_2 %59 %66
OpStore %h2x2 %67
%70 = OpLoad %_arr_mat2v2float_int_2 %h2x2
OpStore %f2x2 %70
%72 = OpLoad %_arr_float_int_4 %f
%73 = OpLoad %_arr_float_int_4 %h
%74 = OpCompositeExtract %float %72 0
%75 = OpCompositeExtract %float %73 0
%76 = OpFOrdEqual %bool %74 %75
%77 = OpCompositeExtract %float %72 1
%78 = OpCompositeExtract %float %73 1
%79 = OpFOrdEqual %bool %77 %78
%80 = OpLogicalAnd %bool %79 %76
%81 = OpCompositeExtract %float %72 2
%82 = OpCompositeExtract %float %73 2
%83 = OpFOrdEqual %bool %81 %82
%84 = OpLogicalAnd %bool %83 %80
%85 = OpCompositeExtract %float %72 3
%86 = OpCompositeExtract %float %73 3
%87 = OpFOrdEqual %bool %85 %86
%88 = OpLogicalAnd %bool %87 %84
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpLoad %_arr_v3int_int_3 %i3
%92 = OpLoad %_arr_v3int_int_3 %s3
%93 = OpCompositeExtract %v3int %91 0
%94 = OpCompositeExtract %v3int %92 0
%95 = OpIEqual %v3bool %93 %94
%97 = OpAll %bool %95
%98 = OpCompositeExtract %v3int %91 1
%99 = OpCompositeExtract %v3int %92 1
%100 = OpIEqual %v3bool %98 %99
%101 = OpAll %bool %100
%102 = OpLogicalAnd %bool %101 %97
%103 = OpCompositeExtract %v3int %91 2
%104 = OpCompositeExtract %v3int %92 2
%105 = OpIEqual %v3bool %103 %104
%106 = OpAll %bool %105
%107 = OpLogicalAnd %bool %106 %102
OpBranch %90
%90 = OpLabel
%108 = OpPhi %bool %false %25 %107 %89
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpLoad %_arr_mat2v2float_int_2 %f2x2
%112 = OpLoad %_arr_mat2v2float_int_2 %h2x2
%113 = OpCompositeExtract %mat2v2float %111 0
%114 = OpCompositeExtract %mat2v2float %112 0
%116 = OpCompositeExtract %v2float %113 0
%117 = OpCompositeExtract %v2float %114 0
%118 = OpFOrdEqual %v2bool %116 %117
%119 = OpAll %bool %118
%120 = OpCompositeExtract %v2float %113 1
%121 = OpCompositeExtract %v2float %114 1
%122 = OpFOrdEqual %v2bool %120 %121
%123 = OpAll %bool %122
%124 = OpLogicalAnd %bool %119 %123
%125 = OpCompositeExtract %mat2v2float %111 1
%126 = OpCompositeExtract %mat2v2float %112 1
%127 = OpCompositeExtract %v2float %125 0
%128 = OpCompositeExtract %v2float %126 0
%129 = OpFOrdEqual %v2bool %127 %128
%130 = OpAll %bool %129
%131 = OpCompositeExtract %v2float %125 1
%132 = OpCompositeExtract %v2float %126 1
%133 = OpFOrdEqual %v2bool %131 %132
%134 = OpAll %bool %133
%135 = OpLogicalAnd %bool %130 %134
%136 = OpLogicalAnd %bool %135 %124
OpBranch %110
%110 = OpLabel
%137 = OpPhi %bool %false %90 %136 %109
OpSelectionMerge %142 None
OpBranchConditional %137 %140 %141
%140 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%146 = OpLoad %v4float %143
OpStore %138 %146
OpBranch %142
%141 = OpLabel
%147 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%148 = OpLoad %v4float %147
OpStore %138 %148
OpBranch %142
%142 = OpLabel
%149 = OpLoad %v4float %138
OpReturnValue %149
OpFunctionEnd
