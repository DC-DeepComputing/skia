OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
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
OpName %keepAlive_vh "keepAlive_vh"
OpName %keepAlive_vf "keepAlive_vf"
OpName %keepAlive_vi "keepAlive_vi"
OpName %main "main"
OpName %i "i"
OpName %i4 "i4"
OpName %f3x3 "f3x3"
OpName %x "x"
OpName %ai "ai"
OpName %ai4 "ai4"
OpName %ah3x3 "ah3x3"
OpName %af4 "af4"
OpName %s "s"
OpName %l "l"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
OpDecorate %22 Binding 0
OpDecorate %22 DescriptorSet 0
OpDecorate %x RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %_arr_v4int_int_1 ArrayStride 16
OpDecorate %_arr_mat3v3float_int_1 ArrayStride 48
OpDecorate %_arr_v4float_int_1 ArrayStride 16
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
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
%22 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%27 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%31 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%36 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Function_int = OpTypePointer Function %int
%42 = OpTypeFunction %void %_ptr_Function_int
%45 = OpTypeFunction %v4float %_ptr_Function_v2float
%int_0 = OpConstant %int 0
%false = OpConstantFalse %bool
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%58 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
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
%72 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%73 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%74 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%75 = OpConstantComposite %mat3v3float %72 %73 %74
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Function__arr_int_int_1 = OpTypePointer Function %_arr_int_int_1
%_arr_v4int_int_1 = OpTypeArray %v4int %int_1
%_ptr_Function__arr_v4int_int_1 = OpTypePointer Function %_arr_v4int_int_1
%_arr_mat3v3float_int_1 = OpTypeArray %mat3v3float %int_1
%_ptr_Function__arr_mat3v3float_int_1 = OpTypePointer Function %_arr_mat3v3float_int_1
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%98 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_S = OpTypePointer Function %S
%106 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%110 = OpConstantComposite %v2float %float_5 %float_5
%114 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Private_float = OpTypePointer Private %float
%127 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %27
%28 = OpLabel
%32 = OpVariable %_ptr_Function_v2float Function
OpStore %32 %31
%34 = OpFunctionCall %v4float %main %32
OpStore %sk_FragColor %34
OpReturn
OpFunctionEnd
%keepAlive_vh = OpFunction %void None %36
%37 = OpFunctionParameter %_ptr_Function_float
%38 = OpLabel
OpReturn
OpFunctionEnd
%keepAlive_vf = OpFunction %void None %36
%39 = OpFunctionParameter %_ptr_Function_float
%40 = OpLabel
OpReturn
OpFunctionEnd
%keepAlive_vi = OpFunction %void None %42
%43 = OpFunctionParameter %_ptr_Function_int
%44 = OpLabel
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %45
%46 = OpFunctionParameter %_ptr_Function_v2float
%47 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%x = OpVariable %_ptr_Function_v4float Function
%ai = OpVariable %_ptr_Function__arr_int_int_1 Function
%ai4 = OpVariable %_ptr_Function__arr_v4int_int_1 Function
%ah3x3 = OpVariable %_ptr_Function__arr_mat3v3float_int_1 Function
%af4 = OpVariable %_ptr_Function__arr_v4float_int_1 Function
%s = OpVariable %_ptr_Function_S Function
%l = OpVariable %_ptr_Function_float Function
%132 = OpVariable %_ptr_Function_float Function
%139 = OpVariable %_ptr_Function_float Function
%143 = OpVariable %_ptr_Function_int Function
%148 = OpVariable %_ptr_Function_int Function
%153 = OpVariable %_ptr_Function_int Function
%159 = OpVariable %_ptr_Function_int Function
%164 = OpVariable %_ptr_Function_float Function
%169 = OpVariable %_ptr_Function_float Function
%173 = OpVariable %_ptr_Function_float Function
%179 = OpVariable %_ptr_Function_float Function
OpStore %i %int_0
OpStore %i4 %58
OpStore %f3x3 %75
%78 = OpAccessChain %_ptr_Function_float %x %int_3
OpStore %78 %float_0
%79 = OpLoad %v4float %x
%80 = OpVectorShuffle %v4float %79 %31 5 4 2 3
OpStore %x %80
%84 = OpAccessChain %_ptr_Function_int %ai %int_0
OpStore %84 %int_0
%88 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
OpStore %88 %58
%92 = OpAccessChain %_ptr_Function_mat3v3float %ah3x3 %int_0
OpStore %92 %75
%96 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%97 = OpAccessChain %_ptr_Function_float %96 %int_0
OpStore %97 %float_0
%99 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%100 = OpLoad %v4float %99
%101 = OpVectorShuffle %v4float %100 %98 6 4 7 5
OpStore %99 %101
%104 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %104 %float_0
%105 = OpAccessChain %_ptr_Function_float %s %int_1 %int_1
OpStore %105 %float_0
%107 = OpAccessChain %_ptr_Function_v4float %s %int_2
%108 = OpLoad %v4float %107
%109 = OpVectorShuffle %v4float %108 %106 5 6 4 3
OpStore %107 %109
%111 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_2
%112 = OpLoad %v4float %111
%113 = OpVectorShuffle %v4float %112 %110 0 4 2 5
OpStore %111 %113
OpStore %globalVar %114
%115 = OpAccessChain %_ptr_Private_float %globalStruct %int_0
OpStore %115 %float_0
OpStore %l %float_0
%118 = OpAccessChain %_ptr_Function_int %ai %int_0
%119 = OpLoad %int %118
%120 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%121 = OpLoad %v4int %120
%122 = OpCompositeExtract %int %121 0
%123 = OpIAdd %int %119 %122
OpStore %118 %123
%124 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %124 %float_1
%125 = OpAccessChain %_ptr_Function_float %s %int_1 %int_0
OpStore %125 %float_2
%126 = OpAccessChain %_ptr_Function_v4float %s %int_2
OpStore %126 %98
%128 = OpAccessChain %_ptr_Function_v4float %s %int_3 %int_0
OpStore %128 %127
%129 = OpAccessChain %_ptr_Function_v4float %af4 %int_0
%130 = OpAccessChain %_ptr_Function_float %129 %int_0
%131 = OpLoad %float %130
OpStore %132 %131
%133 = OpFunctionCall %void %keepAlive_vf %132
%134 = OpLoad %float %132
OpStore %130 %134
%135 = OpAccessChain %_ptr_Function_v3float %ah3x3 %int_0 %int_0
%137 = OpAccessChain %_ptr_Function_float %135 %int_0
%138 = OpLoad %float %137
OpStore %139 %138
%140 = OpFunctionCall %void %keepAlive_vh %139
%141 = OpLoad %float %139
OpStore %137 %141
%142 = OpLoad %int %i
OpStore %143 %142
%144 = OpFunctionCall %void %keepAlive_vi %143
%145 = OpLoad %int %143
OpStore %i %145
%146 = OpAccessChain %_ptr_Function_int %i4 %int_1
%147 = OpLoad %int %146
OpStore %148 %147
%149 = OpFunctionCall %void %keepAlive_vi %148
%150 = OpLoad %int %148
OpStore %146 %150
%151 = OpAccessChain %_ptr_Function_int %ai %int_0
%152 = OpLoad %int %151
OpStore %153 %152
%154 = OpFunctionCall %void %keepAlive_vi %153
%155 = OpLoad %int %153
OpStore %151 %155
%156 = OpAccessChain %_ptr_Function_v4int %ai4 %int_0
%157 = OpAccessChain %_ptr_Function_int %156 %int_0
%158 = OpLoad %int %157
OpStore %159 %158
%160 = OpFunctionCall %void %keepAlive_vi %159
%161 = OpLoad %int %159
OpStore %157 %161
%162 = OpAccessChain %_ptr_Function_float %x %int_1
%163 = OpLoad %float %162
OpStore %164 %163
%165 = OpFunctionCall %void %keepAlive_vh %164
%166 = OpLoad %float %164
OpStore %162 %166
%167 = OpAccessChain %_ptr_Function_float %s %int_0
%168 = OpLoad %float %167
OpStore %169 %168
%170 = OpFunctionCall %void %keepAlive_vf %169
%171 = OpLoad %float %169
OpStore %167 %171
%172 = OpLoad %float %l
OpStore %173 %172
%174 = OpFunctionCall %void %keepAlive_vh %173
%175 = OpLoad %float %173
OpStore %l %175
%176 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%177 = OpAccessChain %_ptr_Function_float %176 %int_0
%178 = OpLoad %float %177
OpStore %179 %178
%180 = OpFunctionCall %void %keepAlive_vf %179
%181 = OpLoad %float %179
OpStore %177 %181
%182 = OpAccessChain %_ptr_Uniform_v4float %22 %int_0
%184 = OpLoad %v4float %182
OpReturnValue %184
OpFunctionEnd
