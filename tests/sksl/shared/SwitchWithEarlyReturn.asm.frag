OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %switch_with_early_return_bi "switch_with_early_return_bi"
OpName %val "val"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%26 = OpTypeFunction %bool %_ptr_Function_int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%42 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%switch_with_early_return_bi = OpFunction %bool None %26
%27 = OpFunctionParameter %_ptr_Function_int
%28 = OpLabel
%val = OpVariable %_ptr_Function_int Function
OpStore %val %int_0
%31 = OpLoad %int %27
OpSelectionMerge %32 None
OpSwitch %31 %34 1 %33
%33 = OpLabel
%36 = OpIAdd %int %int_0 %int_1
OpStore %val %36
OpReturnValue %false
%34 = OpLabel
%38 = OpLoad %int %val
%39 = OpIAdd %int %38 %int_1
OpStore %val %39
OpBranch %32
%32 = OpLabel
%40 = OpLoad %int %val
%41 = OpIEqual %bool %40 %int_1
OpReturnValue %41
OpFunctionEnd
%main = OpFunction %v4float None %42
%43 = OpFunctionParameter %_ptr_Function_v2float
%44 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%51 = OpVariable %_ptr_Function_int Function
%53 = OpVariable %_ptr_Function_v4float Function
%46 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%48 = OpLoad %v4float %46
%49 = OpCompositeExtract %float %48 1
%50 = OpConvertFToS %int %49
OpStore %x %50
OpStore %51 %50
%52 = OpFunctionCall %bool %switch_with_early_return_bi %51
OpSelectionMerge %57 None
OpBranchConditional %52 %55 %56
%55 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%59 = OpLoad %v4float %58
OpStore %53 %59
OpBranch %57
%56 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%61 = OpLoad %v4float %60
OpStore %53 %61
OpBranch %57
%57 = OpLabel
%62 = OpLoad %v4float %53
OpReturnValue %62
OpFunctionEnd
