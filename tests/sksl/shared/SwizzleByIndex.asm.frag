OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorBlack"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_v "_0_v"
OpName %_1_i "_1_i"
OpName %_2_x "_2_x"
OpName %_3_y "_3_y"
OpName %_4_z "_4_z"
OpName %_5_w "_5_w"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %_0_v RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %_2_x RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %_3_y RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %_4_z RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %_5_w RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%float_n1_25 = OpConstant %float -1.25
%77 = OpConstantComposite %v4float %float_n1_25 %float_n1_25 %float_n1_25 %float_0
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%_0_v = OpVariable %_ptr_Function_v4float Function
%_1_i = OpVariable %_ptr_Function_v4int Function
%_2_x = OpVariable %_ptr_Function_float Function
%_3_y = OpVariable %_ptr_Function_float Function
%_4_z = OpVariable %_ptr_Function_float Function
%_5_w = OpVariable %_ptr_Function_float Function
%81 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
OpStore %_0_v %34
%38 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%40 = OpLoad %v4float %38
%41 = OpCompositeExtract %float %40 0
%42 = OpConvertFToS %int %41
%43 = OpCompositeExtract %float %40 1
%44 = OpConvertFToS %int %43
%45 = OpCompositeExtract %float %40 2
%46 = OpConvertFToS %int %45
%47 = OpCompositeExtract %float %40 3
%48 = OpConvertFToS %int %47
%49 = OpCompositeConstruct %v4int %42 %44 %46 %48
OpStore %_1_i %49
%52 = OpLoad %v4float %_0_v
%53 = OpLoad %v4int %_1_i
%54 = OpCompositeExtract %int %53 0
%55 = OpVectorExtractDynamic %float %52 %54
OpStore %_2_x %55
%57 = OpLoad %v4float %_0_v
%58 = OpLoad %v4int %_1_i
%59 = OpCompositeExtract %int %58 1
%60 = OpVectorExtractDynamic %float %57 %59
OpStore %_3_y %60
%62 = OpLoad %v4float %_0_v
%63 = OpLoad %v4int %_1_i
%64 = OpCompositeExtract %int %63 2
%65 = OpVectorExtractDynamic %float %62 %64
OpStore %_4_z %65
%67 = OpLoad %v4float %_0_v
%68 = OpLoad %v4int %_1_i
%69 = OpCompositeExtract %int %68 3
%70 = OpVectorExtractDynamic %float %67 %69
OpStore %_5_w %70
%71 = OpLoad %float %_2_x
%72 = OpLoad %float %_3_y
%73 = OpLoad %float %_4_z
%74 = OpLoad %float %_5_w
%75 = OpCompositeConstruct %v4float %71 %72 %73 %74
%78 = OpFOrdEqual %v4bool %75 %77
%80 = OpAll %bool %78
OpSelectionMerge %84 None
OpBranchConditional %80 %82 %83
%82 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%87 = OpLoad %v4float %85
OpStore %81 %87
OpBranch %84
%83 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%90 = OpLoad %v4float %88
OpStore %81 %90
OpBranch %84
%84 = OpLabel
%91 = OpLoad %v4float %81
OpReturnValue %91
OpFunctionEnd
