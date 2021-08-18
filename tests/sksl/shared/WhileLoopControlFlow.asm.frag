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
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
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
%34 = OpLoad %v4float %x
%35 = OpCompositeExtract %float %34 3
%36 = OpFOrdEqual %bool %35 %float_1
OpBranchConditional %36 %31 %33
%31 = OpLabel
%37 = OpAccessChain %_ptr_Function_float %x %int_0
%41 = OpLoad %float %37
%43 = OpFSub %float %41 %float_0_25
OpStore %37 %43
%44 = OpLoad %v4float %x
%45 = OpCompositeExtract %float %44 0
%46 = OpFOrdLessThanEqual %bool %45 %float_0
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
OpBranch %33
%48 = OpLabel
OpBranch %32
%32 = OpLabel
OpBranch %29
%33 = OpLabel
OpBranch %49
%49 = OpLabel
OpLoopMerge %53 %52 None
OpBranch %50
%50 = OpLabel
%54 = OpLoad %v4float %x
%55 = OpCompositeExtract %float %54 2
%56 = OpFOrdGreaterThan %bool %55 %float_0
OpBranchConditional %56 %51 %53
%51 = OpLabel
%57 = OpAccessChain %_ptr_Function_float %x %int_2
%59 = OpLoad %float %57
%60 = OpFSub %float %59 %float_0_25
OpStore %57 %60
%61 = OpLoad %v4float %x
%62 = OpCompositeExtract %float %61 3
%63 = OpFOrdEqual %bool %62 %float_1
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
OpBranch %52
%65 = OpLabel
%66 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %66 %float_0
OpBranch %52
%52 = OpLabel
OpBranch %49
%53 = OpLabel
%68 = OpLoad %v4float %x
OpReturnValue %68
OpFunctionEnd
