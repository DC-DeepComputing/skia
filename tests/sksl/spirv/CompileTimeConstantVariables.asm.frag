OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %someUniforms "someUniforms"
OpMemberName %someUniforms 0 "value"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpMemberDecorate %someUniforms 0 Offset 0
OpDecorate %someUniforms Block
OpDecorate %3 Binding 0
OpDecorate %3 DescriptorSet 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
%int = OpTypeInt 32 1
%someUniforms = OpTypeStruct %int
%_ptr_Uniform_someUniforms = OpTypePointer Uniform %someUniforms
%3 = OpVariable %_ptr_Uniform_someUniforms Uniform
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%int_0 = OpConstant %int 0
%_ptr_Uniform_int = OpTypePointer Uniform %int
%float_2_1400001 = OpConstant %float 2.1400001
%36 = OpConstantComposite %v4float %float_2_1400001 %float_2_1400001 %float_2_1400001 %float_2_1400001
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%float_0_200000003 = OpConstant %float 0.200000003
%46 = OpConstantComposite %v4float %float_1 %float_0_200000003 %float_2_1400001 %float_1
%float_3_1400001 = OpConstant %float 3.1400001
%48 = OpConstantComposite %v4float %float_3_1400001 %float_3_1400001 %float_3_1400001 %float_3_1400001
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%28 = OpAccessChain %_ptr_Uniform_int %3 %int_0
%30 = OpLoad %int %28
%31 = OpIEqual %bool %30 %int_0
OpSelectionMerge %34 None
OpBranchConditional %31 %32 %33
%32 = OpLabel
OpReturnValue %36
%33 = OpLabel
%37 = OpAccessChain %_ptr_Uniform_int %3 %int_0
%38 = OpLoad %int %37
%40 = OpIEqual %bool %38 %int_1
OpSelectionMerge %43 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
OpReturnValue %46
%42 = OpLabel
OpReturnValue %48
%43 = OpLabel
OpBranch %34
%34 = OpLabel
OpUnreachable
OpFunctionEnd
