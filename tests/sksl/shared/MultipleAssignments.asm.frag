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
OpName %x "x"
OpName %y "y"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
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
%float_1 = OpConstant %float 1
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
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%a = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_float Function
OpStore %y %float_1
OpStore %x %float_1
OpStore %c %float_0
OpStore %b %float_0
OpStore %a %float_0
%32 = OpLoad %float %a
%33 = OpLoad %float %b
%34 = OpFMul %float %32 %33
%35 = OpLoad %float %x
%36 = OpLoad %float %c
%37 = OpLoad %float %y
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpReturnValue %38
OpFunctionEnd
