OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%void = OpTypeVoid
%11 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%true = OpConstantTrue %bool
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%y = OpVariable %_ptr_Function_int Function
%z = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpStore %y %int_0
OpStore %z %int_0
OpSelectionMerge %21 None
OpBranchConditional %true %20 %21
%20 = OpLabel
OpStore %x %int_1
OpBranch %21
%21 = OpLabel
OpSelectionMerge %25 None
OpBranchConditional %false %24 %25
%24 = OpLabel
OpStore %y %int_1
OpBranch %25
%25 = OpLabel
OpSelectionMerge %27 None
OpBranchConditional %false %26 %27
%26 = OpLabel
OpStore %z %int_1
OpBranch %27
%27 = OpLabel
%28 = OpLoad %int %x
%29 = OpConvertSToF %float %28
%30 = OpLoad %int %y
%31 = OpConvertSToF %float %30
%32 = OpLoad %int %z
%33 = OpConvertSToF %float %32
%35 = OpCompositeConstruct %v3float %29 %31 %33
%36 = OpLoad %v4float %sk_FragColor
%37 = OpVectorShuffle %v4float %36 %35 4 5 6 3
OpStore %sk_FragColor %37
OpReturn
OpFunctionEnd
