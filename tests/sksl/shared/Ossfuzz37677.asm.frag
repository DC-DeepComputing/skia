OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %y "y"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_8 ArrayStride 16
OpDecorate %36 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%15 = OpTypeFunction %v4float
%int = OpTypeInt 32 1
%int_8 = OpConstant %int 8
%_arr_float_int_8 = OpTypeArray %float %int_8
%_ptr_Function__arr_float_int_8 = OpTypePointer Function %_arr_float_int_8
%v2int = OpTypeVector %int 2
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%25 = OpConstantComposite %v2int %int_1 %int_3
%_ptr_Function_float = OpTypePointer Function %float
%int_7 = OpConstant %int 7
%int_4 = OpConstant %int 4
%float_0 = OpConstant %float 0
%36 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%y = OpVariable %_ptr_Function__arr_float_int_8 Function
%26 = OpAccessChain %_ptr_Function_float %y %int_3
%28 = OpLoad %float %26
%30 = OpAccessChain %_ptr_Function_float %y %int_7
OpStore %30 %28
%32 = OpVectorExtractDynamic %int %25 %int_4
%33 = OpAccessChain %_ptr_Function_float %y %32
%34 = OpLoad %float %33
OpReturnValue %36
OpFunctionEnd
