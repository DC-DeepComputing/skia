OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %globalVar "globalVar"
OpName %S "S"
OpMemberName %S 0 "f"
OpMemberName %S 1 "af"
OpMemberName %S 2 "h4"
OpMemberName %S 3 "ah4"
OpName %globalStruct "globalStruct"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %i "i"
OpName %i4 "i4"
OpName %f3x3 "f3x3"
OpName %x "x"
OpName %ai "ai"
OpName %ai4 "ai4"
OpName %ah2x4 "ah2x4"
OpName %af4 "af4"
OpName %s "s"
OpName %l "l"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %globalVar RelaxedPrecision
OpDecorate %_arr_float_int_5 ArrayStride 16
OpDecorate %_arr_v4float_int_5 ArrayStride 16
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 16
OpMemberDecorate %S 2 Offset 96
OpMemberDecorate %S 2 RelaxedPrecision
OpMemberDecorate %S 3 Offset 112
OpMemberDecorate %S 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %x RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %_arr_v4int_int_1 ArrayStride 16
OpDecorate %_arr_mat3v3float_int_1 ArrayStride 48
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %_arr_v4float_int_1 ArrayStride 16
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Private_v4float = OpTypePointer Private %v4float
%globalVar = OpVariable %_ptr_Private_v4float Private
%int = OpTypeInt 32 1
%int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_arr_v4float_int_5 = OpTypeArray %v4float %int_5
%S = OpTypeStruct %float %_arr_float_int_5 %v4float %_arr_v4float_int_5
%_ptr_Private_S = OpTypePointer Private %S
%globalStruct = OpVariable %_ptr_Private_S Private
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%24 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%32 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%45 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%59 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%60 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%61 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
%67 = OpConstantComposite %v2float %float_0 %float_0
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
%_arr_v4int_int_1 = OpTypeArray %v4int %int_1
%_ptr_Function__arr_v4int_int_1 = OpTypePointer Function %_arr_v4int_int_1
%_arr_mat3v3float_int_1 = OpTypeArray %mat3v3float %int_1
%_ptr_Function__arr_mat3v3float_int_1 = OpTypePointer Function %_arr_mat3v3float_int_1
%81 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%82 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%83 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%91 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_S = OpTypePointer Function %S
%99 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%103 = OpConstantComposite %v2float %float_5 %float_5
%107 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Private_float = OpTypePointer Private %float
%119 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%121 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %24
%25 = OpLabel
%29 = OpVariable %_ptr_Function_v2float Function
OpStore %29 %28
%31 = OpFunctionCall %v4float %main %29
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %32
%33 = OpFunctionParameter %_ptr_Function_v2float
%34 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah2x4 = OpVariable %_ptr_Function__arr_mat3v3float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%s = OpVariable %_ptr_Function_S Function
%l = OpVariable %_ptr_Function_float Function
OpStore %i %int_0
OpStore %i4 %45
%62 = OpCompositeConstruct %mat3v3float %59 %60 %61
OpStore %f3x3 %62
%65 = OpAccessChain %_ptr_Function_float %x %int_3
OpStore %65 %float_0
%68 = OpLoad %v4float %x
%69 = OpVectorShuffle %v4float %68 %67 5 4 2 3
OpStore %x %69
%73 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %73 %int_0
%77 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
OpStore %77 %45
%84 = OpCompositeConstruct %mat3v3float %81 %82 %83
%85 = OpAccessChain %_ptr_Function_mat3v3float %ah2x4 %int_0
OpStore %85 %84
%89 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%90 = OpAccessChain %_ptr_Function_float %89 %int_0
OpStore %90 %float_0
%92 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%93 = OpLoad %v4float %92
%94 = OpVectorShuffle %v4float %93 %91 6 4 7 5
OpStore %92 %94
%97 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %97 %float_0
%98 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
OpStore %98 %float_0
%100 = OpAccessChain %_ptr_Function_v4float %s %int_2
%101 = OpLoad %v4float %100
%102 = OpVectorShuffle %v4float %101 %99 5 6 4 3
OpStore %100 %102
%104 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_2
%105 = OpLoad %v4float %104
%106 = OpVectorShuffle %v4float %105 %103 0 4 2 5
OpStore %104 %106
OpStore %globalVar %107
%108 = OpAccessChain %_ptr_Private_float %globalStruct %int_0
OpStore %108 %float_0
OpStore %l %float_0
%111 = OpAccessChain %_ptr_Function_int %ai %int_0
%112 = OpLoad %int %111
%113 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%114 = OpLoad %v4int %113
%115 = OpCompositeExtract %int %114 0
%116 = OpIAdd %int %112 %115
OpStore %111 %116
%117 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %117 %float_1
%118 = OpAccessChain %_ptr_Function_float %s %int_1 %int_0
OpStore %118 %float_2
%120 = OpAccessChain %_ptr_Function_v4float %s %int_2
OpStore %120 %119
%122 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
OpStore %122 %121
%123 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%124 = OpLoad %v4float %123
%125 = OpAccessChain %_ptr_Function_v3float %ah2x4 %int_0 %int_0
%127 = OpLoad %v3float %125
%128 = OpCompositeExtract %float %127 0
%129 = OpVectorTimesScalar %v4float %124 %128
OpStore %123 %129
%130 = OpAccessChain %_ptr_Function_int %i4 %int_1
%131 = OpLoad %int %130
%132 = OpLoad %int %i
%133 = OpIMul %int %131 %132
OpStore %130 %133
%134 = OpAccessChain %_ptr_Function_float %x %int_1
%135 = OpLoad %float %134
%136 = OpLoad %float %l
%137 = OpFMul %float %135 %136
OpStore %134 %137
%138 = OpAccessChain %_ptr_Function_float %s %int_0
%139 = OpLoad %float %138
%140 = OpLoad %float %l
%141 = OpFMul %float %139 %140
OpStore %138 %141
%142 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%144 = OpLoad %v4float %142
OpReturnValue %144
OpFunctionEnd
