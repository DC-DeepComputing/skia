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
OpName %switch_with_if_that_returns_b "switch_with_if_that_returns_b"
OpName %_24_switchValue "_24_switchValue"
OpName %_27_switchLoop "_27_switchLoop"
OpName %switch_with_one_sided_if_then_fallthrough_b "switch_with_one_sided_if_then_fallthrough_b"
OpName %_28_switchValue "_28_switchValue"
OpName %_30_switchFallthrough "_30_switchFallthrough"
OpName %_31_switchLoop "_31_switchLoop"
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
OpDecorate %28 Binding 0
OpDecorate %28 DescriptorSet 0
OpDecorate %47 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%28 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%33 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%37 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%41 = OpTypeFunction %bool
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
%366 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %33
%34 = OpLabel
%38 = OpVariable %_ptr_Function_v2float Function
OpStore %38 %37
%40 = OpFunctionCall %v4float %main %38
OpStore %sk_FragColor %40
OpReturn
OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %41
%42 = OpLabel
%43 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%47 = OpLoad %float %43
%49 = OpFOrdEqual %bool %47 %float_1
OpSelectionMerge %52 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
OpReturnValue %true
%51 = OpLabel
OpReturnValue %true
%52 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body_b = OpFunction %bool None %41
%54 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %58
%58 = OpLabel
OpLoopMerge %62 %61 None
OpBranch %59
%59 = OpLabel
%63 = OpLoad %int %x
%65 = OpSLessThanEqual %bool %63 %int_10
OpBranchConditional %65 %60 %62
%60 = OpLabel
OpReturnValue %true
%61 = OpLabel
%67 = OpLoad %int %x
%68 = OpIAdd %int %67 %int_1
OpStore %x %68
OpBranch %58
%62 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body_b = OpFunction %bool None %41
%69 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %71
%71 = OpLabel
OpLoopMerge %75 %74 None
OpBranch %72
%72 = OpLabel
%76 = OpLoad %int %x_0
%77 = OpSLessThanEqual %bool %76 %int_10
OpBranchConditional %77 %73 %75
%73 = OpLabel
OpBranch %74
%74 = OpLabel
%78 = OpLoad %int %x_0
%79 = OpIAdd %int %78 %int_1
OpStore %x_0 %79
OpBranch %71
%75 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return_b = OpFunction %bool None %41
%80 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %82
%82 = OpLabel
OpLoopMerge %86 %85 None
OpBranch %83
%83 = OpLabel
%87 = OpLoad %int %x_1
%88 = OpSLessThanEqual %bool %87 %int_10
OpBranchConditional %88 %84 %86
%84 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%90 = OpLoad %float %89
%91 = OpFOrdEqual %bool %90 %float_1
OpSelectionMerge %94 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
OpReturnValue %true
%93 = OpLabel
OpReturnValue %true
%94 = OpLabel
OpBranch %85
%85 = OpLabel
%95 = OpLoad %int %x_1
%96 = OpIAdd %int %95 %int_1
OpStore %x_1 %96
OpBranch %82
%86 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain_b = OpFunction %bool None %41
%97 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%99 = OpLoad %float %98
%100 = OpFOrdEqual %bool %99 %float_1
OpSelectionMerge %103 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
OpReturnValue %true
%102 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%105 = OpLoad %float %104
%107 = OpFOrdEqual %bool %105 %float_2
OpSelectionMerge %110 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
OpReturnValue %false
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%113 = OpLoad %float %112
%115 = OpFOrdEqual %bool %113 %float_3
OpSelectionMerge %118 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
OpReturnValue %true
%117 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%120 = OpLoad %float %119
%122 = OpFOrdEqual %bool %120 %float_4
OpSelectionMerge %125 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
OpReturnValue %false
%124 = OpLabel
OpReturnValue %true
%125 = OpLabel
OpBranch %118
%118 = OpLabel
OpBranch %110
%110 = OpLabel
OpBranch %103
%103 = OpLabel
OpUnreachable
OpFunctionEnd
%conditional_inside_while_loop_b = OpFunction %bool None %41
%126 = OpLabel
OpBranch %127
%127 = OpLabel
OpLoopMerge %131 %130 None
OpBranch %128
%128 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%133 = OpLoad %float %132
%135 = OpFOrdEqual %bool %133 %float_123
OpBranchConditional %135 %129 %131
%129 = OpLabel
OpReturnValue %true
%130 = OpLabel
OpBranch %127
%131 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_do_loop_b = OpFunction %bool None %41
%136 = OpLabel
OpBranch %137
%137 = OpLabel
OpLoopMerge %141 %140 None
OpBranch %138
%138 = OpLabel
OpReturnValue %true
%139 = OpLabel
OpBranch %140
%140 = OpLabel
OpBranchConditional %true %137 %141
%141 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_while_loop_b = OpFunction %bool None %41
%142 = OpLabel
OpBranch %143
%143 = OpLabel
OpLoopMerge %147 %146 None
OpBranch %144
%144 = OpLabel
OpBranchConditional %true %145 %147
%145 = OpLabel
OpReturnValue %true
%146 = OpLabel
OpBranch %143
%147 = OpLabel
OpUnreachable
OpFunctionEnd
%after_do_loop_b = OpFunction %bool None %41
%148 = OpLabel
OpBranch %149
%149 = OpLabel
OpLoopMerge %153 %152 None
OpBranch %150
%150 = OpLabel
OpBranch %153
%151 = OpLabel
OpBranch %152
%152 = OpLabel
OpBranchConditional %true %149 %153
%153 = OpLabel
OpReturnValue %true
OpFunctionEnd
%after_while_loop_b = OpFunction %bool None %41
%154 = OpLabel
OpBranch %155
%155 = OpLabel
OpLoopMerge %159 %158 None
OpBranch %156
%156 = OpLabel
OpBranchConditional %true %157 %159
%157 = OpLabel
OpBranch %159
%158 = OpLabel
OpBranch %155
%159 = OpLabel
OpReturnValue %true
OpFunctionEnd
%switch_with_all_returns_b = OpFunction %bool None %41
%160 = OpLabel
%_0_switchValue = OpVariable %_ptr_Function_int Function
%_2_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_3_switchLoop = OpVariable %_ptr_Function_int Function
%162 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%163 = OpLoad %float %162
%164 = OpConvertFToS %int %163
OpStore %_0_switchValue %164
OpStore %_2_switchFallthrough %false
OpStore %_3_switchLoop %int_0
OpBranch %168
%168 = OpLabel
OpLoopMerge %172 %171 None
OpBranch %169
%169 = OpLabel
%173 = OpLoad %int %_3_switchLoop
%174 = OpSLessThan %bool %173 %int_1
OpBranchConditional %174 %170 %172
%170 = OpLabel
%175 = OpLoad %int %_0_switchValue
%176 = OpIEqual %bool %175 %int_1
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
OpReturnValue %true
%178 = OpLabel
%179 = OpLoad %bool %_2_switchFallthrough
OpSelectionMerge %181 None
OpBranchConditional %179 %181 %180
%180 = OpLabel
%182 = OpLoad %int %_0_switchValue
%183 = OpIEqual %bool %182 %int_2
OpBranch %181
%181 = OpLabel
%184 = OpPhi %bool %true %178 %183 %180
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
OpReturnValue %true
%186 = OpLabel
OpReturnValue %true
%171 = OpLabel
%187 = OpLoad %int %_3_switchLoop
%188 = OpIAdd %int %187 %int_1
OpStore %_3_switchLoop %188
OpBranch %168
%172 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_only_default_b = OpFunction %bool None %41
%189 = OpLabel
%_7_switchLoop = OpVariable %_ptr_Function_int Function
OpStore %_7_switchLoop %int_0
OpBranch %191
%191 = OpLabel
OpLoopMerge %195 %194 None
OpBranch %192
%192 = OpLabel
%196 = OpLoad %int %_7_switchLoop
%197 = OpSLessThan %bool %196 %int_1
OpBranchConditional %197 %193 %195
%193 = OpLabel
OpReturnValue %true
%194 = OpLabel
%198 = OpLoad %int %_7_switchLoop
%199 = OpIAdd %int %198 %int_1
OpStore %_7_switchLoop %199
OpBranch %191
%195 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_b = OpFunction %bool None %41
%200 = OpLabel
%_8_switchValue = OpVariable %_ptr_Function_int Function
%_10_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_11_switchLoop = OpVariable %_ptr_Function_int Function
%202 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%203 = OpLoad %float %202
%204 = OpConvertFToS %int %203
OpStore %_8_switchValue %204
OpStore %_10_switchFallthrough %false
OpStore %_11_switchLoop %int_0
OpBranch %207
%207 = OpLabel
OpLoopMerge %211 %210 None
OpBranch %208
%208 = OpLabel
%212 = OpLoad %int %_11_switchLoop
%213 = OpSLessThan %bool %212 %int_1
OpBranchConditional %213 %209 %211
%209 = OpLabel
%214 = OpLoad %int %_8_switchValue
%215 = OpIEqual %bool %214 %int_1
OpSelectionMerge %217 None
OpBranchConditional %215 %216 %217
%216 = OpLabel
OpReturnValue %true
%217 = OpLabel
%218 = OpLoad %bool %_10_switchFallthrough
OpSelectionMerge %220 None
OpBranchConditional %218 %220 %219
%219 = OpLabel
%221 = OpLoad %int %_8_switchValue
%222 = OpIEqual %bool %221 %int_2
OpBranch %220
%220 = OpLabel
%223 = OpPhi %bool %true %217 %222 %219
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
OpStore %_10_switchFallthrough %true
OpBranch %225
%225 = OpLabel
OpReturnValue %true
%210 = OpLabel
%226 = OpLoad %int %_11_switchLoop
%227 = OpIAdd %int %226 %int_1
OpStore %_11_switchLoop %227
OpBranch %207
%211 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_twice_b = OpFunction %bool None %41
%228 = OpLabel
%_12_switchValue = OpVariable %_ptr_Function_int Function
%_14_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_15_switchLoop = OpVariable %_ptr_Function_int Function
%230 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%231 = OpLoad %float %230
%232 = OpConvertFToS %int %231
OpStore %_12_switchValue %232
OpStore %_14_switchFallthrough %false
OpStore %_15_switchLoop %int_0
OpBranch %235
%235 = OpLabel
OpLoopMerge %239 %238 None
OpBranch %236
%236 = OpLabel
%240 = OpLoad %int %_15_switchLoop
%241 = OpSLessThan %bool %240 %int_1
OpBranchConditional %241 %237 %239
%237 = OpLabel
%242 = OpLoad %int %_12_switchValue
%243 = OpIEqual %bool %242 %int_1
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
OpStore %_14_switchFallthrough %true
OpBranch %245
%245 = OpLabel
%246 = OpLoad %bool %_14_switchFallthrough
OpSelectionMerge %248 None
OpBranchConditional %246 %248 %247
%247 = OpLabel
%249 = OpLoad %int %_12_switchValue
%250 = OpIEqual %bool %249 %int_2
OpBranch %248
%248 = OpLabel
%251 = OpPhi %bool %true %245 %250 %247
OpSelectionMerge %253 None
OpBranchConditional %251 %252 %253
%252 = OpLabel
OpStore %_14_switchFallthrough %true
OpBranch %253
%253 = OpLabel
OpReturnValue %true
%238 = OpLabel
%254 = OpLoad %int %_15_switchLoop
%255 = OpIAdd %int %254 %int_1
OpStore %_15_switchLoop %255
OpBranch %235
%239 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_break_in_loop_b = OpFunction %bool None %41
%256 = OpLabel
%_16_switchValue = OpVariable %_ptr_Function_int Function
%_18_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_19_switchLoop = OpVariable %_ptr_Function_int Function
%x_2 = OpVariable %_ptr_Function_int Function
%258 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%259 = OpLoad %float %258
%260 = OpConvertFToS %int %259
OpStore %_16_switchValue %260
OpStore %_18_switchFallthrough %false
OpStore %_19_switchLoop %int_0
OpBranch %263
%263 = OpLabel
OpLoopMerge %267 %266 None
OpBranch %264
%264 = OpLabel
%268 = OpLoad %int %_19_switchLoop
%269 = OpSLessThan %bool %268 %int_1
OpBranchConditional %269 %265 %267
%265 = OpLabel
%270 = OpLoad %int %_16_switchValue
%271 = OpIEqual %bool %270 %int_1
OpSelectionMerge %273 None
OpBranchConditional %271 %272 %273
%272 = OpLabel
OpStore %x_2 %int_0
OpBranch %275
%275 = OpLabel
OpLoopMerge %279 %278 None
OpBranch %276
%276 = OpLabel
%280 = OpLoad %int %x_2
%281 = OpSLessThanEqual %bool %280 %int_10
OpBranchConditional %281 %277 %279
%277 = OpLabel
OpBranch %279
%278 = OpLabel
%282 = OpLoad %int %x_2
%283 = OpIAdd %int %282 %int_1
OpStore %x_2 %283
OpBranch %275
%279 = OpLabel
OpStore %_18_switchFallthrough %true
OpBranch %273
%273 = OpLabel
OpReturnValue %true
%266 = OpLabel
%284 = OpLoad %int %_19_switchLoop
%285 = OpIAdd %int %284 %int_1
OpStore %_19_switchLoop %285
OpBranch %263
%267 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_continue_in_loop_b = OpFunction %bool None %41
%286 = OpLabel
%_20_switchValue = OpVariable %_ptr_Function_int Function
%_22_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_23_switchLoop = OpVariable %_ptr_Function_int Function
%x_3 = OpVariable %_ptr_Function_int Function
%288 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%289 = OpLoad %float %288
%290 = OpConvertFToS %int %289
OpStore %_20_switchValue %290
OpStore %_22_switchFallthrough %false
OpStore %_23_switchLoop %int_0
OpBranch %293
%293 = OpLabel
OpLoopMerge %297 %296 None
OpBranch %294
%294 = OpLabel
%298 = OpLoad %int %_23_switchLoop
%299 = OpSLessThan %bool %298 %int_1
OpBranchConditional %299 %295 %297
%295 = OpLabel
%300 = OpLoad %int %_20_switchValue
%301 = OpIEqual %bool %300 %int_1
OpSelectionMerge %303 None
OpBranchConditional %301 %302 %303
%302 = OpLabel
OpStore %x_3 %int_0
OpBranch %305
%305 = OpLabel
OpLoopMerge %309 %308 None
OpBranch %306
%306 = OpLabel
%310 = OpLoad %int %x_3
%311 = OpSLessThanEqual %bool %310 %int_10
OpBranchConditional %311 %307 %309
%307 = OpLabel
OpBranch %308
%308 = OpLabel
%312 = OpLoad %int %x_3
%313 = OpIAdd %int %312 %int_1
OpStore %x_3 %313
OpBranch %305
%309 = OpLabel
OpStore %_22_switchFallthrough %true
OpBranch %303
%303 = OpLabel
OpReturnValue %true
%296 = OpLabel
%314 = OpLoad %int %_23_switchLoop
%315 = OpIAdd %int %314 %int_1
OpStore %_23_switchLoop %315
OpBranch %293
%297 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_if_that_returns_b = OpFunction %bool None %41
%316 = OpLabel
%_24_switchValue = OpVariable %_ptr_Function_int Function
%_27_switchLoop = OpVariable %_ptr_Function_int Function
%318 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%319 = OpLoad %float %318
%320 = OpConvertFToS %int %319
OpStore %_24_switchValue %320
OpStore %_27_switchLoop %int_0
OpBranch %322
%322 = OpLabel
OpLoopMerge %326 %325 None
OpBranch %323
%323 = OpLabel
%327 = OpLoad %int %_27_switchLoop
%328 = OpSLessThan %bool %327 %int_1
OpBranchConditional %328 %324 %326
%324 = OpLabel
%329 = OpLoad %int %_24_switchValue
%330 = OpIEqual %bool %329 %int_1
OpSelectionMerge %332 None
OpBranchConditional %330 %331 %332
%331 = OpLabel
%333 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%334 = OpLoad %float %333
%335 = OpFOrdEqual %bool %334 %float_123
OpSelectionMerge %338 None
OpBranchConditional %335 %336 %337
%336 = OpLabel
OpReturnValue %true
%337 = OpLabel
OpReturnValue %true
%338 = OpLabel
OpBranch %332
%332 = OpLabel
OpReturnValue %true
%325 = OpLabel
%339 = OpLoad %int %_27_switchLoop
%340 = OpIAdd %int %339 %int_1
OpStore %_27_switchLoop %340
OpBranch %322
%326 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_one_sided_if_then_fallthrough_b = OpFunction %bool None %41
%341 = OpLabel
%_28_switchValue = OpVariable %_ptr_Function_int Function
%_30_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_31_switchLoop = OpVariable %_ptr_Function_int Function
%343 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%344 = OpLoad %float %343
%345 = OpConvertFToS %int %344
OpStore %_28_switchValue %345
OpStore %_30_switchFallthrough %false
OpStore %_31_switchLoop %int_0
OpBranch %348
%348 = OpLabel
OpLoopMerge %352 %351 None
OpBranch %349
%349 = OpLabel
%353 = OpLoad %int %_31_switchLoop
%354 = OpSLessThan %bool %353 %int_1
OpBranchConditional %354 %350 %352
%350 = OpLabel
%355 = OpLoad %int %_28_switchValue
%356 = OpIEqual %bool %355 %int_1
OpSelectionMerge %358 None
OpBranchConditional %356 %357 %358
%357 = OpLabel
%359 = OpAccessChain %_ptr_Uniform_float %28 %int_2
%360 = OpLoad %float %359
%361 = OpFOrdEqual %bool %360 %float_123
OpSelectionMerge %363 None
OpBranchConditional %361 %362 %363
%362 = OpLabel
OpReturnValue %true
%363 = OpLabel
OpStore %_30_switchFallthrough %true
OpBranch %358
%358 = OpLabel
OpReturnValue %true
%351 = OpLabel
%364 = OpLoad %int %_31_switchLoop
%365 = OpIAdd %int %364 %int_1
OpStore %_31_switchLoop %365
OpBranch %348
%352 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %366
%367 = OpFunctionParameter %_ptr_Function_v2float
%368 = OpLabel
%441 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %370 None
OpBranchConditional %true %369 %370
%369 = OpLabel
%371 = OpFunctionCall %bool %return_on_both_sides_b
OpBranch %370
%370 = OpLabel
%372 = OpPhi %bool %false %368 %371 %369
OpSelectionMerge %374 None
OpBranchConditional %372 %373 %374
%373 = OpLabel
%375 = OpFunctionCall %bool %for_inside_body_b
OpBranch %374
%374 = OpLabel
%376 = OpPhi %bool %false %370 %375 %373
OpSelectionMerge %378 None
OpBranchConditional %376 %377 %378
%377 = OpLabel
%379 = OpFunctionCall %bool %after_for_body_b
OpBranch %378
%378 = OpLabel
%380 = OpPhi %bool %false %374 %379 %377
OpSelectionMerge %382 None
OpBranchConditional %380 %381 %382
%381 = OpLabel
%383 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
OpBranch %382
%382 = OpLabel
%384 = OpPhi %bool %false %378 %383 %381
OpSelectionMerge %386 None
OpBranchConditional %384 %385 %386
%385 = OpLabel
%387 = OpFunctionCall %bool %if_else_chain_b
OpBranch %386
%386 = OpLabel
%388 = OpPhi %bool %false %382 %387 %385
OpSelectionMerge %390 None
OpBranchConditional %388 %389 %390
%389 = OpLabel
%391 = OpFunctionCall %bool %conditional_inside_while_loop_b
OpBranch %390
%390 = OpLabel
%392 = OpPhi %bool %false %386 %391 %389
OpSelectionMerge %394 None
OpBranchConditional %392 %393 %394
%393 = OpLabel
%395 = OpFunctionCall %bool %inside_do_loop_b
OpBranch %394
%394 = OpLabel
%396 = OpPhi %bool %false %390 %395 %393
OpSelectionMerge %398 None
OpBranchConditional %396 %397 %398
%397 = OpLabel
%399 = OpFunctionCall %bool %inside_while_loop_b
OpBranch %398
%398 = OpLabel
%400 = OpPhi %bool %false %394 %399 %397
OpSelectionMerge %402 None
OpBranchConditional %400 %401 %402
%401 = OpLabel
%403 = OpFunctionCall %bool %after_do_loop_b
OpBranch %402
%402 = OpLabel
%404 = OpPhi %bool %false %398 %403 %401
OpSelectionMerge %406 None
OpBranchConditional %404 %405 %406
%405 = OpLabel
%407 = OpFunctionCall %bool %after_while_loop_b
OpBranch %406
%406 = OpLabel
%408 = OpPhi %bool %false %402 %407 %405
OpSelectionMerge %410 None
OpBranchConditional %408 %409 %410
%409 = OpLabel
%411 = OpFunctionCall %bool %switch_with_all_returns_b
OpBranch %410
%410 = OpLabel
%412 = OpPhi %bool %false %406 %411 %409
OpSelectionMerge %414 None
OpBranchConditional %412 %413 %414
%413 = OpLabel
%415 = OpFunctionCall %bool %switch_only_default_b
OpBranch %414
%414 = OpLabel
%416 = OpPhi %bool %false %410 %415 %413
OpSelectionMerge %418 None
OpBranchConditional %416 %417 %418
%417 = OpLabel
%419 = OpFunctionCall %bool %switch_fallthrough_b
OpBranch %418
%418 = OpLabel
%420 = OpPhi %bool %false %414 %419 %417
OpSelectionMerge %422 None
OpBranchConditional %420 %421 %422
%421 = OpLabel
%423 = OpFunctionCall %bool %switch_fallthrough_twice_b
OpBranch %422
%422 = OpLabel
%424 = OpPhi %bool %false %418 %423 %421
OpSelectionMerge %426 None
OpBranchConditional %424 %425 %426
%425 = OpLabel
%427 = OpFunctionCall %bool %switch_with_break_in_loop_b
OpBranch %426
%426 = OpLabel
%428 = OpPhi %bool %false %422 %427 %425
OpSelectionMerge %430 None
OpBranchConditional %428 %429 %430
%429 = OpLabel
%431 = OpFunctionCall %bool %switch_with_continue_in_loop_b
OpBranch %430
%430 = OpLabel
%432 = OpPhi %bool %false %426 %431 %429
OpSelectionMerge %434 None
OpBranchConditional %432 %433 %434
%433 = OpLabel
%435 = OpFunctionCall %bool %switch_with_if_that_returns_b
OpBranch %434
%434 = OpLabel
%436 = OpPhi %bool %false %430 %435 %433
OpSelectionMerge %438 None
OpBranchConditional %436 %437 %438
%437 = OpLabel
%439 = OpFunctionCall %bool %switch_with_one_sided_if_then_fallthrough_b
OpBranch %438
%438 = OpLabel
%440 = OpPhi %bool %false %434 %439 %437
OpSelectionMerge %445 None
OpBranchConditional %440 %443 %444
%443 = OpLabel
%446 = OpAccessChain %_ptr_Uniform_v4float %28 %int_0
%448 = OpLoad %v4float %446
OpStore %441 %448
OpBranch %445
%444 = OpLabel
%449 = OpAccessChain %_ptr_Uniform_v4float %28 %int_1
%450 = OpLoad %v4float %449
OpStore %441 %450
OpBranch %445
%445 = OpLabel
%451 = OpLoad %v4float %441
OpReturnValue %451
OpFunctionEnd
