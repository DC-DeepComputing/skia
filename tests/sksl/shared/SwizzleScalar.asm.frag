OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %h4 "h4"
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
OpDecorate %h4 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
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
%h4 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_float %12 %int_0
%34 = OpLoad %float %30
%35 = OpCompositeConstruct %v4float %34 %34 %34 %34
OpStore %h4 %35
%36 = OpAccessChain %_ptr_Uniform_float %12 %int_0
%37 = OpLoad %float %36
%38 = OpCompositeConstruct %v2float %37 %37
%39 = OpCompositeExtract %float %38 0
%40 = OpCompositeExtract %float %38 1
%42 = OpCompositeConstruct %v4float %39 %40 %float_0 %float_1
OpStore %h4 %42
%43 = OpAccessChain %_ptr_Uniform_float %12 %int_0
%44 = OpLoad %float %43
%45 = OpCompositeConstruct %v4float %float_0 %44 %float_1 %float_0
OpStore %h4 %45
%46 = OpAccessChain %_ptr_Uniform_float %12 %int_0
%47 = OpLoad %float %46
%48 = OpAccessChain %_ptr_Uniform_float %12 %int_0
%49 = OpLoad %float %48
%50 = OpCompositeConstruct %v4float %float_0 %47 %float_0 %49
OpStore %h4 %50
%51 = OpLoad %v4float %h4
OpReturnValue %51
OpFunctionEnd
