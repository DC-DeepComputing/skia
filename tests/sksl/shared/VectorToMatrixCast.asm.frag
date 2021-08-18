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
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpDecorate %sk_FragCoord BuiltIn FragCoord
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
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
OpDecorate %186 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
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
OpDecorate %236 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
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
OpDecorate %280 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%mat2v2float = OpTypeMatrix %v2float 2
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%v2bool = OpTypeVector %bool 2
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%v4int = OpTypeVector %int 4
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %25
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%281 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%32 = OpLoad %bool %ok
OpSelectionMerge %34 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
%35 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%39 = OpLoad %v4float %35
%40 = OpCompositeExtract %float %39 0
%41 = OpCompositeExtract %float %39 1
%42 = OpCompositeExtract %float %39 2
%43 = OpCompositeExtract %float %39 3
%44 = OpCompositeConstruct %v2float %40 %41
%45 = OpCompositeConstruct %v2float %42 %43
%46 = OpCompositeConstruct %mat2v2float %44 %45
%51 = OpCompositeConstruct %v2float %float_n1_25 %float_0
%52 = OpCompositeConstruct %v2float %float_0_75 %float_2_25
%53 = OpCompositeConstruct %mat2v2float %51 %52
%55 = OpCompositeExtract %v2float %46 0
%56 = OpCompositeExtract %v2float %53 0
%57 = OpFOrdEqual %v2bool %55 %56
%58 = OpAll %bool %57
%59 = OpCompositeExtract %v2float %46 1
%60 = OpCompositeExtract %v2float %53 1
%61 = OpFOrdEqual %v2bool %59 %60
%62 = OpAll %bool %61
%63 = OpLogicalAnd %bool %58 %62
OpBranch %34
%34 = OpLabel
%64 = OpPhi %bool %false %27 %63 %33
OpStore %ok %64
%65 = OpLoad %bool %ok
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%69 = OpLoad %v4float %68
%70 = OpCompositeExtract %float %69 0
%71 = OpCompositeExtract %float %69 1
%72 = OpCompositeExtract %float %69 2
%73 = OpCompositeExtract %float %69 3
%74 = OpCompositeConstruct %v2float %70 %71
%75 = OpCompositeConstruct %v2float %72 %73
%76 = OpCompositeConstruct %mat2v2float %74 %75
%77 = OpCompositeConstruct %v2float %float_n1_25 %float_0
%78 = OpCompositeConstruct %v2float %float_0_75 %float_2_25
%79 = OpCompositeConstruct %mat2v2float %77 %78
%80 = OpCompositeExtract %v2float %76 0
%81 = OpCompositeExtract %v2float %79 0
%82 = OpFOrdEqual %v2bool %80 %81
%83 = OpAll %bool %82
%84 = OpCompositeExtract %v2float %76 1
%85 = OpCompositeExtract %v2float %79 1
%86 = OpFOrdEqual %v2bool %84 %85
%87 = OpAll %bool %86
%88 = OpLogicalAnd %bool %83 %87
OpBranch %67
%67 = OpLabel
%89 = OpPhi %bool %false %34 %88 %66
OpStore %ok %89
%90 = OpLoad %bool %ok
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%95 = OpLoad %v4float %93
%96 = OpCompositeExtract %float %95 0
%97 = OpCompositeExtract %float %95 1
%98 = OpCompositeExtract %float %95 2
%99 = OpCompositeExtract %float %95 3
%100 = OpCompositeConstruct %v2float %96 %97
%101 = OpCompositeConstruct %v2float %98 %99
%102 = OpCompositeConstruct %mat2v2float %100 %101
%104 = OpCompositeConstruct %v2float %float_0 %float_1
%105 = OpCompositeConstruct %v2float %float_0 %float_1
%106 = OpCompositeConstruct %mat2v2float %104 %105
%107 = OpCompositeExtract %v2float %102 0
%108 = OpCompositeExtract %v2float %106 0
%109 = OpFOrdEqual %v2bool %107 %108
%110 = OpAll %bool %109
%111 = OpCompositeExtract %v2float %102 1
%112 = OpCompositeExtract %v2float %106 1
%113 = OpFOrdEqual %v2bool %111 %112
%114 = OpAll %bool %113
%115 = OpLogicalAnd %bool %110 %114
OpBranch %92
%92 = OpLabel
%116 = OpPhi %bool %false %67 %115 %91
OpStore %ok %116
%117 = OpLoad %bool %ok
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%121 = OpLoad %v4float %120
%122 = OpCompositeExtract %float %121 0
%123 = OpCompositeExtract %float %121 1
%124 = OpCompositeExtract %float %121 2
%125 = OpCompositeExtract %float %121 3
%126 = OpCompositeConstruct %v2float %122 %123
%127 = OpCompositeConstruct %v2float %124 %125
%128 = OpCompositeConstruct %mat2v2float %126 %127
%129 = OpCompositeConstruct %v2float %float_0 %float_1
%130 = OpCompositeConstruct %v2float %float_0 %float_1
%131 = OpCompositeConstruct %mat2v2float %129 %130
%132 = OpCompositeExtract %v2float %128 0
%133 = OpCompositeExtract %v2float %131 0
%134 = OpFOrdEqual %v2bool %132 %133
%135 = OpAll %bool %134
%136 = OpCompositeExtract %v2float %128 1
%137 = OpCompositeExtract %v2float %131 1
%138 = OpFOrdEqual %v2bool %136 %137
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %135 %139
OpBranch %119
%119 = OpLabel
%141 = OpPhi %bool %false %92 %140 %118
OpStore %ok %141
%142 = OpLoad %bool %ok
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%146 = OpLoad %v4float %145
%147 = OpCompositeExtract %float %146 0
%148 = OpConvertFToS %int %147
%149 = OpCompositeExtract %float %146 1
%150 = OpConvertFToS %int %149
%151 = OpCompositeExtract %float %146 2
%152 = OpConvertFToS %int %151
%153 = OpCompositeExtract %float %146 3
%154 = OpConvertFToS %int %153
%155 = OpCompositeConstruct %v4int %148 %150 %152 %154
%157 = OpCompositeExtract %int %155 0
%158 = OpConvertSToF %float %157
%159 = OpCompositeExtract %int %155 1
%160 = OpConvertSToF %float %159
%161 = OpCompositeExtract %int %155 2
%162 = OpConvertSToF %float %161
%163 = OpCompositeExtract %int %155 3
%164 = OpConvertSToF %float %163
%165 = OpCompositeConstruct %v4float %158 %160 %162 %164
%166 = OpCompositeExtract %float %165 0
%167 = OpCompositeExtract %float %165 1
%168 = OpCompositeExtract %float %165 2
%169 = OpCompositeExtract %float %165 3
%170 = OpCompositeConstruct %v2float %166 %167
%171 = OpCompositeConstruct %v2float %168 %169
%172 = OpCompositeConstruct %mat2v2float %170 %171
%173 = OpCompositeConstruct %v2float %float_0 %float_1
%174 = OpCompositeConstruct %v2float %float_0 %float_1
%175 = OpCompositeConstruct %mat2v2float %173 %174
%176 = OpCompositeExtract %v2float %172 0
%177 = OpCompositeExtract %v2float %175 0
%178 = OpFOrdEqual %v2bool %176 %177
%179 = OpAll %bool %178
%180 = OpCompositeExtract %v2float %172 1
%181 = OpCompositeExtract %v2float %175 1
%182 = OpFOrdEqual %v2bool %180 %181
%183 = OpAll %bool %182
%184 = OpLogicalAnd %bool %179 %183
OpBranch %144
%144 = OpLabel
%185 = OpPhi %bool %false %119 %184 %143
OpStore %ok %185
%186 = OpLoad %bool %ok
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%190 = OpLoad %v4float %189
%191 = OpCompositeExtract %float %190 0
%192 = OpCompositeExtract %float %190 1
%193 = OpCompositeExtract %float %190 2
%194 = OpCompositeExtract %float %190 3
%195 = OpCompositeConstruct %v2float %191 %192
%196 = OpCompositeConstruct %v2float %193 %194
%197 = OpCompositeConstruct %mat2v2float %195 %196
%198 = OpCompositeConstruct %v2float %float_0 %float_1
%199 = OpCompositeConstruct %v2float %float_0 %float_1
%200 = OpCompositeConstruct %mat2v2float %198 %199
%201 = OpCompositeExtract %v2float %197 0
%202 = OpCompositeExtract %v2float %200 0
%203 = OpFOrdEqual %v2bool %201 %202
%204 = OpAll %bool %203
%205 = OpCompositeExtract %v2float %197 1
%206 = OpCompositeExtract %v2float %200 1
%207 = OpFOrdEqual %v2bool %205 %206
%208 = OpAll %bool %207
%209 = OpLogicalAnd %bool %204 %208
OpBranch %188
%188 = OpLabel
%210 = OpPhi %bool %false %144 %209 %187
OpStore %ok %210
%211 = OpLoad %bool %ok
OpSelectionMerge %213 None
OpBranchConditional %211 %212 %213
%212 = OpLabel
%214 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%215 = OpLoad %v4float %214
%216 = OpCompositeExtract %float %215 0
%217 = OpCompositeExtract %float %215 1
%218 = OpCompositeExtract %float %215 2
%219 = OpCompositeExtract %float %215 3
%220 = OpCompositeConstruct %v2float %216 %217
%221 = OpCompositeConstruct %v2float %218 %219
%222 = OpCompositeConstruct %mat2v2float %220 %221
%223 = OpCompositeConstruct %v2float %float_0 %float_1
%224 = OpCompositeConstruct %v2float %float_0 %float_1
%225 = OpCompositeConstruct %mat2v2float %223 %224
%226 = OpCompositeExtract %v2float %222 0
%227 = OpCompositeExtract %v2float %225 0
%228 = OpFOrdEqual %v2bool %226 %227
%229 = OpAll %bool %228
%230 = OpCompositeExtract %v2float %222 1
%231 = OpCompositeExtract %v2float %225 1
%232 = OpFOrdEqual %v2bool %230 %231
%233 = OpAll %bool %232
%234 = OpLogicalAnd %bool %229 %233
OpBranch %213
%213 = OpLabel
%235 = OpPhi %bool %false %188 %234 %212
OpStore %ok %235
%236 = OpLoad %bool %ok
OpSelectionMerge %238 None
OpBranchConditional %236 %237 %238
%237 = OpLabel
%239 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%240 = OpLoad %v4float %239
%241 = OpCompositeExtract %float %240 0
%242 = OpFUnordNotEqual %bool %241 %float_0
%243 = OpCompositeExtract %float %240 1
%244 = OpFUnordNotEqual %bool %243 %float_0
%245 = OpCompositeExtract %float %240 2
%246 = OpFUnordNotEqual %bool %245 %float_0
%247 = OpCompositeExtract %float %240 3
%248 = OpFUnordNotEqual %bool %247 %float_0
%249 = OpCompositeConstruct %v4bool %242 %244 %246 %248
%251 = OpCompositeExtract %bool %249 0
%252 = OpSelect %float %251 %float_1 %float_0
%253 = OpCompositeExtract %bool %249 1
%254 = OpSelect %float %253 %float_1 %float_0
%255 = OpCompositeExtract %bool %249 2
%256 = OpSelect %float %255 %float_1 %float_0
%257 = OpCompositeExtract %bool %249 3
%258 = OpSelect %float %257 %float_1 %float_0
%259 = OpCompositeConstruct %v4float %252 %254 %256 %258
%260 = OpCompositeExtract %float %259 0
%261 = OpCompositeExtract %float %259 1
%262 = OpCompositeExtract %float %259 2
%263 = OpCompositeExtract %float %259 3
%264 = OpCompositeConstruct %v2float %260 %261
%265 = OpCompositeConstruct %v2float %262 %263
%266 = OpCompositeConstruct %mat2v2float %264 %265
%267 = OpCompositeConstruct %v2float %float_0 %float_1
%268 = OpCompositeConstruct %v2float %float_0 %float_1
%269 = OpCompositeConstruct %mat2v2float %267 %268
%270 = OpCompositeExtract %v2float %266 0
%271 = OpCompositeExtract %v2float %269 0
%272 = OpFOrdEqual %v2bool %270 %271
%273 = OpAll %bool %272
%274 = OpCompositeExtract %v2float %266 1
%275 = OpCompositeExtract %v2float %269 1
%276 = OpFOrdEqual %v2bool %274 %275
%277 = OpAll %bool %276
%278 = OpLogicalAnd %bool %273 %277
OpBranch %238
%238 = OpLabel
%279 = OpPhi %bool %false %213 %278 %237
OpStore %ok %279
%280 = OpLoad %bool %ok
OpSelectionMerge %285 None
OpBranchConditional %280 %283 %284
%283 = OpLabel
%286 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%287 = OpLoad %v4float %286
OpStore %281 %287
OpBranch %285
%284 = OpLabel
%288 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%290 = OpLoad %v4float %288
OpStore %281 %290
OpBranch %285
%285 = OpLabel
%291 = OpLoad %v4float %281
OpReturnValue %291
OpFunctionEnd
