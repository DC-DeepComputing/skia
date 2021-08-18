OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%14 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%22 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_1 = OpConstant %float 1
%float_5 = OpConstant %float 5
%float_4 = OpConstant %float 4
%_entrypoint_v = OpFunction %void None %14
%15 = OpLabel
%19 = OpVariable %_ptr_Function_v2float Function
OpStore %19 %18
%21 = OpFunctionCall %v4float %main %19
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %22
%23 = OpFunctionParameter %_ptr_Function_v2float
%24 = OpLabel
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_float Function
%d = OpVariable %_ptr_Function_float Function
OpStore %b %float_2
OpStore %c %float_3
%31 = OpLoad %float %c
OpStore %d %31
%32 = OpLoad %float %b
%34 = OpFAdd %float %32 %float_1
OpStore %b %34
%35 = OpLoad %float %d
%36 = OpFAdd %float %35 %float_1
OpStore %d %36
%37 = OpLoad %float %b
%38 = OpFOrdEqual %bool %37 %float_2
%39 = OpSelect %float %38 %float_1 %float_0
%40 = OpLoad %float %b
%41 = OpFOrdEqual %bool %40 %float_3
%42 = OpSelect %float %41 %float_1 %float_0
%43 = OpLoad %float %d
%45 = OpFOrdEqual %bool %43 %float_5
%46 = OpSelect %float %45 %float_1 %float_0
%47 = OpLoad %float %d
%49 = OpFOrdEqual %bool %47 %float_4
%50 = OpSelect %float %49 %float_1 %float_0
%51 = OpCompositeConstruct %v4float %39 %42 %46 %50
OpReturnValue %51
OpFunctionEnd
