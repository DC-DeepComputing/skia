OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %foo_ff2 "foo_ff2"
OpName %bar_vf "bar_vf"
OpName %y "y"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%27 = OpTypeFunction %float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%35 = OpTypeFunction %void %_ptr_Function_float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%int_0 = OpConstant %int 0
%float_2 = OpConstant %float 2
%int_1 = OpConstant %int 1
%59 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_10 = OpConstant %float 10
%float_200 = OpConstant %float 200
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%foo_ff2 = OpFunction %float None %27
%28 = OpFunctionParameter %_ptr_Function_v2float
%29 = OpLabel
%30 = OpLoad %v2float %28
%31 = OpCompositeExtract %float %30 0
%32 = OpLoad %v2float %28
%33 = OpCompositeExtract %float %32 1
%34 = OpFMul %float %31 %33
OpReturnValue %34
OpFunctionEnd
%bar_vf = OpFunction %void None %35
%37 = OpFunctionParameter %_ptr_Function_float
%38 = OpLabel
%y = OpVariable %_ptr_Function__arr_float_int_2 Function
%57 = OpVariable %_ptr_Function_v2float Function
%44 = OpLoad %float %37
%46 = OpAccessChain %_ptr_Function_float %y %int_0
OpStore %46 %44
%47 = OpLoad %float %37
%49 = OpFMul %float %47 %float_2
%51 = OpAccessChain %_ptr_Function_float %y %int_1
OpStore %51 %49
%52 = OpAccessChain %_ptr_Function_float %y %int_0
%53 = OpLoad %float %52
%54 = OpAccessChain %_ptr_Function_float %y %int_1
%55 = OpLoad %float %54
%56 = OpCompositeConstruct %v2float %53 %55
OpStore %57 %56
%58 = OpFunctionCall %float %foo_ff2 %57
OpStore %37 %58
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %59
%60 = OpFunctionParameter %_ptr_Function_v2float
%61 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%68 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_10
%64 = OpFunctionCall %void %bar_vf %x
%65 = OpLoad %float %x
%67 = OpFOrdEqual %bool %65 %float_200
OpSelectionMerge %72 None
OpBranchConditional %67 %70 %71
%70 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%75 = OpLoad %v4float %73
OpStore %68 %75
OpBranch %72
%71 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%77 = OpLoad %v4float %76
OpStore %68 %77
OpBranch %72
%72 = OpLabel
%78 = OpLoad %v4float %68
OpReturnValue %78
OpFunctionEnd
