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
OpDecorate %38 RelaxedPrecision
OpDecorate %_arr_v3int_int_3 ArrayStride 16
OpDecorate %52 RelaxedPrecision
OpDecorate %_arr_mat2v2float_int_2 ArrayStride 32
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
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
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v3int = OpTypeVector %int 3
%int_3 = OpConstant %int 3
%_arr_v3int_int_3 = OpTypeArray %v3int %int_3
%_ptr_Function__arr_v3int_int_3 = OpTypePointer Function %_arr_v3int_int_3
%int_1 = OpConstant %int 1
%45 = OpConstantComposite %v3int %int_1 %int_1 %int_1
%int_2 = OpConstant %int 2
%47 = OpConstantComposite %v3int %int_2 %int_2 %int_2
%48 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_2 = OpTypeArray %mat2v2float %int_2
%_ptr_Function__arr_mat2v2float_int_2 = OpTypePointer Function %_arr_mat2v2float_int_2
%57 = OpConstantComposite %v2float %float_1 %float_2
%58 = OpConstantComposite %v2float %float_3 %float_4
%59 = OpConstantComposite %mat2v2float %57 %58
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%64 = OpConstantComposite %v2float %float_5 %float_6
%65 = OpConstantComposite %v2float %float_7 %float_8
%66 = OpConstantComposite %mat2v2float %64 %65
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
%h = OpVariable %_ptr_Function__arr_float_int_4 Function
%i3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
%h2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
%f2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
%127 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f %35
%37 = OpLoad %_arr_float_int_4 %f
OpStore %h %37
%38 = OpLoad %_arr_float_int_4 %h
OpStore %f %38
OpStore %h %38
%49 = OpCompositeConstruct %_arr_v3int_int_3 %45 %47 %48
OpStore %i3 %49
%51 = OpLoad %_arr_v3int_int_3 %i3
OpStore %s3 %51
%52 = OpLoad %_arr_v3int_int_3 %s3
OpStore %i3 %52
OpStore %s3 %52
%67 = OpCompositeConstruct %_arr_mat2v2float_int_2 %59 %66
OpStore %h2x2 %67
%69 = OpLoad %_arr_mat2v2float_int_2 %h2x2
OpStore %f2x2 %69
%70 = OpLoad %_arr_mat2v2float_int_2 %h2x2
OpStore %f2x2 %70
OpStore %h2x2 %70
%72 = OpCompositeExtract %float %38 0
%73 = OpFOrdEqual %bool %72 %72
%74 = OpCompositeExtract %float %38 1
%75 = OpFOrdEqual %bool %74 %74
%76 = OpLogicalAnd %bool %75 %73
%77 = OpCompositeExtract %float %38 2
%78 = OpFOrdEqual %bool %77 %77
%79 = OpLogicalAnd %bool %78 %76
%80 = OpCompositeExtract %float %38 3
%81 = OpFOrdEqual %bool %80 %80
%82 = OpLogicalAnd %bool %81 %79
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpCompositeExtract %v3int %52 0
%86 = OpIEqual %v3bool %85 %85
%88 = OpAll %bool %86
%89 = OpCompositeExtract %v3int %52 1
%90 = OpIEqual %v3bool %89 %89
%91 = OpAll %bool %90
%92 = OpLogicalAnd %bool %91 %88
%93 = OpCompositeExtract %v3int %52 2
%94 = OpIEqual %v3bool %93 %93
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %95 %92
OpBranch %84
%84 = OpLabel
%97 = OpPhi %bool %false %25 %96 %83
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpLoad %_arr_mat2v2float_int_2 %f2x2
%101 = OpLoad %_arr_mat2v2float_int_2 %h2x2
%102 = OpCompositeExtract %mat2v2float %100 0
%103 = OpCompositeExtract %mat2v2float %101 0
%105 = OpCompositeExtract %v2float %102 0
%106 = OpCompositeExtract %v2float %103 0
%107 = OpFOrdEqual %v2bool %105 %106
%108 = OpAll %bool %107
%109 = OpCompositeExtract %v2float %102 1
%110 = OpCompositeExtract %v2float %103 1
%111 = OpFOrdEqual %v2bool %109 %110
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %108 %112
%114 = OpCompositeExtract %mat2v2float %100 1
%115 = OpCompositeExtract %mat2v2float %101 1
%116 = OpCompositeExtract %v2float %114 0
%117 = OpCompositeExtract %v2float %115 0
%118 = OpFOrdEqual %v2bool %116 %117
%119 = OpAll %bool %118
%120 = OpCompositeExtract %v2float %114 1
%121 = OpCompositeExtract %v2float %115 1
%122 = OpFOrdEqual %v2bool %120 %121
%123 = OpAll %bool %122
%124 = OpLogicalAnd %bool %119 %123
%125 = OpLogicalAnd %bool %124 %113
OpBranch %99
%99 = OpLabel
%126 = OpPhi %bool %false %84 %125 %98
OpSelectionMerge %131 None
OpBranchConditional %126 %129 %130
%129 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%135 = OpLoad %v4float %132
OpStore %127 %135
OpBranch %131
%130 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%137 = OpLoad %v4float %136
OpStore %127 %137
OpBranch %131
%131 = OpLabel
%138 = OpLoad %v4float %127
OpReturnValue %138
OpFunctionEnd
