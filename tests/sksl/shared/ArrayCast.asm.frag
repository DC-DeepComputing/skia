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
OpDecorate %76 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
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
%43 = OpConstantComposite %v3int %int_1 %int_1 %int_1
%int_2 = OpConstant %int 2
%45 = OpConstantComposite %v3int %int_2 %int_2 %int_2
%46 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_2 = OpTypeArray %mat2v2float %int_2
%_ptr_Function__arr_mat2v2float_int_2 = OpTypePointer Function %_arr_mat2v2float_int_2
%53 = OpConstantComposite %v2float %float_1 %float_2
%54 = OpConstantComposite %v2float %float_3 %float_4
%55 = OpConstantComposite %mat2v2float %53 %54
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%60 = OpConstantComposite %v2float %float_5 %float_6
%61 = OpConstantComposite %v2float %float_7 %float_8
%62 = OpConstantComposite %mat2v2float %60 %61
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
%122 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f %35
OpStore %h %35
OpStore %f %35
OpStore %h %35
%47 = OpCompositeConstruct %_arr_v3int_int_3 %43 %45 %46
OpStore %i3 %47
OpStore %s3 %47
OpStore %i3 %47
OpStore %s3 %47
%63 = OpCompositeConstruct %_arr_mat2v2float_int_2 %55 %62
OpStore %h2x2 %63
OpStore %f2x2 %63
OpStore %f2x2 %63
OpStore %h2x2 %63
%66 = OpFOrdEqual %bool %float_1 %float_1
%67 = OpFOrdEqual %bool %float_2 %float_2
%68 = OpLogicalAnd %bool %67 %66
%69 = OpFOrdEqual %bool %float_3 %float_3
%70 = OpLogicalAnd %bool %69 %68
%71 = OpFOrdEqual %bool %float_4 %float_4
%72 = OpLogicalAnd %bool %71 %70
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%75 = OpLoad %_arr_v3int_int_3 %i3
%76 = OpLoad %_arr_v3int_int_3 %s3
%77 = OpCompositeExtract %v3int %75 0
%78 = OpCompositeExtract %v3int %76 0
%79 = OpIEqual %v3bool %77 %78
%81 = OpAll %bool %79
%82 = OpCompositeExtract %v3int %75 1
%83 = OpCompositeExtract %v3int %76 1
%84 = OpIEqual %v3bool %82 %83
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %85 %81
%87 = OpCompositeExtract %v3int %75 2
%88 = OpCompositeExtract %v3int %76 2
%89 = OpIEqual %v3bool %87 %88
%90 = OpAll %bool %89
%91 = OpLogicalAnd %bool %90 %86
OpBranch %74
%74 = OpLabel
%92 = OpPhi %bool %false %25 %91 %73
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpLoad %_arr_mat2v2float_int_2 %f2x2
%96 = OpLoad %_arr_mat2v2float_int_2 %h2x2
%97 = OpCompositeExtract %mat2v2float %95 0
%98 = OpCompositeExtract %mat2v2float %96 0
%100 = OpCompositeExtract %v2float %97 0
%101 = OpCompositeExtract %v2float %98 0
%102 = OpFOrdEqual %v2bool %100 %101
%103 = OpAll %bool %102
%104 = OpCompositeExtract %v2float %97 1
%105 = OpCompositeExtract %v2float %98 1
%106 = OpFOrdEqual %v2bool %104 %105
%107 = OpAll %bool %106
%108 = OpLogicalAnd %bool %103 %107
%109 = OpCompositeExtract %mat2v2float %95 1
%110 = OpCompositeExtract %mat2v2float %96 1
%111 = OpCompositeExtract %v2float %109 0
%112 = OpCompositeExtract %v2float %110 0
%113 = OpFOrdEqual %v2bool %111 %112
%114 = OpAll %bool %113
%115 = OpCompositeExtract %v2float %109 1
%116 = OpCompositeExtract %v2float %110 1
%117 = OpFOrdEqual %v2bool %115 %116
%118 = OpAll %bool %117
%119 = OpLogicalAnd %bool %114 %118
%120 = OpLogicalAnd %bool %119 %108
OpBranch %94
%94 = OpLabel
%121 = OpPhi %bool %false %74 %120 %93
OpSelectionMerge %126 None
OpBranchConditional %121 %124 %125
%124 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%130 = OpLoad %v4float %127
OpStore %122 %130
OpBranch %126
%125 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%132 = OpLoad %v4float %131
OpStore %122 %132
OpBranch %126
%126 = OpLabel
%133 = OpLoad %v4float %122
OpReturnValue %133
OpFunctionEnd
