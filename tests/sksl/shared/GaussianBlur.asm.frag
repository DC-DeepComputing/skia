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
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %outputColor_Stage0 RelaxedPrecision
OpDecorate %outputCoverage_Stage0 RelaxedPrecision
OpDecorate %output_Stage1 RelaxedPrecision
OpDecorate %_6_output RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
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
%97 = OpTypeFunction %void
%101 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%105 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%115 = OpConstantComposite %v2float %float_0 %float_0
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
%45 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%47 = OpLoad %v4float %45
%48 = OpVectorShuffle %v2float %47 %47 0 1
%49 = OpFMul %v2float %43 %48
OpStore %_1_inCoord %49
%51 = OpCompositeExtract %float %49 0
%52 = OpAccessChain %_ptr_Function_float %_2_subsetCoord %int_0
OpStore %52 %51
%55 = OpLoad %v2float %_1_inCoord
%56 = OpCompositeExtract %float %55 1
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
%70 = OpLoad %v2float %_1_inCoord
%71 = OpCompositeExtract %float %70 0
%73 = OpFAdd %float %71 %float_0_00100000005
%69 = OpExtInst %float %1 Floor %73
%75 = OpFAdd %float %69 %float_0_5
OpStore %_5_snappedX %75
%78 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%79 = OpLoad %v4float %78
%80 = OpCompositeExtract %float %79 0
%81 = OpFOrdLessThan %bool %75 %80
OpSelectionMerge %83 None
OpBranchConditional %81 %83 %82
%82 = OpLabel
%84 = OpLoad %float %_5_snappedX
%85 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%86 = OpLoad %v4float %85
%87 = OpCompositeExtract %float %86 2
%88 = OpFOrdGreaterThan %bool %84 %87
OpBranch %83
%83 = OpLabel
%89 = OpPhi %bool %true %31 %88 %82
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %4 %int_4
%94 = OpLoad %v4float %93
OpStore %_4_textureColor %94
OpBranch %91
%91 = OpLabel
%95 = OpLoad %v4float %_4_textureColor
OpReturnValue %95
OpFunctionEnd
%main = OpFunction %void None %97
%98 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_6_output = OpVariable %_ptr_Function_v4float Function
%_7_coord = OpVariable %_ptr_Function_v2float Function
%_8_coordSampled = OpVariable %_ptr_Function_v2float Function
%116 = OpVariable %_ptr_Function_v4float Function
%117 = OpVariable %_ptr_Function_v2float Function
%128 = OpVariable %_ptr_Function_v4float Function
%129 = OpVariable %_ptr_Function_v2float Function
%139 = OpVariable %_ptr_Function_v4float Function
%140 = OpVariable %_ptr_Function_v2float Function
%150 = OpVariable %_ptr_Function_v4float Function
%151 = OpVariable %_ptr_Function_v2float Function
%161 = OpVariable %_ptr_Function_v4float Function
%162 = OpVariable %_ptr_Function_v2float Function
%172 = OpVariable %_ptr_Function_v4float Function
%173 = OpVariable %_ptr_Function_v2float Function
%183 = OpVariable %_ptr_Function_v4float Function
%184 = OpVariable %_ptr_Function_v2float Function
%194 = OpVariable %_ptr_Function_v4float Function
%195 = OpVariable %_ptr_Function_v2float Function
%205 = OpVariable %_ptr_Function_v4float Function
%206 = OpVariable %_ptr_Function_v2float Function
%216 = OpVariable %_ptr_Function_v4float Function
%217 = OpVariable %_ptr_Function_v2float Function
%227 = OpVariable %_ptr_Function_v4float Function
%228 = OpVariable %_ptr_Function_v2float Function
%238 = OpVariable %_ptr_Function_v4float Function
%239 = OpVariable %_ptr_Function_v2float Function
%249 = OpVariable %_ptr_Function_v4float Function
%250 = OpVariable %_ptr_Function_v2float Function
%260 = OpVariable %_ptr_Function_v4float Function
%261 = OpVariable %_ptr_Function_v2float Function
%271 = OpVariable %_ptr_Function_v4float Function
%272 = OpVariable %_ptr_Function_v2float Function
%282 = OpVariable %_ptr_Function_v4float Function
%283 = OpVariable %_ptr_Function_v2float Function
%293 = OpVariable %_ptr_Function_v4float Function
%294 = OpVariable %_ptr_Function_v2float Function
%304 = OpVariable %_ptr_Function_v4float Function
%305 = OpVariable %_ptr_Function_v2float Function
%315 = OpVariable %_ptr_Function_v4float Function
%316 = OpVariable %_ptr_Function_v2float Function
%326 = OpVariable %_ptr_Function_v4float Function
%327 = OpVariable %_ptr_Function_v2float Function
%337 = OpVariable %_ptr_Function_v4float Function
%338 = OpVariable %_ptr_Function_v2float Function
%348 = OpVariable %_ptr_Function_v4float Function
%349 = OpVariable %_ptr_Function_v2float Function
%359 = OpVariable %_ptr_Function_v4float Function
%360 = OpVariable %_ptr_Function_v2float Function
%370 = OpVariable %_ptr_Function_v4float Function
%371 = OpVariable %_ptr_Function_v2float Function
%381 = OpVariable %_ptr_Function_v4float Function
%382 = OpVariable %_ptr_Function_v2float Function
OpStore %outputColor_Stage0 %101
OpStore %outputCoverage_Stage0 %101
OpStore %_6_output %105
%107 = OpLoad %v2float %vLocalCoord_Stage0
%109 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%111 = OpLoad %v2float %109
%112 = OpVectorTimesScalar %v2float %111 %float_12
%113 = OpFSub %v2float %107 %112
OpStore %_7_coord %113
OpStore %_8_coordSampled %115
OpStore %_8_coordSampled %113
OpStore %116 %101
OpStore %117 %113
%118 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %116 %117
%120 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%121 = OpLoad %v4float %120
%122 = OpCompositeExtract %float %121 0
%123 = OpVectorTimesScalar %v4float %118 %122
%124 = OpFAdd %v4float %105 %123
OpStore %_6_output %124
%125 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%126 = OpLoad %v2float %125
%127 = OpFAdd %v2float %113 %126
OpStore %_7_coord %127
OpStore %_8_coordSampled %127
OpStore %128 %101
OpStore %129 %127
%130 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %128 %129
%131 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%132 = OpLoad %v4float %131
%133 = OpCompositeExtract %float %132 1
%134 = OpVectorTimesScalar %v4float %130 %133
%135 = OpFAdd %v4float %124 %134
OpStore %_6_output %135
%136 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%137 = OpLoad %v2float %136
%138 = OpFAdd %v2float %127 %137
OpStore %_7_coord %138
OpStore %_8_coordSampled %138
OpStore %139 %101
OpStore %140 %138
%141 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %139 %140
%142 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%143 = OpLoad %v4float %142
%144 = OpCompositeExtract %float %143 2
%145 = OpVectorTimesScalar %v4float %141 %144
%146 = OpFAdd %v4float %135 %145
OpStore %_6_output %146
%147 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%148 = OpLoad %v2float %147
%149 = OpFAdd %v2float %138 %148
OpStore %_7_coord %149
OpStore %_8_coordSampled %149
OpStore %150 %101
OpStore %151 %149
%152 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %150 %151
%153 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%154 = OpLoad %v4float %153
%155 = OpCompositeExtract %float %154 3
%156 = OpVectorTimesScalar %v4float %152 %155
%157 = OpFAdd %v4float %146 %156
OpStore %_6_output %157
%158 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%159 = OpLoad %v2float %158
%160 = OpFAdd %v2float %149 %159
OpStore %_7_coord %160
OpStore %_8_coordSampled %160
OpStore %161 %101
OpStore %162 %160
%163 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %161 %162
%164 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%165 = OpLoad %v4float %164
%166 = OpCompositeExtract %float %165 0
%167 = OpVectorTimesScalar %v4float %163 %166
%168 = OpFAdd %v4float %157 %167
OpStore %_6_output %168
%169 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%170 = OpLoad %v2float %169
%171 = OpFAdd %v2float %160 %170
OpStore %_7_coord %171
OpStore %_8_coordSampled %171
OpStore %172 %101
OpStore %173 %171
%174 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %172 %173
%175 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%176 = OpLoad %v4float %175
%177 = OpCompositeExtract %float %176 1
%178 = OpVectorTimesScalar %v4float %174 %177
%179 = OpFAdd %v4float %168 %178
OpStore %_6_output %179
%180 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%181 = OpLoad %v2float %180
%182 = OpFAdd %v2float %171 %181
OpStore %_7_coord %182
OpStore %_8_coordSampled %182
OpStore %183 %101
OpStore %184 %182
%185 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %183 %184
%186 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%187 = OpLoad %v4float %186
%188 = OpCompositeExtract %float %187 2
%189 = OpVectorTimesScalar %v4float %185 %188
%190 = OpFAdd %v4float %179 %189
OpStore %_6_output %190
%191 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%192 = OpLoad %v2float %191
%193 = OpFAdd %v2float %182 %192
OpStore %_7_coord %193
OpStore %_8_coordSampled %193
OpStore %194 %101
OpStore %195 %193
%196 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %194 %195
%197 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%198 = OpLoad %v4float %197
%199 = OpCompositeExtract %float %198 3
%200 = OpVectorTimesScalar %v4float %196 %199
%201 = OpFAdd %v4float %190 %200
OpStore %_6_output %201
%202 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%203 = OpLoad %v2float %202
%204 = OpFAdd %v2float %193 %203
OpStore %_7_coord %204
OpStore %_8_coordSampled %204
OpStore %205 %101
OpStore %206 %204
%207 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %205 %206
%208 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%209 = OpLoad %v4float %208
%210 = OpCompositeExtract %float %209 0
%211 = OpVectorTimesScalar %v4float %207 %210
%212 = OpFAdd %v4float %201 %211
OpStore %_6_output %212
%213 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%214 = OpLoad %v2float %213
%215 = OpFAdd %v2float %204 %214
OpStore %_7_coord %215
OpStore %_8_coordSampled %215
OpStore %216 %101
OpStore %217 %215
%218 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %216 %217
%219 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%220 = OpLoad %v4float %219
%221 = OpCompositeExtract %float %220 1
%222 = OpVectorTimesScalar %v4float %218 %221
%223 = OpFAdd %v4float %212 %222
OpStore %_6_output %223
%224 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%225 = OpLoad %v2float %224
%226 = OpFAdd %v2float %215 %225
OpStore %_7_coord %226
OpStore %_8_coordSampled %226
OpStore %227 %101
OpStore %228 %226
%229 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %227 %228
%230 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%231 = OpLoad %v4float %230
%232 = OpCompositeExtract %float %231 2
%233 = OpVectorTimesScalar %v4float %229 %232
%234 = OpFAdd %v4float %223 %233
OpStore %_6_output %234
%235 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%236 = OpLoad %v2float %235
%237 = OpFAdd %v2float %226 %236
OpStore %_7_coord %237
OpStore %_8_coordSampled %237
OpStore %238 %101
OpStore %239 %237
%240 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %238 %239
%241 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%242 = OpLoad %v4float %241
%243 = OpCompositeExtract %float %242 3
%244 = OpVectorTimesScalar %v4float %240 %243
%245 = OpFAdd %v4float %234 %244
OpStore %_6_output %245
%246 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%247 = OpLoad %v2float %246
%248 = OpFAdd %v2float %237 %247
OpStore %_7_coord %248
OpStore %_8_coordSampled %248
OpStore %249 %101
OpStore %250 %248
%251 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %249 %250
%252 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%253 = OpLoad %v4float %252
%254 = OpCompositeExtract %float %253 0
%255 = OpVectorTimesScalar %v4float %251 %254
%256 = OpFAdd %v4float %245 %255
OpStore %_6_output %256
%257 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%258 = OpLoad %v2float %257
%259 = OpFAdd %v2float %248 %258
OpStore %_7_coord %259
OpStore %_8_coordSampled %259
OpStore %260 %101
OpStore %261 %259
%262 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %260 %261
%263 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%264 = OpLoad %v4float %263
%265 = OpCompositeExtract %float %264 1
%266 = OpVectorTimesScalar %v4float %262 %265
%267 = OpFAdd %v4float %256 %266
OpStore %_6_output %267
%268 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%269 = OpLoad %v2float %268
%270 = OpFAdd %v2float %259 %269
OpStore %_7_coord %270
OpStore %_8_coordSampled %270
OpStore %271 %101
OpStore %272 %270
%273 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %271 %272
%274 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%275 = OpLoad %v4float %274
%276 = OpCompositeExtract %float %275 2
%277 = OpVectorTimesScalar %v4float %273 %276
%278 = OpFAdd %v4float %267 %277
OpStore %_6_output %278
%279 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%280 = OpLoad %v2float %279
%281 = OpFAdd %v2float %270 %280
OpStore %_7_coord %281
OpStore %_8_coordSampled %281
OpStore %282 %101
OpStore %283 %281
%284 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %282 %283
%285 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%286 = OpLoad %v4float %285
%287 = OpCompositeExtract %float %286 3
%288 = OpVectorTimesScalar %v4float %284 %287
%289 = OpFAdd %v4float %278 %288
OpStore %_6_output %289
%290 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%291 = OpLoad %v2float %290
%292 = OpFAdd %v2float %281 %291
OpStore %_7_coord %292
OpStore %_8_coordSampled %292
OpStore %293 %101
OpStore %294 %292
%295 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %293 %294
%296 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%297 = OpLoad %v4float %296
%298 = OpCompositeExtract %float %297 0
%299 = OpVectorTimesScalar %v4float %295 %298
%300 = OpFAdd %v4float %289 %299
OpStore %_6_output %300
%301 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%302 = OpLoad %v2float %301
%303 = OpFAdd %v2float %292 %302
OpStore %_7_coord %303
OpStore %_8_coordSampled %303
OpStore %304 %101
OpStore %305 %303
%306 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %304 %305
%307 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%308 = OpLoad %v4float %307
%309 = OpCompositeExtract %float %308 1
%310 = OpVectorTimesScalar %v4float %306 %309
%311 = OpFAdd %v4float %300 %310
OpStore %_6_output %311
%312 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%313 = OpLoad %v2float %312
%314 = OpFAdd %v2float %303 %313
OpStore %_7_coord %314
OpStore %_8_coordSampled %314
OpStore %315 %101
OpStore %316 %314
%317 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %315 %316
%318 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%319 = OpLoad %v4float %318
%320 = OpCompositeExtract %float %319 2
%321 = OpVectorTimesScalar %v4float %317 %320
%322 = OpFAdd %v4float %311 %321
OpStore %_6_output %322
%323 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%324 = OpLoad %v2float %323
%325 = OpFAdd %v2float %314 %324
OpStore %_7_coord %325
OpStore %_8_coordSampled %325
OpStore %326 %101
OpStore %327 %325
%328 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %326 %327
%329 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%330 = OpLoad %v4float %329
%331 = OpCompositeExtract %float %330 3
%332 = OpVectorTimesScalar %v4float %328 %331
%333 = OpFAdd %v4float %322 %332
OpStore %_6_output %333
%334 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%335 = OpLoad %v2float %334
%336 = OpFAdd %v2float %325 %335
OpStore %_7_coord %336
OpStore %_8_coordSampled %336
OpStore %337 %101
OpStore %338 %336
%339 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %337 %338
%340 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%341 = OpLoad %v4float %340
%342 = OpCompositeExtract %float %341 0
%343 = OpVectorTimesScalar %v4float %339 %342
%344 = OpFAdd %v4float %333 %343
OpStore %_6_output %344
%345 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%346 = OpLoad %v2float %345
%347 = OpFAdd %v2float %336 %346
OpStore %_7_coord %347
OpStore %_8_coordSampled %347
OpStore %348 %101
OpStore %349 %347
%350 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %348 %349
%351 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%352 = OpLoad %v4float %351
%353 = OpCompositeExtract %float %352 1
%354 = OpVectorTimesScalar %v4float %350 %353
%355 = OpFAdd %v4float %344 %354
OpStore %_6_output %355
%356 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%357 = OpLoad %v2float %356
%358 = OpFAdd %v2float %347 %357
OpStore %_7_coord %358
OpStore %_8_coordSampled %358
OpStore %359 %101
OpStore %360 %358
%361 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %359 %360
%362 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%363 = OpLoad %v4float %362
%364 = OpCompositeExtract %float %363 2
%365 = OpVectorTimesScalar %v4float %361 %364
%366 = OpFAdd %v4float %355 %365
OpStore %_6_output %366
%367 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%368 = OpLoad %v2float %367
%369 = OpFAdd %v2float %358 %368
OpStore %_7_coord %369
OpStore %_8_coordSampled %369
OpStore %370 %101
OpStore %371 %369
%372 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %370 %371
%373 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%374 = OpLoad %v4float %373
%375 = OpCompositeExtract %float %374 3
%376 = OpVectorTimesScalar %v4float %372 %375
%377 = OpFAdd %v4float %366 %376
OpStore %_6_output %377
%378 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%379 = OpLoad %v2float %378
%380 = OpFAdd %v2float %369 %379
OpStore %_7_coord %380
OpStore %_8_coordSampled %380
OpStore %381 %101
OpStore %382 %380
%383 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %381 %382
%384 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%385 = OpLoad %v4float %384
%386 = OpCompositeExtract %float %385 0
%387 = OpVectorTimesScalar %v4float %383 %386
%388 = OpFAdd %v4float %377 %387
OpStore %_6_output %388
%389 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%390 = OpLoad %v2float %389
%391 = OpFAdd %v2float %380 %390
OpStore %_7_coord %391
%392 = OpFMul %v4float %388 %101
OpStore %_6_output %392
OpStore %output_Stage1 %392
%393 = OpFMul %v4float %392 %101
OpStore %sk_FragColor %393
OpReturn
OpFunctionEnd
