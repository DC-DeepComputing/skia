OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %globalArray "globalArray"
OpName %globalMatrix "globalMatrix"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "zero"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %localArray "localArray"
OpName %localMatrix "localMatrix"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %globalArray RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %16 RelaxedPrecision
OpDecorate %globalMatrix RelaxedPrecision
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %23 Binding 0
OpDecorate %23 DescriptorSet 0
OpDecorate %localArray RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %localMatrix RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Private__arr_float_int_2 = OpTypePointer Private %_arr_float_int_2
%globalArray = OpVariable %_ptr_Private__arr_float_int_2 Private
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Private_mat2v2float = OpTypePointer Private %mat2v2float
%globalMatrix = OpVariable %_ptr_Private_mat2v2float Private
%21 = OpConstantComposite %v2float %float_1 %float_1
%22 = OpConstantComposite %mat2v2float %21 %21
%_UniformBuffer = OpTypeStruct %int
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%23 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%28 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%31 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%35 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%45 = OpConstantComposite %v2float %float_0 %float_1
%46 = OpConstantComposite %v2float %float_2 %float_3
%47 = OpConstantComposite %mat2v2float %45 %46
%_ptr_Uniform_int = OpTypePointer Uniform %int
%int_0 = OpConstant %int 0
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Private_v2float = OpTypePointer Private %v2float
%_entrypoint_v = OpFunction %void None %28
%29 = OpLabel
%32 = OpVariable %_ptr_Function_v2float Function
OpStore %32 %31
%34 = OpFunctionCall %v4float %main %32
OpStore %sk_FragColor %34
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %35
%36 = OpFunctionParameter %_ptr_Function_v2float
%37 = OpLabel
%localArray = OpVariable %_ptr_Function__arr_float_int_2 Function
%localMatrix = OpVariable %_ptr_Function_mat2v2float Function
%16 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_1
OpStore %globalArray %16
OpStore %globalMatrix %22
%40 = OpCompositeConstruct %_arr_float_int_2 %float_0 %float_1
OpStore %localArray %40
OpStore %localMatrix %47
%48 = OpAccessChain %_ptr_Uniform_int %23 %int_0
%51 = OpLoad %int %48
%52 = OpAccessChain %_ptr_Private_float %globalArray %51
%54 = OpLoad %float %52
%55 = OpAccessChain %_ptr_Uniform_int %23 %int_0
%56 = OpLoad %int %55
%57 = OpAccessChain %_ptr_Function_float %localArray %56
%59 = OpLoad %float %57
%60 = OpFMul %float %54 %59
%61 = OpAccessChain %_ptr_Uniform_int %23 %int_0
%62 = OpLoad %int %61
%63 = OpVectorExtractDynamic %float %21 %62
%64 = OpAccessChain %_ptr_Uniform_int %23 %int_0
%65 = OpLoad %int %64
%66 = OpVectorExtractDynamic %float %21 %65
%67 = OpFMul %float %63 %66
%68 = OpAccessChain %_ptr_Uniform_int %23 %int_0
%69 = OpLoad %int %68
%70 = OpAccessChain %_ptr_Private_v2float %globalMatrix %69
%72 = OpLoad %v2float %70
%73 = OpAccessChain %_ptr_Uniform_int %23 %int_0
%74 = OpLoad %int %73
%75 = OpAccessChain %_ptr_Function_v2float %localMatrix %74
%76 = OpLoad %v2float %75
%77 = OpFMul %v2float %72 %76
%78 = OpCompositeExtract %float %77 0
%79 = OpCompositeExtract %float %77 1
%80 = OpCompositeConstruct %v4float %60 %67 %78 %79
OpReturnValue %80
OpFunctionEnd
