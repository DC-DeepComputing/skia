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
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_x "_0_x"
OpName %_1_x "_1_x"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %60 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%41 = OpConstantComposite %v2float %float_1 %float_2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%49 = OpConstantComposite %v2float %float_3 %float_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
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
%_0_x = OpVariable %_ptr_Function_float Function
%_1_x = OpVariable %_ptr_Function_v2float Function
OpStore %_0_x %float_1
%32 = OpLoad %float %_0_x
%31 = OpExtInst %float %1 Length %32
OpStore %_0_x %31
%34 = OpLoad %float %_0_x
%33 = OpExtInst %float %1 Distance %34 %float_2
OpStore %_0_x %33
%37 = OpLoad %float %_0_x
%36 = OpFMul %float %37 %float_2
OpStore %_0_x %36
%39 = OpLoad %float %_0_x
%38 = OpExtInst %float %1 Normalize %39
OpStore %_0_x %38
OpStore %_1_x %41
%43 = OpLoad %v2float %_1_x
%42 = OpExtInst %float %1 Length %43
%44 = OpCompositeConstruct %v2float %42 %42
OpStore %_1_x %44
%46 = OpLoad %v2float %_1_x
%45 = OpExtInst %float %1 Distance %46 %49
%50 = OpCompositeConstruct %v2float %45 %45
OpStore %_1_x %50
%52 = OpLoad %v2float %_1_x
%51 = OpDot %float %52 %49
%53 = OpCompositeConstruct %v2float %51 %51
OpStore %_1_x %53
%55 = OpLoad %v2float %_1_x
%54 = OpExtInst %v2float %1 Normalize %55
OpStore %_1_x %54
%56 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%60 = OpLoad %v4float %56
OpReturnValue %60
OpFunctionEnd
