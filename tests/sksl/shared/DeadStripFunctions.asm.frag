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
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %unpremul_h4h4 "unpremul_h4h4"
OpName %live_fn_h4h4h4 "live_fn_h4h4h4"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%27 = OpTypeFunction %v4float %_ptr_Function_v4float
%v3float = OpTypeVector %float 3
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
%float_1 = OpConstant %float 1
%47 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%54 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%60 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%float_n5 = OpConstant %float -5
%63 = OpConstantComposite %v4float %float_n5 %float_n5 %float_n5 %float_n5
%66 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%false = OpConstantFalse %bool
%71 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%unpremul_h4h4 = OpFunction %v4float None %27
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpLabel
%31 = OpLoad %v4float %29
%32 = OpVectorShuffle %v3float %31 %31 0 1 2
%35 = OpLoad %v4float %29
%36 = OpCompositeExtract %float %35 3
%34 = OpExtInst %float %1 FMax %36 %float_9_99999975en05
%39 = OpFDiv %float %float_1 %34
%40 = OpVectorTimesScalar %v3float %32 %39
%41 = OpCompositeExtract %float %40 0
%42 = OpCompositeExtract %float %40 1
%43 = OpCompositeExtract %float %40 2
%44 = OpLoad %v4float %29
%45 = OpCompositeExtract %float %44 3
%46 = OpCompositeConstruct %v4float %41 %42 %43 %45
OpReturnValue %46
OpFunctionEnd
%live_fn_h4h4h4 = OpFunction %v4float None %47
%48 = OpFunctionParameter %_ptr_Function_v4float
%49 = OpFunctionParameter %_ptr_Function_v4float
%50 = OpLabel
%51 = OpLoad %v4float %48
%52 = OpLoad %v4float %49
%53 = OpFAdd %v4float %51 %52
OpReturnValue %53
OpFunctionEnd
%main = OpFunction %v4float None %54
%55 = OpFunctionParameter %_ptr_Function_v2float
%56 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%61 = OpVariable %_ptr_Function_v4float Function
%64 = OpVariable %_ptr_Function_v4float Function
%67 = OpVariable %_ptr_Function_v4float Function
%81 = OpVariable %_ptr_Function_v4float Function
OpStore %61 %60
OpStore %64 %63
%65 = OpFunctionCall %v4float %live_fn_h4h4h4 %61 %64
OpStore %a %65
OpStore %67 %66
%68 = OpFunctionCall %v4float %unpremul_h4h4 %67
OpStore %b %68
%70 = OpLoad %v4float %a
%72 = OpFOrdNotEqual %v4bool %70 %71
%74 = OpAny %bool %72
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %v4float %b
%78 = OpFOrdNotEqual %v4bool %77 %71
%79 = OpAny %bool %78
OpBranch %76
%76 = OpLabel
%80 = OpPhi %bool %false %56 %79 %75
OpSelectionMerge %84 None
OpBranchConditional %80 %82 %83
%82 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%89 = OpLoad %v4float %85
OpStore %81 %89
OpBranch %84
%83 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%92 = OpLoad %v4float %90
OpStore %81 %92
OpBranch %84
%84 = OpLabel
%93 = OpLoad %v4float %81
OpReturnValue %93
OpFunctionEnd
