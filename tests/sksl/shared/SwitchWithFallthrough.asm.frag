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
OpDecorate %44 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
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
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_0 = OpConstant %int 0
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%int_1 = OpConstant %int 1
%true = OpConstantTrue %bool
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%value = OpVariable %_ptr_Function_float Function
%_0_switchValue = OpVariable %_ptr_Function_int Function
%_2_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_3_switchLoop = OpVariable %_ptr_Function_int Function
OpStore %value %float_0
%22 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%25 = OpLoad %float %22
%26 = OpConvertFToS %int %25
OpStore %_0_switchValue %26
OpStore %_2_switchFallthrough %false
OpStore %_3_switchLoop %int_0
OpBranch %31
%31 = OpLabel
OpLoopMerge %35 %34 None
OpBranch %32
%32 = OpLabel
%36 = OpLoad %int %_3_switchLoop
%38 = OpSLessThan %bool %36 %int_1
OpBranchConditional %38 %33 %35
%33 = OpLabel
%39 = OpLoad %int %_0_switchValue
%40 = OpIEqual %bool %39 %int_0
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
OpStore %value %float_0
OpStore %_2_switchFallthrough %true
OpBranch %42
%42 = OpLabel
%44 = OpLoad %bool %_2_switchFallthrough
OpSelectionMerge %46 None
OpBranchConditional %44 %46 %45
%45 = OpLabel
%47 = OpLoad %int %_0_switchValue
%48 = OpIEqual %bool %47 %int_1
OpBranch %46
%46 = OpLabel
%49 = OpPhi %bool %true %42 %48 %45
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
OpStore %value %float_1
OpStore %_2_switchFallthrough %true
OpBranch %51
%51 = OpLabel
OpBranch %34
%34 = OpLabel
%53 = OpLoad %int %_3_switchLoop
%54 = OpIAdd %int %53 %int_1
OpStore %_3_switchLoop %54
OpBranch %31
%35 = OpLabel
%55 = OpLoad %float %value
%56 = OpCompositeConstruct %v4float %55 %55 %55 %55
OpStore %sk_FragColor %56
OpReturn
OpFunctionEnd
