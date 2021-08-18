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
OpName %f "f"
OpName %h "h"
OpName %i3 "i3"
OpName %s3 "s3"
OpName %h2x2 "h2x2"
OpName %f2x2 "f2x2"
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
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %h RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %_arr_v3int_int_3 ArrayStride 16
OpDecorate %57 RelaxedPrecision
OpDecorate %_arr_mat2v2float_int_2 ArrayStride 32
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
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
%49 = OpConstantComposite %v3int %int_1 %int_1 %int_1
%int_2 = OpConstant %int 2
%51 = OpConstantComposite %v3int %int_2 %int_2 %int_2
%52 = OpConstantComposite %v3int %int_3 %int_3 %int_3
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
%f = OpVariable %_ptr_Function__arr_float_int_4 Function
%h = OpVariable %_ptr_Function__arr_float_int_4_0 Function
%i3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_v3int_int_3_0 Function
%h2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
%f2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2_0 Function
%146 = OpVariable %_ptr_Function_v4float Function
%37 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f %37
%40 = OpLoad %_arr_float_int_4 %f
OpStore %h %40
%41 = OpLoad %_arr_float_int_4 %h
OpStore %f %41
%42 = OpLoad %_arr_float_int_4 %f
OpStore %h %42
%53 = OpCompositeConstruct %_arr_v3int_int_3 %49 %51 %52
OpStore %i3 %53
%56 = OpLoad %_arr_v3int_int_3 %i3
OpStore %s3 %56
%57 = OpLoad %_arr_v3int_int_3 %s3
OpStore %i3 %57
%58 = OpLoad %_arr_v3int_int_3 %i3
OpStore %s3 %58
%63 = OpCompositeConstruct %v2float %float_1 %float_2
%64 = OpCompositeConstruct %v2float %float_3 %float_4
%65 = OpCompositeConstruct %mat2v2float %63 %64
%70 = OpCompositeConstruct %v2float %float_5 %float_6
%71 = OpCompositeConstruct %v2float %float_7 %float_8
%72 = OpCompositeConstruct %mat2v2float %70 %71
%73 = OpCompositeConstruct %_arr_mat2v2float_int_2 %65 %72
OpStore %h2x2 %73
%76 = OpLoad %_arr_mat2v2float_int_2 %h2x2
OpStore %f2x2 %76
%77 = OpLoad %_arr_mat2v2float_int_2 %h2x2
OpStore %f2x2 %77
%78 = OpLoad %_arr_mat2v2float_int_2 %f2x2
OpStore %h2x2 %78
%80 = OpLoad %_arr_float_int_4 %f
%81 = OpLoad %_arr_float_int_4 %h
%82 = OpCompositeExtract %float %80 0
%83 = OpCompositeExtract %float %81 0
%84 = OpFOrdEqual %bool %82 %83
%85 = OpCompositeExtract %float %80 1
%86 = OpCompositeExtract %float %81 1
%87 = OpFOrdEqual %bool %85 %86
%88 = OpLogicalAnd %bool %87 %84
%89 = OpCompositeExtract %float %80 2
%90 = OpCompositeExtract %float %81 2
%91 = OpFOrdEqual %bool %89 %90
%92 = OpLogicalAnd %bool %91 %88
%93 = OpCompositeExtract %float %80 3
%94 = OpCompositeExtract %float %81 3
%95 = OpFOrdEqual %bool %93 %94
%96 = OpLogicalAnd %bool %95 %92
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %_arr_v3int_int_3 %i3
%100 = OpLoad %_arr_v3int_int_3 %s3
%101 = OpCompositeExtract %v3int %99 0
%102 = OpCompositeExtract %v3int %100 0
%103 = OpIEqual %v3bool %101 %102
%105 = OpAll %bool %103
%106 = OpCompositeExtract %v3int %99 1
%107 = OpCompositeExtract %v3int %100 1
%108 = OpIEqual %v3bool %106 %107
%109 = OpAll %bool %108
%110 = OpLogicalAnd %bool %109 %105
%111 = OpCompositeExtract %v3int %99 2
%112 = OpCompositeExtract %v3int %100 2
%113 = OpIEqual %v3bool %111 %112
%114 = OpAll %bool %113
%115 = OpLogicalAnd %bool %114 %110
OpBranch %98
%98 = OpLabel
%116 = OpPhi %bool %false %27 %115 %97
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%119 = OpLoad %_arr_mat2v2float_int_2 %f2x2
%120 = OpLoad %_arr_mat2v2float_int_2 %h2x2
%121 = OpCompositeExtract %mat2v2float %119 0
%122 = OpCompositeExtract %mat2v2float %120 0
%124 = OpCompositeExtract %v2float %121 0
%125 = OpCompositeExtract %v2float %122 0
%126 = OpFOrdEqual %v2bool %124 %125
%127 = OpAll %bool %126
%128 = OpCompositeExtract %v2float %121 1
%129 = OpCompositeExtract %v2float %122 1
%130 = OpFOrdEqual %v2bool %128 %129
%131 = OpAll %bool %130
%132 = OpLogicalAnd %bool %127 %131
%133 = OpCompositeExtract %mat2v2float %119 1
%134 = OpCompositeExtract %mat2v2float %120 1
%135 = OpCompositeExtract %v2float %133 0
%136 = OpCompositeExtract %v2float %134 0
%137 = OpFOrdEqual %v2bool %135 %136
%138 = OpAll %bool %137
%139 = OpCompositeExtract %v2float %133 1
%140 = OpCompositeExtract %v2float %134 1
%141 = OpFOrdEqual %v2bool %139 %140
%142 = OpAll %bool %141
%143 = OpLogicalAnd %bool %138 %142
%144 = OpLogicalAnd %bool %143 %132
OpBranch %118
%118 = OpLabel
%145 = OpPhi %bool %false %98 %144 %117
OpSelectionMerge %150 None
OpBranchConditional %145 %148 %149
%148 = OpLabel
%151 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%154 = OpLoad %v4float %151
OpStore %146 %154
OpBranch %150
%149 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%156 = OpLoad %v4float %155
OpStore %146 %156
OpBranch %150
%150 = OpLabel
%157 = OpLoad %v4float %146
OpReturnValue %157
OpFunctionEnd
