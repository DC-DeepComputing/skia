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
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %m1 "m1"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m7 "m7"
OpName %m9 "m9"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %h4 "h4"
OpName %test_comma_b "test_comma_b"
OpName %x "x"
OpName %y "y"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m1 "_1_m1"
OpName %_2_m3 "_2_m3"
OpName %_3_m4 "_3_m4"
OpName %_4_m5 "_4_m5"
OpName %_5_m7 "_5_m7"
OpName %_6_m9 "_6_m9"
OpName %_7_m10 "_7_m10"
OpName %_8_m11 "_8_m11"
OpName %_9_f4 "_9_f4"
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
OpDecorate %m1 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %h4 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %505 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %509 RelaxedPrecision
OpDecorate %510 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %514 RelaxedPrecision
OpDecorate %516 RelaxedPrecision
OpDecorate %517 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %522 RelaxedPrecision
OpDecorate %541 RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %582 RelaxedPrecision
OpDecorate %606 RelaxedPrecision
OpDecorate %632 RelaxedPrecision
OpDecorate %653 RelaxedPrecision
OpDecorate %675 RelaxedPrecision
OpDecorate %704 RelaxedPrecision
OpDecorate %754 RelaxedPrecision
OpDecorate %785 RelaxedPrecision
OpDecorate %786 RelaxedPrecision
OpDecorate %788 RelaxedPrecision
OpDecorate %789 RelaxedPrecision
OpDecorate %790 RelaxedPrecision
OpDecorate %792 RelaxedPrecision
OpDecorate %793 RelaxedPrecision
OpDecorate %794 RelaxedPrecision
OpDecorate %796 RelaxedPrecision
OpDecorate %797 RelaxedPrecision
OpDecorate %798 RelaxedPrecision
OpDecorate %800 RelaxedPrecision
OpDecorate %822 RelaxedPrecision
OpDecorate %866 RelaxedPrecision
OpDecorate %919 RelaxedPrecision
OpDecorate %933 RelaxedPrecision
OpDecorate %935 RelaxedPrecision
OpDecorate %936 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
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
%25 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%float_7 = OpConstant %float 7
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_5 = OpConstant %float 0.5
%499 = OpTypeFunction %v4float %_ptr_Function_v2float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %25
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m7 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
%h4 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%33 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%37 = OpLoad %v4float %33
%38 = OpCompositeExtract %float %37 1
%39 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%41 = OpLoad %v4float %39
%42 = OpCompositeExtract %float %41 0
%44 = OpFMul %float %42 %float_2
%45 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%46 = OpLoad %v4float %45
%47 = OpCompositeExtract %float %46 1
%49 = OpFMul %float %47 %float_3
%50 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%51 = OpLoad %v4float %50
%52 = OpCompositeExtract %float %51 0
%54 = OpFMul %float %52 %float_4
%55 = OpCompositeConstruct %v2float %38 %44
%56 = OpCompositeConstruct %v2float %49 %54
%57 = OpCompositeConstruct %mat2v2float %55 %56
OpStore %m1 %57
%59 = OpLoad %bool %ok
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpLoad %mat2v2float %m1
%64 = OpCompositeConstruct %v2float %float_1 %float_2
%65 = OpCompositeConstruct %v2float %float_3 %float_4
%66 = OpCompositeConstruct %mat2v2float %64 %65
%68 = OpCompositeExtract %v2float %62 0
%69 = OpCompositeExtract %v2float %66 0
%70 = OpFOrdEqual %v2bool %68 %69
%71 = OpAll %bool %70
%72 = OpCompositeExtract %v2float %62 1
%73 = OpCompositeExtract %v2float %66 1
%74 = OpFOrdEqual %v2bool %72 %73
%75 = OpAll %bool %74
%76 = OpLogicalAnd %bool %71 %75
OpBranch %61
%61 = OpLabel
%77 = OpPhi %bool %false %26 %76 %60
OpStore %ok %77
%79 = OpLoad %mat2v2float %m1
OpStore %m3 %79
%80 = OpLoad %bool %ok
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpLoad %mat2v2float %m3
%84 = OpCompositeConstruct %v2float %float_1 %float_2
%85 = OpCompositeConstruct %v2float %float_3 %float_4
%86 = OpCompositeConstruct %mat2v2float %84 %85
%87 = OpCompositeExtract %v2float %83 0
%88 = OpCompositeExtract %v2float %86 0
%89 = OpFOrdEqual %v2bool %87 %88
%90 = OpAll %bool %89
%91 = OpCompositeExtract %v2float %83 1
%92 = OpCompositeExtract %v2float %86 1
%93 = OpFOrdEqual %v2bool %91 %92
%94 = OpAll %bool %93
%95 = OpLogicalAnd %bool %90 %94
OpBranch %82
%82 = OpLabel
%96 = OpPhi %bool %false %61 %95 %81
OpStore %ok %96
%100 = OpCompositeConstruct %v2float %float_6 %float_0
%101 = OpCompositeConstruct %v2float %float_0 %float_6
%99 = OpCompositeConstruct %mat2v2float %100 %101
OpStore %m4 %99
%102 = OpLoad %bool %ok
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %mat2v2float %m4
%106 = OpCompositeConstruct %v2float %float_6 %float_0
%107 = OpCompositeConstruct %v2float %float_0 %float_6
%108 = OpCompositeConstruct %mat2v2float %106 %107
%109 = OpCompositeExtract %v2float %105 0
%110 = OpCompositeExtract %v2float %108 0
%111 = OpFOrdEqual %v2bool %109 %110
%112 = OpAll %bool %111
%113 = OpCompositeExtract %v2float %105 1
%114 = OpCompositeExtract %v2float %108 1
%115 = OpFOrdEqual %v2bool %113 %114
%116 = OpAll %bool %115
%117 = OpLogicalAnd %bool %112 %116
OpBranch %104
%104 = OpLabel
%118 = OpPhi %bool %false %82 %117 %103
OpStore %ok %118
%119 = OpLoad %mat2v2float %m3
%120 = OpLoad %mat2v2float %m4
%121 = OpMatrixTimesMatrix %mat2v2float %119 %120
OpStore %m3 %121
%122 = OpLoad %bool %ok
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpLoad %mat2v2float %m3
%129 = OpCompositeConstruct %v2float %float_6 %float_12
%130 = OpCompositeConstruct %v2float %float_18 %float_24
%131 = OpCompositeConstruct %mat2v2float %129 %130
%132 = OpCompositeExtract %v2float %125 0
%133 = OpCompositeExtract %v2float %131 0
%134 = OpFOrdEqual %v2bool %132 %133
%135 = OpAll %bool %134
%136 = OpCompositeExtract %v2float %125 1
%137 = OpCompositeExtract %v2float %131 1
%138 = OpFOrdEqual %v2bool %136 %137
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %135 %139
OpBranch %124
%124 = OpLabel
%141 = OpPhi %bool %false %104 %140 %123
OpStore %ok %141
%143 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%144 = OpLoad %v2float %143
%145 = OpCompositeExtract %float %144 1
%147 = OpCompositeConstruct %v2float %145 %float_0
%148 = OpCompositeConstruct %v2float %float_0 %145
%146 = OpCompositeConstruct %mat2v2float %147 %148
OpStore %m5 %146
%149 = OpLoad %bool %ok
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%152 = OpLoad %mat2v2float %m5
%153 = OpCompositeConstruct %v2float %float_4 %float_0
%154 = OpCompositeConstruct %v2float %float_0 %float_4
%155 = OpCompositeConstruct %mat2v2float %153 %154
%156 = OpCompositeExtract %v2float %152 0
%157 = OpCompositeExtract %v2float %155 0
%158 = OpFOrdEqual %v2bool %156 %157
%159 = OpAll %bool %158
%160 = OpCompositeExtract %v2float %152 1
%161 = OpCompositeExtract %v2float %155 1
%162 = OpFOrdEqual %v2bool %160 %161
%163 = OpAll %bool %162
%164 = OpLogicalAnd %bool %159 %163
OpBranch %151
%151 = OpLabel
%165 = OpPhi %bool %false %124 %164 %150
OpStore %ok %165
%166 = OpLoad %mat2v2float %m1
%167 = OpLoad %mat2v2float %m5
%168 = OpCompositeExtract %v2float %166 0
%169 = OpCompositeExtract %v2float %167 0
%170 = OpFAdd %v2float %168 %169
%171 = OpCompositeExtract %v2float %166 1
%172 = OpCompositeExtract %v2float %167 1
%173 = OpFAdd %v2float %171 %172
%174 = OpCompositeConstruct %mat2v2float %170 %173
OpStore %m1 %174
%175 = OpLoad %bool %ok
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%178 = OpLoad %mat2v2float %m1
%181 = OpCompositeConstruct %v2float %float_5 %float_2
%182 = OpCompositeConstruct %v2float %float_3 %float_8
%183 = OpCompositeConstruct %mat2v2float %181 %182
%184 = OpCompositeExtract %v2float %178 0
%185 = OpCompositeExtract %v2float %183 0
%186 = OpFOrdEqual %v2bool %184 %185
%187 = OpAll %bool %186
%188 = OpCompositeExtract %v2float %178 1
%189 = OpCompositeExtract %v2float %183 1
%190 = OpFOrdEqual %v2bool %188 %189
%191 = OpAll %bool %190
%192 = OpLogicalAnd %bool %187 %191
OpBranch %177
%177 = OpLabel
%193 = OpPhi %bool %false %151 %192 %176
OpStore %ok %193
%196 = OpCompositeConstruct %v2float %float_5 %float_6
%197 = OpCompositeConstruct %v2float %float_7 %float_8
%198 = OpCompositeConstruct %mat2v2float %196 %197
OpStore %m7 %198
%199 = OpLoad %bool %ok
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
%202 = OpLoad %mat2v2float %m7
%203 = OpCompositeConstruct %v2float %float_5 %float_6
%204 = OpCompositeConstruct %v2float %float_7 %float_8
%205 = OpCompositeConstruct %mat2v2float %203 %204
%206 = OpCompositeExtract %v2float %202 0
%207 = OpCompositeExtract %v2float %205 0
%208 = OpFOrdEqual %v2bool %206 %207
%209 = OpAll %bool %208
%210 = OpCompositeExtract %v2float %202 1
%211 = OpCompositeExtract %v2float %205 1
%212 = OpFOrdEqual %v2bool %210 %211
%213 = OpAll %bool %212
%214 = OpLogicalAnd %bool %209 %213
OpBranch %201
%201 = OpLabel
%215 = OpPhi %bool %false %177 %214 %200
OpStore %ok %215
%222 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%223 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%224 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%221 = OpCompositeConstruct %mat3v3float %222 %223 %224
OpStore %m9 %221
%225 = OpLoad %bool %ok
OpSelectionMerge %227 None
OpBranchConditional %225 %226 %227
%226 = OpLabel
%228 = OpLoad %mat3v3float %m9
%229 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%230 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%231 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%232 = OpCompositeConstruct %mat3v3float %229 %230 %231
%234 = OpCompositeExtract %v3float %228 0
%235 = OpCompositeExtract %v3float %232 0
%236 = OpFOrdEqual %v3bool %234 %235
%237 = OpAll %bool %236
%238 = OpCompositeExtract %v3float %228 1
%239 = OpCompositeExtract %v3float %232 1
%240 = OpFOrdEqual %v3bool %238 %239
%241 = OpAll %bool %240
%242 = OpLogicalAnd %bool %237 %241
%243 = OpCompositeExtract %v3float %228 2
%244 = OpCompositeExtract %v3float %232 2
%245 = OpFOrdEqual %v3bool %243 %244
%246 = OpAll %bool %245
%247 = OpLogicalAnd %bool %242 %246
OpBranch %227
%227 = OpLabel
%248 = OpPhi %bool %false %201 %247 %226
OpStore %ok %248
%254 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%255 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%256 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%257 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%253 = OpCompositeConstruct %mat4v4float %254 %255 %256 %257
OpStore %m10 %253
%258 = OpLoad %bool %ok
OpSelectionMerge %260 None
OpBranchConditional %258 %259 %260
%259 = OpLabel
%261 = OpLoad %mat4v4float %m10
%262 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%263 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%264 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%265 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%266 = OpCompositeConstruct %mat4v4float %262 %263 %264 %265
%268 = OpCompositeExtract %v4float %261 0
%269 = OpCompositeExtract %v4float %266 0
%270 = OpFOrdEqual %v4bool %268 %269
%271 = OpAll %bool %270
%272 = OpCompositeExtract %v4float %261 1
%273 = OpCompositeExtract %v4float %266 1
%274 = OpFOrdEqual %v4bool %272 %273
%275 = OpAll %bool %274
%276 = OpLogicalAnd %bool %271 %275
%277 = OpCompositeExtract %v4float %261 2
%278 = OpCompositeExtract %v4float %266 2
%279 = OpFOrdEqual %v4bool %277 %278
%280 = OpAll %bool %279
%281 = OpLogicalAnd %bool %276 %280
%282 = OpCompositeExtract %v4float %261 3
%283 = OpCompositeExtract %v4float %266 3
%284 = OpFOrdEqual %v4bool %282 %283
%285 = OpAll %bool %284
%286 = OpLogicalAnd %bool %281 %285
OpBranch %260
%260 = OpLabel
%287 = OpPhi %bool %false %227 %286 %259
OpStore %ok %287
%290 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%291 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%292 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%293 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%294 = OpCompositeConstruct %mat4v4float %290 %291 %292 %293
OpStore %m11 %294
%295 = OpLoad %mat4v4float %m11
%296 = OpLoad %mat4v4float %m10
%297 = OpCompositeExtract %v4float %295 0
%298 = OpCompositeExtract %v4float %296 0
%299 = OpFSub %v4float %297 %298
%300 = OpCompositeExtract %v4float %295 1
%301 = OpCompositeExtract %v4float %296 1
%302 = OpFSub %v4float %300 %301
%303 = OpCompositeExtract %v4float %295 2
%304 = OpCompositeExtract %v4float %296 2
%305 = OpFSub %v4float %303 %304
%306 = OpCompositeExtract %v4float %295 3
%307 = OpCompositeExtract %v4float %296 3
%308 = OpFSub %v4float %306 %307
%309 = OpCompositeConstruct %mat4v4float %299 %302 %305 %308
OpStore %m11 %309
%310 = OpLoad %bool %ok
OpSelectionMerge %312 None
OpBranchConditional %310 %311 %312
%311 = OpLabel
%313 = OpLoad %mat4v4float %m11
%314 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%315 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%316 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%317 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%318 = OpCompositeConstruct %mat4v4float %314 %315 %316 %317
%319 = OpCompositeExtract %v4float %313 0
%320 = OpCompositeExtract %v4float %318 0
%321 = OpFOrdEqual %v4bool %319 %320
%322 = OpAll %bool %321
%323 = OpCompositeExtract %v4float %313 1
%324 = OpCompositeExtract %v4float %318 1
%325 = OpFOrdEqual %v4bool %323 %324
%326 = OpAll %bool %325
%327 = OpLogicalAnd %bool %322 %326
%328 = OpCompositeExtract %v4float %313 2
%329 = OpCompositeExtract %v4float %318 2
%330 = OpFOrdEqual %v4bool %328 %329
%331 = OpAll %bool %330
%332 = OpLogicalAnd %bool %327 %331
%333 = OpCompositeExtract %v4float %313 3
%334 = OpCompositeExtract %v4float %318 3
%335 = OpFOrdEqual %v4bool %333 %334
%336 = OpAll %bool %335
%337 = OpLogicalAnd %bool %332 %336
OpBranch %312
%312 = OpLabel
%338 = OpPhi %bool %false %260 %337 %311
OpStore %ok %338
%341 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%342 = OpLoad %v4float %341
%343 = OpCompositeExtract %float %342 1
%344 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%345 = OpLoad %v4float %344
%346 = OpCompositeExtract %float %345 0
%347 = OpFMul %float %346 %float_2
%348 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%349 = OpLoad %v4float %348
%350 = OpCompositeExtract %float %349 1
%351 = OpFMul %float %350 %float_3
%352 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%353 = OpLoad %v4float %352
%354 = OpCompositeExtract %float %353 0
%355 = OpFMul %float %354 %float_4
%356 = OpCompositeConstruct %v4float %343 %347 %351 %355
OpStore %h4 %356
%357 = OpLoad %bool %ok
OpSelectionMerge %359 None
OpBranchConditional %357 %358 %359
%358 = OpLabel
%360 = OpLoad %v4float %h4
%361 = OpVectorShuffle %v2float %360 %360 0 1
%362 = OpLoad %v4float %h4
%363 = OpCompositeExtract %float %362 2
%364 = OpCompositeConstruct %v2float %363 %float_4
%365 = OpCompositeConstruct %mat2v2float %361 %364
%366 = OpCompositeConstruct %v2float %float_1 %float_2
%367 = OpCompositeConstruct %v2float %float_3 %float_4
%368 = OpCompositeConstruct %mat2v2float %366 %367
%369 = OpCompositeExtract %v2float %365 0
%370 = OpCompositeExtract %v2float %368 0
%371 = OpFOrdEqual %v2bool %369 %370
%372 = OpAll %bool %371
%373 = OpCompositeExtract %v2float %365 1
%374 = OpCompositeExtract %v2float %368 1
%375 = OpFOrdEqual %v2bool %373 %374
%376 = OpAll %bool %375
%377 = OpLogicalAnd %bool %372 %376
OpBranch %359
%359 = OpLabel
%378 = OpPhi %bool %false %312 %377 %358
OpStore %ok %378
%379 = OpLoad %bool %ok
OpSelectionMerge %381 None
OpBranchConditional %379 %380 %381
%380 = OpLabel
%382 = OpLoad %v4float %h4
%383 = OpVectorShuffle %v2float %382 %382 0 1
%384 = OpLoad %v4float %h4
%385 = OpCompositeExtract %float %384 2
%386 = OpLoad %v4float %h4
%387 = OpCompositeExtract %float %386 3
%388 = OpLoad %v4float %h4
%389 = OpVectorShuffle %v2float %388 %388 0 1
%390 = OpLoad %v4float %h4
%391 = OpVectorShuffle %v2float %390 %390 2 3
%392 = OpLoad %v4float %h4
%393 = OpCompositeExtract %float %392 0
%394 = OpCompositeExtract %float %383 0
%395 = OpCompositeExtract %float %383 1
%396 = OpCompositeConstruct %v3float %394 %395 %385
%397 = OpCompositeExtract %float %389 0
%398 = OpCompositeExtract %float %389 1
%399 = OpCompositeConstruct %v3float %387 %397 %398
%400 = OpCompositeExtract %float %391 0
%401 = OpCompositeExtract %float %391 1
%402 = OpCompositeConstruct %v3float %400 %401 %393
%403 = OpCompositeConstruct %mat3v3float %396 %399 %402
%404 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%405 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%406 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%407 = OpCompositeConstruct %mat3v3float %404 %405 %406
%408 = OpCompositeExtract %v3float %403 0
%409 = OpCompositeExtract %v3float %407 0
%410 = OpFOrdEqual %v3bool %408 %409
%411 = OpAll %bool %410
%412 = OpCompositeExtract %v3float %403 1
%413 = OpCompositeExtract %v3float %407 1
%414 = OpFOrdEqual %v3bool %412 %413
%415 = OpAll %bool %414
%416 = OpLogicalAnd %bool %411 %415
%417 = OpCompositeExtract %v3float %403 2
%418 = OpCompositeExtract %v3float %407 2
%419 = OpFOrdEqual %v3bool %417 %418
%420 = OpAll %bool %419
%421 = OpLogicalAnd %bool %416 %420
OpBranch %381
%381 = OpLabel
%422 = OpPhi %bool %false %359 %421 %380
OpStore %ok %422
%423 = OpLoad %bool %ok
OpSelectionMerge %425 None
OpBranchConditional %423 %424 %425
%424 = OpLabel
%426 = OpLoad %v4float %h4
%427 = OpVectorShuffle %v2float %426 %426 0 1
%428 = OpLoad %v4float %h4
%429 = OpVectorShuffle %v2float %428 %428 2 3
%430 = OpLoad %v4float %h4
%431 = OpVectorShuffle %v3float %430 %430 0 1 2
%432 = OpLoad %v4float %h4
%433 = OpCompositeExtract %float %432 3
%434 = OpLoad %v4float %h4
%435 = OpLoad %v4float %h4
%436 = OpVectorShuffle %v3float %435 %435 1 2 3
%437 = OpCompositeExtract %float %427 0
%438 = OpCompositeExtract %float %427 1
%439 = OpCompositeExtract %float %429 0
%440 = OpCompositeExtract %float %429 1
%441 = OpCompositeConstruct %v4float %437 %438 %439 %440
%442 = OpCompositeExtract %float %431 0
%443 = OpCompositeExtract %float %431 1
%444 = OpCompositeExtract %float %431 2
%445 = OpCompositeConstruct %v4float %442 %443 %444 %433
%446 = OpCompositeExtract %float %436 0
%447 = OpCompositeExtract %float %436 1
%448 = OpCompositeExtract %float %436 2
%449 = OpCompositeConstruct %v4float %float_1 %446 %447 %448
%450 = OpCompositeConstruct %mat4v4float %441 %445 %434 %449
%451 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%452 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%453 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%454 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%455 = OpCompositeConstruct %mat4v4float %451 %452 %453 %454
%456 = OpCompositeExtract %v4float %450 0
%457 = OpCompositeExtract %v4float %455 0
%458 = OpFOrdEqual %v4bool %456 %457
%459 = OpAll %bool %458
%460 = OpCompositeExtract %v4float %450 1
%461 = OpCompositeExtract %v4float %455 1
%462 = OpFOrdEqual %v4bool %460 %461
%463 = OpAll %bool %462
%464 = OpLogicalAnd %bool %459 %463
%465 = OpCompositeExtract %v4float %450 2
%466 = OpCompositeExtract %v4float %455 2
%467 = OpFOrdEqual %v4bool %465 %466
%468 = OpAll %bool %467
%469 = OpLogicalAnd %bool %464 %468
%470 = OpCompositeExtract %v4float %450 3
%471 = OpCompositeExtract %v4float %455 3
%472 = OpFOrdEqual %v4bool %470 %471
%473 = OpAll %bool %472
%474 = OpLogicalAnd %bool %469 %473
OpBranch %425
%425 = OpLabel
%475 = OpPhi %bool %false %381 %474 %424
OpStore %ok %475
%476 = OpLoad %bool %ok
OpReturnValue %476
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%477 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
%480 = OpCompositeConstruct %v2float %float_1 %float_2
%481 = OpCompositeConstruct %v2float %float_3 %float_4
%482 = OpCompositeConstruct %mat2v2float %480 %481
OpStore %x %482
%484 = OpCompositeConstruct %v2float %float_2 %float_4
%485 = OpCompositeConstruct %v2float %float_6 %float_8
%486 = OpCompositeConstruct %mat2v2float %484 %485
%487 = OpMatrixTimesScalar %mat2v2float %486 %float_0_5
OpStore %y %487
%488 = OpLoad %mat2v2float %x
%489 = OpLoad %mat2v2float %y
%490 = OpCompositeExtract %v2float %488 0
%491 = OpCompositeExtract %v2float %489 0
%492 = OpFOrdEqual %v2bool %490 %491
%493 = OpAll %bool %492
%494 = OpCompositeExtract %v2float %488 1
%495 = OpCompositeExtract %v2float %489 1
%496 = OpFOrdEqual %v2bool %494 %495
%497 = OpAll %bool %496
%498 = OpLogicalAnd %bool %493 %497
OpReturnValue %498
OpFunctionEnd
%main = OpFunction %v4float None %499
%500 = OpFunctionParameter %_ptr_Function_v2float
%501 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%_9_f4 = OpVariable %_ptr_Function_v4float Function
%928 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%504 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%505 = OpLoad %v4float %504
%506 = OpCompositeExtract %float %505 1
%507 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%508 = OpLoad %v4float %507
%509 = OpCompositeExtract %float %508 0
%510 = OpFMul %float %509 %float_2
%511 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%512 = OpLoad %v4float %511
%513 = OpCompositeExtract %float %512 1
%514 = OpFMul %float %513 %float_3
%515 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%516 = OpLoad %v4float %515
%517 = OpCompositeExtract %float %516 0
%518 = OpFMul %float %517 %float_4
%519 = OpCompositeConstruct %v2float %506 %510
%520 = OpCompositeConstruct %v2float %514 %518
%521 = OpCompositeConstruct %mat2v2float %519 %520
OpStore %_1_m1 %521
%522 = OpLoad %bool %_0_ok
OpSelectionMerge %524 None
OpBranchConditional %522 %523 %524
%523 = OpLabel
%525 = OpLoad %mat2v2float %_1_m1
%526 = OpCompositeConstruct %v2float %float_1 %float_2
%527 = OpCompositeConstruct %v2float %float_3 %float_4
%528 = OpCompositeConstruct %mat2v2float %526 %527
%529 = OpCompositeExtract %v2float %525 0
%530 = OpCompositeExtract %v2float %528 0
%531 = OpFOrdEqual %v2bool %529 %530
%532 = OpAll %bool %531
%533 = OpCompositeExtract %v2float %525 1
%534 = OpCompositeExtract %v2float %528 1
%535 = OpFOrdEqual %v2bool %533 %534
%536 = OpAll %bool %535
%537 = OpLogicalAnd %bool %532 %536
OpBranch %524
%524 = OpLabel
%538 = OpPhi %bool %false %501 %537 %523
OpStore %_0_ok %538
%540 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %540
%541 = OpLoad %bool %_0_ok
OpSelectionMerge %543 None
OpBranchConditional %541 %542 %543
%542 = OpLabel
%544 = OpLoad %mat2v2float %_2_m3
%545 = OpCompositeConstruct %v2float %float_1 %float_2
%546 = OpCompositeConstruct %v2float %float_3 %float_4
%547 = OpCompositeConstruct %mat2v2float %545 %546
%548 = OpCompositeExtract %v2float %544 0
%549 = OpCompositeExtract %v2float %547 0
%550 = OpFOrdEqual %v2bool %548 %549
%551 = OpAll %bool %550
%552 = OpCompositeExtract %v2float %544 1
%553 = OpCompositeExtract %v2float %547 1
%554 = OpFOrdEqual %v2bool %552 %553
%555 = OpAll %bool %554
%556 = OpLogicalAnd %bool %551 %555
OpBranch %543
%543 = OpLabel
%557 = OpPhi %bool %false %524 %556 %542
OpStore %_0_ok %557
%560 = OpCompositeConstruct %v2float %float_6 %float_0
%561 = OpCompositeConstruct %v2float %float_0 %float_6
%559 = OpCompositeConstruct %mat2v2float %560 %561
OpStore %_3_m4 %559
%562 = OpLoad %bool %_0_ok
OpSelectionMerge %564 None
OpBranchConditional %562 %563 %564
%563 = OpLabel
%565 = OpLoad %mat2v2float %_3_m4
%566 = OpCompositeConstruct %v2float %float_6 %float_0
%567 = OpCompositeConstruct %v2float %float_0 %float_6
%568 = OpCompositeConstruct %mat2v2float %566 %567
%569 = OpCompositeExtract %v2float %565 0
%570 = OpCompositeExtract %v2float %568 0
%571 = OpFOrdEqual %v2bool %569 %570
%572 = OpAll %bool %571
%573 = OpCompositeExtract %v2float %565 1
%574 = OpCompositeExtract %v2float %568 1
%575 = OpFOrdEqual %v2bool %573 %574
%576 = OpAll %bool %575
%577 = OpLogicalAnd %bool %572 %576
OpBranch %564
%564 = OpLabel
%578 = OpPhi %bool %false %543 %577 %563
OpStore %_0_ok %578
%579 = OpLoad %mat2v2float %_2_m3
%580 = OpLoad %mat2v2float %_3_m4
%581 = OpMatrixTimesMatrix %mat2v2float %579 %580
OpStore %_2_m3 %581
%582 = OpLoad %bool %_0_ok
OpSelectionMerge %584 None
OpBranchConditional %582 %583 %584
%583 = OpLabel
%585 = OpLoad %mat2v2float %_2_m3
%586 = OpCompositeConstruct %v2float %float_6 %float_12
%587 = OpCompositeConstruct %v2float %float_18 %float_24
%588 = OpCompositeConstruct %mat2v2float %586 %587
%589 = OpCompositeExtract %v2float %585 0
%590 = OpCompositeExtract %v2float %588 0
%591 = OpFOrdEqual %v2bool %589 %590
%592 = OpAll %bool %591
%593 = OpCompositeExtract %v2float %585 1
%594 = OpCompositeExtract %v2float %588 1
%595 = OpFOrdEqual %v2bool %593 %594
%596 = OpAll %bool %595
%597 = OpLogicalAnd %bool %592 %596
OpBranch %584
%584 = OpLabel
%598 = OpPhi %bool %false %564 %597 %583
OpStore %_0_ok %598
%600 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%601 = OpLoad %v2float %600
%602 = OpCompositeExtract %float %601 1
%604 = OpCompositeConstruct %v2float %602 %float_0
%605 = OpCompositeConstruct %v2float %float_0 %602
%603 = OpCompositeConstruct %mat2v2float %604 %605
OpStore %_4_m5 %603
%606 = OpLoad %bool %_0_ok
OpSelectionMerge %608 None
OpBranchConditional %606 %607 %608
%607 = OpLabel
%609 = OpLoad %mat2v2float %_4_m5
%610 = OpCompositeConstruct %v2float %float_4 %float_0
%611 = OpCompositeConstruct %v2float %float_0 %float_4
%612 = OpCompositeConstruct %mat2v2float %610 %611
%613 = OpCompositeExtract %v2float %609 0
%614 = OpCompositeExtract %v2float %612 0
%615 = OpFOrdEqual %v2bool %613 %614
%616 = OpAll %bool %615
%617 = OpCompositeExtract %v2float %609 1
%618 = OpCompositeExtract %v2float %612 1
%619 = OpFOrdEqual %v2bool %617 %618
%620 = OpAll %bool %619
%621 = OpLogicalAnd %bool %616 %620
OpBranch %608
%608 = OpLabel
%622 = OpPhi %bool %false %584 %621 %607
OpStore %_0_ok %622
%623 = OpLoad %mat2v2float %_1_m1
%624 = OpLoad %mat2v2float %_4_m5
%625 = OpCompositeExtract %v2float %623 0
%626 = OpCompositeExtract %v2float %624 0
%627 = OpFAdd %v2float %625 %626
%628 = OpCompositeExtract %v2float %623 1
%629 = OpCompositeExtract %v2float %624 1
%630 = OpFAdd %v2float %628 %629
%631 = OpCompositeConstruct %mat2v2float %627 %630
OpStore %_1_m1 %631
%632 = OpLoad %bool %_0_ok
OpSelectionMerge %634 None
OpBranchConditional %632 %633 %634
%633 = OpLabel
%635 = OpLoad %mat2v2float %_1_m1
%636 = OpCompositeConstruct %v2float %float_5 %float_2
%637 = OpCompositeConstruct %v2float %float_3 %float_8
%638 = OpCompositeConstruct %mat2v2float %636 %637
%639 = OpCompositeExtract %v2float %635 0
%640 = OpCompositeExtract %v2float %638 0
%641 = OpFOrdEqual %v2bool %639 %640
%642 = OpAll %bool %641
%643 = OpCompositeExtract %v2float %635 1
%644 = OpCompositeExtract %v2float %638 1
%645 = OpFOrdEqual %v2bool %643 %644
%646 = OpAll %bool %645
%647 = OpLogicalAnd %bool %642 %646
OpBranch %634
%634 = OpLabel
%648 = OpPhi %bool %false %608 %647 %633
OpStore %_0_ok %648
%650 = OpCompositeConstruct %v2float %float_5 %float_6
%651 = OpCompositeConstruct %v2float %float_7 %float_8
%652 = OpCompositeConstruct %mat2v2float %650 %651
OpStore %_5_m7 %652
%653 = OpLoad %bool %_0_ok
OpSelectionMerge %655 None
OpBranchConditional %653 %654 %655
%654 = OpLabel
%656 = OpLoad %mat2v2float %_5_m7
%657 = OpCompositeConstruct %v2float %float_5 %float_6
%658 = OpCompositeConstruct %v2float %float_7 %float_8
%659 = OpCompositeConstruct %mat2v2float %657 %658
%660 = OpCompositeExtract %v2float %656 0
%661 = OpCompositeExtract %v2float %659 0
%662 = OpFOrdEqual %v2bool %660 %661
%663 = OpAll %bool %662
%664 = OpCompositeExtract %v2float %656 1
%665 = OpCompositeExtract %v2float %659 1
%666 = OpFOrdEqual %v2bool %664 %665
%667 = OpAll %bool %666
%668 = OpLogicalAnd %bool %663 %667
OpBranch %655
%655 = OpLabel
%669 = OpPhi %bool %false %634 %668 %654
OpStore %_0_ok %669
%672 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%673 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%674 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%671 = OpCompositeConstruct %mat3v3float %672 %673 %674
OpStore %_6_m9 %671
%675 = OpLoad %bool %_0_ok
OpSelectionMerge %677 None
OpBranchConditional %675 %676 %677
%676 = OpLabel
%678 = OpLoad %mat3v3float %_6_m9
%679 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%680 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%681 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%682 = OpCompositeConstruct %mat3v3float %679 %680 %681
%683 = OpCompositeExtract %v3float %678 0
%684 = OpCompositeExtract %v3float %682 0
%685 = OpFOrdEqual %v3bool %683 %684
%686 = OpAll %bool %685
%687 = OpCompositeExtract %v3float %678 1
%688 = OpCompositeExtract %v3float %682 1
%689 = OpFOrdEqual %v3bool %687 %688
%690 = OpAll %bool %689
%691 = OpLogicalAnd %bool %686 %690
%692 = OpCompositeExtract %v3float %678 2
%693 = OpCompositeExtract %v3float %682 2
%694 = OpFOrdEqual %v3bool %692 %693
%695 = OpAll %bool %694
%696 = OpLogicalAnd %bool %691 %695
OpBranch %677
%677 = OpLabel
%697 = OpPhi %bool %false %655 %696 %676
OpStore %_0_ok %697
%700 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%701 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%702 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%703 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%699 = OpCompositeConstruct %mat4v4float %700 %701 %702 %703
OpStore %_7_m10 %699
%704 = OpLoad %bool %_0_ok
OpSelectionMerge %706 None
OpBranchConditional %704 %705 %706
%705 = OpLabel
%707 = OpLoad %mat4v4float %_7_m10
%708 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%709 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%710 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%711 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%712 = OpCompositeConstruct %mat4v4float %708 %709 %710 %711
%713 = OpCompositeExtract %v4float %707 0
%714 = OpCompositeExtract %v4float %712 0
%715 = OpFOrdEqual %v4bool %713 %714
%716 = OpAll %bool %715
%717 = OpCompositeExtract %v4float %707 1
%718 = OpCompositeExtract %v4float %712 1
%719 = OpFOrdEqual %v4bool %717 %718
%720 = OpAll %bool %719
%721 = OpLogicalAnd %bool %716 %720
%722 = OpCompositeExtract %v4float %707 2
%723 = OpCompositeExtract %v4float %712 2
%724 = OpFOrdEqual %v4bool %722 %723
%725 = OpAll %bool %724
%726 = OpLogicalAnd %bool %721 %725
%727 = OpCompositeExtract %v4float %707 3
%728 = OpCompositeExtract %v4float %712 3
%729 = OpFOrdEqual %v4bool %727 %728
%730 = OpAll %bool %729
%731 = OpLogicalAnd %bool %726 %730
OpBranch %706
%706 = OpLabel
%732 = OpPhi %bool %false %677 %731 %705
OpStore %_0_ok %732
%734 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%735 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%736 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%737 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%738 = OpCompositeConstruct %mat4v4float %734 %735 %736 %737
OpStore %_8_m11 %738
%739 = OpLoad %mat4v4float %_8_m11
%740 = OpLoad %mat4v4float %_7_m10
%741 = OpCompositeExtract %v4float %739 0
%742 = OpCompositeExtract %v4float %740 0
%743 = OpFSub %v4float %741 %742
%744 = OpCompositeExtract %v4float %739 1
%745 = OpCompositeExtract %v4float %740 1
%746 = OpFSub %v4float %744 %745
%747 = OpCompositeExtract %v4float %739 2
%748 = OpCompositeExtract %v4float %740 2
%749 = OpFSub %v4float %747 %748
%750 = OpCompositeExtract %v4float %739 3
%751 = OpCompositeExtract %v4float %740 3
%752 = OpFSub %v4float %750 %751
%753 = OpCompositeConstruct %mat4v4float %743 %746 %749 %752
OpStore %_8_m11 %753
%754 = OpLoad %bool %_0_ok
OpSelectionMerge %756 None
OpBranchConditional %754 %755 %756
%755 = OpLabel
%757 = OpLoad %mat4v4float %_8_m11
%758 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%759 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%760 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%761 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%762 = OpCompositeConstruct %mat4v4float %758 %759 %760 %761
%763 = OpCompositeExtract %v4float %757 0
%764 = OpCompositeExtract %v4float %762 0
%765 = OpFOrdEqual %v4bool %763 %764
%766 = OpAll %bool %765
%767 = OpCompositeExtract %v4float %757 1
%768 = OpCompositeExtract %v4float %762 1
%769 = OpFOrdEqual %v4bool %767 %768
%770 = OpAll %bool %769
%771 = OpLogicalAnd %bool %766 %770
%772 = OpCompositeExtract %v4float %757 2
%773 = OpCompositeExtract %v4float %762 2
%774 = OpFOrdEqual %v4bool %772 %773
%775 = OpAll %bool %774
%776 = OpLogicalAnd %bool %771 %775
%777 = OpCompositeExtract %v4float %757 3
%778 = OpCompositeExtract %v4float %762 3
%779 = OpFOrdEqual %v4bool %777 %778
%780 = OpAll %bool %779
%781 = OpLogicalAnd %bool %776 %780
OpBranch %756
%756 = OpLabel
%782 = OpPhi %bool %false %706 %781 %755
OpStore %_0_ok %782
%784 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%785 = OpLoad %v4float %784
%786 = OpCompositeExtract %float %785 1
%787 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%788 = OpLoad %v4float %787
%789 = OpCompositeExtract %float %788 0
%790 = OpFMul %float %789 %float_2
%791 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%792 = OpLoad %v4float %791
%793 = OpCompositeExtract %float %792 1
%794 = OpFMul %float %793 %float_3
%795 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%796 = OpLoad %v4float %795
%797 = OpCompositeExtract %float %796 0
%798 = OpFMul %float %797 %float_4
%799 = OpCompositeConstruct %v4float %786 %790 %794 %798
OpStore %_9_f4 %799
%800 = OpLoad %bool %_0_ok
OpSelectionMerge %802 None
OpBranchConditional %800 %801 %802
%801 = OpLabel
%803 = OpLoad %v4float %_9_f4
%804 = OpVectorShuffle %v2float %803 %803 0 1
%805 = OpLoad %v4float %_9_f4
%806 = OpCompositeExtract %float %805 2
%807 = OpCompositeConstruct %v2float %806 %float_4
%808 = OpCompositeConstruct %mat2v2float %804 %807
%809 = OpCompositeConstruct %v2float %float_1 %float_2
%810 = OpCompositeConstruct %v2float %float_3 %float_4
%811 = OpCompositeConstruct %mat2v2float %809 %810
%812 = OpCompositeExtract %v2float %808 0
%813 = OpCompositeExtract %v2float %811 0
%814 = OpFOrdEqual %v2bool %812 %813
%815 = OpAll %bool %814
%816 = OpCompositeExtract %v2float %808 1
%817 = OpCompositeExtract %v2float %811 1
%818 = OpFOrdEqual %v2bool %816 %817
%819 = OpAll %bool %818
%820 = OpLogicalAnd %bool %815 %819
OpBranch %802
%802 = OpLabel
%821 = OpPhi %bool %false %756 %820 %801
OpStore %_0_ok %821
%822 = OpLoad %bool %_0_ok
OpSelectionMerge %824 None
OpBranchConditional %822 %823 %824
%823 = OpLabel
%825 = OpLoad %v4float %_9_f4
%826 = OpVectorShuffle %v2float %825 %825 0 1
%827 = OpLoad %v4float %_9_f4
%828 = OpCompositeExtract %float %827 2
%829 = OpLoad %v4float %_9_f4
%830 = OpCompositeExtract %float %829 3
%831 = OpLoad %v4float %_9_f4
%832 = OpVectorShuffle %v2float %831 %831 0 1
%833 = OpLoad %v4float %_9_f4
%834 = OpVectorShuffle %v2float %833 %833 2 3
%835 = OpLoad %v4float %_9_f4
%836 = OpCompositeExtract %float %835 0
%837 = OpCompositeExtract %float %826 0
%838 = OpCompositeExtract %float %826 1
%839 = OpCompositeConstruct %v3float %837 %838 %828
%840 = OpCompositeExtract %float %832 0
%841 = OpCompositeExtract %float %832 1
%842 = OpCompositeConstruct %v3float %830 %840 %841
%843 = OpCompositeExtract %float %834 0
%844 = OpCompositeExtract %float %834 1
%845 = OpCompositeConstruct %v3float %843 %844 %836
%846 = OpCompositeConstruct %mat3v3float %839 %842 %845
%847 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%848 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%849 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%850 = OpCompositeConstruct %mat3v3float %847 %848 %849
%851 = OpCompositeExtract %v3float %846 0
%852 = OpCompositeExtract %v3float %850 0
%853 = OpFOrdEqual %v3bool %851 %852
%854 = OpAll %bool %853
%855 = OpCompositeExtract %v3float %846 1
%856 = OpCompositeExtract %v3float %850 1
%857 = OpFOrdEqual %v3bool %855 %856
%858 = OpAll %bool %857
%859 = OpLogicalAnd %bool %854 %858
%860 = OpCompositeExtract %v3float %846 2
%861 = OpCompositeExtract %v3float %850 2
%862 = OpFOrdEqual %v3bool %860 %861
%863 = OpAll %bool %862
%864 = OpLogicalAnd %bool %859 %863
OpBranch %824
%824 = OpLabel
%865 = OpPhi %bool %false %802 %864 %823
OpStore %_0_ok %865
%866 = OpLoad %bool %_0_ok
OpSelectionMerge %868 None
OpBranchConditional %866 %867 %868
%867 = OpLabel
%869 = OpLoad %v4float %_9_f4
%870 = OpVectorShuffle %v2float %869 %869 0 1
%871 = OpLoad %v4float %_9_f4
%872 = OpVectorShuffle %v2float %871 %871 2 3
%873 = OpLoad %v4float %_9_f4
%874 = OpVectorShuffle %v3float %873 %873 0 1 2
%875 = OpLoad %v4float %_9_f4
%876 = OpCompositeExtract %float %875 3
%877 = OpLoad %v4float %_9_f4
%878 = OpLoad %v4float %_9_f4
%879 = OpVectorShuffle %v3float %878 %878 1 2 3
%880 = OpCompositeExtract %float %870 0
%881 = OpCompositeExtract %float %870 1
%882 = OpCompositeExtract %float %872 0
%883 = OpCompositeExtract %float %872 1
%884 = OpCompositeConstruct %v4float %880 %881 %882 %883
%885 = OpCompositeExtract %float %874 0
%886 = OpCompositeExtract %float %874 1
%887 = OpCompositeExtract %float %874 2
%888 = OpCompositeConstruct %v4float %885 %886 %887 %876
%889 = OpCompositeExtract %float %879 0
%890 = OpCompositeExtract %float %879 1
%891 = OpCompositeExtract %float %879 2
%892 = OpCompositeConstruct %v4float %float_1 %889 %890 %891
%893 = OpCompositeConstruct %mat4v4float %884 %888 %877 %892
%894 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%895 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%896 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%897 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%898 = OpCompositeConstruct %mat4v4float %894 %895 %896 %897
%899 = OpCompositeExtract %v4float %893 0
%900 = OpCompositeExtract %v4float %898 0
%901 = OpFOrdEqual %v4bool %899 %900
%902 = OpAll %bool %901
%903 = OpCompositeExtract %v4float %893 1
%904 = OpCompositeExtract %v4float %898 1
%905 = OpFOrdEqual %v4bool %903 %904
%906 = OpAll %bool %905
%907 = OpLogicalAnd %bool %902 %906
%908 = OpCompositeExtract %v4float %893 2
%909 = OpCompositeExtract %v4float %898 2
%910 = OpFOrdEqual %v4bool %908 %909
%911 = OpAll %bool %910
%912 = OpLogicalAnd %bool %907 %911
%913 = OpCompositeExtract %v4float %893 3
%914 = OpCompositeExtract %v4float %898 3
%915 = OpFOrdEqual %v4bool %913 %914
%916 = OpAll %bool %915
%917 = OpLogicalAnd %bool %912 %916
OpBranch %868
%868 = OpLabel
%918 = OpPhi %bool %false %824 %917 %867
OpStore %_0_ok %918
%919 = OpLoad %bool %_0_ok
OpSelectionMerge %921 None
OpBranchConditional %919 %920 %921
%920 = OpLabel
%922 = OpFunctionCall %bool %test_half_b
OpBranch %921
%921 = OpLabel
%923 = OpPhi %bool %false %868 %922 %920
OpSelectionMerge %925 None
OpBranchConditional %923 %924 %925
%924 = OpLabel
%926 = OpFunctionCall %bool %test_comma_b
OpBranch %925
%925 = OpLabel
%927 = OpPhi %bool %false %921 %926 %924
OpSelectionMerge %931 None
OpBranchConditional %927 %929 %930
%929 = OpLabel
%932 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%933 = OpLoad %v4float %932
OpStore %928 %933
OpBranch %931
%930 = OpLabel
%934 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%935 = OpLoad %v4float %934
OpStore %928 %935
OpBranch %931
%931 = OpLabel
%936 = OpLoad %v4float %928
OpReturnValue %936
OpFunctionEnd
