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
OpName %test1 "test1"
OpName %test2 "test2"
OpName %test3 "test3"
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
OpDecorate %_arr_v2float_int_2 ArrayStride 16
OpDecorate %_arr_mat4v4float_int_1 ArrayStride 64
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
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
%int_2 = OpConstant %int 2
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2
%_ptr_Function__arr_v2float_int_2 = OpTypePointer Function %_arr_v2float_int_2
%42 = OpConstantComposite %v2float %float_1 %float_2
%43 = OpConstantComposite %v2float %float_3 %float_4
%mat4v4float = OpTypeMatrix %v4float 4
%int_1 = OpConstant %int 1
%_arr_mat4v4float_int_1 = OpTypeArray %mat4v4float %int_1
%_ptr_Function__arr_mat4v4float_int_1 = OpTypePointer Function %_arr_mat4v4float_int_1
%float_16 = OpConstant %float 16
%int_3 = OpConstant %int 3
%_ptr_Function_float = OpTypePointer Function %float
%int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_24 = OpConstant %float 24
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%test1 = OpVariable %_ptr_Function__arr_float_int_4 Function
%test2 = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%test3 = OpVariable %_ptr_Function__arr_mat4v4float_int_1 Function
%73 = OpVariable %_ptr_Function_v4float Function
%37 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %test1 %37
%44 = OpCompositeConstruct %_arr_v2float_int_2 %42 %43
OpStore %test2 %44
%52 = OpCompositeConstruct %v4float %float_16 %float_0 %float_0 %float_0
%53 = OpCompositeConstruct %v4float %float_0 %float_16 %float_0 %float_0
%54 = OpCompositeConstruct %v4float %float_0 %float_0 %float_16 %float_0
%55 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_16
%51 = OpCompositeConstruct %mat4v4float %52 %53 %54 %55
%56 = OpCompositeConstruct %_arr_mat4v4float_int_1 %51
OpStore %test3 %56
%58 = OpAccessChain %_ptr_Function_float %test1 %int_3
%60 = OpLoad %float %58
%61 = OpAccessChain %_ptr_Function_v2float %test2 %int_1
%62 = OpLoad %v2float %61
%63 = OpCompositeExtract %float %62 1
%64 = OpFAdd %float %60 %63
%66 = OpAccessChain %_ptr_Function_v4float %test3 %int_0 %int_3
%68 = OpLoad %v4float %66
%69 = OpCompositeExtract %float %68 3
%70 = OpFAdd %float %64 %69
%72 = OpFOrdEqual %bool %70 %float_24
OpSelectionMerge %76 None
OpBranchConditional %72 %74 %75
%74 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%79 = OpLoad %v4float %77
OpStore %73 %79
OpBranch %76
%75 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%81 = OpLoad %v4float %80
OpStore %73 %81
OpBranch %76
%76 = OpLabel
%82 = OpLoad %v4float %73
OpReturnValue %82
OpFunctionEnd
