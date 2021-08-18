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
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpDecorate %m1 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %537 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %616 RelaxedPrecision
OpDecorate %633 RelaxedPrecision
OpDecorate %635 RelaxedPrecision
OpDecorate %636 RelaxedPrecision
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
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%27 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
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
%float_0_5 = OpConstant %float 0.5
%347 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %27
%28 = OpLabel
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
%39 = OpCompositeConstruct %v2float %float_1 %float_2
%40 = OpCompositeConstruct %v2float %float_3 %float_4
%41 = OpCompositeConstruct %mat2v2float %39 %40
OpStore %m1 %41
%43 = OpLoad %bool %ok
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%46 = OpLoad %mat2v2float %m1
%47 = OpCompositeConstruct %v2float %float_1 %float_2
%48 = OpCompositeConstruct %v2float %float_3 %float_4
%49 = OpCompositeConstruct %mat2v2float %47 %48
%51 = OpCompositeExtract %v2float %46 0
%52 = OpCompositeExtract %v2float %49 0
%53 = OpFOrdEqual %v2bool %51 %52
%54 = OpAll %bool %53
%55 = OpCompositeExtract %v2float %46 1
%56 = OpCompositeExtract %v2float %49 1
%57 = OpFOrdEqual %v2bool %55 %56
%58 = OpAll %bool %57
%59 = OpLogicalAnd %bool %54 %58
OpBranch %45
%45 = OpLabel
%60 = OpPhi %bool %false %28 %59 %44
OpStore %ok %60
%62 = OpLoad %mat2v2float %m1
OpStore %m3 %62
%63 = OpLoad %bool %ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpLoad %mat2v2float %m3
%67 = OpCompositeConstruct %v2float %float_1 %float_2
%68 = OpCompositeConstruct %v2float %float_3 %float_4
%69 = OpCompositeConstruct %mat2v2float %67 %68
%70 = OpCompositeExtract %v2float %66 0
%71 = OpCompositeExtract %v2float %69 0
%72 = OpFOrdEqual %v2bool %70 %71
%73 = OpAll %bool %72
%74 = OpCompositeExtract %v2float %66 1
%75 = OpCompositeExtract %v2float %69 1
%76 = OpFOrdEqual %v2bool %74 %75
%77 = OpAll %bool %76
%78 = OpLogicalAnd %bool %73 %77
OpBranch %65
%65 = OpLabel
%79 = OpPhi %bool %false %45 %78 %64
OpStore %ok %79
%83 = OpCompositeConstruct %v2float %float_6 %float_0
%84 = OpCompositeConstruct %v2float %float_0 %float_6
%82 = OpCompositeConstruct %mat2v2float %83 %84
OpStore %m4 %82
%85 = OpLoad %bool %ok
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %mat2v2float %m4
%89 = OpCompositeConstruct %v2float %float_6 %float_0
%90 = OpCompositeConstruct %v2float %float_0 %float_6
%91 = OpCompositeConstruct %mat2v2float %89 %90
%92 = OpCompositeExtract %v2float %88 0
%93 = OpCompositeExtract %v2float %91 0
%94 = OpFOrdEqual %v2bool %92 %93
%95 = OpAll %bool %94
%96 = OpCompositeExtract %v2float %88 1
%97 = OpCompositeExtract %v2float %91 1
%98 = OpFOrdEqual %v2bool %96 %97
%99 = OpAll %bool %98
%100 = OpLogicalAnd %bool %95 %99
OpBranch %87
%87 = OpLabel
%101 = OpPhi %bool %false %65 %100 %86
OpStore %ok %101
%102 = OpLoad %mat2v2float %m3
%103 = OpLoad %mat2v2float %m4
%104 = OpMatrixTimesMatrix %mat2v2float %102 %103
OpStore %m3 %104
%105 = OpLoad %bool %ok
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %mat2v2float %m3
%112 = OpCompositeConstruct %v2float %float_6 %float_12
%113 = OpCompositeConstruct %v2float %float_18 %float_24
%114 = OpCompositeConstruct %mat2v2float %112 %113
%115 = OpCompositeExtract %v2float %108 0
%116 = OpCompositeExtract %v2float %114 0
%117 = OpFOrdEqual %v2bool %115 %116
%118 = OpAll %bool %117
%119 = OpCompositeExtract %v2float %108 1
%120 = OpCompositeExtract %v2float %114 1
%121 = OpFOrdEqual %v2bool %119 %120
%122 = OpAll %bool %121
%123 = OpLogicalAnd %bool %118 %122
OpBranch %107
%107 = OpLabel
%124 = OpPhi %bool %false %87 %123 %106
OpStore %ok %124
%128 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%129 = OpLoad %v2float %128
%130 = OpCompositeExtract %float %129 1
%132 = OpCompositeConstruct %v2float %130 %float_0
%133 = OpCompositeConstruct %v2float %float_0 %130
%131 = OpCompositeConstruct %mat2v2float %132 %133
OpStore %m5 %131
%134 = OpLoad %bool %ok
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpLoad %mat2v2float %m5
%138 = OpCompositeConstruct %v2float %float_4 %float_0
%139 = OpCompositeConstruct %v2float %float_0 %float_4
%140 = OpCompositeConstruct %mat2v2float %138 %139
%141 = OpCompositeExtract %v2float %137 0
%142 = OpCompositeExtract %v2float %140 0
%143 = OpFOrdEqual %v2bool %141 %142
%144 = OpAll %bool %143
%145 = OpCompositeExtract %v2float %137 1
%146 = OpCompositeExtract %v2float %140 1
%147 = OpFOrdEqual %v2bool %145 %146
%148 = OpAll %bool %147
%149 = OpLogicalAnd %bool %144 %148
OpBranch %136
%136 = OpLabel
%150 = OpPhi %bool %false %107 %149 %135
OpStore %ok %150
%151 = OpLoad %mat2v2float %m1
%152 = OpLoad %mat2v2float %m5
%153 = OpCompositeExtract %v2float %151 0
%154 = OpCompositeExtract %v2float %152 0
%155 = OpFAdd %v2float %153 %154
%156 = OpCompositeExtract %v2float %151 1
%157 = OpCompositeExtract %v2float %152 1
%158 = OpFAdd %v2float %156 %157
%159 = OpCompositeConstruct %mat2v2float %155 %158
OpStore %m1 %159
%160 = OpLoad %bool %ok
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%163 = OpLoad %mat2v2float %m1
%166 = OpCompositeConstruct %v2float %float_5 %float_2
%167 = OpCompositeConstruct %v2float %float_3 %float_8
%168 = OpCompositeConstruct %mat2v2float %166 %167
%169 = OpCompositeExtract %v2float %163 0
%170 = OpCompositeExtract %v2float %168 0
%171 = OpFOrdEqual %v2bool %169 %170
%172 = OpAll %bool %171
%173 = OpCompositeExtract %v2float %163 1
%174 = OpCompositeExtract %v2float %168 1
%175 = OpFOrdEqual %v2bool %173 %174
%176 = OpAll %bool %175
%177 = OpLogicalAnd %bool %172 %176
OpBranch %162
%162 = OpLabel
%178 = OpPhi %bool %false %136 %177 %161
OpStore %ok %178
%181 = OpCompositeConstruct %v2float %float_5 %float_6
%182 = OpCompositeConstruct %v2float %float_7 %float_8
%183 = OpCompositeConstruct %mat2v2float %181 %182
OpStore %m7 %183
%184 = OpLoad %bool %ok
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpLoad %mat2v2float %m7
%188 = OpCompositeConstruct %v2float %float_5 %float_6
%189 = OpCompositeConstruct %v2float %float_7 %float_8
%190 = OpCompositeConstruct %mat2v2float %188 %189
%191 = OpCompositeExtract %v2float %187 0
%192 = OpCompositeExtract %v2float %190 0
%193 = OpFOrdEqual %v2bool %191 %192
%194 = OpAll %bool %193
%195 = OpCompositeExtract %v2float %187 1
%196 = OpCompositeExtract %v2float %190 1
%197 = OpFOrdEqual %v2bool %195 %196
%198 = OpAll %bool %197
%199 = OpLogicalAnd %bool %194 %198
OpBranch %186
%186 = OpLabel
%200 = OpPhi %bool %false %162 %199 %185
OpStore %ok %200
%207 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%208 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%209 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%206 = OpCompositeConstruct %mat3v3float %207 %208 %209
OpStore %m9 %206
%210 = OpLoad %bool %ok
OpSelectionMerge %212 None
OpBranchConditional %210 %211 %212
%211 = OpLabel
%213 = OpLoad %mat3v3float %m9
%214 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%215 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%216 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%217 = OpCompositeConstruct %mat3v3float %214 %215 %216
%219 = OpCompositeExtract %v3float %213 0
%220 = OpCompositeExtract %v3float %217 0
%221 = OpFOrdEqual %v3bool %219 %220
%222 = OpAll %bool %221
%223 = OpCompositeExtract %v3float %213 1
%224 = OpCompositeExtract %v3float %217 1
%225 = OpFOrdEqual %v3bool %223 %224
%226 = OpAll %bool %225
%227 = OpLogicalAnd %bool %222 %226
%228 = OpCompositeExtract %v3float %213 2
%229 = OpCompositeExtract %v3float %217 2
%230 = OpFOrdEqual %v3bool %228 %229
%231 = OpAll %bool %230
%232 = OpLogicalAnd %bool %227 %231
OpBranch %212
%212 = OpLabel
%233 = OpPhi %bool %false %186 %232 %211
OpStore %ok %233
%239 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%240 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%241 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%242 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%238 = OpCompositeConstruct %mat4v4float %239 %240 %241 %242
OpStore %m10 %238
%243 = OpLoad %bool %ok
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
%246 = OpLoad %mat4v4float %m10
%247 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%248 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%249 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%250 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%251 = OpCompositeConstruct %mat4v4float %247 %248 %249 %250
%253 = OpCompositeExtract %v4float %246 0
%254 = OpCompositeExtract %v4float %251 0
%255 = OpFOrdEqual %v4bool %253 %254
%256 = OpAll %bool %255
%257 = OpCompositeExtract %v4float %246 1
%258 = OpCompositeExtract %v4float %251 1
%259 = OpFOrdEqual %v4bool %257 %258
%260 = OpAll %bool %259
%261 = OpLogicalAnd %bool %256 %260
%262 = OpCompositeExtract %v4float %246 2
%263 = OpCompositeExtract %v4float %251 2
%264 = OpFOrdEqual %v4bool %262 %263
%265 = OpAll %bool %264
%266 = OpLogicalAnd %bool %261 %265
%267 = OpCompositeExtract %v4float %246 3
%268 = OpCompositeExtract %v4float %251 3
%269 = OpFOrdEqual %v4bool %267 %268
%270 = OpAll %bool %269
%271 = OpLogicalAnd %bool %266 %270
OpBranch %245
%245 = OpLabel
%272 = OpPhi %bool %false %212 %271 %244
OpStore %ok %272
%275 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%276 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%277 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%278 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%279 = OpCompositeConstruct %mat4v4float %275 %276 %277 %278
OpStore %m11 %279
%280 = OpLoad %mat4v4float %m11
%281 = OpLoad %mat4v4float %m10
%282 = OpCompositeExtract %v4float %280 0
%283 = OpCompositeExtract %v4float %281 0
%284 = OpFSub %v4float %282 %283
%285 = OpCompositeExtract %v4float %280 1
%286 = OpCompositeExtract %v4float %281 1
%287 = OpFSub %v4float %285 %286
%288 = OpCompositeExtract %v4float %280 2
%289 = OpCompositeExtract %v4float %281 2
%290 = OpFSub %v4float %288 %289
%291 = OpCompositeExtract %v4float %280 3
%292 = OpCompositeExtract %v4float %281 3
%293 = OpFSub %v4float %291 %292
%294 = OpCompositeConstruct %mat4v4float %284 %287 %290 %293
OpStore %m11 %294
%295 = OpLoad %bool %ok
OpSelectionMerge %297 None
OpBranchConditional %295 %296 %297
%296 = OpLabel
%298 = OpLoad %mat4v4float %m11
%299 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%300 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%301 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%302 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%303 = OpCompositeConstruct %mat4v4float %299 %300 %301 %302
%304 = OpCompositeExtract %v4float %298 0
%305 = OpCompositeExtract %v4float %303 0
%306 = OpFOrdEqual %v4bool %304 %305
%307 = OpAll %bool %306
%308 = OpCompositeExtract %v4float %298 1
%309 = OpCompositeExtract %v4float %303 1
%310 = OpFOrdEqual %v4bool %308 %309
%311 = OpAll %bool %310
%312 = OpLogicalAnd %bool %307 %311
%313 = OpCompositeExtract %v4float %298 2
%314 = OpCompositeExtract %v4float %303 2
%315 = OpFOrdEqual %v4bool %313 %314
%316 = OpAll %bool %315
%317 = OpLogicalAnd %bool %312 %316
%318 = OpCompositeExtract %v4float %298 3
%319 = OpCompositeExtract %v4float %303 3
%320 = OpFOrdEqual %v4bool %318 %319
%321 = OpAll %bool %320
%322 = OpLogicalAnd %bool %317 %321
OpBranch %297
%297 = OpLabel
%323 = OpPhi %bool %false %245 %322 %296
OpStore %ok %323
%324 = OpLoad %bool %ok
OpReturnValue %324
OpFunctionEnd
%test_comma_b = OpFunction %bool None %27
%325 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
%328 = OpCompositeConstruct %v2float %float_1 %float_2
%329 = OpCompositeConstruct %v2float %float_3 %float_4
%330 = OpCompositeConstruct %mat2v2float %328 %329
OpStore %x %330
%332 = OpCompositeConstruct %v2float %float_2 %float_4
%333 = OpCompositeConstruct %v2float %float_6 %float_8
%334 = OpCompositeConstruct %mat2v2float %332 %333
%335 = OpMatrixTimesScalar %mat2v2float %334 %float_0_5
OpStore %y %335
%336 = OpLoad %mat2v2float %x
%337 = OpLoad %mat2v2float %y
%338 = OpCompositeExtract %v2float %336 0
%339 = OpCompositeExtract %v2float %337 0
%340 = OpFOrdEqual %v2bool %338 %339
%341 = OpAll %bool %340
%342 = OpCompositeExtract %v2float %336 1
%343 = OpCompositeExtract %v2float %337 1
%344 = OpFOrdEqual %v2bool %342 %343
%345 = OpAll %bool %344
%346 = OpLogicalAnd %bool %341 %345
OpReturnValue %346
OpFunctionEnd
%main = OpFunction %v4float None %347
%348 = OpFunctionParameter %_ptr_Function_v2float
%349 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%625 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%352 = OpCompositeConstruct %v2float %float_1 %float_2
%353 = OpCompositeConstruct %v2float %float_3 %float_4
%354 = OpCompositeConstruct %mat2v2float %352 %353
OpStore %_1_m1 %354
%355 = OpLoad %bool %_0_ok
OpSelectionMerge %357 None
OpBranchConditional %355 %356 %357
%356 = OpLabel
%358 = OpLoad %mat2v2float %_1_m1
%359 = OpCompositeConstruct %v2float %float_1 %float_2
%360 = OpCompositeConstruct %v2float %float_3 %float_4
%361 = OpCompositeConstruct %mat2v2float %359 %360
%362 = OpCompositeExtract %v2float %358 0
%363 = OpCompositeExtract %v2float %361 0
%364 = OpFOrdEqual %v2bool %362 %363
%365 = OpAll %bool %364
%366 = OpCompositeExtract %v2float %358 1
%367 = OpCompositeExtract %v2float %361 1
%368 = OpFOrdEqual %v2bool %366 %367
%369 = OpAll %bool %368
%370 = OpLogicalAnd %bool %365 %369
OpBranch %357
%357 = OpLabel
%371 = OpPhi %bool %false %349 %370 %356
OpStore %_0_ok %371
%373 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %373
%374 = OpLoad %bool %_0_ok
OpSelectionMerge %376 None
OpBranchConditional %374 %375 %376
%375 = OpLabel
%377 = OpLoad %mat2v2float %_2_m3
%378 = OpCompositeConstruct %v2float %float_1 %float_2
%379 = OpCompositeConstruct %v2float %float_3 %float_4
%380 = OpCompositeConstruct %mat2v2float %378 %379
%381 = OpCompositeExtract %v2float %377 0
%382 = OpCompositeExtract %v2float %380 0
%383 = OpFOrdEqual %v2bool %381 %382
%384 = OpAll %bool %383
%385 = OpCompositeExtract %v2float %377 1
%386 = OpCompositeExtract %v2float %380 1
%387 = OpFOrdEqual %v2bool %385 %386
%388 = OpAll %bool %387
%389 = OpLogicalAnd %bool %384 %388
OpBranch %376
%376 = OpLabel
%390 = OpPhi %bool %false %357 %389 %375
OpStore %_0_ok %390
%393 = OpCompositeConstruct %v2float %float_6 %float_0
%394 = OpCompositeConstruct %v2float %float_0 %float_6
%392 = OpCompositeConstruct %mat2v2float %393 %394
OpStore %_3_m4 %392
%395 = OpLoad %bool %_0_ok
OpSelectionMerge %397 None
OpBranchConditional %395 %396 %397
%396 = OpLabel
%398 = OpLoad %mat2v2float %_3_m4
%399 = OpCompositeConstruct %v2float %float_6 %float_0
%400 = OpCompositeConstruct %v2float %float_0 %float_6
%401 = OpCompositeConstruct %mat2v2float %399 %400
%402 = OpCompositeExtract %v2float %398 0
%403 = OpCompositeExtract %v2float %401 0
%404 = OpFOrdEqual %v2bool %402 %403
%405 = OpAll %bool %404
%406 = OpCompositeExtract %v2float %398 1
%407 = OpCompositeExtract %v2float %401 1
%408 = OpFOrdEqual %v2bool %406 %407
%409 = OpAll %bool %408
%410 = OpLogicalAnd %bool %405 %409
OpBranch %397
%397 = OpLabel
%411 = OpPhi %bool %false %376 %410 %396
OpStore %_0_ok %411
%412 = OpLoad %mat2v2float %_2_m3
%413 = OpLoad %mat2v2float %_3_m4
%414 = OpMatrixTimesMatrix %mat2v2float %412 %413
OpStore %_2_m3 %414
%415 = OpLoad %bool %_0_ok
OpSelectionMerge %417 None
OpBranchConditional %415 %416 %417
%416 = OpLabel
%418 = OpLoad %mat2v2float %_2_m3
%419 = OpCompositeConstruct %v2float %float_6 %float_12
%420 = OpCompositeConstruct %v2float %float_18 %float_24
%421 = OpCompositeConstruct %mat2v2float %419 %420
%422 = OpCompositeExtract %v2float %418 0
%423 = OpCompositeExtract %v2float %421 0
%424 = OpFOrdEqual %v2bool %422 %423
%425 = OpAll %bool %424
%426 = OpCompositeExtract %v2float %418 1
%427 = OpCompositeExtract %v2float %421 1
%428 = OpFOrdEqual %v2bool %426 %427
%429 = OpAll %bool %428
%430 = OpLogicalAnd %bool %425 %429
OpBranch %417
%417 = OpLabel
%431 = OpPhi %bool %false %397 %430 %416
OpStore %_0_ok %431
%433 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%434 = OpLoad %v2float %433
%435 = OpCompositeExtract %float %434 1
%437 = OpCompositeConstruct %v2float %435 %float_0
%438 = OpCompositeConstruct %v2float %float_0 %435
%436 = OpCompositeConstruct %mat2v2float %437 %438
OpStore %_4_m5 %436
%439 = OpLoad %bool %_0_ok
OpSelectionMerge %441 None
OpBranchConditional %439 %440 %441
%440 = OpLabel
%442 = OpLoad %mat2v2float %_4_m5
%443 = OpCompositeConstruct %v2float %float_4 %float_0
%444 = OpCompositeConstruct %v2float %float_0 %float_4
%445 = OpCompositeConstruct %mat2v2float %443 %444
%446 = OpCompositeExtract %v2float %442 0
%447 = OpCompositeExtract %v2float %445 0
%448 = OpFOrdEqual %v2bool %446 %447
%449 = OpAll %bool %448
%450 = OpCompositeExtract %v2float %442 1
%451 = OpCompositeExtract %v2float %445 1
%452 = OpFOrdEqual %v2bool %450 %451
%453 = OpAll %bool %452
%454 = OpLogicalAnd %bool %449 %453
OpBranch %441
%441 = OpLabel
%455 = OpPhi %bool %false %417 %454 %440
OpStore %_0_ok %455
%456 = OpLoad %mat2v2float %_1_m1
%457 = OpLoad %mat2v2float %_4_m5
%458 = OpCompositeExtract %v2float %456 0
%459 = OpCompositeExtract %v2float %457 0
%460 = OpFAdd %v2float %458 %459
%461 = OpCompositeExtract %v2float %456 1
%462 = OpCompositeExtract %v2float %457 1
%463 = OpFAdd %v2float %461 %462
%464 = OpCompositeConstruct %mat2v2float %460 %463
OpStore %_1_m1 %464
%465 = OpLoad %bool %_0_ok
OpSelectionMerge %467 None
OpBranchConditional %465 %466 %467
%466 = OpLabel
%468 = OpLoad %mat2v2float %_1_m1
%469 = OpCompositeConstruct %v2float %float_5 %float_2
%470 = OpCompositeConstruct %v2float %float_3 %float_8
%471 = OpCompositeConstruct %mat2v2float %469 %470
%472 = OpCompositeExtract %v2float %468 0
%473 = OpCompositeExtract %v2float %471 0
%474 = OpFOrdEqual %v2bool %472 %473
%475 = OpAll %bool %474
%476 = OpCompositeExtract %v2float %468 1
%477 = OpCompositeExtract %v2float %471 1
%478 = OpFOrdEqual %v2bool %476 %477
%479 = OpAll %bool %478
%480 = OpLogicalAnd %bool %475 %479
OpBranch %467
%467 = OpLabel
%481 = OpPhi %bool %false %441 %480 %466
OpStore %_0_ok %481
%483 = OpCompositeConstruct %v2float %float_5 %float_6
%484 = OpCompositeConstruct %v2float %float_7 %float_8
%485 = OpCompositeConstruct %mat2v2float %483 %484
OpStore %_5_m7 %485
%486 = OpLoad %bool %_0_ok
OpSelectionMerge %488 None
OpBranchConditional %486 %487 %488
%487 = OpLabel
%489 = OpLoad %mat2v2float %_5_m7
%490 = OpCompositeConstruct %v2float %float_5 %float_6
%491 = OpCompositeConstruct %v2float %float_7 %float_8
%492 = OpCompositeConstruct %mat2v2float %490 %491
%493 = OpCompositeExtract %v2float %489 0
%494 = OpCompositeExtract %v2float %492 0
%495 = OpFOrdEqual %v2bool %493 %494
%496 = OpAll %bool %495
%497 = OpCompositeExtract %v2float %489 1
%498 = OpCompositeExtract %v2float %492 1
%499 = OpFOrdEqual %v2bool %497 %498
%500 = OpAll %bool %499
%501 = OpLogicalAnd %bool %496 %500
OpBranch %488
%488 = OpLabel
%502 = OpPhi %bool %false %467 %501 %487
OpStore %_0_ok %502
%505 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%506 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%507 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%504 = OpCompositeConstruct %mat3v3float %505 %506 %507
OpStore %_6_m9 %504
%508 = OpLoad %bool %_0_ok
OpSelectionMerge %510 None
OpBranchConditional %508 %509 %510
%509 = OpLabel
%511 = OpLoad %mat3v3float %_6_m9
%512 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%513 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%514 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%515 = OpCompositeConstruct %mat3v3float %512 %513 %514
%516 = OpCompositeExtract %v3float %511 0
%517 = OpCompositeExtract %v3float %515 0
%518 = OpFOrdEqual %v3bool %516 %517
%519 = OpAll %bool %518
%520 = OpCompositeExtract %v3float %511 1
%521 = OpCompositeExtract %v3float %515 1
%522 = OpFOrdEqual %v3bool %520 %521
%523 = OpAll %bool %522
%524 = OpLogicalAnd %bool %519 %523
%525 = OpCompositeExtract %v3float %511 2
%526 = OpCompositeExtract %v3float %515 2
%527 = OpFOrdEqual %v3bool %525 %526
%528 = OpAll %bool %527
%529 = OpLogicalAnd %bool %524 %528
OpBranch %510
%510 = OpLabel
%530 = OpPhi %bool %false %488 %529 %509
OpStore %_0_ok %530
%533 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%534 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%535 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%536 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%532 = OpCompositeConstruct %mat4v4float %533 %534 %535 %536
OpStore %_7_m10 %532
%537 = OpLoad %bool %_0_ok
OpSelectionMerge %539 None
OpBranchConditional %537 %538 %539
%538 = OpLabel
%540 = OpLoad %mat4v4float %_7_m10
%541 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%542 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%543 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%544 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%545 = OpCompositeConstruct %mat4v4float %541 %542 %543 %544
%546 = OpCompositeExtract %v4float %540 0
%547 = OpCompositeExtract %v4float %545 0
%548 = OpFOrdEqual %v4bool %546 %547
%549 = OpAll %bool %548
%550 = OpCompositeExtract %v4float %540 1
%551 = OpCompositeExtract %v4float %545 1
%552 = OpFOrdEqual %v4bool %550 %551
%553 = OpAll %bool %552
%554 = OpLogicalAnd %bool %549 %553
%555 = OpCompositeExtract %v4float %540 2
%556 = OpCompositeExtract %v4float %545 2
%557 = OpFOrdEqual %v4bool %555 %556
%558 = OpAll %bool %557
%559 = OpLogicalAnd %bool %554 %558
%560 = OpCompositeExtract %v4float %540 3
%561 = OpCompositeExtract %v4float %545 3
%562 = OpFOrdEqual %v4bool %560 %561
%563 = OpAll %bool %562
%564 = OpLogicalAnd %bool %559 %563
OpBranch %539
%539 = OpLabel
%565 = OpPhi %bool %false %510 %564 %538
OpStore %_0_ok %565
%567 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%568 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%569 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%570 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%571 = OpCompositeConstruct %mat4v4float %567 %568 %569 %570
OpStore %_8_m11 %571
%572 = OpLoad %mat4v4float %_8_m11
%573 = OpLoad %mat4v4float %_7_m10
%574 = OpCompositeExtract %v4float %572 0
%575 = OpCompositeExtract %v4float %573 0
%576 = OpFSub %v4float %574 %575
%577 = OpCompositeExtract %v4float %572 1
%578 = OpCompositeExtract %v4float %573 1
%579 = OpFSub %v4float %577 %578
%580 = OpCompositeExtract %v4float %572 2
%581 = OpCompositeExtract %v4float %573 2
%582 = OpFSub %v4float %580 %581
%583 = OpCompositeExtract %v4float %572 3
%584 = OpCompositeExtract %v4float %573 3
%585 = OpFSub %v4float %583 %584
%586 = OpCompositeConstruct %mat4v4float %576 %579 %582 %585
OpStore %_8_m11 %586
%587 = OpLoad %bool %_0_ok
OpSelectionMerge %589 None
OpBranchConditional %587 %588 %589
%588 = OpLabel
%590 = OpLoad %mat4v4float %_8_m11
%591 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%592 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%593 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%594 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%595 = OpCompositeConstruct %mat4v4float %591 %592 %593 %594
%596 = OpCompositeExtract %v4float %590 0
%597 = OpCompositeExtract %v4float %595 0
%598 = OpFOrdEqual %v4bool %596 %597
%599 = OpAll %bool %598
%600 = OpCompositeExtract %v4float %590 1
%601 = OpCompositeExtract %v4float %595 1
%602 = OpFOrdEqual %v4bool %600 %601
%603 = OpAll %bool %602
%604 = OpLogicalAnd %bool %599 %603
%605 = OpCompositeExtract %v4float %590 2
%606 = OpCompositeExtract %v4float %595 2
%607 = OpFOrdEqual %v4bool %605 %606
%608 = OpAll %bool %607
%609 = OpLogicalAnd %bool %604 %608
%610 = OpCompositeExtract %v4float %590 3
%611 = OpCompositeExtract %v4float %595 3
%612 = OpFOrdEqual %v4bool %610 %611
%613 = OpAll %bool %612
%614 = OpLogicalAnd %bool %609 %613
OpBranch %589
%589 = OpLabel
%615 = OpPhi %bool %false %539 %614 %588
OpStore %_0_ok %615
%616 = OpLoad %bool %_0_ok
OpSelectionMerge %618 None
OpBranchConditional %616 %617 %618
%617 = OpLabel
%619 = OpFunctionCall %bool %test_half_b
OpBranch %618
%618 = OpLabel
%620 = OpPhi %bool %false %589 %619 %617
OpSelectionMerge %622 None
OpBranchConditional %620 %621 %622
%621 = OpLabel
%623 = OpFunctionCall %bool %test_comma_b
OpBranch %622
%622 = OpLabel
%624 = OpPhi %bool %false %618 %623 %621
OpSelectionMerge %629 None
OpBranchConditional %624 %627 %628
%627 = OpLabel
%630 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%633 = OpLoad %v4float %630
OpStore %625 %633
OpBranch %629
%628 = OpLabel
%634 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%635 = OpLoad %v4float %634
OpStore %625 %635
OpBranch %629
%629 = OpLabel
%636 = OpLoad %v4float %625
OpReturnValue %636
OpFunctionEnd
