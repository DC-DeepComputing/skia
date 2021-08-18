OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %x "x"
OpName %r "r"
OpName %b "b"
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
OpDecorate %x RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%float_n5 = OpConstant %float -5
%float_5 = OpConstant %float 5
%float_1 = OpConstant %float 1
%int_2 = OpConstant %int 2
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
%x = OpVariable %_ptr_Function_v4float Function
%r = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
OpStore %x %34
OpStore %r %float_n5
OpBranch %38
%38 = OpLabel
OpLoopMerge %42 %41 None
OpBranch %39
%39 = OpLabel
%43 = OpLoad %float %r
%45 = OpFOrdLessThan %bool %43 %float_5
OpBranchConditional %45 %40 %42
%40 = OpLabel
%47 = OpLoad %float %r
%46 = OpExtInst %float %1 FClamp %47 %float_0 %float_1
%49 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %49 %46
%50 = OpLoad %v4float %x
%51 = OpCompositeExtract %float %50 0
%52 = OpFOrdEqual %bool %51 %float_0
OpSelectionMerge %54 None
OpBranchConditional %52 %53 %54
%53 = OpLabel
OpBranch %42
%54 = OpLabel
OpBranch %41
%41 = OpLabel
%55 = OpLoad %float %r
%56 = OpFAdd %float %55 %float_1
OpStore %r %56
OpBranch %38
%42 = OpLabel
OpStore %b %float_5
OpBranch %58
%58 = OpLabel
OpLoopMerge %62 %61 None
OpBranch %59
%59 = OpLabel
%63 = OpLoad %float %b
%64 = OpFOrdGreaterThanEqual %bool %63 %float_0
OpBranchConditional %64 %60 %62
%60 = OpLabel
%65 = OpLoad %float %b
%66 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %66 %65
%68 = OpLoad %v4float %x
%69 = OpCompositeExtract %float %68 3
%70 = OpFOrdEqual %bool %69 %float_1
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
OpBranch %61
%72 = OpLabel
%73 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %73 %float_0
OpBranch %61
%61 = OpLabel
%75 = OpLoad %float %b
%76 = OpFSub %float %75 %float_1
OpStore %b %76
OpBranch %58
%62 = OpLabel
%77 = OpLoad %v4float %x
OpReturnValue %77
OpFunctionEnd
