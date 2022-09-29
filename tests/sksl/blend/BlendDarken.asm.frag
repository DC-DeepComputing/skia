OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_src_over_h4h4h4 "blend_src_over_h4h4h4"
OpName %main "main"
OpName %_0_a "_0_a"
OpName %_1_b "_1_b"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %19 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %_0_a RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %_1_b RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v4float = OpTypePointer Function %v4float
%15 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%28 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%blend_src_over_h4h4h4 = OpFunction %v4float None %15
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%19 = OpLoad %v4float %16
%21 = OpLoad %v4float %16
%22 = OpCompositeExtract %float %21 3
%23 = OpFSub %float %float_1 %22
%24 = OpLoad %v4float %17
%25 = OpVectorTimesScalar %v4float %24 %23
%26 = OpFAdd %v4float %19 %25
OpReturnValue %26
OpFunctionEnd
%main = OpFunction %void None %28
%29 = OpLabel
%_0_a = OpVariable %_ptr_Function_v4float Function
%36 = OpVariable %_ptr_Function_v4float Function
%40 = OpVariable %_ptr_Function_v4float Function
%_1_b = OpVariable %_ptr_Function_v3float Function
%31 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%35 = OpLoad %v4float %31
OpStore %36 %35
%37 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%39 = OpLoad %v4float %37
OpStore %40 %39
%41 = OpFunctionCall %v4float %blend_src_over_h4h4h4 %36 %40
OpStore %_0_a %41
%45 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%46 = OpLoad %v4float %45
%47 = OpCompositeExtract %float %46 3
%48 = OpFSub %float %float_1 %47
%49 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v3float %50 %50 0 1 2
%52 = OpVectorTimesScalar %v3float %51 %48
%53 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%54 = OpLoad %v4float %53
%55 = OpVectorShuffle %v3float %54 %54 0 1 2
%56 = OpFAdd %v3float %52 %55
OpStore %_1_b %56
%58 = OpVectorShuffle %v3float %41 %41 0 1 2
%57 = OpExtInst %v3float %1 FMin %58 %56
%59 = OpLoad %v4float %_0_a
%60 = OpVectorShuffle %v4float %59 %57 4 5 6 3
OpStore %_0_a %60
OpStore %sk_FragColor %60
OpReturn
OpFunctionEnd
