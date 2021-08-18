OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %x RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%14 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%22 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%28 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_25 = OpConstant %float 0.25
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %14
%15 = OpLabel
%19 = OpVariable %_ptr_Function_v2float Function
OpStore %19 %18
%21 = OpFunctionCall %v4float %main %19
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %22
%23 = OpFunctionParameter %_ptr_Function_v2float
%24 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
OpStore %x %28
OpBranch %29
%29 = OpLabel
OpLoopMerge %33 %32 None
OpBranch %30
%30 = OpLabel
%34 = OpAccessChain %_ptr_Function_float %x %int_0
%38 = OpLoad %float %34
%40 = OpFSub %float %38 %float_0_25
OpStore %34 %40
%41 = OpLoad %v4float %x
%42 = OpCompositeExtract %float %41 0
%43 = OpFOrdLessThanEqual %bool %42 %float_0
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
OpBranch %33
%45 = OpLabel
OpBranch %31
%31 = OpLabel
OpBranch %32
%32 = OpLabel
%46 = OpLoad %v4float %x
%47 = OpCompositeExtract %float %46 3
%48 = OpFOrdEqual %bool %47 %float_1
OpBranchConditional %48 %29 %33
%33 = OpLabel
OpBranch %49
%49 = OpLabel
OpLoopMerge %53 %52 None
OpBranch %50
%50 = OpLabel
%54 = OpAccessChain %_ptr_Function_float %x %int_2
%56 = OpLoad %float %54
%57 = OpFSub %float %56 %float_0_25
OpStore %54 %57
%58 = OpLoad %v4float %x
%59 = OpCompositeExtract %float %58 3
%60 = OpFOrdEqual %bool %59 %float_1
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
OpBranch %52
%62 = OpLabel
%63 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %63 %float_0
OpBranch %51
%51 = OpLabel
OpBranch %52
%52 = OpLabel
%65 = OpLoad %v4float %x
%66 = OpCompositeExtract %float %65 2
%67 = OpFOrdGreaterThan %bool %66 %float_0
OpBranchConditional %67 %49 %53
%53 = OpLabel
%68 = OpLoad %v4float %x
OpReturnValue %68
OpFunctionEnd
