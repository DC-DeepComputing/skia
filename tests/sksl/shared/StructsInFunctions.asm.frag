OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %returns_a_struct_S "returns_a_struct_S"
OpName %s "s"
OpName %constructs_a_struct_S "constructs_a_struct_S"
OpName %accepts_a_struct_fS "accepts_a_struct_fS"
OpName %modifies_a_struct_vS "modifies_a_struct_vS"
OpName %main "main"
OpName %s_0 "s"
OpName %x "x"
OpName %expected "expected"
OpName %Nested "Nested"
OpMemberName %Nested 0 "a"
OpMemberName %Nested 1 "b"
OpName %n1 "n1"
OpName %n2 "n2"
OpName %n3 "n3"
OpName %Compound "Compound"
OpMemberName %Compound 0 "f2x2"
OpMemberName %Compound 1 "i4"
OpName %c1 "c1"
OpName %c2 "c2"
OpName %c3 "c3"
OpName %valid "valid"
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
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %41 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpMemberDecorate %Nested 0 Offset 0
OpMemberDecorate %Nested 0 RelaxedPrecision
OpMemberDecorate %Nested 1 Offset 16
OpMemberDecorate %Nested 1 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpMemberDecorate %Compound 0 Offset 0
OpMemberDecorate %Compound 0 ColMajor
OpMemberDecorate %Compound 0 MatrixStride 16
OpMemberDecorate %Compound 1 Offset 32
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
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
%int = OpTypeInt 32 1
%S = OpTypeStruct %float %int
%29 = OpTypeFunction %S
%_ptr_Function_S = OpTypePointer Function %S
%float_1 = OpConstant %float 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%46 = OpTypeFunction %float %_ptr_Function_S
%55 = OpTypeFunction %void %_ptr_Function_S
%64 = OpTypeFunction %v4float %_ptr_Function_v2float
%Nested = OpTypeStruct %S %S
%_ptr_Function_Nested = OpTypePointer Function %Nested
%mat2v2float = OpTypeMatrix %v2float 2
%v4int = OpTypeVector %int 4
%Compound = OpTypeStruct %mat2v2float %v4int
%_ptr_Function_Compound = OpTypePointer Function %Compound
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%int_8 = OpConstant %int 8
%107 = OpConstantComposite %v4int %int_5 %int_6 %int_7 %int_8
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%returns_a_struct_S = OpFunction %S None %29
%30 = OpLabel
%s = OpVariable %_ptr_Function_S Function
%35 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %35 %float_1
%39 = OpAccessChain %_ptr_Function_int %s %int_1
OpStore %39 %int_2
%41 = OpLoad %S %s
OpReturnValue %41
OpFunctionEnd
%constructs_a_struct_S = OpFunction %S None %29
%42 = OpLabel
%45 = OpCompositeConstruct %S %float_2 %int_3
OpReturnValue %45
OpFunctionEnd
%accepts_a_struct_fS = OpFunction %float None %46
%47 = OpFunctionParameter %_ptr_Function_S
%48 = OpLabel
%49 = OpAccessChain %_ptr_Function_float %47 %int_0
%50 = OpLoad %float %49
%51 = OpAccessChain %_ptr_Function_int %47 %int_1
%52 = OpLoad %int %51
%53 = OpConvertSToF %float %52
%54 = OpFAdd %float %50 %53
OpReturnValue %54
OpFunctionEnd
%modifies_a_struct_vS = OpFunction %void None %55
%56 = OpFunctionParameter %_ptr_Function_S
%57 = OpLabel
%58 = OpAccessChain %_ptr_Function_float %56 %int_0
%59 = OpLoad %float %58
%60 = OpFAdd %float %59 %float_1
OpStore %58 %60
%61 = OpAccessChain %_ptr_Function_int %56 %int_1
%62 = OpLoad %int %61
%63 = OpIAdd %int %62 %int_1
OpStore %61 %63
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %64
%65 = OpFunctionParameter %_ptr_Function_v2float
%66 = OpLabel
%s_0 = OpVariable %_ptr_Function_S Function
%x = OpVariable %_ptr_Function_float Function
%71 = OpVariable %_ptr_Function_S Function
%expected = OpVariable %_ptr_Function_S Function
%n1 = OpVariable %_ptr_Function_Nested Function
%n2 = OpVariable %_ptr_Function_Nested Function
%n3 = OpVariable %_ptr_Function_Nested Function
%90 = OpVariable %_ptr_Function_S Function
%c1 = OpVariable %_ptr_Function_Compound Function
%c2 = OpVariable %_ptr_Function_Compound Function
%c3 = OpVariable %_ptr_Function_Compound Function
%valid = OpVariable %_ptr_Function_bool Function
%298 = OpVariable %_ptr_Function_v4float Function
%68 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %68
%70 = OpLoad %S %s_0
OpStore %71 %70
%72 = OpFunctionCall %float %accepts_a_struct_fS %71
OpStore %x %72
%73 = OpFunctionCall %void %modifies_a_struct_vS %s_0
%75 = OpFunctionCall %S %constructs_a_struct_S
OpStore %expected %75
%81 = OpFunctionCall %S %returns_a_struct_S
%82 = OpAccessChain %_ptr_Function_S %n1 %int_0
OpStore %82 %81
%83 = OpAccessChain %_ptr_Function_S %n1 %int_0
%84 = OpLoad %S %83
%85 = OpAccessChain %_ptr_Function_S %n1 %int_1
OpStore %85 %84
%86 = OpLoad %Nested %n1
OpStore %n2 %86
%87 = OpLoad %Nested %n2
OpStore %n3 %87
%88 = OpAccessChain %_ptr_Function_S %n3 %int_1
%89 = OpLoad %S %88
OpStore %90 %89
%91 = OpFunctionCall %void %modifies_a_struct_vS %90
%92 = OpLoad %S %90
OpStore %88 %92
%100 = OpCompositeConstruct %v2float %float_1 %float_2
%101 = OpCompositeConstruct %v2float %float_3 %float_4
%102 = OpCompositeConstruct %mat2v2float %100 %101
%108 = OpCompositeConstruct %Compound %102 %107
OpStore %c1 %108
%110 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%112 = OpLoad %v4float %110
%113 = OpCompositeExtract %float %112 1
%114 = OpCompositeConstruct %v2float %113 %float_2
%115 = OpCompositeConstruct %v2float %float_3 %float_4
%116 = OpCompositeConstruct %mat2v2float %114 %115
%117 = OpCompositeConstruct %Compound %116 %107
OpStore %c2 %117
%119 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%120 = OpLoad %v4float %119
%121 = OpCompositeExtract %float %120 0
%122 = OpCompositeConstruct %v2float %121 %float_2
%123 = OpCompositeConstruct %v2float %float_3 %float_4
%124 = OpCompositeConstruct %mat2v2float %122 %123
%125 = OpCompositeConstruct %Compound %124 %107
OpStore %c3 %125
%129 = OpLoad %float %x
%130 = OpFOrdEqual %bool %129 %float_3
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%134 = OpLoad %float %133
%135 = OpFOrdEqual %bool %134 %float_2
OpBranch %132
%132 = OpLabel
%136 = OpPhi %bool %false %66 %135 %131
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%139 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%140 = OpLoad %int %139
%141 = OpIEqual %bool %140 %int_3
OpBranch %138
%138 = OpLabel
%142 = OpPhi %bool %false %132 %141 %137
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%145 = OpLoad %S %s_0
%146 = OpLoad %S %expected
%147 = OpCompositeExtract %float %145 0
%148 = OpCompositeExtract %float %146 0
%149 = OpFOrdEqual %bool %147 %148
%150 = OpCompositeExtract %int %145 1
%151 = OpCompositeExtract %int %146 1
%152 = OpIEqual %bool %150 %151
%153 = OpLogicalAnd %bool %152 %149
OpBranch %144
%144 = OpLabel
%154 = OpPhi %bool %false %138 %153 %143
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%157 = OpLoad %S %s_0
%158 = OpCompositeConstruct %S %float_2 %int_3
%159 = OpCompositeExtract %float %157 0
%160 = OpCompositeExtract %float %158 0
%161 = OpFOrdEqual %bool %159 %160
%162 = OpCompositeExtract %int %157 1
%163 = OpCompositeExtract %int %158 1
%164 = OpIEqual %bool %162 %163
%165 = OpLogicalAnd %bool %164 %161
OpBranch %156
%156 = OpLabel
%166 = OpPhi %bool %false %144 %165 %155
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%169 = OpLoad %S %s_0
%170 = OpFunctionCall %S %returns_a_struct_S
%171 = OpCompositeExtract %float %169 0
%172 = OpCompositeExtract %float %170 0
%173 = OpFOrdNotEqual %bool %171 %172
%174 = OpCompositeExtract %int %169 1
%175 = OpCompositeExtract %int %170 1
%176 = OpINotEqual %bool %174 %175
%177 = OpLogicalOr %bool %176 %173
OpBranch %168
%168 = OpLabel
%178 = OpPhi %bool %false %156 %177 %167
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%181 = OpLoad %Nested %n1
%182 = OpLoad %Nested %n2
%183 = OpCompositeExtract %S %181 0
%184 = OpCompositeExtract %S %182 0
%185 = OpCompositeExtract %float %183 0
%186 = OpCompositeExtract %float %184 0
%187 = OpFOrdEqual %bool %185 %186
%188 = OpCompositeExtract %int %183 1
%189 = OpCompositeExtract %int %184 1
%190 = OpIEqual %bool %188 %189
%191 = OpLogicalAnd %bool %190 %187
%192 = OpCompositeExtract %S %181 1
%193 = OpCompositeExtract %S %182 1
%194 = OpCompositeExtract %float %192 0
%195 = OpCompositeExtract %float %193 0
%196 = OpFOrdEqual %bool %194 %195
%197 = OpCompositeExtract %int %192 1
%198 = OpCompositeExtract %int %193 1
%199 = OpIEqual %bool %197 %198
%200 = OpLogicalAnd %bool %199 %196
%201 = OpLogicalAnd %bool %200 %191
OpBranch %180
%180 = OpLabel
%202 = OpPhi %bool %false %168 %201 %179
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpLoad %Nested %n1
%206 = OpLoad %Nested %n3
%207 = OpCompositeExtract %S %205 0
%208 = OpCompositeExtract %S %206 0
%209 = OpCompositeExtract %float %207 0
%210 = OpCompositeExtract %float %208 0
%211 = OpFOrdNotEqual %bool %209 %210
%212 = OpCompositeExtract %int %207 1
%213 = OpCompositeExtract %int %208 1
%214 = OpINotEqual %bool %212 %213
%215 = OpLogicalOr %bool %214 %211
%216 = OpCompositeExtract %S %205 1
%217 = OpCompositeExtract %S %206 1
%218 = OpCompositeExtract %float %216 0
%219 = OpCompositeExtract %float %217 0
%220 = OpFOrdNotEqual %bool %218 %219
%221 = OpCompositeExtract %int %216 1
%222 = OpCompositeExtract %int %217 1
%223 = OpINotEqual %bool %221 %222
%224 = OpLogicalOr %bool %223 %220
%225 = OpLogicalOr %bool %224 %215
OpBranch %204
%204 = OpLabel
%226 = OpPhi %bool %false %180 %225 %203
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%229 = OpLoad %Nested %n3
%230 = OpCompositeConstruct %S %float_1 %int_2
%231 = OpCompositeConstruct %S %float_2 %int_3
%232 = OpCompositeConstruct %Nested %230 %231
%233 = OpCompositeExtract %S %229 0
%234 = OpCompositeExtract %S %232 0
%235 = OpCompositeExtract %float %233 0
%236 = OpCompositeExtract %float %234 0
%237 = OpFOrdEqual %bool %235 %236
%238 = OpCompositeExtract %int %233 1
%239 = OpCompositeExtract %int %234 1
%240 = OpIEqual %bool %238 %239
%241 = OpLogicalAnd %bool %240 %237
%242 = OpCompositeExtract %S %229 1
%243 = OpCompositeExtract %S %232 1
%244 = OpCompositeExtract %float %242 0
%245 = OpCompositeExtract %float %243 0
%246 = OpFOrdEqual %bool %244 %245
%247 = OpCompositeExtract %int %242 1
%248 = OpCompositeExtract %int %243 1
%249 = OpIEqual %bool %247 %248
%250 = OpLogicalAnd %bool %249 %246
%251 = OpLogicalAnd %bool %250 %241
OpBranch %228
%228 = OpLabel
%252 = OpPhi %bool %false %204 %251 %227
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
%255 = OpLoad %Compound %c1
%256 = OpLoad %Compound %c2
%257 = OpCompositeExtract %mat2v2float %255 0
%258 = OpCompositeExtract %mat2v2float %256 0
%260 = OpCompositeExtract %v2float %257 0
%261 = OpCompositeExtract %v2float %258 0
%262 = OpFOrdEqual %v2bool %260 %261
%263 = OpAll %bool %262
%264 = OpCompositeExtract %v2float %257 1
%265 = OpCompositeExtract %v2float %258 1
%266 = OpFOrdEqual %v2bool %264 %265
%267 = OpAll %bool %266
%268 = OpLogicalAnd %bool %263 %267
%269 = OpCompositeExtract %v4int %255 1
%270 = OpCompositeExtract %v4int %256 1
%271 = OpIEqual %v4bool %269 %270
%273 = OpAll %bool %271
%274 = OpLogicalAnd %bool %273 %268
OpBranch %254
%254 = OpLabel
%275 = OpPhi %bool %false %228 %274 %253
OpSelectionMerge %277 None
OpBranchConditional %275 %276 %277
%276 = OpLabel
%278 = OpLoad %Compound %c2
%279 = OpLoad %Compound %c3
%280 = OpCompositeExtract %mat2v2float %278 0
%281 = OpCompositeExtract %mat2v2float %279 0
%282 = OpCompositeExtract %v2float %280 0
%283 = OpCompositeExtract %v2float %281 0
%284 = OpFOrdNotEqual %v2bool %282 %283
%285 = OpAny %bool %284
%286 = OpCompositeExtract %v2float %280 1
%287 = OpCompositeExtract %v2float %281 1
%288 = OpFOrdNotEqual %v2bool %286 %287
%289 = OpAny %bool %288
%290 = OpLogicalOr %bool %285 %289
%291 = OpCompositeExtract %v4int %278 1
%292 = OpCompositeExtract %v4int %279 1
%293 = OpINotEqual %v4bool %291 %292
%294 = OpAny %bool %293
%295 = OpLogicalOr %bool %294 %290
OpBranch %277
%277 = OpLabel
%296 = OpPhi %bool %false %254 %295 %276
OpStore %valid %296
%297 = OpLoad %bool %valid
OpSelectionMerge %302 None
OpBranchConditional %297 %300 %301
%300 = OpLabel
%303 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%304 = OpLoad %v4float %303
OpStore %298 %304
OpBranch %302
%301 = OpLabel
%305 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%306 = OpLoad %v4float %305
OpStore %298 %306
OpBranch %302
%302 = OpLabel
%307 = OpLoad %v4float %298
OpReturnValue %307
OpFunctionEnd
