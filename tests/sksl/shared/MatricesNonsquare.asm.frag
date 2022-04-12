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
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %m23 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
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
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %503 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %557 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %620 RelaxedPrecision
OpDecorate %637 RelaxedPrecision
OpDecorate %651 RelaxedPrecision
OpDecorate %654 RelaxedPrecision
OpDecorate %655 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%35 = OpConstantComposite %v3float %float_2 %float_0 %float_0
%36 = OpConstantComposite %v3float %float_0 %float_2 %float_0
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%59 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%60 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%82 = OpConstantComposite %v2float %float_4 %float_0
%83 = OpConstantComposite %v2float %float_0 %float_4
%84 = OpConstantComposite %v2float %float_0 %float_0
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%111 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%112 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%113 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%139 = OpConstantComposite %v2float %float_6 %float_0
%140 = OpConstantComposite %v2float %float_0 %float_6
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%171 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%172 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%173 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%174 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%212 = OpConstantComposite %v2float %float_8 %float_0
%213 = OpConstantComposite %v2float %float_0 %float_8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%236 = OpConstantComposite %v3float %float_35 %float_0 %float_0
%237 = OpConstantComposite %v3float %float_0 %float_35 %float_0
%238 = OpConstantComposite %v3float %float_0 %float_0 %float_35
%float_1 = OpConstant %float 1
%256 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%269 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%270 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%283 = OpConstantComposite %v2float %float_2 %float_2
%float_n2 = OpConstant %float -2
%300 = OpConstantComposite %v2float %float_2 %float_n2
%301 = OpConstantComposite %v2float %float_n2 %float_2
%302 = OpConstantComposite %v2float %float_n2 %float_n2
%320 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_0_75 = OpConstant %float 0.75
%334 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%335 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%348 = OpTypeFunction %v4float %_ptr_Function_v2float
%354 = OpConstantComposite %v3float %float_2 %float_0 %float_0
%355 = OpConstantComposite %v3float %float_0 %float_2 %float_0
%373 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%374 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%392 = OpConstantComposite %v2float %float_4 %float_0
%393 = OpConstantComposite %v2float %float_0 %float_4
%416 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%417 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%418 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%441 = OpConstantComposite %v2float %float_6 %float_0
%442 = OpConstantComposite %v2float %float_0 %float_6
%470 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%471 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%472 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%473 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%508 = OpConstantComposite %v2float %float_8 %float_0
%509 = OpConstantComposite %v2float %float_0 %float_8
%529 = OpConstantComposite %v3float %float_35 %float_0 %float_0
%530 = OpConstantComposite %v3float %float_0 %float_35 %float_0
%531 = OpConstantComposite %v3float %float_0 %float_0 %float_35
%548 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%561 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%562 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%575 = OpConstantComposite %v2float %float_2 %float_2
%591 = OpConstantComposite %v2float %float_2 %float_n2
%592 = OpConstantComposite %v2float %float_n2 %float_2
%593 = OpConstantComposite %v2float %float_n2 %float_n2
%611 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%624 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%625 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
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
%34 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %m23 %34
%38 = OpLoad %bool %ok
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%41 = OpLoad %mat2v3float %m23
%42 = OpCompositeConstruct %mat2v3float %35 %36
%44 = OpCompositeExtract %v3float %41 0
%45 = OpCompositeExtract %v3float %42 0
%46 = OpFOrdEqual %v3bool %44 %45
%47 = OpAll %bool %46
%48 = OpCompositeExtract %v3float %41 1
%49 = OpCompositeExtract %v3float %42 1
%50 = OpFOrdEqual %v3bool %48 %49
%51 = OpAll %bool %50
%52 = OpLogicalAnd %bool %47 %51
OpBranch %40
%40 = OpLabel
%53 = OpPhi %bool %false %25 %52 %39
OpStore %ok %53
%58 = OpCompositeConstruct %mat2v4float %59 %60
OpStore %m24 %58
%61 = OpLoad %bool %ok
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpLoad %mat2v4float %m24
%65 = OpCompositeConstruct %mat2v4float %59 %60
%67 = OpCompositeExtract %v4float %64 0
%68 = OpCompositeExtract %v4float %65 0
%69 = OpFOrdEqual %v4bool %67 %68
%70 = OpAll %bool %69
%71 = OpCompositeExtract %v4float %64 1
%72 = OpCompositeExtract %v4float %65 1
%73 = OpFOrdEqual %v4bool %71 %72
%74 = OpAll %bool %73
%75 = OpLogicalAnd %bool %70 %74
OpBranch %63
%63 = OpLabel
%76 = OpPhi %bool %false %40 %75 %62
OpStore %ok %76
%81 = OpCompositeConstruct %mat3v2float %82 %83 %84
OpStore %m32 %81
%85 = OpLoad %bool %ok
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %mat3v2float %m32
%89 = OpCompositeConstruct %mat3v2float %82 %83 %84
%91 = OpCompositeExtract %v2float %88 0
%92 = OpCompositeExtract %v2float %89 0
%93 = OpFOrdEqual %v2bool %91 %92
%94 = OpAll %bool %93
%95 = OpCompositeExtract %v2float %88 1
%96 = OpCompositeExtract %v2float %89 1
%97 = OpFOrdEqual %v2bool %95 %96
%98 = OpAll %bool %97
%99 = OpLogicalAnd %bool %94 %98
%100 = OpCompositeExtract %v2float %88 2
%101 = OpCompositeExtract %v2float %89 2
%102 = OpFOrdEqual %v2bool %100 %101
%103 = OpAll %bool %102
%104 = OpLogicalAnd %bool %99 %103
OpBranch %87
%87 = OpLabel
%105 = OpPhi %bool %false %63 %104 %86
OpStore %ok %105
%110 = OpCompositeConstruct %mat3v4float %111 %112 %113
OpStore %m34 %110
%114 = OpLoad %bool %ok
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%117 = OpLoad %mat3v4float %m34
%118 = OpCompositeConstruct %mat3v4float %111 %112 %113
%119 = OpCompositeExtract %v4float %117 0
%120 = OpCompositeExtract %v4float %118 0
%121 = OpFOrdEqual %v4bool %119 %120
%122 = OpAll %bool %121
%123 = OpCompositeExtract %v4float %117 1
%124 = OpCompositeExtract %v4float %118 1
%125 = OpFOrdEqual %v4bool %123 %124
%126 = OpAll %bool %125
%127 = OpLogicalAnd %bool %122 %126
%128 = OpCompositeExtract %v4float %117 2
%129 = OpCompositeExtract %v4float %118 2
%130 = OpFOrdEqual %v4bool %128 %129
%131 = OpAll %bool %130
%132 = OpLogicalAnd %bool %127 %131
OpBranch %116
%116 = OpLabel
%133 = OpPhi %bool %false %87 %132 %115
OpStore %ok %133
%138 = OpCompositeConstruct %mat4v2float %139 %140 %84 %84
OpStore %m42 %138
%141 = OpLoad %bool %ok
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%144 = OpLoad %mat4v2float %m42
%145 = OpCompositeConstruct %mat4v2float %139 %140 %84 %84
%146 = OpCompositeExtract %v2float %144 0
%147 = OpCompositeExtract %v2float %145 0
%148 = OpFOrdEqual %v2bool %146 %147
%149 = OpAll %bool %148
%150 = OpCompositeExtract %v2float %144 1
%151 = OpCompositeExtract %v2float %145 1
%152 = OpFOrdEqual %v2bool %150 %151
%153 = OpAll %bool %152
%154 = OpLogicalAnd %bool %149 %153
%155 = OpCompositeExtract %v2float %144 2
%156 = OpCompositeExtract %v2float %145 2
%157 = OpFOrdEqual %v2bool %155 %156
%158 = OpAll %bool %157
%159 = OpLogicalAnd %bool %154 %158
%160 = OpCompositeExtract %v2float %144 3
%161 = OpCompositeExtract %v2float %145 3
%162 = OpFOrdEqual %v2bool %160 %161
%163 = OpAll %bool %162
%164 = OpLogicalAnd %bool %159 %163
OpBranch %143
%143 = OpLabel
%165 = OpPhi %bool %false %116 %164 %142
OpStore %ok %165
%170 = OpCompositeConstruct %mat4v3float %171 %172 %173 %174
OpStore %m43 %170
%175 = OpLoad %bool %ok
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%178 = OpLoad %mat4v3float %m43
%179 = OpCompositeConstruct %mat4v3float %171 %172 %173 %174
%180 = OpCompositeExtract %v3float %178 0
%181 = OpCompositeExtract %v3float %179 0
%182 = OpFOrdEqual %v3bool %180 %181
%183 = OpAll %bool %182
%184 = OpCompositeExtract %v3float %178 1
%185 = OpCompositeExtract %v3float %179 1
%186 = OpFOrdEqual %v3bool %184 %185
%187 = OpAll %bool %186
%188 = OpLogicalAnd %bool %183 %187
%189 = OpCompositeExtract %v3float %178 2
%190 = OpCompositeExtract %v3float %179 2
%191 = OpFOrdEqual %v3bool %189 %190
%192 = OpAll %bool %191
%193 = OpLogicalAnd %bool %188 %192
%194 = OpCompositeExtract %v3float %178 3
%195 = OpCompositeExtract %v3float %179 3
%196 = OpFOrdEqual %v3bool %194 %195
%197 = OpAll %bool %196
%198 = OpLogicalAnd %bool %193 %197
OpBranch %177
%177 = OpLabel
%199 = OpPhi %bool %false %143 %198 %176
OpStore %ok %199
%203 = OpLoad %mat3v2float %m32
%204 = OpLoad %mat2v3float %m23
%205 = OpMatrixTimesMatrix %mat2v2float %203 %204
OpStore %m22 %205
%206 = OpLoad %bool %ok
OpSelectionMerge %208 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
%209 = OpLoad %mat2v2float %m22
%211 = OpCompositeConstruct %mat2v2float %212 %213
%214 = OpCompositeExtract %v2float %209 0
%215 = OpCompositeExtract %v2float %211 0
%216 = OpFOrdEqual %v2bool %214 %215
%217 = OpAll %bool %216
%218 = OpCompositeExtract %v2float %209 1
%219 = OpCompositeExtract %v2float %211 1
%220 = OpFOrdEqual %v2bool %218 %219
%221 = OpAll %bool %220
%222 = OpLogicalAnd %bool %217 %221
OpBranch %208
%208 = OpLabel
%223 = OpPhi %bool %false %177 %222 %207
OpStore %ok %223
%227 = OpLoad %mat4v3float %m43
%228 = OpLoad %mat3v4float %m34
%229 = OpMatrixTimesMatrix %mat3v3float %227 %228
OpStore %m33 %229
%230 = OpLoad %bool %ok
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %mat3v3float %m33
%235 = OpCompositeConstruct %mat3v3float %236 %237 %238
%239 = OpCompositeExtract %v3float %233 0
%240 = OpCompositeExtract %v3float %235 0
%241 = OpFOrdEqual %v3bool %239 %240
%242 = OpAll %bool %241
%243 = OpCompositeExtract %v3float %233 1
%244 = OpCompositeExtract %v3float %235 1
%245 = OpFOrdEqual %v3bool %243 %244
%246 = OpAll %bool %245
%247 = OpLogicalAnd %bool %242 %246
%248 = OpCompositeExtract %v3float %233 2
%249 = OpCompositeExtract %v3float %235 2
%250 = OpFOrdEqual %v3bool %248 %249
%251 = OpAll %bool %250
%252 = OpLogicalAnd %bool %247 %251
OpBranch %232
%232 = OpLabel
%253 = OpPhi %bool %false %208 %252 %231
OpStore %ok %253
%254 = OpLoad %mat2v3float %m23
%257 = OpCompositeConstruct %mat2v3float %256 %256
%258 = OpCompositeExtract %v3float %254 0
%259 = OpCompositeExtract %v3float %257 0
%260 = OpFAdd %v3float %258 %259
%261 = OpCompositeExtract %v3float %254 1
%262 = OpCompositeExtract %v3float %257 1
%263 = OpFAdd %v3float %261 %262
%264 = OpCompositeConstruct %mat2v3float %260 %263
OpStore %m23 %264
%265 = OpLoad %bool %ok
OpSelectionMerge %267 None
OpBranchConditional %265 %266 %267
%266 = OpLabel
%268 = OpLoad %mat2v3float %m23
%271 = OpCompositeConstruct %mat2v3float %269 %270
%272 = OpCompositeExtract %v3float %268 0
%273 = OpCompositeExtract %v3float %271 0
%274 = OpFOrdEqual %v3bool %272 %273
%275 = OpAll %bool %274
%276 = OpCompositeExtract %v3float %268 1
%277 = OpCompositeExtract %v3float %271 1
%278 = OpFOrdEqual %v3bool %276 %277
%279 = OpAll %bool %278
%280 = OpLogicalAnd %bool %275 %279
OpBranch %267
%267 = OpLabel
%281 = OpPhi %bool %false %232 %280 %266
OpStore %ok %281
%282 = OpLoad %mat3v2float %m32
%284 = OpCompositeConstruct %mat3v2float %283 %283 %283
%285 = OpCompositeExtract %v2float %282 0
%286 = OpCompositeExtract %v2float %284 0
%287 = OpFSub %v2float %285 %286
%288 = OpCompositeExtract %v2float %282 1
%289 = OpCompositeExtract %v2float %284 1
%290 = OpFSub %v2float %288 %289
%291 = OpCompositeExtract %v2float %282 2
%292 = OpCompositeExtract %v2float %284 2
%293 = OpFSub %v2float %291 %292
%294 = OpCompositeConstruct %mat3v2float %287 %290 %293
OpStore %m32 %294
%295 = OpLoad %bool %ok
OpSelectionMerge %297 None
OpBranchConditional %295 %296 %297
%296 = OpLabel
%298 = OpLoad %mat3v2float %m32
%303 = OpCompositeConstruct %mat3v2float %300 %301 %302
%304 = OpCompositeExtract %v2float %298 0
%305 = OpCompositeExtract %v2float %303 0
%306 = OpFOrdEqual %v2bool %304 %305
%307 = OpAll %bool %306
%308 = OpCompositeExtract %v2float %298 1
%309 = OpCompositeExtract %v2float %303 1
%310 = OpFOrdEqual %v2bool %308 %309
%311 = OpAll %bool %310
%312 = OpLogicalAnd %bool %307 %311
%313 = OpCompositeExtract %v2float %298 2
%314 = OpCompositeExtract %v2float %303 2
%315 = OpFOrdEqual %v2bool %313 %314
%316 = OpAll %bool %315
%317 = OpLogicalAnd %bool %312 %316
OpBranch %297
%297 = OpLabel
%318 = OpPhi %bool %false %267 %317 %296
OpStore %ok %318
%319 = OpLoad %mat2v4float %m24
%321 = OpCompositeConstruct %mat2v4float %320 %320
%322 = OpCompositeExtract %v4float %319 0
%323 = OpCompositeExtract %v4float %321 0
%324 = OpFDiv %v4float %322 %323
%325 = OpCompositeExtract %v4float %319 1
%326 = OpCompositeExtract %v4float %321 1
%327 = OpFDiv %v4float %325 %326
%328 = OpCompositeConstruct %mat2v4float %324 %327
OpStore %m24 %328
%329 = OpLoad %bool %ok
OpSelectionMerge %331 None
OpBranchConditional %329 %330 %331
%330 = OpLabel
%332 = OpLoad %mat2v4float %m24
%336 = OpCompositeConstruct %mat2v4float %334 %335
%337 = OpCompositeExtract %v4float %332 0
%338 = OpCompositeExtract %v4float %336 0
%339 = OpFOrdEqual %v4bool %337 %338
%340 = OpAll %bool %339
%341 = OpCompositeExtract %v4float %332 1
%342 = OpCompositeExtract %v4float %336 1
%343 = OpFOrdEqual %v4bool %341 %342
%344 = OpAll %bool %343
%345 = OpLogicalAnd %bool %340 %344
OpBranch %331
%331 = OpLabel
%346 = OpPhi %bool %false %297 %345 %330
OpStore %ok %346
%347 = OpLoad %bool %ok
OpReturnValue %347
OpFunctionEnd
%main = OpFunction %v4float None %348
%349 = OpFunctionParameter %_ptr_Function_v2float
%350 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%642 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%353 = OpCompositeConstruct %mat2v3float %354 %355
OpStore %_1_m23 %353
%356 = OpLoad %bool %_0_ok
OpSelectionMerge %358 None
OpBranchConditional %356 %357 %358
%357 = OpLabel
%359 = OpLoad %mat2v3float %_1_m23
%360 = OpCompositeConstruct %mat2v3float %354 %355
%361 = OpCompositeExtract %v3float %359 0
%362 = OpCompositeExtract %v3float %360 0
%363 = OpFOrdEqual %v3bool %361 %362
%364 = OpAll %bool %363
%365 = OpCompositeExtract %v3float %359 1
%366 = OpCompositeExtract %v3float %360 1
%367 = OpFOrdEqual %v3bool %365 %366
%368 = OpAll %bool %367
%369 = OpLogicalAnd %bool %364 %368
OpBranch %358
%358 = OpLabel
%370 = OpPhi %bool %false %350 %369 %357
OpStore %_0_ok %370
%372 = OpCompositeConstruct %mat2v4float %373 %374
OpStore %_2_m24 %372
%375 = OpLoad %bool %_0_ok
OpSelectionMerge %377 None
OpBranchConditional %375 %376 %377
%376 = OpLabel
%378 = OpLoad %mat2v4float %_2_m24
%379 = OpCompositeConstruct %mat2v4float %373 %374
%380 = OpCompositeExtract %v4float %378 0
%381 = OpCompositeExtract %v4float %379 0
%382 = OpFOrdEqual %v4bool %380 %381
%383 = OpAll %bool %382
%384 = OpCompositeExtract %v4float %378 1
%385 = OpCompositeExtract %v4float %379 1
%386 = OpFOrdEqual %v4bool %384 %385
%387 = OpAll %bool %386
%388 = OpLogicalAnd %bool %383 %387
OpBranch %377
%377 = OpLabel
%389 = OpPhi %bool %false %358 %388 %376
OpStore %_0_ok %389
%391 = OpCompositeConstruct %mat3v2float %392 %393 %20
OpStore %_3_m32 %391
%394 = OpLoad %bool %_0_ok
OpSelectionMerge %396 None
OpBranchConditional %394 %395 %396
%395 = OpLabel
%397 = OpLoad %mat3v2float %_3_m32
%398 = OpCompositeConstruct %mat3v2float %392 %393 %20
%399 = OpCompositeExtract %v2float %397 0
%400 = OpCompositeExtract %v2float %398 0
%401 = OpFOrdEqual %v2bool %399 %400
%402 = OpAll %bool %401
%403 = OpCompositeExtract %v2float %397 1
%404 = OpCompositeExtract %v2float %398 1
%405 = OpFOrdEqual %v2bool %403 %404
%406 = OpAll %bool %405
%407 = OpLogicalAnd %bool %402 %406
%408 = OpCompositeExtract %v2float %397 2
%409 = OpCompositeExtract %v2float %398 2
%410 = OpFOrdEqual %v2bool %408 %409
%411 = OpAll %bool %410
%412 = OpLogicalAnd %bool %407 %411
OpBranch %396
%396 = OpLabel
%413 = OpPhi %bool %false %377 %412 %395
OpStore %_0_ok %413
%415 = OpCompositeConstruct %mat3v4float %416 %417 %418
OpStore %_4_m34 %415
%419 = OpLoad %bool %_0_ok
OpSelectionMerge %421 None
OpBranchConditional %419 %420 %421
%420 = OpLabel
%422 = OpLoad %mat3v4float %_4_m34
%423 = OpCompositeConstruct %mat3v4float %416 %417 %418
%424 = OpCompositeExtract %v4float %422 0
%425 = OpCompositeExtract %v4float %423 0
%426 = OpFOrdEqual %v4bool %424 %425
%427 = OpAll %bool %426
%428 = OpCompositeExtract %v4float %422 1
%429 = OpCompositeExtract %v4float %423 1
%430 = OpFOrdEqual %v4bool %428 %429
%431 = OpAll %bool %430
%432 = OpLogicalAnd %bool %427 %431
%433 = OpCompositeExtract %v4float %422 2
%434 = OpCompositeExtract %v4float %423 2
%435 = OpFOrdEqual %v4bool %433 %434
%436 = OpAll %bool %435
%437 = OpLogicalAnd %bool %432 %436
OpBranch %421
%421 = OpLabel
%438 = OpPhi %bool %false %396 %437 %420
OpStore %_0_ok %438
%440 = OpCompositeConstruct %mat4v2float %441 %442 %20 %20
OpStore %_5_m42 %440
%443 = OpLoad %bool %_0_ok
OpSelectionMerge %445 None
OpBranchConditional %443 %444 %445
%444 = OpLabel
%446 = OpLoad %mat4v2float %_5_m42
%447 = OpCompositeConstruct %mat4v2float %441 %442 %20 %20
%448 = OpCompositeExtract %v2float %446 0
%449 = OpCompositeExtract %v2float %447 0
%450 = OpFOrdEqual %v2bool %448 %449
%451 = OpAll %bool %450
%452 = OpCompositeExtract %v2float %446 1
%453 = OpCompositeExtract %v2float %447 1
%454 = OpFOrdEqual %v2bool %452 %453
%455 = OpAll %bool %454
%456 = OpLogicalAnd %bool %451 %455
%457 = OpCompositeExtract %v2float %446 2
%458 = OpCompositeExtract %v2float %447 2
%459 = OpFOrdEqual %v2bool %457 %458
%460 = OpAll %bool %459
%461 = OpLogicalAnd %bool %456 %460
%462 = OpCompositeExtract %v2float %446 3
%463 = OpCompositeExtract %v2float %447 3
%464 = OpFOrdEqual %v2bool %462 %463
%465 = OpAll %bool %464
%466 = OpLogicalAnd %bool %461 %465
OpBranch %445
%445 = OpLabel
%467 = OpPhi %bool %false %421 %466 %444
OpStore %_0_ok %467
%469 = OpCompositeConstruct %mat4v3float %470 %471 %472 %473
OpStore %_6_m43 %469
%474 = OpLoad %bool %_0_ok
OpSelectionMerge %476 None
OpBranchConditional %474 %475 %476
%475 = OpLabel
%477 = OpLoad %mat4v3float %_6_m43
%478 = OpCompositeConstruct %mat4v3float %470 %471 %472 %473
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
%493 = OpCompositeExtract %v3float %477 3
%494 = OpCompositeExtract %v3float %478 3
%495 = OpFOrdEqual %v3bool %493 %494
%496 = OpAll %bool %495
%497 = OpLogicalAnd %bool %492 %496
OpBranch %476
%476 = OpLabel
%498 = OpPhi %bool %false %445 %497 %475
OpStore %_0_ok %498
%500 = OpLoad %mat3v2float %_3_m32
%501 = OpLoad %mat2v3float %_1_m23
%502 = OpMatrixTimesMatrix %mat2v2float %500 %501
OpStore %_7_m22 %502
%503 = OpLoad %bool %_0_ok
OpSelectionMerge %505 None
OpBranchConditional %503 %504 %505
%504 = OpLabel
%506 = OpLoad %mat2v2float %_7_m22
%507 = OpCompositeConstruct %mat2v2float %508 %509
%510 = OpCompositeExtract %v2float %506 0
%511 = OpCompositeExtract %v2float %507 0
%512 = OpFOrdEqual %v2bool %510 %511
%513 = OpAll %bool %512
%514 = OpCompositeExtract %v2float %506 1
%515 = OpCompositeExtract %v2float %507 1
%516 = OpFOrdEqual %v2bool %514 %515
%517 = OpAll %bool %516
%518 = OpLogicalAnd %bool %513 %517
OpBranch %505
%505 = OpLabel
%519 = OpPhi %bool %false %476 %518 %504
OpStore %_0_ok %519
%521 = OpLoad %mat4v3float %_6_m43
%522 = OpLoad %mat3v4float %_4_m34
%523 = OpMatrixTimesMatrix %mat3v3float %521 %522
OpStore %_8_m33 %523
%524 = OpLoad %bool %_0_ok
OpSelectionMerge %526 None
OpBranchConditional %524 %525 %526
%525 = OpLabel
%527 = OpLoad %mat3v3float %_8_m33
%528 = OpCompositeConstruct %mat3v3float %529 %530 %531
%532 = OpCompositeExtract %v3float %527 0
%533 = OpCompositeExtract %v3float %528 0
%534 = OpFOrdEqual %v3bool %532 %533
%535 = OpAll %bool %534
%536 = OpCompositeExtract %v3float %527 1
%537 = OpCompositeExtract %v3float %528 1
%538 = OpFOrdEqual %v3bool %536 %537
%539 = OpAll %bool %538
%540 = OpLogicalAnd %bool %535 %539
%541 = OpCompositeExtract %v3float %527 2
%542 = OpCompositeExtract %v3float %528 2
%543 = OpFOrdEqual %v3bool %541 %542
%544 = OpAll %bool %543
%545 = OpLogicalAnd %bool %540 %544
OpBranch %526
%526 = OpLabel
%546 = OpPhi %bool %false %505 %545 %525
OpStore %_0_ok %546
%547 = OpLoad %mat2v3float %_1_m23
%549 = OpCompositeConstruct %mat2v3float %548 %548
%550 = OpCompositeExtract %v3float %547 0
%551 = OpCompositeExtract %v3float %549 0
%552 = OpFAdd %v3float %550 %551
%553 = OpCompositeExtract %v3float %547 1
%554 = OpCompositeExtract %v3float %549 1
%555 = OpFAdd %v3float %553 %554
%556 = OpCompositeConstruct %mat2v3float %552 %555
OpStore %_1_m23 %556
%557 = OpLoad %bool %_0_ok
OpSelectionMerge %559 None
OpBranchConditional %557 %558 %559
%558 = OpLabel
%560 = OpLoad %mat2v3float %_1_m23
%563 = OpCompositeConstruct %mat2v3float %561 %562
%564 = OpCompositeExtract %v3float %560 0
%565 = OpCompositeExtract %v3float %563 0
%566 = OpFOrdEqual %v3bool %564 %565
%567 = OpAll %bool %566
%568 = OpCompositeExtract %v3float %560 1
%569 = OpCompositeExtract %v3float %563 1
%570 = OpFOrdEqual %v3bool %568 %569
%571 = OpAll %bool %570
%572 = OpLogicalAnd %bool %567 %571
OpBranch %559
%559 = OpLabel
%573 = OpPhi %bool %false %526 %572 %558
OpStore %_0_ok %573
%574 = OpLoad %mat3v2float %_3_m32
%576 = OpCompositeConstruct %mat3v2float %575 %575 %575
%577 = OpCompositeExtract %v2float %574 0
%578 = OpCompositeExtract %v2float %576 0
%579 = OpFSub %v2float %577 %578
%580 = OpCompositeExtract %v2float %574 1
%581 = OpCompositeExtract %v2float %576 1
%582 = OpFSub %v2float %580 %581
%583 = OpCompositeExtract %v2float %574 2
%584 = OpCompositeExtract %v2float %576 2
%585 = OpFSub %v2float %583 %584
%586 = OpCompositeConstruct %mat3v2float %579 %582 %585
OpStore %_3_m32 %586
%587 = OpLoad %bool %_0_ok
OpSelectionMerge %589 None
OpBranchConditional %587 %588 %589
%588 = OpLabel
%590 = OpLoad %mat3v2float %_3_m32
%594 = OpCompositeConstruct %mat3v2float %591 %592 %593
%595 = OpCompositeExtract %v2float %590 0
%596 = OpCompositeExtract %v2float %594 0
%597 = OpFOrdEqual %v2bool %595 %596
%598 = OpAll %bool %597
%599 = OpCompositeExtract %v2float %590 1
%600 = OpCompositeExtract %v2float %594 1
%601 = OpFOrdEqual %v2bool %599 %600
%602 = OpAll %bool %601
%603 = OpLogicalAnd %bool %598 %602
%604 = OpCompositeExtract %v2float %590 2
%605 = OpCompositeExtract %v2float %594 2
%606 = OpFOrdEqual %v2bool %604 %605
%607 = OpAll %bool %606
%608 = OpLogicalAnd %bool %603 %607
OpBranch %589
%589 = OpLabel
%609 = OpPhi %bool %false %559 %608 %588
OpStore %_0_ok %609
%610 = OpLoad %mat2v4float %_2_m24
%612 = OpCompositeConstruct %mat2v4float %611 %611
%613 = OpCompositeExtract %v4float %610 0
%614 = OpCompositeExtract %v4float %612 0
%615 = OpFDiv %v4float %613 %614
%616 = OpCompositeExtract %v4float %610 1
%617 = OpCompositeExtract %v4float %612 1
%618 = OpFDiv %v4float %616 %617
%619 = OpCompositeConstruct %mat2v4float %615 %618
OpStore %_2_m24 %619
%620 = OpLoad %bool %_0_ok
OpSelectionMerge %622 None
OpBranchConditional %620 %621 %622
%621 = OpLabel
%623 = OpLoad %mat2v4float %_2_m24
%626 = OpCompositeConstruct %mat2v4float %624 %625
%627 = OpCompositeExtract %v4float %623 0
%628 = OpCompositeExtract %v4float %626 0
%629 = OpFOrdEqual %v4bool %627 %628
%630 = OpAll %bool %629
%631 = OpCompositeExtract %v4float %623 1
%632 = OpCompositeExtract %v4float %626 1
%633 = OpFOrdEqual %v4bool %631 %632
%634 = OpAll %bool %633
%635 = OpLogicalAnd %bool %630 %634
OpBranch %622
%622 = OpLabel
%636 = OpPhi %bool %false %589 %635 %621
OpStore %_0_ok %636
%637 = OpLoad %bool %_0_ok
OpSelectionMerge %639 None
OpBranchConditional %637 %638 %639
%638 = OpLabel
%640 = OpFunctionCall %bool %test_half_b
OpBranch %639
%639 = OpLabel
%641 = OpPhi %bool %false %622 %640 %638
OpSelectionMerge %646 None
OpBranchConditional %641 %644 %645
%644 = OpLabel
%647 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%651 = OpLoad %v4float %647
OpStore %642 %651
OpBranch %646
%645 = OpLabel
%652 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%654 = OpLoad %v4float %652
OpStore %642 %654
OpBranch %646
%646 = OpLabel
%655 = OpLoad %v4float %642
OpReturnValue %655
OpFunctionEnd
