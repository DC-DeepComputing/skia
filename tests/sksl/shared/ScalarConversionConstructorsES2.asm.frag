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
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %b "b"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %i1 "i1"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %b1 "b1"
OpName %b2 "b2"
OpName %b3 "b3"
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
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
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_bool = OpTypePointer Function %bool
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%float_9 = OpConstant %float 9
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %25
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%b = OpVariable %_ptr_Function_bool Function
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%i1 = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_int Function
%i3 = OpVariable %_ptr_Function_int Function
%b1 = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_bool Function
%b3 = OpVariable %_ptr_Function_bool Function
%99 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
%35 = OpCompositeExtract %float %34 1
OpStore %f %35
%38 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%39 = OpLoad %v4float %38
%40 = OpCompositeExtract %float %39 1
%41 = OpConvertFToS %int %40
OpStore %i %41
%44 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%45 = OpLoad %v4float %44
%46 = OpCompositeExtract %float %45 1
%47 = OpFUnordNotEqual %bool %46 %float_0
OpStore %b %47
%49 = OpLoad %float %f
OpStore %f1 %49
%51 = OpLoad %int %i
%52 = OpConvertSToF %float %51
OpStore %f2 %52
%54 = OpLoad %bool %b
%55 = OpSelect %float %54 %float_1 %float_0
OpStore %f3 %55
%58 = OpLoad %float %f
%59 = OpConvertFToS %int %58
OpStore %i1 %59
%61 = OpLoad %int %i
OpStore %i2 %61
%63 = OpLoad %bool %b
%64 = OpSelect %int %63 %int_1 %int_0
OpStore %i3 %64
%67 = OpLoad %float %f
%68 = OpFUnordNotEqual %bool %67 %float_0
OpStore %b1 %68
%70 = OpLoad %int %i
%71 = OpINotEqual %bool %70 %int_0
OpStore %b2 %71
%73 = OpLoad %bool %b
OpStore %b3 %73
%74 = OpLoad %float %f1
%75 = OpLoad %float %f2
%76 = OpFAdd %float %74 %75
%77 = OpLoad %float %f3
%78 = OpFAdd %float %76 %77
%79 = OpLoad %int %i1
%80 = OpConvertSToF %float %79
%81 = OpFAdd %float %78 %80
%82 = OpLoad %int %i2
%83 = OpConvertSToF %float %82
%84 = OpFAdd %float %81 %83
%85 = OpLoad %int %i3
%86 = OpConvertSToF %float %85
%87 = OpFAdd %float %84 %86
%88 = OpLoad %bool %b1
%89 = OpSelect %float %88 %float_1 %float_0
%90 = OpFAdd %float %87 %89
%91 = OpLoad %bool %b2
%92 = OpSelect %float %91 %float_1 %float_0
%93 = OpFAdd %float %90 %92
%94 = OpLoad %bool %b3
%95 = OpSelect %float %94 %float_1 %float_0
%96 = OpFAdd %float %93 %95
%98 = OpFOrdEqual %bool %96 %float_9
OpSelectionMerge %103 None
OpBranchConditional %98 %101 %102
%101 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%105 = OpLoad %v4float %104
OpStore %99 %105
OpBranch %103
%102 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%107 = OpLoad %v4float %106
OpStore %99 %107
OpBranch %103
%103 = OpLabel
%108 = OpLoad %v4float %99
OpReturnValue %108
OpFunctionEnd
