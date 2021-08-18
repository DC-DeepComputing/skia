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
OpName %b "b"
OpName %b4 "b4"
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
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
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
%b = OpVariable %_ptr_Function_bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
%30 = OpAccessChain %_ptr_Uniform_float %12 %int_0
%34 = OpLoad %float %30
%35 = OpFUnordNotEqual %bool %34 %float_0
OpStore %b %35
%39 = OpLoad %bool %b
%40 = OpCompositeConstruct %v4bool %39 %39 %39 %39
OpStore %b4 %40
%41 = OpLoad %bool %b
%42 = OpCompositeConstruct %v2bool %41 %41
%44 = OpCompositeExtract %bool %42 0
%45 = OpCompositeExtract %bool %42 1
%48 = OpCompositeConstruct %v4bool %44 %45 %false %true
OpStore %b4 %48
%49 = OpLoad %bool %b
%50 = OpCompositeConstruct %v4bool %false %49 %true %false
OpStore %b4 %50
%51 = OpLoad %bool %b
%52 = OpLoad %bool %b
%53 = OpCompositeConstruct %v4bool %false %51 %false %52
OpStore %b4 %53
%54 = OpLoad %v4bool %b4
%55 = OpCompositeExtract %bool %54 0
%56 = OpSelect %float %55 %float_1 %float_0
%58 = OpCompositeExtract %bool %54 1
%59 = OpSelect %float %58 %float_1 %float_0
%60 = OpCompositeExtract %bool %54 2
%61 = OpSelect %float %60 %float_1 %float_0
%62 = OpCompositeExtract %bool %54 3
%63 = OpSelect %float %62 %float_1 %float_0
%64 = OpCompositeConstruct %v4float %56 %59 %61 %63
OpReturnValue %64
OpFunctionEnd
