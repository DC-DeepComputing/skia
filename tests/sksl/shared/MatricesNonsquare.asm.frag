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
OpName %m23 "m23"
OpName %m24 "m24"
OpName %m32 "m32"
OpName %m34 "m34"
OpName %m42 "m42"
OpName %m43 "m43"
OpName %m22 "m22"
OpName %m33 "m33"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m23 "_1_m23"
OpName %_2_m24 "_2_m24"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
OpName %_5_m42 "_5_m42"
OpName %_6_m43 "_6_m43"
OpName %_7_m22 "_7_m22"
OpName %_8_m33 "_8_m33"
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
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %m23 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
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
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %546 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
OpDecorate %600 RelaxedPrecision
OpDecorate %630 RelaxedPrecision
OpDecorate %663 RelaxedPrecision
OpDecorate %680 RelaxedPrecision
OpDecorate %694 RelaxedPrecision
OpDecorate %697 RelaxedPrecision
OpDecorate %698 RelaxedPrecision
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
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_0_75 = OpConstant %float 0.75
%370 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %26
%27 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %ok %true
%37 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%38 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%36 = OpCompositeConstruct %mat2v3float %37 %38
OpStore %m23 %36
%40 = OpLoad %bool %ok
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%43 = OpLoad %mat2v3float %m23
%44 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%45 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%46 = OpCompositeConstruct %mat2v3float %44 %45
%48 = OpCompositeExtract %v3float %43 0
%49 = OpCompositeExtract %v3float %46 0
%50 = OpFOrdEqual %v3bool %48 %49
%51 = OpAll %bool %50
%52 = OpCompositeExtract %v3float %43 1
%53 = OpCompositeExtract %v3float %46 1
%54 = OpFOrdEqual %v3bool %52 %53
%55 = OpAll %bool %54
%56 = OpLogicalAnd %bool %51 %55
OpBranch %42
%42 = OpLabel
%57 = OpPhi %bool %false %27 %56 %41
OpStore %ok %57
%63 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%64 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%62 = OpCompositeConstruct %mat2v4float %63 %64
OpStore %m24 %62
%65 = OpLoad %bool %ok
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%68 = OpLoad %mat2v4float %m24
%69 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%70 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%71 = OpCompositeConstruct %mat2v4float %69 %70
%73 = OpCompositeExtract %v4float %68 0
%74 = OpCompositeExtract %v4float %71 0
%75 = OpFOrdEqual %v4bool %73 %74
%76 = OpAll %bool %75
%77 = OpCompositeExtract %v4float %68 1
%78 = OpCompositeExtract %v4float %71 1
%79 = OpFOrdEqual %v4bool %77 %78
%80 = OpAll %bool %79
%81 = OpLogicalAnd %bool %76 %80
OpBranch %67
%67 = OpLabel
%82 = OpPhi %bool %false %42 %81 %66
OpStore %ok %82
%88 = OpCompositeConstruct %v2float %float_4 %float_0
%89 = OpCompositeConstruct %v2float %float_0 %float_4
%90 = OpCompositeConstruct %v2float %float_0 %float_0
%87 = OpCompositeConstruct %mat3v2float %88 %89 %90
OpStore %m32 %87
%91 = OpLoad %bool %ok
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpLoad %mat3v2float %m32
%95 = OpCompositeConstruct %v2float %float_4 %float_0
%96 = OpCompositeConstruct %v2float %float_0 %float_4
%97 = OpCompositeConstruct %v2float %float_0 %float_0
%98 = OpCompositeConstruct %mat3v2float %95 %96 %97
%100 = OpCompositeExtract %v2float %94 0
%101 = OpCompositeExtract %v2float %98 0
%102 = OpFOrdEqual %v2bool %100 %101
%103 = OpAll %bool %102
%104 = OpCompositeExtract %v2float %94 1
%105 = OpCompositeExtract %v2float %98 1
%106 = OpFOrdEqual %v2bool %104 %105
%107 = OpAll %bool %106
%108 = OpLogicalAnd %bool %103 %107
%109 = OpCompositeExtract %v2float %94 2
%110 = OpCompositeExtract %v2float %98 2
%111 = OpFOrdEqual %v2bool %109 %110
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %108 %112
OpBranch %93
%93 = OpLabel
%114 = OpPhi %bool %false %67 %113 %92
OpStore %ok %114
%120 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%121 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%122 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%119 = OpCompositeConstruct %mat3v4float %120 %121 %122
OpStore %m34 %119
%123 = OpLoad %bool %ok
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpLoad %mat3v4float %m34
%127 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%128 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%129 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%130 = OpCompositeConstruct %mat3v4float %127 %128 %129
%131 = OpCompositeExtract %v4float %126 0
%132 = OpCompositeExtract %v4float %130 0
%133 = OpFOrdEqual %v4bool %131 %132
%134 = OpAll %bool %133
%135 = OpCompositeExtract %v4float %126 1
%136 = OpCompositeExtract %v4float %130 1
%137 = OpFOrdEqual %v4bool %135 %136
%138 = OpAll %bool %137
%139 = OpLogicalAnd %bool %134 %138
%140 = OpCompositeExtract %v4float %126 2
%141 = OpCompositeExtract %v4float %130 2
%142 = OpFOrdEqual %v4bool %140 %141
%143 = OpAll %bool %142
%144 = OpLogicalAnd %bool %139 %143
OpBranch %125
%125 = OpLabel
%145 = OpPhi %bool %false %93 %144 %124
OpStore %ok %145
%151 = OpCompositeConstruct %v2float %float_6 %float_0
%152 = OpCompositeConstruct %v2float %float_0 %float_6
%153 = OpCompositeConstruct %v2float %float_0 %float_0
%154 = OpCompositeConstruct %v2float %float_0 %float_0
%150 = OpCompositeConstruct %mat4v2float %151 %152 %153 %154
OpStore %m42 %150
%155 = OpLoad %bool %ok
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpLoad %mat4v2float %m42
%159 = OpCompositeConstruct %v2float %float_6 %float_0
%160 = OpCompositeConstruct %v2float %float_0 %float_6
%161 = OpCompositeConstruct %v2float %float_0 %float_0
%162 = OpCompositeConstruct %v2float %float_0 %float_0
%163 = OpCompositeConstruct %mat4v2float %159 %160 %161 %162
%164 = OpCompositeExtract %v2float %158 0
%165 = OpCompositeExtract %v2float %163 0
%166 = OpFOrdEqual %v2bool %164 %165
%167 = OpAll %bool %166
%168 = OpCompositeExtract %v2float %158 1
%169 = OpCompositeExtract %v2float %163 1
%170 = OpFOrdEqual %v2bool %168 %169
%171 = OpAll %bool %170
%172 = OpLogicalAnd %bool %167 %171
%173 = OpCompositeExtract %v2float %158 2
%174 = OpCompositeExtract %v2float %163 2
%175 = OpFOrdEqual %v2bool %173 %174
%176 = OpAll %bool %175
%177 = OpLogicalAnd %bool %172 %176
%178 = OpCompositeExtract %v2float %158 3
%179 = OpCompositeExtract %v2float %163 3
%180 = OpFOrdEqual %v2bool %178 %179
%181 = OpAll %bool %180
%182 = OpLogicalAnd %bool %177 %181
OpBranch %157
%157 = OpLabel
%183 = OpPhi %bool %false %125 %182 %156
OpStore %ok %183
%189 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%190 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%191 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%192 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%188 = OpCompositeConstruct %mat4v3float %189 %190 %191 %192
OpStore %m43 %188
%193 = OpLoad %bool %ok
OpSelectionMerge %195 None
OpBranchConditional %193 %194 %195
%194 = OpLabel
%196 = OpLoad %mat4v3float %m43
%197 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%198 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%199 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%200 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%201 = OpCompositeConstruct %mat4v3float %197 %198 %199 %200
%202 = OpCompositeExtract %v3float %196 0
%203 = OpCompositeExtract %v3float %201 0
%204 = OpFOrdEqual %v3bool %202 %203
%205 = OpAll %bool %204
%206 = OpCompositeExtract %v3float %196 1
%207 = OpCompositeExtract %v3float %201 1
%208 = OpFOrdEqual %v3bool %206 %207
%209 = OpAll %bool %208
%210 = OpLogicalAnd %bool %205 %209
%211 = OpCompositeExtract %v3float %196 2
%212 = OpCompositeExtract %v3float %201 2
%213 = OpFOrdEqual %v3bool %211 %212
%214 = OpAll %bool %213
%215 = OpLogicalAnd %bool %210 %214
%216 = OpCompositeExtract %v3float %196 3
%217 = OpCompositeExtract %v3float %201 3
%218 = OpFOrdEqual %v3bool %216 %217
%219 = OpAll %bool %218
%220 = OpLogicalAnd %bool %215 %219
OpBranch %195
%195 = OpLabel
%221 = OpPhi %bool %false %157 %220 %194
OpStore %ok %221
%225 = OpLoad %mat3v2float %m32
%226 = OpLoad %mat2v3float %m23
%227 = OpMatrixTimesMatrix %mat2v2float %225 %226
OpStore %m22 %227
%228 = OpLoad %bool %ok
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%231 = OpLoad %mat2v2float %m22
%234 = OpCompositeConstruct %v2float %float_8 %float_0
%235 = OpCompositeConstruct %v2float %float_0 %float_8
%233 = OpCompositeConstruct %mat2v2float %234 %235
%236 = OpCompositeExtract %v2float %231 0
%237 = OpCompositeExtract %v2float %233 0
%238 = OpFOrdEqual %v2bool %236 %237
%239 = OpAll %bool %238
%240 = OpCompositeExtract %v2float %231 1
%241 = OpCompositeExtract %v2float %233 1
%242 = OpFOrdEqual %v2bool %240 %241
%243 = OpAll %bool %242
%244 = OpLogicalAnd %bool %239 %243
OpBranch %230
%230 = OpLabel
%245 = OpPhi %bool %false %195 %244 %229
OpStore %ok %245
%249 = OpLoad %mat4v3float %m43
%250 = OpLoad %mat3v4float %m34
%251 = OpMatrixTimesMatrix %mat3v3float %249 %250
OpStore %m33 %251
%252 = OpLoad %bool %ok
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
%255 = OpLoad %mat3v3float %m33
%258 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%259 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%260 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%257 = OpCompositeConstruct %mat3v3float %258 %259 %260
%261 = OpCompositeExtract %v3float %255 0
%262 = OpCompositeExtract %v3float %257 0
%263 = OpFOrdEqual %v3bool %261 %262
%264 = OpAll %bool %263
%265 = OpCompositeExtract %v3float %255 1
%266 = OpCompositeExtract %v3float %257 1
%267 = OpFOrdEqual %v3bool %265 %266
%268 = OpAll %bool %267
%269 = OpLogicalAnd %bool %264 %268
%270 = OpCompositeExtract %v3float %255 2
%271 = OpCompositeExtract %v3float %257 2
%272 = OpFOrdEqual %v3bool %270 %271
%273 = OpAll %bool %272
%274 = OpLogicalAnd %bool %269 %273
OpBranch %254
%254 = OpLabel
%275 = OpPhi %bool %false %230 %274 %253
OpStore %ok %275
%276 = OpLoad %mat2v3float %m23
%278 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%279 = OpCompositeConstruct %mat2v3float %278 %278
%280 = OpCompositeExtract %v3float %276 0
%281 = OpCompositeExtract %v3float %279 0
%282 = OpFAdd %v3float %280 %281
%283 = OpCompositeExtract %v3float %276 1
%284 = OpCompositeExtract %v3float %279 1
%285 = OpFAdd %v3float %283 %284
%286 = OpCompositeConstruct %mat2v3float %282 %285
OpStore %m23 %286
%287 = OpLoad %bool %ok
OpSelectionMerge %289 None
OpBranchConditional %287 %288 %289
%288 = OpLabel
%290 = OpLoad %mat2v3float %m23
%291 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%292 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%293 = OpCompositeConstruct %mat2v3float %291 %292
%294 = OpCompositeExtract %v3float %290 0
%295 = OpCompositeExtract %v3float %293 0
%296 = OpFOrdEqual %v3bool %294 %295
%297 = OpAll %bool %296
%298 = OpCompositeExtract %v3float %290 1
%299 = OpCompositeExtract %v3float %293 1
%300 = OpFOrdEqual %v3bool %298 %299
%301 = OpAll %bool %300
%302 = OpLogicalAnd %bool %297 %301
OpBranch %289
%289 = OpLabel
%303 = OpPhi %bool %false %254 %302 %288
OpStore %ok %303
%304 = OpLoad %mat3v2float %m32
%305 = OpCompositeConstruct %v2float %float_2 %float_2
%306 = OpCompositeConstruct %mat3v2float %305 %305 %305
%307 = OpCompositeExtract %v2float %304 0
%308 = OpCompositeExtract %v2float %306 0
%309 = OpFSub %v2float %307 %308
%310 = OpCompositeExtract %v2float %304 1
%311 = OpCompositeExtract %v2float %306 1
%312 = OpFSub %v2float %310 %311
%313 = OpCompositeExtract %v2float %304 2
%314 = OpCompositeExtract %v2float %306 2
%315 = OpFSub %v2float %313 %314
%316 = OpCompositeConstruct %mat3v2float %309 %312 %315
OpStore %m32 %316
%317 = OpLoad %bool %ok
OpSelectionMerge %319 None
OpBranchConditional %317 %318 %319
%318 = OpLabel
%320 = OpLoad %mat3v2float %m32
%322 = OpCompositeConstruct %v2float %float_2 %float_n2
%323 = OpCompositeConstruct %v2float %float_n2 %float_2
%324 = OpCompositeConstruct %v2float %float_n2 %float_n2
%325 = OpCompositeConstruct %mat3v2float %322 %323 %324
%326 = OpCompositeExtract %v2float %320 0
%327 = OpCompositeExtract %v2float %325 0
%328 = OpFOrdEqual %v2bool %326 %327
%329 = OpAll %bool %328
%330 = OpCompositeExtract %v2float %320 1
%331 = OpCompositeExtract %v2float %325 1
%332 = OpFOrdEqual %v2bool %330 %331
%333 = OpAll %bool %332
%334 = OpLogicalAnd %bool %329 %333
%335 = OpCompositeExtract %v2float %320 2
%336 = OpCompositeExtract %v2float %325 2
%337 = OpFOrdEqual %v2bool %335 %336
%338 = OpAll %bool %337
%339 = OpLogicalAnd %bool %334 %338
OpBranch %319
%319 = OpLabel
%340 = OpPhi %bool %false %289 %339 %318
OpStore %ok %340
%341 = OpLoad %mat2v4float %m24
%342 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%343 = OpCompositeConstruct %mat2v4float %342 %342
%344 = OpCompositeExtract %v4float %341 0
%345 = OpCompositeExtract %v4float %343 0
%346 = OpFDiv %v4float %344 %345
%347 = OpCompositeExtract %v4float %341 1
%348 = OpCompositeExtract %v4float %343 1
%349 = OpFDiv %v4float %347 %348
%350 = OpCompositeConstruct %mat2v4float %346 %349
OpStore %m24 %350
%351 = OpLoad %bool %ok
OpSelectionMerge %353 None
OpBranchConditional %351 %352 %353
%352 = OpLabel
%354 = OpLoad %mat2v4float %m24
%356 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%357 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%358 = OpCompositeConstruct %mat2v4float %356 %357
%359 = OpCompositeExtract %v4float %354 0
%360 = OpCompositeExtract %v4float %358 0
%361 = OpFOrdEqual %v4bool %359 %360
%362 = OpAll %bool %361
%363 = OpCompositeExtract %v4float %354 1
%364 = OpCompositeExtract %v4float %358 1
%365 = OpFOrdEqual %v4bool %363 %364
%366 = OpAll %bool %365
%367 = OpLogicalAnd %bool %362 %366
OpBranch %353
%353 = OpLabel
%368 = OpPhi %bool %false %319 %367 %352
OpStore %ok %368
%369 = OpLoad %bool %ok
OpReturnValue %369
OpFunctionEnd
%main = OpFunction %v4float None %370
%371 = OpFunctionParameter %_ptr_Function_v2float
%372 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%685 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%376 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%377 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%375 = OpCompositeConstruct %mat2v3float %376 %377
OpStore %_1_m23 %375
%378 = OpLoad %bool %_0_ok
OpSelectionMerge %380 None
OpBranchConditional %378 %379 %380
%379 = OpLabel
%381 = OpLoad %mat2v3float %_1_m23
%382 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%383 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%384 = OpCompositeConstruct %mat2v3float %382 %383
%385 = OpCompositeExtract %v3float %381 0
%386 = OpCompositeExtract %v3float %384 0
%387 = OpFOrdEqual %v3bool %385 %386
%388 = OpAll %bool %387
%389 = OpCompositeExtract %v3float %381 1
%390 = OpCompositeExtract %v3float %384 1
%391 = OpFOrdEqual %v3bool %389 %390
%392 = OpAll %bool %391
%393 = OpLogicalAnd %bool %388 %392
OpBranch %380
%380 = OpLabel
%394 = OpPhi %bool %false %372 %393 %379
OpStore %_0_ok %394
%397 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%398 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%396 = OpCompositeConstruct %mat2v4float %397 %398
OpStore %_2_m24 %396
%399 = OpLoad %bool %_0_ok
OpSelectionMerge %401 None
OpBranchConditional %399 %400 %401
%400 = OpLabel
%402 = OpLoad %mat2v4float %_2_m24
%403 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%404 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%405 = OpCompositeConstruct %mat2v4float %403 %404
%406 = OpCompositeExtract %v4float %402 0
%407 = OpCompositeExtract %v4float %405 0
%408 = OpFOrdEqual %v4bool %406 %407
%409 = OpAll %bool %408
%410 = OpCompositeExtract %v4float %402 1
%411 = OpCompositeExtract %v4float %405 1
%412 = OpFOrdEqual %v4bool %410 %411
%413 = OpAll %bool %412
%414 = OpLogicalAnd %bool %409 %413
OpBranch %401
%401 = OpLabel
%415 = OpPhi %bool %false %380 %414 %400
OpStore %_0_ok %415
%418 = OpCompositeConstruct %v2float %float_4 %float_0
%419 = OpCompositeConstruct %v2float %float_0 %float_4
%420 = OpCompositeConstruct %v2float %float_0 %float_0
%417 = OpCompositeConstruct %mat3v2float %418 %419 %420
OpStore %_3_m32 %417
%421 = OpLoad %bool %_0_ok
OpSelectionMerge %423 None
OpBranchConditional %421 %422 %423
%422 = OpLabel
%424 = OpLoad %mat3v2float %_3_m32
%425 = OpCompositeConstruct %v2float %float_4 %float_0
%426 = OpCompositeConstruct %v2float %float_0 %float_4
%427 = OpCompositeConstruct %v2float %float_0 %float_0
%428 = OpCompositeConstruct %mat3v2float %425 %426 %427
%429 = OpCompositeExtract %v2float %424 0
%430 = OpCompositeExtract %v2float %428 0
%431 = OpFOrdEqual %v2bool %429 %430
%432 = OpAll %bool %431
%433 = OpCompositeExtract %v2float %424 1
%434 = OpCompositeExtract %v2float %428 1
%435 = OpFOrdEqual %v2bool %433 %434
%436 = OpAll %bool %435
%437 = OpLogicalAnd %bool %432 %436
%438 = OpCompositeExtract %v2float %424 2
%439 = OpCompositeExtract %v2float %428 2
%440 = OpFOrdEqual %v2bool %438 %439
%441 = OpAll %bool %440
%442 = OpLogicalAnd %bool %437 %441
OpBranch %423
%423 = OpLabel
%443 = OpPhi %bool %false %401 %442 %422
OpStore %_0_ok %443
%446 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%447 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%448 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%445 = OpCompositeConstruct %mat3v4float %446 %447 %448
OpStore %_4_m34 %445
%449 = OpLoad %bool %_0_ok
OpSelectionMerge %451 None
OpBranchConditional %449 %450 %451
%450 = OpLabel
%452 = OpLoad %mat3v4float %_4_m34
%453 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%454 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%455 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%456 = OpCompositeConstruct %mat3v4float %453 %454 %455
%457 = OpCompositeExtract %v4float %452 0
%458 = OpCompositeExtract %v4float %456 0
%459 = OpFOrdEqual %v4bool %457 %458
%460 = OpAll %bool %459
%461 = OpCompositeExtract %v4float %452 1
%462 = OpCompositeExtract %v4float %456 1
%463 = OpFOrdEqual %v4bool %461 %462
%464 = OpAll %bool %463
%465 = OpLogicalAnd %bool %460 %464
%466 = OpCompositeExtract %v4float %452 2
%467 = OpCompositeExtract %v4float %456 2
%468 = OpFOrdEqual %v4bool %466 %467
%469 = OpAll %bool %468
%470 = OpLogicalAnd %bool %465 %469
OpBranch %451
%451 = OpLabel
%471 = OpPhi %bool %false %423 %470 %450
OpStore %_0_ok %471
%474 = OpCompositeConstruct %v2float %float_6 %float_0
%475 = OpCompositeConstruct %v2float %float_0 %float_6
%476 = OpCompositeConstruct %v2float %float_0 %float_0
%477 = OpCompositeConstruct %v2float %float_0 %float_0
%473 = OpCompositeConstruct %mat4v2float %474 %475 %476 %477
OpStore %_5_m42 %473
%478 = OpLoad %bool %_0_ok
OpSelectionMerge %480 None
OpBranchConditional %478 %479 %480
%479 = OpLabel
%481 = OpLoad %mat4v2float %_5_m42
%482 = OpCompositeConstruct %v2float %float_6 %float_0
%483 = OpCompositeConstruct %v2float %float_0 %float_6
%484 = OpCompositeConstruct %v2float %float_0 %float_0
%485 = OpCompositeConstruct %v2float %float_0 %float_0
%486 = OpCompositeConstruct %mat4v2float %482 %483 %484 %485
%487 = OpCompositeExtract %v2float %481 0
%488 = OpCompositeExtract %v2float %486 0
%489 = OpFOrdEqual %v2bool %487 %488
%490 = OpAll %bool %489
%491 = OpCompositeExtract %v2float %481 1
%492 = OpCompositeExtract %v2float %486 1
%493 = OpFOrdEqual %v2bool %491 %492
%494 = OpAll %bool %493
%495 = OpLogicalAnd %bool %490 %494
%496 = OpCompositeExtract %v2float %481 2
%497 = OpCompositeExtract %v2float %486 2
%498 = OpFOrdEqual %v2bool %496 %497
%499 = OpAll %bool %498
%500 = OpLogicalAnd %bool %495 %499
%501 = OpCompositeExtract %v2float %481 3
%502 = OpCompositeExtract %v2float %486 3
%503 = OpFOrdEqual %v2bool %501 %502
%504 = OpAll %bool %503
%505 = OpLogicalAnd %bool %500 %504
OpBranch %480
%480 = OpLabel
%506 = OpPhi %bool %false %451 %505 %479
OpStore %_0_ok %506
%509 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%510 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%511 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%512 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%508 = OpCompositeConstruct %mat4v3float %509 %510 %511 %512
OpStore %_6_m43 %508
%513 = OpLoad %bool %_0_ok
OpSelectionMerge %515 None
OpBranchConditional %513 %514 %515
%514 = OpLabel
%516 = OpLoad %mat4v3float %_6_m43
%517 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%518 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%519 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%520 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%521 = OpCompositeConstruct %mat4v3float %517 %518 %519 %520
%522 = OpCompositeExtract %v3float %516 0
%523 = OpCompositeExtract %v3float %521 0
%524 = OpFOrdEqual %v3bool %522 %523
%525 = OpAll %bool %524
%526 = OpCompositeExtract %v3float %516 1
%527 = OpCompositeExtract %v3float %521 1
%528 = OpFOrdEqual %v3bool %526 %527
%529 = OpAll %bool %528
%530 = OpLogicalAnd %bool %525 %529
%531 = OpCompositeExtract %v3float %516 2
%532 = OpCompositeExtract %v3float %521 2
%533 = OpFOrdEqual %v3bool %531 %532
%534 = OpAll %bool %533
%535 = OpLogicalAnd %bool %530 %534
%536 = OpCompositeExtract %v3float %516 3
%537 = OpCompositeExtract %v3float %521 3
%538 = OpFOrdEqual %v3bool %536 %537
%539 = OpAll %bool %538
%540 = OpLogicalAnd %bool %535 %539
OpBranch %515
%515 = OpLabel
%541 = OpPhi %bool %false %480 %540 %514
OpStore %_0_ok %541
%543 = OpLoad %mat3v2float %_3_m32
%544 = OpLoad %mat2v3float %_1_m23
%545 = OpMatrixTimesMatrix %mat2v2float %543 %544
OpStore %_7_m22 %545
%546 = OpLoad %bool %_0_ok
OpSelectionMerge %548 None
OpBranchConditional %546 %547 %548
%547 = OpLabel
%549 = OpLoad %mat2v2float %_7_m22
%551 = OpCompositeConstruct %v2float %float_8 %float_0
%552 = OpCompositeConstruct %v2float %float_0 %float_8
%550 = OpCompositeConstruct %mat2v2float %551 %552
%553 = OpCompositeExtract %v2float %549 0
%554 = OpCompositeExtract %v2float %550 0
%555 = OpFOrdEqual %v2bool %553 %554
%556 = OpAll %bool %555
%557 = OpCompositeExtract %v2float %549 1
%558 = OpCompositeExtract %v2float %550 1
%559 = OpFOrdEqual %v2bool %557 %558
%560 = OpAll %bool %559
%561 = OpLogicalAnd %bool %556 %560
OpBranch %548
%548 = OpLabel
%562 = OpPhi %bool %false %515 %561 %547
OpStore %_0_ok %562
%564 = OpLoad %mat4v3float %_6_m43
%565 = OpLoad %mat3v4float %_4_m34
%566 = OpMatrixTimesMatrix %mat3v3float %564 %565
OpStore %_8_m33 %566
%567 = OpLoad %bool %_0_ok
OpSelectionMerge %569 None
OpBranchConditional %567 %568 %569
%568 = OpLabel
%570 = OpLoad %mat3v3float %_8_m33
%572 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%573 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%574 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%571 = OpCompositeConstruct %mat3v3float %572 %573 %574
%575 = OpCompositeExtract %v3float %570 0
%576 = OpCompositeExtract %v3float %571 0
%577 = OpFOrdEqual %v3bool %575 %576
%578 = OpAll %bool %577
%579 = OpCompositeExtract %v3float %570 1
%580 = OpCompositeExtract %v3float %571 1
%581 = OpFOrdEqual %v3bool %579 %580
%582 = OpAll %bool %581
%583 = OpLogicalAnd %bool %578 %582
%584 = OpCompositeExtract %v3float %570 2
%585 = OpCompositeExtract %v3float %571 2
%586 = OpFOrdEqual %v3bool %584 %585
%587 = OpAll %bool %586
%588 = OpLogicalAnd %bool %583 %587
OpBranch %569
%569 = OpLabel
%589 = OpPhi %bool %false %548 %588 %568
OpStore %_0_ok %589
%590 = OpLoad %mat2v3float %_1_m23
%591 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%592 = OpCompositeConstruct %mat2v3float %591 %591
%593 = OpCompositeExtract %v3float %590 0
%594 = OpCompositeExtract %v3float %592 0
%595 = OpFAdd %v3float %593 %594
%596 = OpCompositeExtract %v3float %590 1
%597 = OpCompositeExtract %v3float %592 1
%598 = OpFAdd %v3float %596 %597
%599 = OpCompositeConstruct %mat2v3float %595 %598
OpStore %_1_m23 %599
%600 = OpLoad %bool %_0_ok
OpSelectionMerge %602 None
OpBranchConditional %600 %601 %602
%601 = OpLabel
%603 = OpLoad %mat2v3float %_1_m23
%604 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%605 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%606 = OpCompositeConstruct %mat2v3float %604 %605
%607 = OpCompositeExtract %v3float %603 0
%608 = OpCompositeExtract %v3float %606 0
%609 = OpFOrdEqual %v3bool %607 %608
%610 = OpAll %bool %609
%611 = OpCompositeExtract %v3float %603 1
%612 = OpCompositeExtract %v3float %606 1
%613 = OpFOrdEqual %v3bool %611 %612
%614 = OpAll %bool %613
%615 = OpLogicalAnd %bool %610 %614
OpBranch %602
%602 = OpLabel
%616 = OpPhi %bool %false %569 %615 %601
OpStore %_0_ok %616
%617 = OpLoad %mat3v2float %_3_m32
%618 = OpCompositeConstruct %v2float %float_2 %float_2
%619 = OpCompositeConstruct %mat3v2float %618 %618 %618
%620 = OpCompositeExtract %v2float %617 0
%621 = OpCompositeExtract %v2float %619 0
%622 = OpFSub %v2float %620 %621
%623 = OpCompositeExtract %v2float %617 1
%624 = OpCompositeExtract %v2float %619 1
%625 = OpFSub %v2float %623 %624
%626 = OpCompositeExtract %v2float %617 2
%627 = OpCompositeExtract %v2float %619 2
%628 = OpFSub %v2float %626 %627
%629 = OpCompositeConstruct %mat3v2float %622 %625 %628
OpStore %_3_m32 %629
%630 = OpLoad %bool %_0_ok
OpSelectionMerge %632 None
OpBranchConditional %630 %631 %632
%631 = OpLabel
%633 = OpLoad %mat3v2float %_3_m32
%634 = OpCompositeConstruct %v2float %float_2 %float_n2
%635 = OpCompositeConstruct %v2float %float_n2 %float_2
%636 = OpCompositeConstruct %v2float %float_n2 %float_n2
%637 = OpCompositeConstruct %mat3v2float %634 %635 %636
%638 = OpCompositeExtract %v2float %633 0
%639 = OpCompositeExtract %v2float %637 0
%640 = OpFOrdEqual %v2bool %638 %639
%641 = OpAll %bool %640
%642 = OpCompositeExtract %v2float %633 1
%643 = OpCompositeExtract %v2float %637 1
%644 = OpFOrdEqual %v2bool %642 %643
%645 = OpAll %bool %644
%646 = OpLogicalAnd %bool %641 %645
%647 = OpCompositeExtract %v2float %633 2
%648 = OpCompositeExtract %v2float %637 2
%649 = OpFOrdEqual %v2bool %647 %648
%650 = OpAll %bool %649
%651 = OpLogicalAnd %bool %646 %650
OpBranch %632
%632 = OpLabel
%652 = OpPhi %bool %false %602 %651 %631
OpStore %_0_ok %652
%653 = OpLoad %mat2v4float %_2_m24
%654 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%655 = OpCompositeConstruct %mat2v4float %654 %654
%656 = OpCompositeExtract %v4float %653 0
%657 = OpCompositeExtract %v4float %655 0
%658 = OpFDiv %v4float %656 %657
%659 = OpCompositeExtract %v4float %653 1
%660 = OpCompositeExtract %v4float %655 1
%661 = OpFDiv %v4float %659 %660
%662 = OpCompositeConstruct %mat2v4float %658 %661
OpStore %_2_m24 %662
%663 = OpLoad %bool %_0_ok
OpSelectionMerge %665 None
OpBranchConditional %663 %664 %665
%664 = OpLabel
%666 = OpLoad %mat2v4float %_2_m24
%667 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%668 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%669 = OpCompositeConstruct %mat2v4float %667 %668
%670 = OpCompositeExtract %v4float %666 0
%671 = OpCompositeExtract %v4float %669 0
%672 = OpFOrdEqual %v4bool %670 %671
%673 = OpAll %bool %672
%674 = OpCompositeExtract %v4float %666 1
%675 = OpCompositeExtract %v4float %669 1
%676 = OpFOrdEqual %v4bool %674 %675
%677 = OpAll %bool %676
%678 = OpLogicalAnd %bool %673 %677
OpBranch %665
%665 = OpLabel
%679 = OpPhi %bool %false %632 %678 %664
OpStore %_0_ok %679
%680 = OpLoad %bool %_0_ok
OpSelectionMerge %682 None
OpBranchConditional %680 %681 %682
%681 = OpLabel
%683 = OpFunctionCall %bool %test_half_b
OpBranch %682
%682 = OpLabel
%684 = OpPhi %bool %false %665 %683 %681
OpSelectionMerge %689 None
OpBranchConditional %684 %687 %688
%687 = OpLabel
%690 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%694 = OpLoad %v4float %690
OpStore %685 %694
OpBranch %689
%688 = OpLabel
%695 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%697 = OpLoad %v4float %695
OpStore %685 %697
OpBranch %689
%689 = OpLabel
%698 = OpLoad %v4float %685
OpReturnValue %698
OpFunctionEnd
