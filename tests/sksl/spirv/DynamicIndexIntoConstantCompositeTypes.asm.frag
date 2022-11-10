OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "idx"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %array "array"
OpName %matrix "matrix"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %array RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %27 RelaxedPrecision
OpDecorate %matrix RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%_UniformBuffer = OpTypeStruct %int
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%19 = OpTypeFunction %v4float
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%34 = OpConstantComposite %v2float %float_1 %float_2
%35 = OpConstantComposite %v2float %float_3 %float_4
%36 = OpConstantComposite %mat2v2float %34 %35
%_ptr_Uniform_int = OpTypePointer Uniform %int
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %19
%20 = OpLabel
%array = OpVariable %_ptr_Function__arr_float_int_2 Function
%matrix = OpVariable %_ptr_Function_mat2v2float Function
%27 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %array %27
OpStore %matrix %36
%37 = OpAccessChain %_ptr_Uniform_int %10 %int_0
%40 = OpLoad %int %37
%41 = OpAccessChain %_ptr_Function_float %array %40
%43 = OpLoad %float %41
%44 = OpAccessChain %_ptr_Uniform_int %10 %int_0
%45 = OpLoad %int %44
%46 = OpVectorExtractDynamic %float %34 %45
%47 = OpAccessChain %_ptr_Uniform_int %10 %int_0
%48 = OpLoad %int %47
%49 = OpAccessChain %_ptr_Function_v2float %matrix %48
%51 = OpLoad %v2float %49
%52 = OpCompositeExtract %float %51 0
%53 = OpCompositeExtract %float %51 1
%54 = OpCompositeConstruct %v4float %43 %46 %52 %53
OpReturnValue %54
OpFunctionEnd
