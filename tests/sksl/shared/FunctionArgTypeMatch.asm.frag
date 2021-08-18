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
OpName %takes_void_b "takes_void_b"
OpName %takes_float_bf "takes_float_bf"
OpName %takes_float2_bf2 "takes_float2_bf2"
OpName %takes_float3_bf3 "takes_float3_bf3"
OpName %takes_float4_bf4 "takes_float4_bf4"
OpName %takes_float2x2_bf22 "takes_float2x2_bf22"
OpName %takes_float3x3_bf33 "takes_float3x3_bf33"
OpName %takes_float4x4_bf44 "takes_float4x4_bf44"
OpName %takes_half_bh "takes_half_bh"
OpName %takes_half2_bh2 "takes_half2_bh2"
OpName %takes_half3_bh3 "takes_half3_bh3"
OpName %takes_half4_bh4 "takes_half4_bh4"
OpName %takes_half2x2_bh22 "takes_half2x2_bh22"
OpName %takes_half3x3_bh33 "takes_half3x3_bh33"
OpName %takes_half4x4_bh44 "takes_half4x4_bh44"
OpName %takes_bool_bb "takes_bool_bb"
OpName %takes_bool2_bb2 "takes_bool2_bb2"
OpName %takes_bool3_bb3 "takes_bool3_bb3"
OpName %takes_bool4_bb4 "takes_bool4_bb4"
OpName %takes_int_bi "takes_int_bi"
OpName %takes_int2_bi2 "takes_int2_bi2"
OpName %takes_int3_bi3 "takes_int3_bi3"
OpName %takes_int4_bi4 "takes_int4_bi4"
OpName %main "main"
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
OpDecorate %35 Binding 0
OpDecorate %35 DescriptorSet 0
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
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
%35 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%40 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%44 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%48 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%_ptr_Function_float = OpTypePointer Function %float
%51 = OpTypeFunction %bool %_ptr_Function_float
%55 = OpTypeFunction %bool %_ptr_Function_v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%59 = OpTypeFunction %bool %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%63 = OpTypeFunction %bool %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%68 = OpTypeFunction %bool %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%73 = OpTypeFunction %bool %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%78 = OpTypeFunction %bool %_ptr_Function_mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%96 = OpTypeFunction %bool %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%101 = OpTypeFunction %bool %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%106 = OpTypeFunction %bool %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%111 = OpTypeFunction %bool %_ptr_Function_v4bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%116 = OpTypeFunction %bool %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%121 = OpTypeFunction %bool %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%126 = OpTypeFunction %bool %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%131 = OpTypeFunction %bool %_ptr_Function_v4int
%135 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%152 = OpConstantComposite %v2float %float_2 %float_2
%float_3 = OpConstant %float 3
%159 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%float_4 = OpConstant %float 4
%166 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%251 = OpConstantComposite %v2bool %true %true
%257 = OpConstantComposite %v3bool %true %true %true
%263 = OpConstantComposite %v4bool %true %true %true %true
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%276 = OpConstantComposite %v2int %int_2 %int_2
%int_3 = OpConstant %int 3
%283 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%int_4 = OpConstant %int 4
%290 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %40
%41 = OpLabel
%45 = OpVariable %_ptr_Function_v2float Function
OpStore %45 %44
%47 = OpFunctionCall %v4float %main %45
OpStore %sk_FragColor %47
OpReturn
OpFunctionEnd
%takes_void_b = OpFunction %bool None %48
%49 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float_bf = OpFunction %bool None %51
%53 = OpFunctionParameter %_ptr_Function_float
%54 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2_bf2 = OpFunction %bool None %55
%56 = OpFunctionParameter %_ptr_Function_v2float
%57 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3_bf3 = OpFunction %bool None %59
%61 = OpFunctionParameter %_ptr_Function_v3float
%62 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4_bf4 = OpFunction %bool None %63
%65 = OpFunctionParameter %_ptr_Function_v4float
%66 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2x2_bf22 = OpFunction %bool None %68
%70 = OpFunctionParameter %_ptr_Function_mat2v2float
%71 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3x3_bf33 = OpFunction %bool None %73
%75 = OpFunctionParameter %_ptr_Function_mat3v3float
%76 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4x4_bf44 = OpFunction %bool None %78
%80 = OpFunctionParameter %_ptr_Function_mat4v4float
%81 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half_bh = OpFunction %bool None %51
%82 = OpFunctionParameter %_ptr_Function_float
%83 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2_bh2 = OpFunction %bool None %55
%84 = OpFunctionParameter %_ptr_Function_v2float
%85 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3_bh3 = OpFunction %bool None %59
%86 = OpFunctionParameter %_ptr_Function_v3float
%87 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4_bh4 = OpFunction %bool None %63
%88 = OpFunctionParameter %_ptr_Function_v4float
%89 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2x2_bh22 = OpFunction %bool None %68
%90 = OpFunctionParameter %_ptr_Function_mat2v2float
%91 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3x3_bh33 = OpFunction %bool None %73
%92 = OpFunctionParameter %_ptr_Function_mat3v3float
%93 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4x4_bh44 = OpFunction %bool None %78
%94 = OpFunctionParameter %_ptr_Function_mat4v4float
%95 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool_bb = OpFunction %bool None %96
%98 = OpFunctionParameter %_ptr_Function_bool
%99 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool2_bb2 = OpFunction %bool None %101
%103 = OpFunctionParameter %_ptr_Function_v2bool
%104 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool3_bb3 = OpFunction %bool None %106
%108 = OpFunctionParameter %_ptr_Function_v3bool
%109 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool4_bb4 = OpFunction %bool None %111
%113 = OpFunctionParameter %_ptr_Function_v4bool
%114 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int_bi = OpFunction %bool None %116
%118 = OpFunctionParameter %_ptr_Function_int
%119 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int2_bi2 = OpFunction %bool None %121
%123 = OpFunctionParameter %_ptr_Function_v2int
%124 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int3_bi3 = OpFunction %bool None %126
%128 = OpFunctionParameter %_ptr_Function_v3int
%129 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int4_bi4 = OpFunction %bool None %131
%133 = OpFunctionParameter %_ptr_Function_v4int
%134 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %135
%136 = OpFunctionParameter %_ptr_Function_v2float
%137 = OpLabel
%146 = OpVariable %_ptr_Function_float Function
%153 = OpVariable %_ptr_Function_v2float Function
%160 = OpVariable %_ptr_Function_v3float Function
%167 = OpVariable %_ptr_Function_v4float Function
%175 = OpVariable %_ptr_Function_mat2v2float Function
%184 = OpVariable %_ptr_Function_mat3v3float Function
%194 = OpVariable %_ptr_Function_mat4v4float Function
%199 = OpVariable %_ptr_Function_float Function
%204 = OpVariable %_ptr_Function_v2float Function
%209 = OpVariable %_ptr_Function_v3float Function
%214 = OpVariable %_ptr_Function_v4float Function
%222 = OpVariable %_ptr_Function_mat2v2float Function
%231 = OpVariable %_ptr_Function_mat3v3float Function
%241 = OpVariable %_ptr_Function_mat4v4float Function
%246 = OpVariable %_ptr_Function_bool Function
%252 = OpVariable %_ptr_Function_v2bool Function
%258 = OpVariable %_ptr_Function_v3bool Function
%264 = OpVariable %_ptr_Function_v4bool Function
%270 = OpVariable %_ptr_Function_int Function
%277 = OpVariable %_ptr_Function_v2int Function
%284 = OpVariable %_ptr_Function_v3int Function
%291 = OpVariable %_ptr_Function_v4int Function
%294 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %140 None
OpBranchConditional %true %139 %140
%139 = OpLabel
%141 = OpFunctionCall %bool %takes_void_b
OpBranch %140
%140 = OpLabel
%142 = OpPhi %bool %false %137 %141 %139
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
OpStore %146 %float_1
%147 = OpFunctionCall %bool %takes_float_bf %146
OpBranch %144
%144 = OpLabel
%148 = OpPhi %bool %false %140 %147 %143
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
OpStore %153 %152
%154 = OpFunctionCall %bool %takes_float2_bf2 %153
OpBranch %150
%150 = OpLabel
%155 = OpPhi %bool %false %144 %154 %149
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
OpStore %160 %159
%161 = OpFunctionCall %bool %takes_float3_bf3 %160
OpBranch %157
%157 = OpLabel
%162 = OpPhi %bool %false %150 %161 %156
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
OpStore %167 %166
%168 = OpFunctionCall %bool %takes_float4_bf4 %167
OpBranch %164
%164 = OpLabel
%169 = OpPhi %bool %false %157 %168 %163
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%173 = OpCompositeConstruct %v2float %float_2 %float_0
%174 = OpCompositeConstruct %v2float %float_0 %float_2
%172 = OpCompositeConstruct %mat2v2float %173 %174
OpStore %175 %172
%176 = OpFunctionCall %bool %takes_float2x2_bf22 %175
OpBranch %171
%171 = OpLabel
%177 = OpPhi %bool %false %164 %176 %170
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%181 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%182 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%183 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%180 = OpCompositeConstruct %mat3v3float %181 %182 %183
OpStore %184 %180
%185 = OpFunctionCall %bool %takes_float3x3_bf33 %184
OpBranch %179
%179 = OpLabel
%186 = OpPhi %bool %false %171 %185 %178
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%190 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%191 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%192 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%193 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%189 = OpCompositeConstruct %mat4v4float %190 %191 %192 %193
OpStore %194 %189
%195 = OpFunctionCall %bool %takes_float4x4_bf44 %194
OpBranch %188
%188 = OpLabel
%196 = OpPhi %bool %false %179 %195 %187
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
OpStore %199 %float_1
%200 = OpFunctionCall %bool %takes_half_bh %199
OpBranch %198
%198 = OpLabel
%201 = OpPhi %bool %false %188 %200 %197
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
OpStore %204 %152
%205 = OpFunctionCall %bool %takes_half2_bh2 %204
OpBranch %203
%203 = OpLabel
%206 = OpPhi %bool %false %198 %205 %202
OpSelectionMerge %208 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
OpStore %209 %159
%210 = OpFunctionCall %bool %takes_half3_bh3 %209
OpBranch %208
%208 = OpLabel
%211 = OpPhi %bool %false %203 %210 %207
OpSelectionMerge %213 None
OpBranchConditional %211 %212 %213
%212 = OpLabel
OpStore %214 %166
%215 = OpFunctionCall %bool %takes_half4_bh4 %214
OpBranch %213
%213 = OpLabel
%216 = OpPhi %bool %false %208 %215 %212
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%220 = OpCompositeConstruct %v2float %float_2 %float_0
%221 = OpCompositeConstruct %v2float %float_0 %float_2
%219 = OpCompositeConstruct %mat2v2float %220 %221
OpStore %222 %219
%223 = OpFunctionCall %bool %takes_half2x2_bh22 %222
OpBranch %218
%218 = OpLabel
%224 = OpPhi %bool %false %213 %223 %217
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
%228 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%229 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%230 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%227 = OpCompositeConstruct %mat3v3float %228 %229 %230
OpStore %231 %227
%232 = OpFunctionCall %bool %takes_half3x3_bh33 %231
OpBranch %226
%226 = OpLabel
%233 = OpPhi %bool %false %218 %232 %225
OpSelectionMerge %235 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
%237 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%238 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%239 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%240 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%236 = OpCompositeConstruct %mat4v4float %237 %238 %239 %240
OpStore %241 %236
%242 = OpFunctionCall %bool %takes_half4x4_bh44 %241
OpBranch %235
%235 = OpLabel
%243 = OpPhi %bool %false %226 %242 %234
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
OpStore %246 %true
%247 = OpFunctionCall %bool %takes_bool_bb %246
OpBranch %245
%245 = OpLabel
%248 = OpPhi %bool %false %235 %247 %244
OpSelectionMerge %250 None
OpBranchConditional %248 %249 %250
%249 = OpLabel
OpStore %252 %251
%253 = OpFunctionCall %bool %takes_bool2_bb2 %252
OpBranch %250
%250 = OpLabel
%254 = OpPhi %bool %false %245 %253 %249
OpSelectionMerge %256 None
OpBranchConditional %254 %255 %256
%255 = OpLabel
OpStore %258 %257
%259 = OpFunctionCall %bool %takes_bool3_bb3 %258
OpBranch %256
%256 = OpLabel
%260 = OpPhi %bool %false %250 %259 %255
OpSelectionMerge %262 None
OpBranchConditional %260 %261 %262
%261 = OpLabel
OpStore %264 %263
%265 = OpFunctionCall %bool %takes_bool4_bb4 %264
OpBranch %262
%262 = OpLabel
%266 = OpPhi %bool %false %256 %265 %261
OpSelectionMerge %268 None
OpBranchConditional %266 %267 %268
%267 = OpLabel
OpStore %270 %int_1
%271 = OpFunctionCall %bool %takes_int_bi %270
OpBranch %268
%268 = OpLabel
%272 = OpPhi %bool %false %262 %271 %267
OpSelectionMerge %274 None
OpBranchConditional %272 %273 %274
%273 = OpLabel
OpStore %277 %276
%278 = OpFunctionCall %bool %takes_int2_bi2 %277
OpBranch %274
%274 = OpLabel
%279 = OpPhi %bool %false %268 %278 %273
OpSelectionMerge %281 None
OpBranchConditional %279 %280 %281
%280 = OpLabel
OpStore %284 %283
%285 = OpFunctionCall %bool %takes_int3_bi3 %284
OpBranch %281
%281 = OpLabel
%286 = OpPhi %bool %false %274 %285 %280
OpSelectionMerge %288 None
OpBranchConditional %286 %287 %288
%287 = OpLabel
OpStore %291 %290
%292 = OpFunctionCall %bool %takes_int4_bi4 %291
OpBranch %288
%288 = OpLabel
%293 = OpPhi %bool %false %281 %292 %287
OpSelectionMerge %297 None
OpBranchConditional %293 %295 %296
%295 = OpLabel
%298 = OpAccessChain %_ptr_Uniform_v4float %35 %int_0
%301 = OpLoad %v4float %298
OpStore %294 %301
OpBranch %297
%296 = OpLabel
%302 = OpAccessChain %_ptr_Uniform_v4float %35 %int_1
%303 = OpLoad %v4float %302
OpStore %294 %303
OpBranch %297
%297 = OpLabel
%304 = OpLoad %v4float %294
OpReturnValue %304
OpFunctionEnd
