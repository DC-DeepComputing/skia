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
OpDecorate %47 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%int_1 = OpConstant %int 1
%_ptr_Uniform_float = OpTypePointer Uniform %float
%float_2 = OpConstant %float 2
%true = OpConstantTrue %bool
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%value = OpVariable %_ptr_Function_float Function
%_0_switchValue = OpVariable %_ptr_Function_int Function
%_2_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_3_switchLoop = OpVariable %_ptr_Function_int Function
OpStore %value %float_0
OpStore %_0_switchValue %int_0
OpStore %_2_switchFallthrough %false
OpStore %_3_switchLoop %int_0
OpBranch %27
%27 = OpLabel
OpLoopMerge %31 %30 None
OpBranch %28
%28 = OpLabel
%32 = OpLoad %int %_3_switchLoop
%34 = OpSLessThan %bool %32 %int_1
OpBranchConditional %34 %29 %31
%29 = OpLabel
%35 = OpLoad %int %_0_switchValue
%36 = OpIEqual %bool %35 %int_0
OpSelectionMerge %38 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
OpStore %value %float_0
%39 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%41 = OpLoad %float %39
%43 = OpFOrdEqual %bool %41 %float_2
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
OpBranch %31
%45 = OpLabel
OpStore %_2_switchFallthrough %true
OpBranch %38
%38 = OpLabel
%47 = OpLoad %bool %_2_switchFallthrough
OpSelectionMerge %49 None
OpBranchConditional %47 %49 %48
%48 = OpLabel
%50 = OpLoad %int %_0_switchValue
%51 = OpIEqual %bool %50 %int_1
OpBranch %49
%49 = OpLabel
%52 = OpPhi %bool %true %38 %51 %48
OpSelectionMerge %54 None
OpBranchConditional %52 %53 %54
%53 = OpLabel
OpStore %value %float_1
OpStore %_2_switchFallthrough %true
OpBranch %54
%54 = OpLabel
OpBranch %30
%30 = OpLabel
%56 = OpLoad %int %_3_switchLoop
%57 = OpIAdd %int %56 %int_1
OpStore %_3_switchLoop %57
OpBranch %27
%31 = OpLabel
%58 = OpLoad %float %value
%59 = OpCompositeConstruct %v4float %58 %58 %58 %58
OpStore %sk_FragColor %59
OpReturn
OpFunctionEnd
