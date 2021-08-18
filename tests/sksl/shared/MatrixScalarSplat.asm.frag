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
OpName %main "main"
OpName %_0_ok "_0_ok"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
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
OpDecorate %306 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %517 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %572 RelaxedPrecision
OpDecorate %586 RelaxedPrecision
OpDecorate %589 RelaxedPrecision
OpDecorate %590 RelaxedPrecision
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
%false = OpConstantFalse %bool
%float_2 = OpConstant %float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%float_4 = OpConstant %float 4
%float_6 = OpConstant %float 6
%v3bool = OpTypeVector %bool 3
%float_n2 = OpConstant %float -2
%float_n4 = OpConstant %float -4
%float_8 = OpConstant %float 8
%float_0_25 = OpConstant %float 0.25
%float_0_5 = OpConstant %float 0.5
%mat2v2float = OpTypeMatrix %v2float 2
%v2bool = OpTypeVector %bool 2
%307 = OpTypeFunction %v4float %_ptr_Function_v2float
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
OpStore %ok %true
%32 = OpLoad %bool %ok
OpSelectionMerge %34 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
%38 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%39 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%40 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%36 = OpCompositeConstruct %mat3v3float %38 %39 %40
%43 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%44 = OpCompositeConstruct %mat3v3float %43 %43 %43
%45 = OpCompositeExtract %v3float %36 0
%46 = OpCompositeExtract %v3float %44 0
%47 = OpFAdd %v3float %45 %46
%48 = OpCompositeExtract %v3float %36 1
%49 = OpCompositeExtract %v3float %44 1
%50 = OpFAdd %v3float %48 %49
%51 = OpCompositeExtract %v3float %36 2
%52 = OpCompositeExtract %v3float %44 2
%53 = OpFAdd %v3float %51 %52
%54 = OpCompositeConstruct %mat3v3float %47 %50 %53
%56 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%57 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%58 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%59 = OpCompositeConstruct %mat3v3float %56 %57 %58
%61 = OpCompositeExtract %v3float %54 0
%62 = OpCompositeExtract %v3float %59 0
%63 = OpFOrdEqual %v3bool %61 %62
%64 = OpAll %bool %63
%65 = OpCompositeExtract %v3float %54 1
%66 = OpCompositeExtract %v3float %59 1
%67 = OpFOrdEqual %v3bool %65 %66
%68 = OpAll %bool %67
%69 = OpLogicalAnd %bool %64 %68
%70 = OpCompositeExtract %v3float %54 2
%71 = OpCompositeExtract %v3float %59 2
%72 = OpFOrdEqual %v3bool %70 %71
%73 = OpAll %bool %72
%74 = OpLogicalAnd %bool %69 %73
OpBranch %34
%34 = OpLabel
%75 = OpPhi %bool %false %27 %74 %33
OpStore %ok %75
%76 = OpLoad %bool %ok
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%80 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%81 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%82 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%79 = OpCompositeConstruct %mat3v3float %80 %81 %82
%83 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%84 = OpCompositeConstruct %mat3v3float %83 %83 %83
%85 = OpCompositeExtract %v3float %79 0
%86 = OpCompositeExtract %v3float %84 0
%87 = OpFSub %v3float %85 %86
%88 = OpCompositeExtract %v3float %79 1
%89 = OpCompositeExtract %v3float %84 1
%90 = OpFSub %v3float %88 %89
%91 = OpCompositeExtract %v3float %79 2
%92 = OpCompositeExtract %v3float %84 2
%93 = OpFSub %v3float %91 %92
%94 = OpCompositeConstruct %mat3v3float %87 %90 %93
%97 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%98 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%99 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%100 = OpCompositeConstruct %mat3v3float %97 %98 %99
%101 = OpCompositeExtract %v3float %94 0
%102 = OpCompositeExtract %v3float %100 0
%103 = OpFOrdEqual %v3bool %101 %102
%104 = OpAll %bool %103
%105 = OpCompositeExtract %v3float %94 1
%106 = OpCompositeExtract %v3float %100 1
%107 = OpFOrdEqual %v3bool %105 %106
%108 = OpAll %bool %107
%109 = OpLogicalAnd %bool %104 %108
%110 = OpCompositeExtract %v3float %94 2
%111 = OpCompositeExtract %v3float %100 2
%112 = OpFOrdEqual %v3bool %110 %111
%113 = OpAll %bool %112
%114 = OpLogicalAnd %bool %109 %113
OpBranch %78
%78 = OpLabel
%115 = OpPhi %bool %false %34 %114 %77
OpStore %ok %115
%116 = OpLoad %bool %ok
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%120 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%121 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%122 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%119 = OpCompositeConstruct %mat3v3float %120 %121 %122
%123 = OpMatrixTimesScalar %mat3v3float %119 %float_4
%126 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%127 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%128 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%125 = OpCompositeConstruct %mat3v3float %126 %127 %128
%129 = OpCompositeExtract %v3float %123 0
%130 = OpCompositeExtract %v3float %125 0
%131 = OpFOrdEqual %v3bool %129 %130
%132 = OpAll %bool %131
%133 = OpCompositeExtract %v3float %123 1
%134 = OpCompositeExtract %v3float %125 1
%135 = OpFOrdEqual %v3bool %133 %134
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %132 %136
%138 = OpCompositeExtract %v3float %123 2
%139 = OpCompositeExtract %v3float %125 2
%140 = OpFOrdEqual %v3bool %138 %139
%141 = OpAll %bool %140
%142 = OpLogicalAnd %bool %137 %141
OpBranch %118
%118 = OpLabel
%143 = OpPhi %bool %false %78 %142 %117
OpStore %ok %143
%144 = OpLoad %bool %ok
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%148 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%149 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%150 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%147 = OpCompositeConstruct %mat3v3float %148 %149 %150
%152 = OpMatrixTimesScalar %mat3v3float %147 %float_0_25
%155 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%156 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%157 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%154 = OpCompositeConstruct %mat3v3float %155 %156 %157
%158 = OpCompositeExtract %v3float %152 0
%159 = OpCompositeExtract %v3float %154 0
%160 = OpFOrdEqual %v3bool %158 %159
%161 = OpAll %bool %160
%162 = OpCompositeExtract %v3float %152 1
%163 = OpCompositeExtract %v3float %154 1
%164 = OpFOrdEqual %v3bool %162 %163
%165 = OpAll %bool %164
%166 = OpLogicalAnd %bool %161 %165
%167 = OpCompositeExtract %v3float %152 2
%168 = OpCompositeExtract %v3float %154 2
%169 = OpFOrdEqual %v3bool %167 %168
%170 = OpAll %bool %169
%171 = OpLogicalAnd %bool %166 %170
OpBranch %146
%146 = OpLabel
%172 = OpPhi %bool %false %118 %171 %145
OpStore %ok %172
%173 = OpLoad %bool %ok
OpSelectionMerge %175 None
OpBranchConditional %173 %174 %175
%174 = OpLabel
%177 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%178 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%179 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%176 = OpCompositeConstruct %mat3v3float %177 %178 %179
%180 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%181 = OpCompositeConstruct %mat3v3float %180 %180 %180
%182 = OpCompositeExtract %v3float %181 0
%183 = OpCompositeExtract %v3float %176 0
%184 = OpFAdd %v3float %182 %183
%185 = OpCompositeExtract %v3float %181 1
%186 = OpCompositeExtract %v3float %176 1
%187 = OpFAdd %v3float %185 %186
%188 = OpCompositeExtract %v3float %181 2
%189 = OpCompositeExtract %v3float %176 2
%190 = OpFAdd %v3float %188 %189
%191 = OpCompositeConstruct %mat3v3float %184 %187 %190
%192 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%193 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%194 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%195 = OpCompositeConstruct %mat3v3float %192 %193 %194
%196 = OpCompositeExtract %v3float %191 0
%197 = OpCompositeExtract %v3float %195 0
%198 = OpFOrdEqual %v3bool %196 %197
%199 = OpAll %bool %198
%200 = OpCompositeExtract %v3float %191 1
%201 = OpCompositeExtract %v3float %195 1
%202 = OpFOrdEqual %v3bool %200 %201
%203 = OpAll %bool %202
%204 = OpLogicalAnd %bool %199 %203
%205 = OpCompositeExtract %v3float %191 2
%206 = OpCompositeExtract %v3float %195 2
%207 = OpFOrdEqual %v3bool %205 %206
%208 = OpAll %bool %207
%209 = OpLogicalAnd %bool %204 %208
OpBranch %175
%175 = OpLabel
%210 = OpPhi %bool %false %146 %209 %174
OpStore %ok %210
%211 = OpLoad %bool %ok
OpSelectionMerge %213 None
OpBranchConditional %211 %212 %213
%212 = OpLabel
%215 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%216 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%217 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%214 = OpCompositeConstruct %mat3v3float %215 %216 %217
%218 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%219 = OpCompositeConstruct %mat3v3float %218 %218 %218
%220 = OpCompositeExtract %v3float %219 0
%221 = OpCompositeExtract %v3float %214 0
%222 = OpFSub %v3float %220 %221
%223 = OpCompositeExtract %v3float %219 1
%224 = OpCompositeExtract %v3float %214 1
%225 = OpFSub %v3float %223 %224
%226 = OpCompositeExtract %v3float %219 2
%227 = OpCompositeExtract %v3float %214 2
%228 = OpFSub %v3float %226 %227
%229 = OpCompositeConstruct %mat3v3float %222 %225 %228
%230 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%231 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%232 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%233 = OpCompositeConstruct %mat3v3float %230 %231 %232
%234 = OpCompositeExtract %v3float %229 0
%235 = OpCompositeExtract %v3float %233 0
%236 = OpFOrdEqual %v3bool %234 %235
%237 = OpAll %bool %236
%238 = OpCompositeExtract %v3float %229 1
%239 = OpCompositeExtract %v3float %233 1
%240 = OpFOrdEqual %v3bool %238 %239
%241 = OpAll %bool %240
%242 = OpLogicalAnd %bool %237 %241
%243 = OpCompositeExtract %v3float %229 2
%244 = OpCompositeExtract %v3float %233 2
%245 = OpFOrdEqual %v3bool %243 %244
%246 = OpAll %bool %245
%247 = OpLogicalAnd %bool %242 %246
OpBranch %213
%213 = OpLabel
%248 = OpPhi %bool %false %175 %247 %212
OpStore %ok %248
%249 = OpLoad %bool %ok
OpSelectionMerge %251 None
OpBranchConditional %249 %250 %251
%250 = OpLabel
%253 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%254 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%255 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%252 = OpCompositeConstruct %mat3v3float %253 %254 %255
%256 = OpMatrixTimesScalar %mat3v3float %252 %float_4
%258 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%259 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%260 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%257 = OpCompositeConstruct %mat3v3float %258 %259 %260
%261 = OpCompositeExtract %v3float %256 0
%262 = OpCompositeExtract %v3float %257 0
%263 = OpFOrdEqual %v3bool %261 %262
%264 = OpAll %bool %263
%265 = OpCompositeExtract %v3float %256 1
%266 = OpCompositeExtract %v3float %257 1
%267 = OpFOrdEqual %v3bool %265 %266
%268 = OpAll %bool %267
%269 = OpLogicalAnd %bool %264 %268
%270 = OpCompositeExtract %v3float %256 2
%271 = OpCompositeExtract %v3float %257 2
%272 = OpFOrdEqual %v3bool %270 %271
%273 = OpAll %bool %272
%274 = OpLogicalAnd %bool %269 %273
OpBranch %251
%251 = OpLabel
%275 = OpPhi %bool %false %213 %274 %250
OpStore %ok %275
%276 = OpLoad %bool %ok
OpSelectionMerge %278 None
OpBranchConditional %276 %277 %278
%277 = OpLabel
%279 = OpCompositeConstruct %v2float %float_2 %float_2
%280 = OpCompositeConstruct %v2float %float_2 %float_2
%281 = OpCompositeConstruct %mat2v2float %279 %280
%283 = OpCompositeConstruct %v2float %float_4 %float_4
%284 = OpCompositeConstruct %mat2v2float %283 %283
%285 = OpCompositeExtract %v2float %284 0
%286 = OpCompositeExtract %v2float %281 0
%287 = OpFDiv %v2float %285 %286
%288 = OpCompositeExtract %v2float %284 1
%289 = OpCompositeExtract %v2float %281 1
%290 = OpFDiv %v2float %288 %289
%291 = OpCompositeConstruct %mat2v2float %287 %290
%292 = OpCompositeConstruct %v2float %float_2 %float_2
%293 = OpCompositeConstruct %v2float %float_2 %float_2
%294 = OpCompositeConstruct %mat2v2float %292 %293
%296 = OpCompositeExtract %v2float %291 0
%297 = OpCompositeExtract %v2float %294 0
%298 = OpFOrdEqual %v2bool %296 %297
%299 = OpAll %bool %298
%300 = OpCompositeExtract %v2float %291 1
%301 = OpCompositeExtract %v2float %294 1
%302 = OpFOrdEqual %v2bool %300 %301
%303 = OpAll %bool %302
%304 = OpLogicalAnd %bool %299 %303
OpBranch %278
%278 = OpLabel
%305 = OpPhi %bool %false %251 %304 %277
OpStore %ok %305
%306 = OpLoad %bool %ok
OpReturnValue %306
OpFunctionEnd
%main = OpFunction %v4float None %307
%308 = OpFunctionParameter %_ptr_Function_v2float
%309 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%577 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%311 = OpLoad %bool %_0_ok
OpSelectionMerge %313 None
OpBranchConditional %311 %312 %313
%312 = OpLabel
%315 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%316 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%317 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%314 = OpCompositeConstruct %mat3v3float %315 %316 %317
%318 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%319 = OpCompositeConstruct %mat3v3float %318 %318 %318
%320 = OpCompositeExtract %v3float %314 0
%321 = OpCompositeExtract %v3float %319 0
%322 = OpFAdd %v3float %320 %321
%323 = OpCompositeExtract %v3float %314 1
%324 = OpCompositeExtract %v3float %319 1
%325 = OpFAdd %v3float %323 %324
%326 = OpCompositeExtract %v3float %314 2
%327 = OpCompositeExtract %v3float %319 2
%328 = OpFAdd %v3float %326 %327
%329 = OpCompositeConstruct %mat3v3float %322 %325 %328
%330 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%331 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%332 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%333 = OpCompositeConstruct %mat3v3float %330 %331 %332
%334 = OpCompositeExtract %v3float %329 0
%335 = OpCompositeExtract %v3float %333 0
%336 = OpFOrdEqual %v3bool %334 %335
%337 = OpAll %bool %336
%338 = OpCompositeExtract %v3float %329 1
%339 = OpCompositeExtract %v3float %333 1
%340 = OpFOrdEqual %v3bool %338 %339
%341 = OpAll %bool %340
%342 = OpLogicalAnd %bool %337 %341
%343 = OpCompositeExtract %v3float %329 2
%344 = OpCompositeExtract %v3float %333 2
%345 = OpFOrdEqual %v3bool %343 %344
%346 = OpAll %bool %345
%347 = OpLogicalAnd %bool %342 %346
OpBranch %313
%313 = OpLabel
%348 = OpPhi %bool %false %309 %347 %312
OpStore %_0_ok %348
%349 = OpLoad %bool %_0_ok
OpSelectionMerge %351 None
OpBranchConditional %349 %350 %351
%350 = OpLabel
%353 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%354 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%355 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%352 = OpCompositeConstruct %mat3v3float %353 %354 %355
%356 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%357 = OpCompositeConstruct %mat3v3float %356 %356 %356
%358 = OpCompositeExtract %v3float %352 0
%359 = OpCompositeExtract %v3float %357 0
%360 = OpFSub %v3float %358 %359
%361 = OpCompositeExtract %v3float %352 1
%362 = OpCompositeExtract %v3float %357 1
%363 = OpFSub %v3float %361 %362
%364 = OpCompositeExtract %v3float %352 2
%365 = OpCompositeExtract %v3float %357 2
%366 = OpFSub %v3float %364 %365
%367 = OpCompositeConstruct %mat3v3float %360 %363 %366
%368 = OpCompositeConstruct %v3float %float_n2 %float_n4 %float_n4
%369 = OpCompositeConstruct %v3float %float_n4 %float_n2 %float_n4
%370 = OpCompositeConstruct %v3float %float_n4 %float_n4 %float_n2
%371 = OpCompositeConstruct %mat3v3float %368 %369 %370
%372 = OpCompositeExtract %v3float %367 0
%373 = OpCompositeExtract %v3float %371 0
%374 = OpFOrdEqual %v3bool %372 %373
%375 = OpAll %bool %374
%376 = OpCompositeExtract %v3float %367 1
%377 = OpCompositeExtract %v3float %371 1
%378 = OpFOrdEqual %v3bool %376 %377
%379 = OpAll %bool %378
%380 = OpLogicalAnd %bool %375 %379
%381 = OpCompositeExtract %v3float %367 2
%382 = OpCompositeExtract %v3float %371 2
%383 = OpFOrdEqual %v3bool %381 %382
%384 = OpAll %bool %383
%385 = OpLogicalAnd %bool %380 %384
OpBranch %351
%351 = OpLabel
%386 = OpPhi %bool %false %313 %385 %350
OpStore %_0_ok %386
%387 = OpLoad %bool %_0_ok
OpSelectionMerge %389 None
OpBranchConditional %387 %388 %389
%388 = OpLabel
%391 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%392 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%393 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%390 = OpCompositeConstruct %mat3v3float %391 %392 %393
%394 = OpMatrixTimesScalar %mat3v3float %390 %float_4
%396 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%397 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%398 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%395 = OpCompositeConstruct %mat3v3float %396 %397 %398
%399 = OpCompositeExtract %v3float %394 0
%400 = OpCompositeExtract %v3float %395 0
%401 = OpFOrdEqual %v3bool %399 %400
%402 = OpAll %bool %401
%403 = OpCompositeExtract %v3float %394 1
%404 = OpCompositeExtract %v3float %395 1
%405 = OpFOrdEqual %v3bool %403 %404
%406 = OpAll %bool %405
%407 = OpLogicalAnd %bool %402 %406
%408 = OpCompositeExtract %v3float %394 2
%409 = OpCompositeExtract %v3float %395 2
%410 = OpFOrdEqual %v3bool %408 %409
%411 = OpAll %bool %410
%412 = OpLogicalAnd %bool %407 %411
OpBranch %389
%389 = OpLabel
%413 = OpPhi %bool %false %351 %412 %388
OpStore %_0_ok %413
%414 = OpLoad %bool %_0_ok
OpSelectionMerge %416 None
OpBranchConditional %414 %415 %416
%415 = OpLabel
%418 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%419 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%420 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%417 = OpCompositeConstruct %mat3v3float %418 %419 %420
%421 = OpMatrixTimesScalar %mat3v3float %417 %float_0_25
%423 = OpCompositeConstruct %v3float %float_0_5 %float_0 %float_0
%424 = OpCompositeConstruct %v3float %float_0 %float_0_5 %float_0
%425 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0_5
%422 = OpCompositeConstruct %mat3v3float %423 %424 %425
%426 = OpCompositeExtract %v3float %421 0
%427 = OpCompositeExtract %v3float %422 0
%428 = OpFOrdEqual %v3bool %426 %427
%429 = OpAll %bool %428
%430 = OpCompositeExtract %v3float %421 1
%431 = OpCompositeExtract %v3float %422 1
%432 = OpFOrdEqual %v3bool %430 %431
%433 = OpAll %bool %432
%434 = OpLogicalAnd %bool %429 %433
%435 = OpCompositeExtract %v3float %421 2
%436 = OpCompositeExtract %v3float %422 2
%437 = OpFOrdEqual %v3bool %435 %436
%438 = OpAll %bool %437
%439 = OpLogicalAnd %bool %434 %438
OpBranch %416
%416 = OpLabel
%440 = OpPhi %bool %false %389 %439 %415
OpStore %_0_ok %440
%441 = OpLoad %bool %_0_ok
OpSelectionMerge %443 None
OpBranchConditional %441 %442 %443
%442 = OpLabel
%445 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%446 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%447 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%444 = OpCompositeConstruct %mat3v3float %445 %446 %447
%448 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%449 = OpCompositeConstruct %mat3v3float %448 %448 %448
%450 = OpCompositeExtract %v3float %449 0
%451 = OpCompositeExtract %v3float %444 0
%452 = OpFAdd %v3float %450 %451
%453 = OpCompositeExtract %v3float %449 1
%454 = OpCompositeExtract %v3float %444 1
%455 = OpFAdd %v3float %453 %454
%456 = OpCompositeExtract %v3float %449 2
%457 = OpCompositeExtract %v3float %444 2
%458 = OpFAdd %v3float %456 %457
%459 = OpCompositeConstruct %mat3v3float %452 %455 %458
%460 = OpCompositeConstruct %v3float %float_6 %float_4 %float_4
%461 = OpCompositeConstruct %v3float %float_4 %float_6 %float_4
%462 = OpCompositeConstruct %v3float %float_4 %float_4 %float_6
%463 = OpCompositeConstruct %mat3v3float %460 %461 %462
%464 = OpCompositeExtract %v3float %459 0
%465 = OpCompositeExtract %v3float %463 0
%466 = OpFOrdEqual %v3bool %464 %465
%467 = OpAll %bool %466
%468 = OpCompositeExtract %v3float %459 1
%469 = OpCompositeExtract %v3float %463 1
%470 = OpFOrdEqual %v3bool %468 %469
%471 = OpAll %bool %470
%472 = OpLogicalAnd %bool %467 %471
%473 = OpCompositeExtract %v3float %459 2
%474 = OpCompositeExtract %v3float %463 2
%475 = OpFOrdEqual %v3bool %473 %474
%476 = OpAll %bool %475
%477 = OpLogicalAnd %bool %472 %476
OpBranch %443
%443 = OpLabel
%478 = OpPhi %bool %false %416 %477 %442
OpStore %_0_ok %478
%479 = OpLoad %bool %_0_ok
OpSelectionMerge %481 None
OpBranchConditional %479 %480 %481
%480 = OpLabel
%483 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%484 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%485 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%482 = OpCompositeConstruct %mat3v3float %483 %484 %485
%486 = OpCompositeConstruct %v3float %float_4 %float_4 %float_4
%487 = OpCompositeConstruct %mat3v3float %486 %486 %486
%488 = OpCompositeExtract %v3float %487 0
%489 = OpCompositeExtract %v3float %482 0
%490 = OpFSub %v3float %488 %489
%491 = OpCompositeExtract %v3float %487 1
%492 = OpCompositeExtract %v3float %482 1
%493 = OpFSub %v3float %491 %492
%494 = OpCompositeExtract %v3float %487 2
%495 = OpCompositeExtract %v3float %482 2
%496 = OpFSub %v3float %494 %495
%497 = OpCompositeConstruct %mat3v3float %490 %493 %496
%498 = OpCompositeConstruct %v3float %float_2 %float_4 %float_4
%499 = OpCompositeConstruct %v3float %float_4 %float_2 %float_4
%500 = OpCompositeConstruct %v3float %float_4 %float_4 %float_2
%501 = OpCompositeConstruct %mat3v3float %498 %499 %500
%502 = OpCompositeExtract %v3float %497 0
%503 = OpCompositeExtract %v3float %501 0
%504 = OpFOrdEqual %v3bool %502 %503
%505 = OpAll %bool %504
%506 = OpCompositeExtract %v3float %497 1
%507 = OpCompositeExtract %v3float %501 1
%508 = OpFOrdEqual %v3bool %506 %507
%509 = OpAll %bool %508
%510 = OpLogicalAnd %bool %505 %509
%511 = OpCompositeExtract %v3float %497 2
%512 = OpCompositeExtract %v3float %501 2
%513 = OpFOrdEqual %v3bool %511 %512
%514 = OpAll %bool %513
%515 = OpLogicalAnd %bool %510 %514
OpBranch %481
%481 = OpLabel
%516 = OpPhi %bool %false %443 %515 %480
OpStore %_0_ok %516
%517 = OpLoad %bool %_0_ok
OpSelectionMerge %519 None
OpBranchConditional %517 %518 %519
%518 = OpLabel
%521 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%522 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%523 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%520 = OpCompositeConstruct %mat3v3float %521 %522 %523
%524 = OpMatrixTimesScalar %mat3v3float %520 %float_4
%526 = OpCompositeConstruct %v3float %float_8 %float_0 %float_0
%527 = OpCompositeConstruct %v3float %float_0 %float_8 %float_0
%528 = OpCompositeConstruct %v3float %float_0 %float_0 %float_8
%525 = OpCompositeConstruct %mat3v3float %526 %527 %528
%529 = OpCompositeExtract %v3float %524 0
%530 = OpCompositeExtract %v3float %525 0
%531 = OpFOrdEqual %v3bool %529 %530
%532 = OpAll %bool %531
%533 = OpCompositeExtract %v3float %524 1
%534 = OpCompositeExtract %v3float %525 1
%535 = OpFOrdEqual %v3bool %533 %534
%536 = OpAll %bool %535
%537 = OpLogicalAnd %bool %532 %536
%538 = OpCompositeExtract %v3float %524 2
%539 = OpCompositeExtract %v3float %525 2
%540 = OpFOrdEqual %v3bool %538 %539
%541 = OpAll %bool %540
%542 = OpLogicalAnd %bool %537 %541
OpBranch %519
%519 = OpLabel
%543 = OpPhi %bool %false %481 %542 %518
OpStore %_0_ok %543
%544 = OpLoad %bool %_0_ok
OpSelectionMerge %546 None
OpBranchConditional %544 %545 %546
%545 = OpLabel
%547 = OpCompositeConstruct %v2float %float_2 %float_2
%548 = OpCompositeConstruct %v2float %float_2 %float_2
%549 = OpCompositeConstruct %mat2v2float %547 %548
%550 = OpCompositeConstruct %v2float %float_4 %float_4
%551 = OpCompositeConstruct %mat2v2float %550 %550
%552 = OpCompositeExtract %v2float %551 0
%553 = OpCompositeExtract %v2float %549 0
%554 = OpFDiv %v2float %552 %553
%555 = OpCompositeExtract %v2float %551 1
%556 = OpCompositeExtract %v2float %549 1
%557 = OpFDiv %v2float %555 %556
%558 = OpCompositeConstruct %mat2v2float %554 %557
%559 = OpCompositeConstruct %v2float %float_2 %float_2
%560 = OpCompositeConstruct %v2float %float_2 %float_2
%561 = OpCompositeConstruct %mat2v2float %559 %560
%562 = OpCompositeExtract %v2float %558 0
%563 = OpCompositeExtract %v2float %561 0
%564 = OpFOrdEqual %v2bool %562 %563
%565 = OpAll %bool %564
%566 = OpCompositeExtract %v2float %558 1
%567 = OpCompositeExtract %v2float %561 1
%568 = OpFOrdEqual %v2bool %566 %567
%569 = OpAll %bool %568
%570 = OpLogicalAnd %bool %565 %569
OpBranch %546
%546 = OpLabel
%571 = OpPhi %bool %false %519 %570 %545
OpStore %_0_ok %571
%572 = OpLoad %bool %_0_ok
OpSelectionMerge %574 None
OpBranchConditional %572 %573 %574
%573 = OpLabel
%575 = OpFunctionCall %bool %test_half_b
OpBranch %574
%574 = OpLabel
%576 = OpPhi %bool %false %546 %575 %573
OpSelectionMerge %581 None
OpBranchConditional %576 %579 %580
%579 = OpLabel
%582 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%586 = OpLoad %v4float %582
OpStore %577 %586
OpBranch %581
%580 = OpLabel
%587 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%589 = OpLoad %v4float %587
OpStore %577 %589
OpBranch %581
%581 = OpLabel
%590 = OpLoad %v4float %577
OpReturnValue %590
OpFunctionEnd
