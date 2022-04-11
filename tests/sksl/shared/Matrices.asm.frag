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
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %543 RelaxedPrecision
OpDecorate %572 RelaxedPrecision
OpDecorate %589 RelaxedPrecision
OpDecorate %591 RelaxedPrecision
OpDecorate %592 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%37 = OpConstantComposite %v2float %float_1 %float_2
%38 = OpConstantComposite %v2float %float_3 %float_4
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%77 = OpConstantComposite %v2float %float_6 %float_0
%78 = OpConstantComposite %v2float %float_0 %float_6
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%104 = OpConstantComposite %v2float %float_6 %float_12
%105 = OpConstantComposite %v2float %float_18 %float_24
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%130 = OpConstantComposite %v2float %float_4 %float_0
%131 = OpConstantComposite %v2float %float_0 %float_4
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%158 = OpConstantComposite %v2float %float_5 %float_2
%159 = OpConstantComposite %v2float %float_3 %float_8
%float_7 = OpConstant %float 7
%173 = OpConstantComposite %v2float %float_5 %float_6
%174 = OpConstantComposite %v2float %float_7 %float_8
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%197 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%198 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%199 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%226 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%227 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%228 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%229 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%258 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%279 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%280 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%281 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%282 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%308 = OpConstantComposite %v2float %float_1 %float_2
%309 = OpConstantComposite %v2float %float_3 %float_4
%323 = OpTypeFunction %v4float %_ptr_Function_v2float
%363 = OpConstantComposite %v2float %float_6 %float_0
%364 = OpConstantComposite %v2float %float_0 %float_6
%387 = OpConstantComposite %v2float %float_6 %float_12
%388 = OpConstantComposite %v2float %float_18 %float_24
%411 = OpConstantComposite %v2float %float_4 %float_0
%412 = OpConstantComposite %v2float %float_0 %float_4
%437 = OpConstantComposite %v2float %float_5 %float_2
%438 = OpConstantComposite %v2float %float_3 %float_8
%451 = OpConstantComposite %v2float %float_5 %float_6
%452 = OpConstantComposite %v2float %float_7 %float_8
%471 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%472 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%473 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%496 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%497 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%498 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%499 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%526 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%547 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%548 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%549 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%550 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
OpStore %ok %true
%39 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %m1 %39
%41 = OpLoad %bool %ok
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpLoad %mat2v2float %m1
%45 = OpCompositeConstruct %mat2v2float %37 %38
%47 = OpCompositeExtract %v2float %44 0
%48 = OpCompositeExtract %v2float %45 0
%49 = OpFOrdEqual %v2bool %47 %48
%50 = OpAll %bool %49
%51 = OpCompositeExtract %v2float %44 1
%52 = OpCompositeExtract %v2float %45 1
%53 = OpFOrdEqual %v2bool %51 %52
%54 = OpAll %bool %53
%55 = OpLogicalAnd %bool %50 %54
OpBranch %43
%43 = OpLabel
%56 = OpPhi %bool %false %26 %55 %42
OpStore %ok %56
%58 = OpLoad %mat2v2float %m1
OpStore %m3 %58
%59 = OpLoad %bool %ok
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpLoad %mat2v2float %m3
%63 = OpCompositeConstruct %mat2v2float %37 %38
%64 = OpCompositeExtract %v2float %62 0
%65 = OpCompositeExtract %v2float %63 0
%66 = OpFOrdEqual %v2bool %64 %65
%67 = OpAll %bool %66
%68 = OpCompositeExtract %v2float %62 1
%69 = OpCompositeExtract %v2float %63 1
%70 = OpFOrdEqual %v2bool %68 %69
%71 = OpAll %bool %70
%72 = OpLogicalAnd %bool %67 %71
OpBranch %61
%61 = OpLabel
%73 = OpPhi %bool %false %43 %72 %60
OpStore %ok %73
%76 = OpCompositeConstruct %mat2v2float %77 %78
OpStore %m4 %76
%79 = OpLoad %bool %ok
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %mat2v2float %m4
%83 = OpCompositeConstruct %mat2v2float %77 %78
%84 = OpCompositeExtract %v2float %82 0
%85 = OpCompositeExtract %v2float %83 0
%86 = OpFOrdEqual %v2bool %84 %85
%87 = OpAll %bool %86
%88 = OpCompositeExtract %v2float %82 1
%89 = OpCompositeExtract %v2float %83 1
%90 = OpFOrdEqual %v2bool %88 %89
%91 = OpAll %bool %90
%92 = OpLogicalAnd %bool %87 %91
OpBranch %81
%81 = OpLabel
%93 = OpPhi %bool %false %61 %92 %80
OpStore %ok %93
%94 = OpLoad %mat2v2float %m3
%95 = OpLoad %mat2v2float %m4
%96 = OpMatrixTimesMatrix %mat2v2float %94 %95
OpStore %m3 %96
%97 = OpLoad %bool %ok
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpLoad %mat2v2float %m3
%106 = OpCompositeConstruct %mat2v2float %104 %105
%107 = OpCompositeExtract %v2float %100 0
%108 = OpCompositeExtract %v2float %106 0
%109 = OpFOrdEqual %v2bool %107 %108
%110 = OpAll %bool %109
%111 = OpCompositeExtract %v2float %100 1
%112 = OpCompositeExtract %v2float %106 1
%113 = OpFOrdEqual %v2bool %111 %112
%114 = OpAll %bool %113
%115 = OpLogicalAnd %bool %110 %114
OpBranch %99
%99 = OpLabel
%116 = OpPhi %bool %false %81 %115 %98
OpStore %ok %116
%120 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%121 = OpLoad %v2float %120
%122 = OpCompositeExtract %float %121 1
%124 = OpCompositeConstruct %v2float %122 %float_0
%125 = OpCompositeConstruct %v2float %float_0 %122
%123 = OpCompositeConstruct %mat2v2float %124 %125
OpStore %m5 %123
%126 = OpLoad %bool %ok
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%129 = OpLoad %mat2v2float %m5
%132 = OpCompositeConstruct %mat2v2float %130 %131
%133 = OpCompositeExtract %v2float %129 0
%134 = OpCompositeExtract %v2float %132 0
%135 = OpFOrdEqual %v2bool %133 %134
%136 = OpAll %bool %135
%137 = OpCompositeExtract %v2float %129 1
%138 = OpCompositeExtract %v2float %132 1
%139 = OpFOrdEqual %v2bool %137 %138
%140 = OpAll %bool %139
%141 = OpLogicalAnd %bool %136 %140
OpBranch %128
%128 = OpLabel
%142 = OpPhi %bool %false %99 %141 %127
OpStore %ok %142
%143 = OpLoad %mat2v2float %m1
%144 = OpLoad %mat2v2float %m5
%145 = OpCompositeExtract %v2float %143 0
%146 = OpCompositeExtract %v2float %144 0
%147 = OpFAdd %v2float %145 %146
%148 = OpCompositeExtract %v2float %143 1
%149 = OpCompositeExtract %v2float %144 1
%150 = OpFAdd %v2float %148 %149
%151 = OpCompositeConstruct %mat2v2float %147 %150
OpStore %m1 %151
%152 = OpLoad %bool %ok
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpLoad %mat2v2float %m1
%160 = OpCompositeConstruct %mat2v2float %158 %159
%161 = OpCompositeExtract %v2float %155 0
%162 = OpCompositeExtract %v2float %160 0
%163 = OpFOrdEqual %v2bool %161 %162
%164 = OpAll %bool %163
%165 = OpCompositeExtract %v2float %155 1
%166 = OpCompositeExtract %v2float %160 1
%167 = OpFOrdEqual %v2bool %165 %166
%168 = OpAll %bool %167
%169 = OpLogicalAnd %bool %164 %168
OpBranch %154
%154 = OpLabel
%170 = OpPhi %bool %false %128 %169 %153
OpStore %ok %170
%175 = OpCompositeConstruct %mat2v2float %173 %174
OpStore %m7 %175
%176 = OpLoad %bool %ok
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%179 = OpLoad %mat2v2float %m7
%180 = OpCompositeConstruct %mat2v2float %173 %174
%181 = OpCompositeExtract %v2float %179 0
%182 = OpCompositeExtract %v2float %180 0
%183 = OpFOrdEqual %v2bool %181 %182
%184 = OpAll %bool %183
%185 = OpCompositeExtract %v2float %179 1
%186 = OpCompositeExtract %v2float %180 1
%187 = OpFOrdEqual %v2bool %185 %186
%188 = OpAll %bool %187
%189 = OpLogicalAnd %bool %184 %188
OpBranch %178
%178 = OpLabel
%190 = OpPhi %bool %false %154 %189 %177
OpStore %ok %190
%196 = OpCompositeConstruct %mat3v3float %197 %198 %199
OpStore %m9 %196
%200 = OpLoad %bool %ok
OpSelectionMerge %202 None
OpBranchConditional %200 %201 %202
%201 = OpLabel
%203 = OpLoad %mat3v3float %m9
%204 = OpCompositeConstruct %mat3v3float %197 %198 %199
%206 = OpCompositeExtract %v3float %203 0
%207 = OpCompositeExtract %v3float %204 0
%208 = OpFOrdEqual %v3bool %206 %207
%209 = OpAll %bool %208
%210 = OpCompositeExtract %v3float %203 1
%211 = OpCompositeExtract %v3float %204 1
%212 = OpFOrdEqual %v3bool %210 %211
%213 = OpAll %bool %212
%214 = OpLogicalAnd %bool %209 %213
%215 = OpCompositeExtract %v3float %203 2
%216 = OpCompositeExtract %v3float %204 2
%217 = OpFOrdEqual %v3bool %215 %216
%218 = OpAll %bool %217
%219 = OpLogicalAnd %bool %214 %218
OpBranch %202
%202 = OpLabel
%220 = OpPhi %bool %false %178 %219 %201
OpStore %ok %220
%225 = OpCompositeConstruct %mat4v4float %226 %227 %228 %229
OpStore %m10 %225
%230 = OpLoad %bool %ok
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %mat4v4float %m10
%234 = OpCompositeConstruct %mat4v4float %226 %227 %228 %229
%236 = OpCompositeExtract %v4float %233 0
%237 = OpCompositeExtract %v4float %234 0
%238 = OpFOrdEqual %v4bool %236 %237
%239 = OpAll %bool %238
%240 = OpCompositeExtract %v4float %233 1
%241 = OpCompositeExtract %v4float %234 1
%242 = OpFOrdEqual %v4bool %240 %241
%243 = OpAll %bool %242
%244 = OpLogicalAnd %bool %239 %243
%245 = OpCompositeExtract %v4float %233 2
%246 = OpCompositeExtract %v4float %234 2
%247 = OpFOrdEqual %v4bool %245 %246
%248 = OpAll %bool %247
%249 = OpLogicalAnd %bool %244 %248
%250 = OpCompositeExtract %v4float %233 3
%251 = OpCompositeExtract %v4float %234 3
%252 = OpFOrdEqual %v4bool %250 %251
%253 = OpAll %bool %252
%254 = OpLogicalAnd %bool %249 %253
OpBranch %232
%232 = OpLabel
%255 = OpPhi %bool %false %202 %254 %231
OpStore %ok %255
%259 = OpCompositeConstruct %mat4v4float %258 %258 %258 %258
OpStore %m11 %259
%260 = OpLoad %mat4v4float %m11
%261 = OpLoad %mat4v4float %m10
%262 = OpCompositeExtract %v4float %260 0
%263 = OpCompositeExtract %v4float %261 0
%264 = OpFSub %v4float %262 %263
%265 = OpCompositeExtract %v4float %260 1
%266 = OpCompositeExtract %v4float %261 1
%267 = OpFSub %v4float %265 %266
%268 = OpCompositeExtract %v4float %260 2
%269 = OpCompositeExtract %v4float %261 2
%270 = OpFSub %v4float %268 %269
%271 = OpCompositeExtract %v4float %260 3
%272 = OpCompositeExtract %v4float %261 3
%273 = OpFSub %v4float %271 %272
%274 = OpCompositeConstruct %mat4v4float %264 %267 %270 %273
OpStore %m11 %274
%275 = OpLoad %bool %ok
OpSelectionMerge %277 None
OpBranchConditional %275 %276 %277
%276 = OpLabel
%278 = OpLoad %mat4v4float %m11
%283 = OpCompositeConstruct %mat4v4float %279 %280 %281 %282
%284 = OpCompositeExtract %v4float %278 0
%285 = OpCompositeExtract %v4float %283 0
%286 = OpFOrdEqual %v4bool %284 %285
%287 = OpAll %bool %286
%288 = OpCompositeExtract %v4float %278 1
%289 = OpCompositeExtract %v4float %283 1
%290 = OpFOrdEqual %v4bool %288 %289
%291 = OpAll %bool %290
%292 = OpLogicalAnd %bool %287 %291
%293 = OpCompositeExtract %v4float %278 2
%294 = OpCompositeExtract %v4float %283 2
%295 = OpFOrdEqual %v4bool %293 %294
%296 = OpAll %bool %295
%297 = OpLogicalAnd %bool %292 %296
%298 = OpCompositeExtract %v4float %278 3
%299 = OpCompositeExtract %v4float %283 3
%300 = OpFOrdEqual %v4bool %298 %299
%301 = OpAll %bool %300
%302 = OpLogicalAnd %bool %297 %301
OpBranch %277
%277 = OpLabel
%303 = OpPhi %bool %false %232 %302 %276
OpStore %ok %303
%304 = OpLoad %bool %ok
OpReturnValue %304
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%305 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
%310 = OpCompositeConstruct %mat2v2float %308 %309
OpStore %x %310
%311 = OpCompositeConstruct %mat2v2float %308 %309
OpStore %y %311
%312 = OpLoad %mat2v2float %x
%313 = OpLoad %mat2v2float %y
%314 = OpCompositeExtract %v2float %312 0
%315 = OpCompositeExtract %v2float %313 0
%316 = OpFOrdEqual %v2bool %314 %315
%317 = OpAll %bool %316
%318 = OpCompositeExtract %v2float %312 1
%319 = OpCompositeExtract %v2float %313 1
%320 = OpFOrdEqual %v2bool %318 %319
%321 = OpAll %bool %320
%322 = OpLogicalAnd %bool %317 %321
OpReturnValue %322
OpFunctionEnd
%main = OpFunction %v4float None %323
%324 = OpFunctionParameter %_ptr_Function_v2float
%325 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%581 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%328 = OpCompositeConstruct %mat2v2float %308 %309
OpStore %_1_m1 %328
%329 = OpLoad %bool %_0_ok
OpSelectionMerge %331 None
OpBranchConditional %329 %330 %331
%330 = OpLabel
%332 = OpLoad %mat2v2float %_1_m1
%333 = OpCompositeConstruct %mat2v2float %308 %309
%334 = OpCompositeExtract %v2float %332 0
%335 = OpCompositeExtract %v2float %333 0
%336 = OpFOrdEqual %v2bool %334 %335
%337 = OpAll %bool %336
%338 = OpCompositeExtract %v2float %332 1
%339 = OpCompositeExtract %v2float %333 1
%340 = OpFOrdEqual %v2bool %338 %339
%341 = OpAll %bool %340
%342 = OpLogicalAnd %bool %337 %341
OpBranch %331
%331 = OpLabel
%343 = OpPhi %bool %false %325 %342 %330
OpStore %_0_ok %343
%345 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %345
%346 = OpLoad %bool %_0_ok
OpSelectionMerge %348 None
OpBranchConditional %346 %347 %348
%347 = OpLabel
%349 = OpLoad %mat2v2float %_2_m3
%350 = OpCompositeConstruct %mat2v2float %308 %309
%351 = OpCompositeExtract %v2float %349 0
%352 = OpCompositeExtract %v2float %350 0
%353 = OpFOrdEqual %v2bool %351 %352
%354 = OpAll %bool %353
%355 = OpCompositeExtract %v2float %349 1
%356 = OpCompositeExtract %v2float %350 1
%357 = OpFOrdEqual %v2bool %355 %356
%358 = OpAll %bool %357
%359 = OpLogicalAnd %bool %354 %358
OpBranch %348
%348 = OpLabel
%360 = OpPhi %bool %false %331 %359 %347
OpStore %_0_ok %360
%362 = OpCompositeConstruct %mat2v2float %363 %364
OpStore %_3_m4 %362
%365 = OpLoad %bool %_0_ok
OpSelectionMerge %367 None
OpBranchConditional %365 %366 %367
%366 = OpLabel
%368 = OpLoad %mat2v2float %_3_m4
%369 = OpCompositeConstruct %mat2v2float %363 %364
%370 = OpCompositeExtract %v2float %368 0
%371 = OpCompositeExtract %v2float %369 0
%372 = OpFOrdEqual %v2bool %370 %371
%373 = OpAll %bool %372
%374 = OpCompositeExtract %v2float %368 1
%375 = OpCompositeExtract %v2float %369 1
%376 = OpFOrdEqual %v2bool %374 %375
%377 = OpAll %bool %376
%378 = OpLogicalAnd %bool %373 %377
OpBranch %367
%367 = OpLabel
%379 = OpPhi %bool %false %348 %378 %366
OpStore %_0_ok %379
%380 = OpLoad %mat2v2float %_2_m3
%381 = OpLoad %mat2v2float %_3_m4
%382 = OpMatrixTimesMatrix %mat2v2float %380 %381
OpStore %_2_m3 %382
%383 = OpLoad %bool %_0_ok
OpSelectionMerge %385 None
OpBranchConditional %383 %384 %385
%384 = OpLabel
%386 = OpLoad %mat2v2float %_2_m3
%389 = OpCompositeConstruct %mat2v2float %387 %388
%390 = OpCompositeExtract %v2float %386 0
%391 = OpCompositeExtract %v2float %389 0
%392 = OpFOrdEqual %v2bool %390 %391
%393 = OpAll %bool %392
%394 = OpCompositeExtract %v2float %386 1
%395 = OpCompositeExtract %v2float %389 1
%396 = OpFOrdEqual %v2bool %394 %395
%397 = OpAll %bool %396
%398 = OpLogicalAnd %bool %393 %397
OpBranch %385
%385 = OpLabel
%399 = OpPhi %bool %false %367 %398 %384
OpStore %_0_ok %399
%401 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%402 = OpLoad %v2float %401
%403 = OpCompositeExtract %float %402 1
%405 = OpCompositeConstruct %v2float %403 %float_0
%406 = OpCompositeConstruct %v2float %float_0 %403
%404 = OpCompositeConstruct %mat2v2float %405 %406
OpStore %_4_m5 %404
%407 = OpLoad %bool %_0_ok
OpSelectionMerge %409 None
OpBranchConditional %407 %408 %409
%408 = OpLabel
%410 = OpLoad %mat2v2float %_4_m5
%413 = OpCompositeConstruct %mat2v2float %411 %412
%414 = OpCompositeExtract %v2float %410 0
%415 = OpCompositeExtract %v2float %413 0
%416 = OpFOrdEqual %v2bool %414 %415
%417 = OpAll %bool %416
%418 = OpCompositeExtract %v2float %410 1
%419 = OpCompositeExtract %v2float %413 1
%420 = OpFOrdEqual %v2bool %418 %419
%421 = OpAll %bool %420
%422 = OpLogicalAnd %bool %417 %421
OpBranch %409
%409 = OpLabel
%423 = OpPhi %bool %false %385 %422 %408
OpStore %_0_ok %423
%424 = OpLoad %mat2v2float %_1_m1
%425 = OpLoad %mat2v2float %_4_m5
%426 = OpCompositeExtract %v2float %424 0
%427 = OpCompositeExtract %v2float %425 0
%428 = OpFAdd %v2float %426 %427
%429 = OpCompositeExtract %v2float %424 1
%430 = OpCompositeExtract %v2float %425 1
%431 = OpFAdd %v2float %429 %430
%432 = OpCompositeConstruct %mat2v2float %428 %431
OpStore %_1_m1 %432
%433 = OpLoad %bool %_0_ok
OpSelectionMerge %435 None
OpBranchConditional %433 %434 %435
%434 = OpLabel
%436 = OpLoad %mat2v2float %_1_m1
%439 = OpCompositeConstruct %mat2v2float %437 %438
%440 = OpCompositeExtract %v2float %436 0
%441 = OpCompositeExtract %v2float %439 0
%442 = OpFOrdEqual %v2bool %440 %441
%443 = OpAll %bool %442
%444 = OpCompositeExtract %v2float %436 1
%445 = OpCompositeExtract %v2float %439 1
%446 = OpFOrdEqual %v2bool %444 %445
%447 = OpAll %bool %446
%448 = OpLogicalAnd %bool %443 %447
OpBranch %435
%435 = OpLabel
%449 = OpPhi %bool %false %409 %448 %434
OpStore %_0_ok %449
%453 = OpCompositeConstruct %mat2v2float %451 %452
OpStore %_5_m7 %453
%454 = OpLoad %bool %_0_ok
OpSelectionMerge %456 None
OpBranchConditional %454 %455 %456
%455 = OpLabel
%457 = OpLoad %mat2v2float %_5_m7
%458 = OpCompositeConstruct %mat2v2float %451 %452
%459 = OpCompositeExtract %v2float %457 0
%460 = OpCompositeExtract %v2float %458 0
%461 = OpFOrdEqual %v2bool %459 %460
%462 = OpAll %bool %461
%463 = OpCompositeExtract %v2float %457 1
%464 = OpCompositeExtract %v2float %458 1
%465 = OpFOrdEqual %v2bool %463 %464
%466 = OpAll %bool %465
%467 = OpLogicalAnd %bool %462 %466
OpBranch %456
%456 = OpLabel
%468 = OpPhi %bool %false %435 %467 %455
OpStore %_0_ok %468
%470 = OpCompositeConstruct %mat3v3float %471 %472 %473
OpStore %_6_m9 %470
%474 = OpLoad %bool %_0_ok
OpSelectionMerge %476 None
OpBranchConditional %474 %475 %476
%475 = OpLabel
%477 = OpLoad %mat3v3float %_6_m9
%478 = OpCompositeConstruct %mat3v3float %471 %472 %473
%479 = OpCompositeExtract %v3float %477 0
%480 = OpCompositeExtract %v3float %478 0
%481 = OpFOrdEqual %v3bool %479 %480
%482 = OpAll %bool %481
%483 = OpCompositeExtract %v3float %477 1
%484 = OpCompositeExtract %v3float %478 1
%485 = OpFOrdEqual %v3bool %483 %484
%486 = OpAll %bool %485
%487 = OpLogicalAnd %bool %482 %486
%488 = OpCompositeExtract %v3float %477 2
%489 = OpCompositeExtract %v3float %478 2
%490 = OpFOrdEqual %v3bool %488 %489
%491 = OpAll %bool %490
%492 = OpLogicalAnd %bool %487 %491
OpBranch %476
%476 = OpLabel
%493 = OpPhi %bool %false %456 %492 %475
OpStore %_0_ok %493
%495 = OpCompositeConstruct %mat4v4float %496 %497 %498 %499
OpStore %_7_m10 %495
%500 = OpLoad %bool %_0_ok
OpSelectionMerge %502 None
OpBranchConditional %500 %501 %502
%501 = OpLabel
%503 = OpLoad %mat4v4float %_7_m10
%504 = OpCompositeConstruct %mat4v4float %496 %497 %498 %499
%505 = OpCompositeExtract %v4float %503 0
%506 = OpCompositeExtract %v4float %504 0
%507 = OpFOrdEqual %v4bool %505 %506
%508 = OpAll %bool %507
%509 = OpCompositeExtract %v4float %503 1
%510 = OpCompositeExtract %v4float %504 1
%511 = OpFOrdEqual %v4bool %509 %510
%512 = OpAll %bool %511
%513 = OpLogicalAnd %bool %508 %512
%514 = OpCompositeExtract %v4float %503 2
%515 = OpCompositeExtract %v4float %504 2
%516 = OpFOrdEqual %v4bool %514 %515
%517 = OpAll %bool %516
%518 = OpLogicalAnd %bool %513 %517
%519 = OpCompositeExtract %v4float %503 3
%520 = OpCompositeExtract %v4float %504 3
%521 = OpFOrdEqual %v4bool %519 %520
%522 = OpAll %bool %521
%523 = OpLogicalAnd %bool %518 %522
OpBranch %502
%502 = OpLabel
%524 = OpPhi %bool %false %476 %523 %501
OpStore %_0_ok %524
%527 = OpCompositeConstruct %mat4v4float %526 %526 %526 %526
OpStore %_8_m11 %527
%528 = OpLoad %mat4v4float %_8_m11
%529 = OpLoad %mat4v4float %_7_m10
%530 = OpCompositeExtract %v4float %528 0
%531 = OpCompositeExtract %v4float %529 0
%532 = OpFSub %v4float %530 %531
%533 = OpCompositeExtract %v4float %528 1
%534 = OpCompositeExtract %v4float %529 1
%535 = OpFSub %v4float %533 %534
%536 = OpCompositeExtract %v4float %528 2
%537 = OpCompositeExtract %v4float %529 2
%538 = OpFSub %v4float %536 %537
%539 = OpCompositeExtract %v4float %528 3
%540 = OpCompositeExtract %v4float %529 3
%541 = OpFSub %v4float %539 %540
%542 = OpCompositeConstruct %mat4v4float %532 %535 %538 %541
OpStore %_8_m11 %542
%543 = OpLoad %bool %_0_ok
OpSelectionMerge %545 None
OpBranchConditional %543 %544 %545
%544 = OpLabel
%546 = OpLoad %mat4v4float %_8_m11
%551 = OpCompositeConstruct %mat4v4float %547 %548 %549 %550
%552 = OpCompositeExtract %v4float %546 0
%553 = OpCompositeExtract %v4float %551 0
%554 = OpFOrdEqual %v4bool %552 %553
%555 = OpAll %bool %554
%556 = OpCompositeExtract %v4float %546 1
%557 = OpCompositeExtract %v4float %551 1
%558 = OpFOrdEqual %v4bool %556 %557
%559 = OpAll %bool %558
%560 = OpLogicalAnd %bool %555 %559
%561 = OpCompositeExtract %v4float %546 2
%562 = OpCompositeExtract %v4float %551 2
%563 = OpFOrdEqual %v4bool %561 %562
%564 = OpAll %bool %563
%565 = OpLogicalAnd %bool %560 %564
%566 = OpCompositeExtract %v4float %546 3
%567 = OpCompositeExtract %v4float %551 3
%568 = OpFOrdEqual %v4bool %566 %567
%569 = OpAll %bool %568
%570 = OpLogicalAnd %bool %565 %569
OpBranch %545
%545 = OpLabel
%571 = OpPhi %bool %false %502 %570 %544
OpStore %_0_ok %571
%572 = OpLoad %bool %_0_ok
OpSelectionMerge %574 None
OpBranchConditional %572 %573 %574
%573 = OpLabel
%575 = OpFunctionCall %bool %test_half_b
OpBranch %574
%574 = OpLabel
%576 = OpPhi %bool %false %545 %575 %573
OpSelectionMerge %578 None
OpBranchConditional %576 %577 %578
%577 = OpLabel
%579 = OpFunctionCall %bool %test_comma_b
OpBranch %578
%578 = OpLabel
%580 = OpPhi %bool %false %574 %579 %577
OpSelectionMerge %585 None
OpBranchConditional %580 %583 %584
%583 = OpLabel
%586 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%589 = OpLoad %v4float %586
OpStore %581 %589
OpBranch %585
%584 = OpLabel
%590 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%591 = OpLoad %v4float %590
OpStore %581 %591
OpBranch %585
%585 = OpLabel
%592 = OpLoad %v4float %581
OpReturnValue %592
OpFunctionEnd
