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
OpName %u1 "u1"
OpName %u2 "u2"
OpName %u3 "u3"
OpName %u4 "u4"
OpName %u5 "u5"
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
OpDecorate %u5 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
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
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_305441741 = OpConstant %uint 305441741
%uint_2147483646 = OpConstant %uint 2147483646
%uint_4294967294 = OpConstant %uint 4294967294
%uint_65534 = OpConstant %uint 65534
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
%u1 = OpVariable %_ptr_Function_uint Function
%u2 = OpVariable %_ptr_Function_uint Function
%u3 = OpVariable %_ptr_Function_uint Function
%u4 = OpVariable %_ptr_Function_uint Function
%u5 = OpVariable %_ptr_Function_uint Function
OpStore %u1 %uint_0
%32 = OpLoad %uint %u1
%34 = OpIAdd %uint %32 %uint_1
OpStore %u1 %34
OpStore %u2 %uint_305441741
%37 = OpLoad %uint %u2
%38 = OpIAdd %uint %37 %uint_1
OpStore %u2 %38
OpStore %u3 %uint_2147483646
%41 = OpLoad %uint %u3
%42 = OpIAdd %uint %41 %uint_1
OpStore %u3 %42
OpStore %u4 %uint_4294967294
%45 = OpLoad %uint %u4
%46 = OpIAdd %uint %45 %uint_1
OpStore %u4 %46
OpStore %u5 %uint_65534
%49 = OpLoad %uint %u5
%50 = OpIAdd %uint %49 %uint_1
OpStore %u5 %50
%51 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%55 = OpLoad %v4float %51
OpReturnValue %55
OpFunctionEnd
