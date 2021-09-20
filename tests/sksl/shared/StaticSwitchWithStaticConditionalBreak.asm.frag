OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpName %_0_switchValue "_0_switchValue"
OpName %_2_switchFallthrough "_2_switchFallthrough"
OpName %_3_switchLoop "_3_switchLoop"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %42 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%true = OpConstantTrue %bool
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%_0_switchValue = OpVariable %_ptr_Function_int Function
%_2_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_3_switchLoop = OpVariable %_ptr_Function_int Function
OpStore %x %float_0
OpStore %_0_switchValue %int_0
OpStore %_2_switchFallthrough %false
OpStore %_3_switchLoop %int_0
OpBranch %24
%24 = OpLabel
OpLoopMerge %28 %27 None
OpBranch %25
%25 = OpLabel
%29 = OpLoad %int %_3_switchLoop
%31 = OpSLessThan %bool %29 %int_1
OpBranchConditional %31 %26 %28
%26 = OpLabel
%32 = OpLoad %int %_0_switchValue
%33 = OpIEqual %bool %32 %int_0
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
OpStore %x %float_0
%36 = OpLoad %float %x
%38 = OpFOrdLessThan %bool %36 %float_1
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
OpBranch %28
%40 = OpLabel
OpStore %_2_switchFallthrough %true
OpBranch %35
%35 = OpLabel
%42 = OpLoad %bool %_2_switchFallthrough
OpSelectionMerge %44 None
OpBranchConditional %42 %44 %43
%43 = OpLabel
%45 = OpLoad %int %_0_switchValue
%46 = OpIEqual %bool %45 %int_1
OpBranch %44
%44 = OpLabel
%47 = OpPhi %bool %true %35 %46 %43
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
OpStore %x %float_1
OpStore %_2_switchFallthrough %true
OpBranch %49
%49 = OpLabel
OpBranch %27
%27 = OpLabel
%50 = OpLoad %int %_3_switchLoop
%51 = OpIAdd %int %50 %int_1
OpStore %_3_switchLoop %51
OpBranch %24
%28 = OpLabel
%52 = OpLoad %float %x
%53 = OpCompositeConstruct %v4float %52 %52 %52 %52
OpStore %sk_FragColor %53
OpReturn
OpFunctionEnd
