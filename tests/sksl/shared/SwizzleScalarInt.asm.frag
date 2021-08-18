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
OpName %i "i"
OpName %i4 "i4"
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
OpDecorate %34 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
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
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%v2int = OpTypeVector %int 2
%int_1 = OpConstant %int 1
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
%i = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%31 = OpAccessChain %_ptr_Uniform_float %12 %int_0
%34 = OpLoad %float %31
%35 = OpConvertFToS %int %34
OpStore %i %35
%39 = OpLoad %int %i
%40 = OpCompositeConstruct %v4int %39 %39 %39 %39
OpStore %i4 %40
%41 = OpLoad %int %i
%42 = OpCompositeConstruct %v2int %41 %41
%44 = OpCompositeExtract %int %42 0
%45 = OpCompositeExtract %int %42 1
%47 = OpCompositeConstruct %v4int %44 %45 %int_0 %int_1
OpStore %i4 %47
%48 = OpLoad %int %i
%49 = OpCompositeConstruct %v4int %int_0 %48 %int_1 %int_0
OpStore %i4 %49
%50 = OpLoad %int %i
%51 = OpLoad %int %i
%52 = OpCompositeConstruct %v4int %int_0 %50 %int_0 %51
OpStore %i4 %52
%53 = OpLoad %v4int %i4
%54 = OpCompositeExtract %int %53 0
%55 = OpConvertSToF %float %54
%56 = OpCompositeExtract %int %53 1
%57 = OpConvertSToF %float %56
%58 = OpCompositeExtract %int %53 2
%59 = OpConvertSToF %float %58
%60 = OpCompositeExtract %int %53 3
%61 = OpConvertSToF %float %60
%62 = OpCompositeConstruct %v4float %55 %57 %59 %61
OpReturnValue %62
OpFunctionEnd
