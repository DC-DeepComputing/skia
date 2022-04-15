OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %vLocalCoord_Stage0
OpExecutionMode %main OriginUpperLeft
OpName %uniformBuffer "uniformBuffer"
OpMemberName %uniformBuffer 0 "sk_RTAdjust"
OpMemberName %uniformBuffer 1 "uIncrement_Stage1_c0"
OpMemberName %uniformBuffer 2 "uKernel_Stage1_c0"
OpMemberName %uniformBuffer 3 "umatrix_Stage1_c0_c0"
OpMemberName %uniformBuffer 4 "uborder_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 5 "usubset_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 6 "unorm_Stage1_c0_c0_c0"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %uTextureSampler_0_Stage1 "uTextureSampler_0_Stage1"
OpName %vLocalCoord_Stage0 "vLocalCoord_Stage0"
OpName %MatrixEffect_Stage1_c0_c0_h4h4f2 "MatrixEffect_Stage1_c0_c0_h4h4f2"
OpName %_1_inCoord "_1_inCoord"
OpName %_2_subsetCoord "_2_subsetCoord"
OpName %_3_clampedCoord "_3_clampedCoord"
OpName %_4_textureColor "_4_textureColor"
OpName %_5_snappedX "_5_snappedX"
OpName %main "main"
OpName %outputColor_Stage0 "outputColor_Stage0"
OpName %outputCoverage_Stage0 "outputCoverage_Stage0"
OpName %output_Stage1 "output_Stage1"
OpName %_6_output "_6_output"
OpName %_7_coord "_7_coord"
OpName %_8_coordSampled "_8_coordSampled"
OpDecorate %_arr_v4float_int_7 ArrayStride 16
OpMemberDecorate %uniformBuffer 0 Offset 0
OpMemberDecorate %uniformBuffer 1 Offset 16
OpMemberDecorate %uniformBuffer 1 RelaxedPrecision
OpMemberDecorate %uniformBuffer 2 Offset 32
OpMemberDecorate %uniformBuffer 2 RelaxedPrecision
OpMemberDecorate %uniformBuffer 3 Offset 144
OpMemberDecorate %uniformBuffer 3 ColMajor
OpMemberDecorate %uniformBuffer 3 MatrixStride 16
OpMemberDecorate %uniformBuffer 4 Offset 192
OpMemberDecorate %uniformBuffer 4 RelaxedPrecision
OpMemberDecorate %uniformBuffer 5 Offset 208
OpMemberDecorate %uniformBuffer 6 Offset 224
OpDecorate %uniformBuffer Block
OpDecorate %4 Binding 0
OpDecorate %4 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %uTextureSampler_0_Stage1 RelaxedPrecision
OpDecorate %uTextureSampler_0_Stage1 Binding 0
OpDecorate %uTextureSampler_0_Stage1 DescriptorSet 0
OpDecorate %vLocalCoord_Stage0 Location 0
OpDecorate %_4_textureColor RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %outputColor_Stage0 RelaxedPrecision
OpDecorate %outputCoverage_Stage0 RelaxedPrecision
OpDecorate %output_Stage1 RelaxedPrecision
OpDecorate %_6_output RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%v2float = OpTypeVector %float 2
%int = OpTypeInt 32 1
%int_7 = OpConstant %int 7
%_arr_v4float_int_7 = OpTypeArray %v4float %int_7
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%uniformBuffer = OpTypeStruct %v4float %v2float %_arr_v4float_int_7 %mat3v3float %v4float %v4float %v4float
%_ptr_Uniform_uniformBuffer = OpTypePointer Uniform %uniformBuffer
%4 = OpVariable %_ptr_Uniform_uniformBuffer Uniform
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%21 = OpTypeImage %float 2D 0 0 0 1 Unknown
%22 = OpTypeSampledImage %21
%_ptr_UniformConstant_22 = OpTypePointer UniformConstant %22
%uTextureSampler_0_Stage1 = OpVariable %_ptr_UniformConstant_22 UniformConstant
%_ptr_Input_v2float = OpTypePointer Input %v2float
%vLocalCoord_Stage0 = OpVariable %_ptr_Input_v2float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%28 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v2float
%int_3 = OpConstant %int 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%float_1 = OpConstant %float 1
%int_6 = OpConstant %int 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_float = OpTypePointer Function %float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_0_00100000005 = OpConstant %float 0.00100000005
%float_0_5 = OpConstant %float 0.5
%true = OpConstantTrue %bool
%int_5 = OpConstant %int 5
%int_4 = OpConstant %int 4
%void = OpTypeVoid
%96 = OpTypeFunction %void
%100 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%104 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%114 = OpConstantComposite %v2float %float_0 %float_0
%int_2 = OpConstant %int 2
%MatrixEffect_Stage1_c0_c0_h4h4f2 = OpFunction %v4float None %28
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%_1_inCoord = OpVariable %_ptr_Function_v2float Function
%_2_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_3_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_4_textureColor = OpVariable %_ptr_Function_v4float Function
%_5_snappedX = OpVariable %_ptr_Function_float Function
%34 = OpAccessChain %_ptr_Uniform_mat3v3float %4 %int_3
%36 = OpLoad %mat3v3float %34
%37 = OpLoad %v2float %30
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%41 = OpCompositeConstruct %v3float %38 %39 %float_1
%42 = OpMatrixTimesVector %v3float %36 %41
%43 = OpVectorShuffle %v2float %42 %42 0 1
OpStore %_1_inCoord %43
%44 = OpLoad %v2float %_1_inCoord
%46 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%48 = OpLoad %v4float %46
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpFMul %v2float %44 %49
OpStore %_1_inCoord %50
%52 = OpCompositeExtract %float %50 0
%53 = OpAccessChain %_ptr_Function_float %_2_subsetCoord %int_0
OpStore %53 %52
%56 = OpCompositeExtract %float %50 1
%57 = OpAccessChain %_ptr_Function_float %_2_subsetCoord %int_1
OpStore %57 %56
%60 = OpLoad %v2float %_2_subsetCoord
OpStore %_3_clampedCoord %60
%63 = OpLoad %22 %uTextureSampler_0_Stage1
%64 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%65 = OpLoad %v4float %64
%66 = OpVectorShuffle %v2float %65 %65 2 3
%67 = OpFMul %v2float %60 %66
%62 = OpImageSampleImplicitLod %v4float %63 %67
OpStore %_4_textureColor %62
%71 = OpFAdd %float %52 %float_0_00100000005
%69 = OpExtInst %float %1 Floor %71
%73 = OpFAdd %float %69 %float_0_5
OpStore %_5_snappedX %73
%75 = OpLoad %float %_5_snappedX
%77 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%78 = OpLoad %v4float %77
%79 = OpCompositeExtract %float %78 0
%80 = OpFOrdLessThan %bool %75 %79
OpSelectionMerge %82 None
OpBranchConditional %80 %82 %81
%81 = OpLabel
%83 = OpLoad %float %_5_snappedX
%84 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 2
%87 = OpFOrdGreaterThan %bool %83 %86
OpBranch %82
%82 = OpLabel
%88 = OpPhi %bool %true %31 %87 %81
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_v4float %4 %int_4
%93 = OpLoad %v4float %92
OpStore %_4_textureColor %93
OpBranch %90
%90 = OpLabel
%94 = OpLoad %v4float %_4_textureColor
OpReturnValue %94
OpFunctionEnd
%main = OpFunction %void None %96
%97 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_6_output = OpVariable %_ptr_Function_v4float Function
%_7_coord = OpVariable %_ptr_Function_v2float Function
%_8_coordSampled = OpVariable %_ptr_Function_v2float Function
%116 = OpVariable %_ptr_Function_v4float Function
%117 = OpVariable %_ptr_Function_v2float Function
%129 = OpVariable %_ptr_Function_v4float Function
%130 = OpVariable %_ptr_Function_v2float Function
%140 = OpVariable %_ptr_Function_v4float Function
%141 = OpVariable %_ptr_Function_v2float Function
%151 = OpVariable %_ptr_Function_v4float Function
%152 = OpVariable %_ptr_Function_v2float Function
%162 = OpVariable %_ptr_Function_v4float Function
%163 = OpVariable %_ptr_Function_v2float Function
%173 = OpVariable %_ptr_Function_v4float Function
%174 = OpVariable %_ptr_Function_v2float Function
%184 = OpVariable %_ptr_Function_v4float Function
%185 = OpVariable %_ptr_Function_v2float Function
%195 = OpVariable %_ptr_Function_v4float Function
%196 = OpVariable %_ptr_Function_v2float Function
%206 = OpVariable %_ptr_Function_v4float Function
%207 = OpVariable %_ptr_Function_v2float Function
%217 = OpVariable %_ptr_Function_v4float Function
%218 = OpVariable %_ptr_Function_v2float Function
%228 = OpVariable %_ptr_Function_v4float Function
%229 = OpVariable %_ptr_Function_v2float Function
%239 = OpVariable %_ptr_Function_v4float Function
%240 = OpVariable %_ptr_Function_v2float Function
%250 = OpVariable %_ptr_Function_v4float Function
%251 = OpVariable %_ptr_Function_v2float Function
%261 = OpVariable %_ptr_Function_v4float Function
%262 = OpVariable %_ptr_Function_v2float Function
%272 = OpVariable %_ptr_Function_v4float Function
%273 = OpVariable %_ptr_Function_v2float Function
%283 = OpVariable %_ptr_Function_v4float Function
%284 = OpVariable %_ptr_Function_v2float Function
%294 = OpVariable %_ptr_Function_v4float Function
%295 = OpVariable %_ptr_Function_v2float Function
%305 = OpVariable %_ptr_Function_v4float Function
%306 = OpVariable %_ptr_Function_v2float Function
%316 = OpVariable %_ptr_Function_v4float Function
%317 = OpVariable %_ptr_Function_v2float Function
%327 = OpVariable %_ptr_Function_v4float Function
%328 = OpVariable %_ptr_Function_v2float Function
%338 = OpVariable %_ptr_Function_v4float Function
%339 = OpVariable %_ptr_Function_v2float Function
%349 = OpVariable %_ptr_Function_v4float Function
%350 = OpVariable %_ptr_Function_v2float Function
%360 = OpVariable %_ptr_Function_v4float Function
%361 = OpVariable %_ptr_Function_v2float Function
%371 = OpVariable %_ptr_Function_v4float Function
%372 = OpVariable %_ptr_Function_v2float Function
%382 = OpVariable %_ptr_Function_v4float Function
%383 = OpVariable %_ptr_Function_v2float Function
OpStore %outputColor_Stage0 %100
OpStore %outputCoverage_Stage0 %100
OpStore %_6_output %104
%106 = OpLoad %v2float %vLocalCoord_Stage0
%108 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%110 = OpLoad %v2float %108
%111 = OpVectorTimesScalar %v2float %110 %float_12
%112 = OpFSub %v2float %106 %111
OpStore %_7_coord %112
OpStore %_8_coordSampled %114
%115 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %115
OpStore %116 %100
OpStore %117 %115
%118 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %116 %117
%120 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%121 = OpLoad %v4float %120
%122 = OpCompositeExtract %float %121 0
%123 = OpVectorTimesScalar %v4float %118 %122
%124 = OpFAdd %v4float %104 %123
OpStore %_6_output %124
%125 = OpLoad %v2float %_7_coord
%126 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%127 = OpLoad %v2float %126
%128 = OpFAdd %v2float %125 %127
OpStore %_7_coord %128
OpStore %_8_coordSampled %128
OpStore %129 %100
OpStore %130 %128
%131 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %129 %130
%132 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%133 = OpLoad %v4float %132
%134 = OpCompositeExtract %float %133 1
%135 = OpVectorTimesScalar %v4float %131 %134
%136 = OpFAdd %v4float %124 %135
OpStore %_6_output %136
%137 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%138 = OpLoad %v2float %137
%139 = OpFAdd %v2float %128 %138
OpStore %_7_coord %139
OpStore %_8_coordSampled %139
OpStore %140 %100
OpStore %141 %139
%142 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %140 %141
%143 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%144 = OpLoad %v4float %143
%145 = OpCompositeExtract %float %144 2
%146 = OpVectorTimesScalar %v4float %142 %145
%147 = OpFAdd %v4float %136 %146
OpStore %_6_output %147
%148 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%149 = OpLoad %v2float %148
%150 = OpFAdd %v2float %139 %149
OpStore %_7_coord %150
OpStore %_8_coordSampled %150
OpStore %151 %100
OpStore %152 %150
%153 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %151 %152
%154 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%155 = OpLoad %v4float %154
%156 = OpCompositeExtract %float %155 3
%157 = OpVectorTimesScalar %v4float %153 %156
%158 = OpFAdd %v4float %147 %157
OpStore %_6_output %158
%159 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%160 = OpLoad %v2float %159
%161 = OpFAdd %v2float %150 %160
OpStore %_7_coord %161
OpStore %_8_coordSampled %161
OpStore %162 %100
OpStore %163 %161
%164 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %162 %163
%165 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%166 = OpLoad %v4float %165
%167 = OpCompositeExtract %float %166 0
%168 = OpVectorTimesScalar %v4float %164 %167
%169 = OpFAdd %v4float %158 %168
OpStore %_6_output %169
%170 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%171 = OpLoad %v2float %170
%172 = OpFAdd %v2float %161 %171
OpStore %_7_coord %172
OpStore %_8_coordSampled %172
OpStore %173 %100
OpStore %174 %172
%175 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %173 %174
%176 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%177 = OpLoad %v4float %176
%178 = OpCompositeExtract %float %177 1
%179 = OpVectorTimesScalar %v4float %175 %178
%180 = OpFAdd %v4float %169 %179
OpStore %_6_output %180
%181 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%182 = OpLoad %v2float %181
%183 = OpFAdd %v2float %172 %182
OpStore %_7_coord %183
OpStore %_8_coordSampled %183
OpStore %184 %100
OpStore %185 %183
%186 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %184 %185
%187 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%188 = OpLoad %v4float %187
%189 = OpCompositeExtract %float %188 2
%190 = OpVectorTimesScalar %v4float %186 %189
%191 = OpFAdd %v4float %180 %190
OpStore %_6_output %191
%192 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%193 = OpLoad %v2float %192
%194 = OpFAdd %v2float %183 %193
OpStore %_7_coord %194
OpStore %_8_coordSampled %194
OpStore %195 %100
OpStore %196 %194
%197 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %195 %196
%198 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%199 = OpLoad %v4float %198
%200 = OpCompositeExtract %float %199 3
%201 = OpVectorTimesScalar %v4float %197 %200
%202 = OpFAdd %v4float %191 %201
OpStore %_6_output %202
%203 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%204 = OpLoad %v2float %203
%205 = OpFAdd %v2float %194 %204
OpStore %_7_coord %205
OpStore %_8_coordSampled %205
OpStore %206 %100
OpStore %207 %205
%208 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %206 %207
%209 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%210 = OpLoad %v4float %209
%211 = OpCompositeExtract %float %210 0
%212 = OpVectorTimesScalar %v4float %208 %211
%213 = OpFAdd %v4float %202 %212
OpStore %_6_output %213
%214 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%215 = OpLoad %v2float %214
%216 = OpFAdd %v2float %205 %215
OpStore %_7_coord %216
OpStore %_8_coordSampled %216
OpStore %217 %100
OpStore %218 %216
%219 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %217 %218
%220 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%221 = OpLoad %v4float %220
%222 = OpCompositeExtract %float %221 1
%223 = OpVectorTimesScalar %v4float %219 %222
%224 = OpFAdd %v4float %213 %223
OpStore %_6_output %224
%225 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%226 = OpLoad %v2float %225
%227 = OpFAdd %v2float %216 %226
OpStore %_7_coord %227
OpStore %_8_coordSampled %227
OpStore %228 %100
OpStore %229 %227
%230 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %228 %229
%231 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%232 = OpLoad %v4float %231
%233 = OpCompositeExtract %float %232 2
%234 = OpVectorTimesScalar %v4float %230 %233
%235 = OpFAdd %v4float %224 %234
OpStore %_6_output %235
%236 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%237 = OpLoad %v2float %236
%238 = OpFAdd %v2float %227 %237
OpStore %_7_coord %238
OpStore %_8_coordSampled %238
OpStore %239 %100
OpStore %240 %238
%241 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %239 %240
%242 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%243 = OpLoad %v4float %242
%244 = OpCompositeExtract %float %243 3
%245 = OpVectorTimesScalar %v4float %241 %244
%246 = OpFAdd %v4float %235 %245
OpStore %_6_output %246
%247 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%248 = OpLoad %v2float %247
%249 = OpFAdd %v2float %238 %248
OpStore %_7_coord %249
OpStore %_8_coordSampled %249
OpStore %250 %100
OpStore %251 %249
%252 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %250 %251
%253 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%254 = OpLoad %v4float %253
%255 = OpCompositeExtract %float %254 0
%256 = OpVectorTimesScalar %v4float %252 %255
%257 = OpFAdd %v4float %246 %256
OpStore %_6_output %257
%258 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%259 = OpLoad %v2float %258
%260 = OpFAdd %v2float %249 %259
OpStore %_7_coord %260
OpStore %_8_coordSampled %260
OpStore %261 %100
OpStore %262 %260
%263 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %261 %262
%264 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%265 = OpLoad %v4float %264
%266 = OpCompositeExtract %float %265 1
%267 = OpVectorTimesScalar %v4float %263 %266
%268 = OpFAdd %v4float %257 %267
OpStore %_6_output %268
%269 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%270 = OpLoad %v2float %269
%271 = OpFAdd %v2float %260 %270
OpStore %_7_coord %271
OpStore %_8_coordSampled %271
OpStore %272 %100
OpStore %273 %271
%274 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %272 %273
%275 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%276 = OpLoad %v4float %275
%277 = OpCompositeExtract %float %276 2
%278 = OpVectorTimesScalar %v4float %274 %277
%279 = OpFAdd %v4float %268 %278
OpStore %_6_output %279
%280 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%281 = OpLoad %v2float %280
%282 = OpFAdd %v2float %271 %281
OpStore %_7_coord %282
OpStore %_8_coordSampled %282
OpStore %283 %100
OpStore %284 %282
%285 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %283 %284
%286 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%287 = OpLoad %v4float %286
%288 = OpCompositeExtract %float %287 3
%289 = OpVectorTimesScalar %v4float %285 %288
%290 = OpFAdd %v4float %279 %289
OpStore %_6_output %290
%291 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%292 = OpLoad %v2float %291
%293 = OpFAdd %v2float %282 %292
OpStore %_7_coord %293
OpStore %_8_coordSampled %293
OpStore %294 %100
OpStore %295 %293
%296 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %294 %295
%297 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%298 = OpLoad %v4float %297
%299 = OpCompositeExtract %float %298 0
%300 = OpVectorTimesScalar %v4float %296 %299
%301 = OpFAdd %v4float %290 %300
OpStore %_6_output %301
%302 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%303 = OpLoad %v2float %302
%304 = OpFAdd %v2float %293 %303
OpStore %_7_coord %304
OpStore %_8_coordSampled %304
OpStore %305 %100
OpStore %306 %304
%307 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %305 %306
%308 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%309 = OpLoad %v4float %308
%310 = OpCompositeExtract %float %309 1
%311 = OpVectorTimesScalar %v4float %307 %310
%312 = OpFAdd %v4float %301 %311
OpStore %_6_output %312
%313 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%314 = OpLoad %v2float %313
%315 = OpFAdd %v2float %304 %314
OpStore %_7_coord %315
OpStore %_8_coordSampled %315
OpStore %316 %100
OpStore %317 %315
%318 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %316 %317
%319 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%320 = OpLoad %v4float %319
%321 = OpCompositeExtract %float %320 2
%322 = OpVectorTimesScalar %v4float %318 %321
%323 = OpFAdd %v4float %312 %322
OpStore %_6_output %323
%324 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%325 = OpLoad %v2float %324
%326 = OpFAdd %v2float %315 %325
OpStore %_7_coord %326
OpStore %_8_coordSampled %326
OpStore %327 %100
OpStore %328 %326
%329 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %327 %328
%330 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%331 = OpLoad %v4float %330
%332 = OpCompositeExtract %float %331 3
%333 = OpVectorTimesScalar %v4float %329 %332
%334 = OpFAdd %v4float %323 %333
OpStore %_6_output %334
%335 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%336 = OpLoad %v2float %335
%337 = OpFAdd %v2float %326 %336
OpStore %_7_coord %337
OpStore %_8_coordSampled %337
OpStore %338 %100
OpStore %339 %337
%340 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %338 %339
%341 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%342 = OpLoad %v4float %341
%343 = OpCompositeExtract %float %342 0
%344 = OpVectorTimesScalar %v4float %340 %343
%345 = OpFAdd %v4float %334 %344
OpStore %_6_output %345
%346 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%347 = OpLoad %v2float %346
%348 = OpFAdd %v2float %337 %347
OpStore %_7_coord %348
OpStore %_8_coordSampled %348
OpStore %349 %100
OpStore %350 %348
%351 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %349 %350
%352 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%353 = OpLoad %v4float %352
%354 = OpCompositeExtract %float %353 1
%355 = OpVectorTimesScalar %v4float %351 %354
%356 = OpFAdd %v4float %345 %355
OpStore %_6_output %356
%357 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%358 = OpLoad %v2float %357
%359 = OpFAdd %v2float %348 %358
OpStore %_7_coord %359
OpStore %_8_coordSampled %359
OpStore %360 %100
OpStore %361 %359
%362 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %360 %361
%363 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%364 = OpLoad %v4float %363
%365 = OpCompositeExtract %float %364 2
%366 = OpVectorTimesScalar %v4float %362 %365
%367 = OpFAdd %v4float %356 %366
OpStore %_6_output %367
%368 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%369 = OpLoad %v2float %368
%370 = OpFAdd %v2float %359 %369
OpStore %_7_coord %370
OpStore %_8_coordSampled %370
OpStore %371 %100
OpStore %372 %370
%373 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %371 %372
%374 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%375 = OpLoad %v4float %374
%376 = OpCompositeExtract %float %375 3
%377 = OpVectorTimesScalar %v4float %373 %376
%378 = OpFAdd %v4float %367 %377
OpStore %_6_output %378
%379 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%380 = OpLoad %v2float %379
%381 = OpFAdd %v2float %370 %380
OpStore %_7_coord %381
OpStore %_8_coordSampled %381
OpStore %382 %100
OpStore %383 %381
%384 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %382 %383
%385 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%386 = OpLoad %v4float %385
%387 = OpCompositeExtract %float %386 0
%388 = OpVectorTimesScalar %v4float %384 %387
%389 = OpFAdd %v4float %378 %388
OpStore %_6_output %389
%390 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%391 = OpLoad %v2float %390
%392 = OpFAdd %v2float %381 %391
OpStore %_7_coord %392
%393 = OpFMul %v4float %389 %100
OpStore %_6_output %393
OpStore %output_Stage1 %393
%394 = OpFMul %v4float %393 %100
OpStore %sk_FragColor %394
OpReturn
OpFunctionEnd
