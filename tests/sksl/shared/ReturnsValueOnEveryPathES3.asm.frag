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
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %return_on_both_sides_b "return_on_both_sides_b"
OpName %for_inside_body_b "for_inside_body_b"
OpName %x "x"
OpName %after_for_body_b "after_for_body_b"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return_b "for_with_double_sided_conditional_return_b"
OpName %x_1 "x"
OpName %if_else_chain_b "if_else_chain_b"
OpName %conditional_inside_while_loop_b "conditional_inside_while_loop_b"
OpName %inside_do_loop_b "inside_do_loop_b"
OpName %inside_while_loop_b "inside_while_loop_b"
OpName %after_do_loop_b "after_do_loop_b"
OpName %after_while_loop_b "after_while_loop_b"
OpName %switch_with_all_returns_b "switch_with_all_returns_b"
OpName %_0_switchValue "_0_switchValue"
OpName %_2_switchFallthrough "_2_switchFallthrough"
OpName %_3_switchLoop "_3_switchLoop"
OpName %switch_only_default_b "switch_only_default_b"
OpName %_7_switchLoop "_7_switchLoop"
OpName %switch_fallthrough_b "switch_fallthrough_b"
OpName %_8_switchValue "_8_switchValue"
OpName %_10_switchFallthrough "_10_switchFallthrough"
OpName %_11_switchLoop "_11_switchLoop"
OpName %switch_fallthrough_twice_b "switch_fallthrough_twice_b"
OpName %_12_switchValue "_12_switchValue"
OpName %_14_switchFallthrough "_14_switchFallthrough"
OpName %_15_switchLoop "_15_switchLoop"
OpName %switch_with_break_in_loop_b "switch_with_break_in_loop_b"
OpName %_16_switchValue "_16_switchValue"
OpName %_18_switchFallthrough "_18_switchFallthrough"
OpName %_19_switchLoop "_19_switchLoop"
OpName %x_2 "x"
OpName %switch_with_continue_in_loop_b "switch_with_continue_in_loop_b"
OpName %_20_switchValue "_20_switchValue"
OpName %_22_switchFallthrough "_22_switchFallthrough"
OpName %_23_switchLoop "_23_switchLoop"
OpName %x_3 "x"
OpName %loop_with_continue_in_switch_b "loop_with_continue_in_switch_b"
OpName %x_4 "x"
OpName %_24_switchValue "_24_switchValue"
OpName %_25_switchContinue "_25_switchContinue"
OpName %_27_switchLoop "_27_switchLoop"
OpName %switch_with_if_that_returns_b "switch_with_if_that_returns_b"
OpName %_28_switchValue "_28_switchValue"
OpName %_31_switchLoop "_31_switchLoop"
OpName %switch_with_one_sided_if_then_fallthrough_b "switch_with_one_sided_if_then_fallthrough_b"
OpName %_32_switchValue "_32_switchValue"
OpName %_34_switchFallthrough "_34_switchFallthrough"
OpName %_35_switchLoop "_35_switchLoop"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %29 Binding 0
OpDecorate %29 DescriptorSet 0
OpDecorate %48 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %488 RelaxedPrecision
OpDecorate %489 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%29 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%34 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%38 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%42 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_123 = OpConstant %float 123
%_ptr_Function_bool = OpTypePointer Function %bool
%400 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %34
%35 = OpLabel
%39 = OpVariable %_ptr_Function_v2float Function
OpStore %39 %38
%41 = OpFunctionCall %v4float %main %39
OpStore %sk_FragColor %41
OpReturn
OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %42
%43 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%48 = OpLoad %float %44
%50 = OpFOrdEqual %bool %48 %float_1
OpSelectionMerge %53 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
OpReturnValue %true
%52 = OpLabel
OpReturnValue %true
%53 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body_b = OpFunction %bool None %42
%55 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %59
%59 = OpLabel
OpLoopMerge %63 %62 None
OpBranch %60
%60 = OpLabel
%64 = OpLoad %int %x
%66 = OpSLessThanEqual %bool %64 %int_10
OpBranchConditional %66 %61 %63
%61 = OpLabel
OpReturnValue %true
%62 = OpLabel
%68 = OpLoad %int %x
%69 = OpIAdd %int %68 %int_1
OpStore %x %69
OpBranch %59
%63 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body_b = OpFunction %bool None %42
%70 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %72
%72 = OpLabel
OpLoopMerge %76 %75 None
OpBranch %73
%73 = OpLabel
%77 = OpLoad %int %x_0
%78 = OpSLessThanEqual %bool %77 %int_10
OpBranchConditional %78 %74 %76
%74 = OpLabel
OpBranch %75
%75 = OpLabel
%79 = OpLoad %int %x_0
%80 = OpIAdd %int %79 %int_1
OpStore %x_0 %80
OpBranch %72
%76 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return_b = OpFunction %bool None %42
%81 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %83
%83 = OpLabel
OpLoopMerge %87 %86 None
OpBranch %84
%84 = OpLabel
%88 = OpLoad %int %x_1
%89 = OpSLessThanEqual %bool %88 %int_10
OpBranchConditional %89 %85 %87
%85 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%91 = OpLoad %float %90
%92 = OpFOrdEqual %bool %91 %float_1
OpSelectionMerge %95 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
OpReturnValue %true
%94 = OpLabel
OpReturnValue %true
%95 = OpLabel
OpBranch %86
%86 = OpLabel
%96 = OpLoad %int %x_1
%97 = OpIAdd %int %96 %int_1
OpStore %x_1 %97
OpBranch %83
%87 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain_b = OpFunction %bool None %42
%98 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%100 = OpLoad %float %99
%101 = OpFOrdEqual %bool %100 %float_1
OpSelectionMerge %104 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
OpReturnValue %true
%103 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%106 = OpLoad %float %105
%108 = OpFOrdEqual %bool %106 %float_2
OpSelectionMerge %111 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
OpReturnValue %false
%110 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%114 = OpLoad %float %113
%116 = OpFOrdEqual %bool %114 %float_3
OpSelectionMerge %119 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
OpReturnValue %true
%118 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%121 = OpLoad %float %120
%123 = OpFOrdEqual %bool %121 %float_4
OpSelectionMerge %126 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
OpReturnValue %false
%125 = OpLabel
OpReturnValue %true
%126 = OpLabel
OpBranch %119
%119 = OpLabel
OpBranch %111
%111 = OpLabel
OpBranch %104
%104 = OpLabel
OpUnreachable
OpFunctionEnd
%conditional_inside_while_loop_b = OpFunction %bool None %42
%127 = OpLabel
OpBranch %128
%128 = OpLabel
OpLoopMerge %132 %131 None
OpBranch %129
%129 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%134 = OpLoad %float %133
%136 = OpFOrdEqual %bool %134 %float_123
OpBranchConditional %136 %130 %132
%130 = OpLabel
OpReturnValue %true
%131 = OpLabel
OpBranch %128
%132 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_do_loop_b = OpFunction %bool None %42
%137 = OpLabel
OpBranch %138
%138 = OpLabel
OpLoopMerge %142 %141 None
OpBranch %139
%139 = OpLabel
OpReturnValue %true
%140 = OpLabel
OpBranch %141
%141 = OpLabel
OpBranchConditional %true %138 %142
%142 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_while_loop_b = OpFunction %bool None %42
%143 = OpLabel
OpBranch %144
%144 = OpLabel
OpLoopMerge %148 %147 None
OpBranch %145
%145 = OpLabel
OpBranchConditional %true %146 %148
%146 = OpLabel
OpReturnValue %true
%147 = OpLabel
OpBranch %144
%148 = OpLabel
OpUnreachable
OpFunctionEnd
%after_do_loop_b = OpFunction %bool None %42
%149 = OpLabel
OpBranch %150
%150 = OpLabel
OpLoopMerge %154 %153 None
OpBranch %151
%151 = OpLabel
OpBranch %154
%152 = OpLabel
OpBranch %153
%153 = OpLabel
OpBranchConditional %true %150 %154
%154 = OpLabel
OpReturnValue %true
OpFunctionEnd
%after_while_loop_b = OpFunction %bool None %42
%155 = OpLabel
OpBranch %156
%156 = OpLabel
OpLoopMerge %160 %159 None
OpBranch %157
%157 = OpLabel
OpBranchConditional %true %158 %160
%158 = OpLabel
OpBranch %160
%159 = OpLabel
OpBranch %156
%160 = OpLabel
OpReturnValue %true
OpFunctionEnd
%switch_with_all_returns_b = OpFunction %bool None %42
%161 = OpLabel
%_0_switchValue = OpVariable %_ptr_Function_int Function
%_2_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_3_switchLoop = OpVariable %_ptr_Function_int Function
%163 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%164 = OpLoad %float %163
%165 = OpConvertFToS %int %164
OpStore %_0_switchValue %165
OpStore %_2_switchFallthrough %false
OpStore %_3_switchLoop %int_0
OpBranch %169
%169 = OpLabel
OpLoopMerge %173 %172 None
OpBranch %170
%170 = OpLabel
%174 = OpLoad %int %_3_switchLoop
%175 = OpSLessThan %bool %174 %int_1
OpBranchConditional %175 %171 %173
%171 = OpLabel
%176 = OpLoad %int %_0_switchValue
%177 = OpIEqual %bool %176 %int_1
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
OpReturnValue %true
%179 = OpLabel
%180 = OpLoad %bool %_2_switchFallthrough
OpSelectionMerge %182 None
OpBranchConditional %180 %182 %181
%181 = OpLabel
%183 = OpLoad %int %_0_switchValue
%184 = OpIEqual %bool %183 %int_2
OpBranch %182
%182 = OpLabel
%185 = OpPhi %bool %true %179 %184 %181
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
OpReturnValue %true
%187 = OpLabel
OpReturnValue %true
%172 = OpLabel
%188 = OpLoad %int %_3_switchLoop
%189 = OpIAdd %int %188 %int_1
OpStore %_3_switchLoop %189
OpBranch %169
%173 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_only_default_b = OpFunction %bool None %42
%190 = OpLabel
%_7_switchLoop = OpVariable %_ptr_Function_int Function
OpStore %_7_switchLoop %int_0
OpBranch %192
%192 = OpLabel
OpLoopMerge %196 %195 None
OpBranch %193
%193 = OpLabel
%197 = OpLoad %int %_7_switchLoop
%198 = OpSLessThan %bool %197 %int_1
OpBranchConditional %198 %194 %196
%194 = OpLabel
OpReturnValue %true
%195 = OpLabel
%199 = OpLoad %int %_7_switchLoop
%200 = OpIAdd %int %199 %int_1
OpStore %_7_switchLoop %200
OpBranch %192
%196 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_b = OpFunction %bool None %42
%201 = OpLabel
%_8_switchValue = OpVariable %_ptr_Function_int Function
%_10_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_11_switchLoop = OpVariable %_ptr_Function_int Function
%203 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%204 = OpLoad %float %203
%205 = OpConvertFToS %int %204
OpStore %_8_switchValue %205
OpStore %_10_switchFallthrough %false
OpStore %_11_switchLoop %int_0
OpBranch %208
%208 = OpLabel
OpLoopMerge %212 %211 None
OpBranch %209
%209 = OpLabel
%213 = OpLoad %int %_11_switchLoop
%214 = OpSLessThan %bool %213 %int_1
OpBranchConditional %214 %210 %212
%210 = OpLabel
%215 = OpLoad %int %_8_switchValue
%216 = OpIEqual %bool %215 %int_1
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
OpReturnValue %true
%218 = OpLabel
%219 = OpLoad %bool %_10_switchFallthrough
OpSelectionMerge %221 None
OpBranchConditional %219 %221 %220
%220 = OpLabel
%222 = OpLoad %int %_8_switchValue
%223 = OpIEqual %bool %222 %int_2
OpBranch %221
%221 = OpLabel
%224 = OpPhi %bool %true %218 %223 %220
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
OpStore %_10_switchFallthrough %true
OpBranch %226
%226 = OpLabel
OpReturnValue %true
%211 = OpLabel
%227 = OpLoad %int %_11_switchLoop
%228 = OpIAdd %int %227 %int_1
OpStore %_11_switchLoop %228
OpBranch %208
%212 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_twice_b = OpFunction %bool None %42
%229 = OpLabel
%_12_switchValue = OpVariable %_ptr_Function_int Function
%_14_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_15_switchLoop = OpVariable %_ptr_Function_int Function
%231 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%232 = OpLoad %float %231
%233 = OpConvertFToS %int %232
OpStore %_12_switchValue %233
OpStore %_14_switchFallthrough %false
OpStore %_15_switchLoop %int_0
OpBranch %236
%236 = OpLabel
OpLoopMerge %240 %239 None
OpBranch %237
%237 = OpLabel
%241 = OpLoad %int %_15_switchLoop
%242 = OpSLessThan %bool %241 %int_1
OpBranchConditional %242 %238 %240
%238 = OpLabel
%243 = OpLoad %int %_12_switchValue
%244 = OpIEqual %bool %243 %int_1
OpSelectionMerge %246 None
OpBranchConditional %244 %245 %246
%245 = OpLabel
OpStore %_14_switchFallthrough %true
OpBranch %246
%246 = OpLabel
%247 = OpLoad %bool %_14_switchFallthrough
OpSelectionMerge %249 None
OpBranchConditional %247 %249 %248
%248 = OpLabel
%250 = OpLoad %int %_12_switchValue
%251 = OpIEqual %bool %250 %int_2
OpBranch %249
%249 = OpLabel
%252 = OpPhi %bool %true %246 %251 %248
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
OpStore %_14_switchFallthrough %true
OpBranch %254
%254 = OpLabel
OpReturnValue %true
%239 = OpLabel
%255 = OpLoad %int %_15_switchLoop
%256 = OpIAdd %int %255 %int_1
OpStore %_15_switchLoop %256
OpBranch %236
%240 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_break_in_loop_b = OpFunction %bool None %42
%257 = OpLabel
%_16_switchValue = OpVariable %_ptr_Function_int Function
%_18_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_19_switchLoop = OpVariable %_ptr_Function_int Function
%x_2 = OpVariable %_ptr_Function_int Function
%259 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%260 = OpLoad %float %259
%261 = OpConvertFToS %int %260
OpStore %_16_switchValue %261
OpStore %_18_switchFallthrough %false
OpStore %_19_switchLoop %int_0
OpBranch %264
%264 = OpLabel
OpLoopMerge %268 %267 None
OpBranch %265
%265 = OpLabel
%269 = OpLoad %int %_19_switchLoop
%270 = OpSLessThan %bool %269 %int_1
OpBranchConditional %270 %266 %268
%266 = OpLabel
%271 = OpLoad %int %_16_switchValue
%272 = OpIEqual %bool %271 %int_1
OpSelectionMerge %274 None
OpBranchConditional %272 %273 %274
%273 = OpLabel
OpStore %x_2 %int_0
OpBranch %276
%276 = OpLabel
OpLoopMerge %280 %279 None
OpBranch %277
%277 = OpLabel
%281 = OpLoad %int %x_2
%282 = OpSLessThanEqual %bool %281 %int_10
OpBranchConditional %282 %278 %280
%278 = OpLabel
OpBranch %280
%279 = OpLabel
%283 = OpLoad %int %x_2
%284 = OpIAdd %int %283 %int_1
OpStore %x_2 %284
OpBranch %276
%280 = OpLabel
OpStore %_18_switchFallthrough %true
OpBranch %274
%274 = OpLabel
OpReturnValue %true
%267 = OpLabel
%285 = OpLoad %int %_19_switchLoop
%286 = OpIAdd %int %285 %int_1
OpStore %_19_switchLoop %286
OpBranch %264
%268 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_continue_in_loop_b = OpFunction %bool None %42
%287 = OpLabel
%_20_switchValue = OpVariable %_ptr_Function_int Function
%_22_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_23_switchLoop = OpVariable %_ptr_Function_int Function
%x_3 = OpVariable %_ptr_Function_int Function
%289 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%290 = OpLoad %float %289
%291 = OpConvertFToS %int %290
OpStore %_20_switchValue %291
OpStore %_22_switchFallthrough %false
OpStore %_23_switchLoop %int_0
OpBranch %294
%294 = OpLabel
OpLoopMerge %298 %297 None
OpBranch %295
%295 = OpLabel
%299 = OpLoad %int %_23_switchLoop
%300 = OpSLessThan %bool %299 %int_1
OpBranchConditional %300 %296 %298
%296 = OpLabel
%301 = OpLoad %int %_20_switchValue
%302 = OpIEqual %bool %301 %int_1
OpSelectionMerge %304 None
OpBranchConditional %302 %303 %304
%303 = OpLabel
OpStore %x_3 %int_0
OpBranch %306
%306 = OpLabel
OpLoopMerge %310 %309 None
OpBranch %307
%307 = OpLabel
%311 = OpLoad %int %x_3
%312 = OpSLessThanEqual %bool %311 %int_10
OpBranchConditional %312 %308 %310
%308 = OpLabel
OpBranch %309
%309 = OpLabel
%313 = OpLoad %int %x_3
%314 = OpIAdd %int %313 %int_1
OpStore %x_3 %314
OpBranch %306
%310 = OpLabel
OpStore %_22_switchFallthrough %true
OpBranch %304
%304 = OpLabel
OpReturnValue %true
%297 = OpLabel
%315 = OpLoad %int %_23_switchLoop
%316 = OpIAdd %int %315 %int_1
OpStore %_23_switchLoop %316
OpBranch %294
%298 = OpLabel
OpUnreachable
OpFunctionEnd
%loop_with_continue_in_switch_b = OpFunction %bool None %42
%317 = OpLabel
%x_4 = OpVariable %_ptr_Function_int Function
%_24_switchValue = OpVariable %_ptr_Function_int Function
%_25_switchContinue = OpVariable %_ptr_Function_bool Function
%_27_switchLoop = OpVariable %_ptr_Function_int Function
OpStore %x_4 %int_0
OpBranch %319
%319 = OpLabel
OpLoopMerge %323 %322 None
OpBranch %320
%320 = OpLabel
%324 = OpLoad %int %x_4
%325 = OpSLessThanEqual %bool %324 %int_10
OpBranchConditional %325 %321 %323
%321 = OpLabel
%327 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%328 = OpLoad %float %327
%329 = OpConvertFToS %int %328
OpStore %_24_switchValue %329
OpStore %_25_switchContinue %false
OpStore %_27_switchLoop %int_0
OpBranch %332
%332 = OpLabel
OpLoopMerge %336 %335 None
OpBranch %333
%333 = OpLabel
%337 = OpLoad %int %_27_switchLoop
%338 = OpSLessThan %bool %337 %int_1
OpBranchConditional %338 %334 %336
%334 = OpLabel
%339 = OpLoad %int %_24_switchValue
%340 = OpIEqual %bool %339 %int_1
OpSelectionMerge %342 None
OpBranchConditional %340 %341 %342
%341 = OpLabel
OpStore %_25_switchContinue %true
OpBranch %336
%342 = OpLabel
OpReturnValue %true
%335 = OpLabel
%343 = OpLoad %int %_27_switchLoop
%344 = OpIAdd %int %343 %int_1
OpStore %_27_switchLoop %344
OpBranch %332
%336 = OpLabel
%345 = OpLoad %bool %_25_switchContinue
OpSelectionMerge %347 None
OpBranchConditional %345 %346 %347
%346 = OpLabel
OpBranch %322
%347 = OpLabel
OpBranch %322
%322 = OpLabel
%348 = OpLoad %int %x_4
%349 = OpIAdd %int %348 %int_1
OpStore %x_4 %349
OpBranch %319
%323 = OpLabel
OpReturnValue %true
OpFunctionEnd
%switch_with_if_that_returns_b = OpFunction %bool None %42
%350 = OpLabel
%_28_switchValue = OpVariable %_ptr_Function_int Function
%_31_switchLoop = OpVariable %_ptr_Function_int Function
%352 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%353 = OpLoad %float %352
%354 = OpConvertFToS %int %353
OpStore %_28_switchValue %354
OpStore %_31_switchLoop %int_0
OpBranch %356
%356 = OpLabel
OpLoopMerge %360 %359 None
OpBranch %357
%357 = OpLabel
%361 = OpLoad %int %_31_switchLoop
%362 = OpSLessThan %bool %361 %int_1
OpBranchConditional %362 %358 %360
%358 = OpLabel
%363 = OpLoad %int %_28_switchValue
%364 = OpIEqual %bool %363 %int_1
OpSelectionMerge %366 None
OpBranchConditional %364 %365 %366
%365 = OpLabel
%367 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%368 = OpLoad %float %367
%369 = OpFOrdEqual %bool %368 %float_123
OpSelectionMerge %372 None
OpBranchConditional %369 %370 %371
%370 = OpLabel
OpReturnValue %true
%371 = OpLabel
OpReturnValue %true
%372 = OpLabel
OpBranch %366
%366 = OpLabel
OpReturnValue %true
%359 = OpLabel
%373 = OpLoad %int %_31_switchLoop
%374 = OpIAdd %int %373 %int_1
OpStore %_31_switchLoop %374
OpBranch %356
%360 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_one_sided_if_then_fallthrough_b = OpFunction %bool None %42
%375 = OpLabel
%_32_switchValue = OpVariable %_ptr_Function_int Function
%_34_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_35_switchLoop = OpVariable %_ptr_Function_int Function
%377 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%378 = OpLoad %float %377
%379 = OpConvertFToS %int %378
OpStore %_32_switchValue %379
OpStore %_34_switchFallthrough %false
OpStore %_35_switchLoop %int_0
OpBranch %382
%382 = OpLabel
OpLoopMerge %386 %385 None
OpBranch %383
%383 = OpLabel
%387 = OpLoad %int %_35_switchLoop
%388 = OpSLessThan %bool %387 %int_1
OpBranchConditional %388 %384 %386
%384 = OpLabel
%389 = OpLoad %int %_32_switchValue
%390 = OpIEqual %bool %389 %int_1
OpSelectionMerge %392 None
OpBranchConditional %390 %391 %392
%391 = OpLabel
%393 = OpAccessChain %_ptr_Uniform_float %29 %int_2
%394 = OpLoad %float %393
%395 = OpFOrdEqual %bool %394 %float_123
OpSelectionMerge %397 None
OpBranchConditional %395 %396 %397
%396 = OpLabel
OpReturnValue %true
%397 = OpLabel
OpStore %_34_switchFallthrough %true
OpBranch %392
%392 = OpLabel
OpReturnValue %true
%385 = OpLabel
%398 = OpLoad %int %_35_switchLoop
%399 = OpIAdd %int %398 %int_1
OpStore %_35_switchLoop %399
OpBranch %382
%386 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %400
%401 = OpFunctionParameter %_ptr_Function_v2float
%402 = OpLabel
%479 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %404 None
OpBranchConditional %true %403 %404
%403 = OpLabel
%405 = OpFunctionCall %bool %return_on_both_sides_b
OpBranch %404
%404 = OpLabel
%406 = OpPhi %bool %false %402 %405 %403
OpSelectionMerge %408 None
OpBranchConditional %406 %407 %408
%407 = OpLabel
%409 = OpFunctionCall %bool %for_inside_body_b
OpBranch %408
%408 = OpLabel
%410 = OpPhi %bool %false %404 %409 %407
OpSelectionMerge %412 None
OpBranchConditional %410 %411 %412
%411 = OpLabel
%413 = OpFunctionCall %bool %after_for_body_b
OpBranch %412
%412 = OpLabel
%414 = OpPhi %bool %false %408 %413 %411
OpSelectionMerge %416 None
OpBranchConditional %414 %415 %416
%415 = OpLabel
%417 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
OpBranch %416
%416 = OpLabel
%418 = OpPhi %bool %false %412 %417 %415
OpSelectionMerge %420 None
OpBranchConditional %418 %419 %420
%419 = OpLabel
%421 = OpFunctionCall %bool %if_else_chain_b
OpBranch %420
%420 = OpLabel
%422 = OpPhi %bool %false %416 %421 %419
OpSelectionMerge %424 None
OpBranchConditional %422 %423 %424
%423 = OpLabel
%425 = OpFunctionCall %bool %conditional_inside_while_loop_b
OpBranch %424
%424 = OpLabel
%426 = OpPhi %bool %false %420 %425 %423
OpSelectionMerge %428 None
OpBranchConditional %426 %427 %428
%427 = OpLabel
%429 = OpFunctionCall %bool %inside_do_loop_b
OpBranch %428
%428 = OpLabel
%430 = OpPhi %bool %false %424 %429 %427
OpSelectionMerge %432 None
OpBranchConditional %430 %431 %432
%431 = OpLabel
%433 = OpFunctionCall %bool %inside_while_loop_b
OpBranch %432
%432 = OpLabel
%434 = OpPhi %bool %false %428 %433 %431
OpSelectionMerge %436 None
OpBranchConditional %434 %435 %436
%435 = OpLabel
%437 = OpFunctionCall %bool %after_do_loop_b
OpBranch %436
%436 = OpLabel
%438 = OpPhi %bool %false %432 %437 %435
OpSelectionMerge %440 None
OpBranchConditional %438 %439 %440
%439 = OpLabel
%441 = OpFunctionCall %bool %after_while_loop_b
OpBranch %440
%440 = OpLabel
%442 = OpPhi %bool %false %436 %441 %439
OpSelectionMerge %444 None
OpBranchConditional %442 %443 %444
%443 = OpLabel
%445 = OpFunctionCall %bool %switch_with_all_returns_b
OpBranch %444
%444 = OpLabel
%446 = OpPhi %bool %false %440 %445 %443
OpSelectionMerge %448 None
OpBranchConditional %446 %447 %448
%447 = OpLabel
%449 = OpFunctionCall %bool %switch_only_default_b
OpBranch %448
%448 = OpLabel
%450 = OpPhi %bool %false %444 %449 %447
OpSelectionMerge %452 None
OpBranchConditional %450 %451 %452
%451 = OpLabel
%453 = OpFunctionCall %bool %switch_fallthrough_b
OpBranch %452
%452 = OpLabel
%454 = OpPhi %bool %false %448 %453 %451
OpSelectionMerge %456 None
OpBranchConditional %454 %455 %456
%455 = OpLabel
%457 = OpFunctionCall %bool %switch_fallthrough_twice_b
OpBranch %456
%456 = OpLabel
%458 = OpPhi %bool %false %452 %457 %455
OpSelectionMerge %460 None
OpBranchConditional %458 %459 %460
%459 = OpLabel
%461 = OpFunctionCall %bool %switch_with_break_in_loop_b
OpBranch %460
%460 = OpLabel
%462 = OpPhi %bool %false %456 %461 %459
OpSelectionMerge %464 None
OpBranchConditional %462 %463 %464
%463 = OpLabel
%465 = OpFunctionCall %bool %loop_with_continue_in_switch_b
OpBranch %464
%464 = OpLabel
%466 = OpPhi %bool %false %460 %465 %463
OpSelectionMerge %468 None
OpBranchConditional %466 %467 %468
%467 = OpLabel
%469 = OpFunctionCall %bool %switch_with_continue_in_loop_b
OpBranch %468
%468 = OpLabel
%470 = OpPhi %bool %false %464 %469 %467
OpSelectionMerge %472 None
OpBranchConditional %470 %471 %472
%471 = OpLabel
%473 = OpFunctionCall %bool %switch_with_if_that_returns_b
OpBranch %472
%472 = OpLabel
%474 = OpPhi %bool %false %468 %473 %471
OpSelectionMerge %476 None
OpBranchConditional %474 %475 %476
%475 = OpLabel
%477 = OpFunctionCall %bool %switch_with_one_sided_if_then_fallthrough_b
OpBranch %476
%476 = OpLabel
%478 = OpPhi %bool %false %472 %477 %475
OpSelectionMerge %483 None
OpBranchConditional %478 %481 %482
%481 = OpLabel
%484 = OpAccessChain %_ptr_Uniform_v4float %29 %int_0
%486 = OpLoad %v4float %484
OpStore %479 %486
OpBranch %483
%482 = OpLabel
%487 = OpAccessChain %_ptr_Uniform_v4float %29 %int_1
%488 = OpLoad %v4float %487
OpStore %479 %488
OpBranch %483
%483 = OpLabel
%489 = OpLoad %v4float %479
OpReturnValue %489
OpFunctionEnd
