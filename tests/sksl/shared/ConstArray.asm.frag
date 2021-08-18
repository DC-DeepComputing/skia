OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %test "test"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %test RelaxedPrecision
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %19 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Private__arr_float_int_4 = OpTypePointer Private %_arr_float_int_4
%test = OpVariable %_ptr_Private__arr_float_int_4 Private
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%22 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%29 = OpTypeFunction %v4float %_ptr_Function_v2float
%int_0 = OpConstant %int 0
%_ptr_Private_float = OpTypePointer Private %float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %22
%23 = OpLabel
%26 = OpVariable %_ptr_Function_v2float Function
OpStore %26 %25
%28 = OpFunctionCall %v4float %main %26
OpStore %sk_FragColor %28
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %29
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%19 = OpCompositeConstruct %_arr_float_int_4 %float_0 %float_1 %float_0 %float_1
OpStore %test %19
%33 = OpAccessChain %_ptr_Private_float %test %int_0
%35 = OpLoad %float %33
%37 = OpAccessChain %_ptr_Private_float %test %int_1
%38 = OpLoad %float %37
%40 = OpAccessChain %_ptr_Private_float %test %int_2
%41 = OpLoad %float %40
%43 = OpAccessChain %_ptr_Private_float %test %int_3
%44 = OpLoad %float %43
%45 = OpCompositeConstruct %v4float %35 %38 %41 %44
OpReturnValue %45
OpFunctionEnd
