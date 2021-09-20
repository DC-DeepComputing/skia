OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "unknownInput"
OpName %main "main"
OpName %value "value"
OpName %_0_switchValue "_0_switchValue"
OpName %_2_switchFallthrough "_2_switchFallthrough"
OpName %_3_switchLoop "_3_switchLoop"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %value RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%_UniformBuffer = OpTypeStruct %int
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_int = OpTypePointer Uniform %int
%int_0 = OpConstant %int 0
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0
%true = OpConstantTrue %bool
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%main = OpFunction %void None %15
%16 = OpLabel
%value = OpVariable %_ptr_Function_float Function
%_0_switchValue = OpVariable %_ptr_Function_int Function
%_2_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_3_switchLoop = OpVariable %_ptr_Function_int Function
%21 = OpAccessChain %_ptr_Uniform_int %10 %int_0
%24 = OpLoad %int %21
OpStore %_0_switchValue %24
OpStore %_2_switchFallthrough %false
OpStore %_3_switchLoop %int_0
OpBranch %29
%29 = OpLabel
OpLoopMerge %33 %32 None
OpBranch %30
%30 = OpLabel
%34 = OpLoad %int %_3_switchLoop
%36 = OpSLessThan %bool %34 %int_1
OpBranchConditional %36 %31 %33
%31 = OpLabel
%37 = OpLoad %int %_0_switchValue
%38 = OpIEqual %bool %37 %int_0
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
OpStore %value %float_0
OpStore %_2_switchFallthrough %true
OpBranch %40
%40 = OpLabel
%43 = OpLoad %bool %_2_switchFallthrough
OpSelectionMerge %45 None
OpBranchConditional %43 %45 %44
%44 = OpLabel
%46 = OpLoad %int %_0_switchValue
%47 = OpIEqual %bool %46 %int_1
OpBranch %45
%45 = OpLabel
%48 = OpPhi %bool %true %40 %47 %44
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
OpStore %value %float_1
OpStore %_2_switchFallthrough %true
OpBranch %50
%50 = OpLabel
OpStore %value %float_2
OpBranch %32
%32 = OpLabel
%53 = OpLoad %int %_3_switchLoop
%54 = OpIAdd %int %53 %int_1
OpStore %_3_switchLoop %54
OpBranch %29
%33 = OpLabel
%55 = OpLoad %float %value
%56 = OpCompositeConstruct %v4float %55 %55 %55 %55
OpStore %sk_FragColor %56
OpReturn
OpFunctionEnd
