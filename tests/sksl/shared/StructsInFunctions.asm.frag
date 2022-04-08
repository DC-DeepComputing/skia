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
OpMemberName %Compound 0 "f4"
OpMemberName %Compound 1 "i3"
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
OpDecorate %73 RelaxedPrecision
OpMemberDecorate %Nested 0 Offset 0
OpMemberDecorate %Nested 0 RelaxedPrecision
OpMemberDecorate %Nested 1 Offset 16
OpMemberDecorate %Nested 1 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpMemberDecorate %Compound 0 Offset 0
OpMemberDecorate %Compound 1 Offset 16
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
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
%45 = OpConstantComposite %S %float_2 %int_3
%46 = OpTypeFunction %float %_ptr_Function_S
%55 = OpTypeFunction %void %_ptr_Function_S
%64 = OpTypeFunction %v4float %_ptr_Function_v2float
%Nested = OpTypeStruct %S %S
%_ptr_Function_Nested = OpTypePointer Function %Nested
%v3int = OpTypeVector %int 3
%Compound = OpTypeStruct %v4float %v3int
%_ptr_Function_Compound = OpTypePointer Function %Compound
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%102 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%106 = OpConstantComposite %v3int %int_5 %int_6 %int_7
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%224 = OpConstantComposite %S %float_1 %int_2
%v4bool = OpTypeVector %bool 4
%v3bool = OpTypeVector %bool 3
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
%74 = OpVariable %_ptr_Function_S Function
%expected = OpVariable %_ptr_Function_S Function
%n1 = OpVariable %_ptr_Function_Nested Function
%n2 = OpVariable %_ptr_Function_Nested Function
%n3 = OpVariable %_ptr_Function_Nested Function
%93 = OpVariable %_ptr_Function_S Function
%c1 = OpVariable %_ptr_Function_Compound Function
%c2 = OpVariable %_ptr_Function_Compound Function
%c3 = OpVariable %_ptr_Function_Compound Function
%valid = OpVariable %_ptr_Function_bool Function
%277 = OpVariable %_ptr_Function_v4float Function
%68 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %68
%70 = OpLoad %S %s_0
OpStore %71 %70
%72 = OpFunctionCall %float %accepts_a_struct_fS %71
OpStore %x %72
%73 = OpLoad %S %s_0
OpStore %74 %73
%75 = OpFunctionCall %void %modifies_a_struct_vS %74
%76 = OpLoad %S %74
OpStore %s_0 %76
%78 = OpFunctionCall %S %constructs_a_struct_S
OpStore %expected %78
%84 = OpFunctionCall %S %returns_a_struct_S
%85 = OpAccessChain %_ptr_Function_S %n1 %int_0
OpStore %85 %84
%86 = OpAccessChain %_ptr_Function_S %n1 %int_0
%87 = OpLoad %S %86
%88 = OpAccessChain %_ptr_Function_S %n1 %int_1
OpStore %88 %87
%89 = OpLoad %Nested %n1
OpStore %n2 %89
%90 = OpLoad %Nested %n2
OpStore %n3 %90
%91 = OpAccessChain %_ptr_Function_S %n3 %int_1
%92 = OpLoad %S %91
OpStore %93 %92
%94 = OpFunctionCall %void %modifies_a_struct_vS %93
%95 = OpLoad %S %93
OpStore %91 %95
%107 = OpCompositeConstruct %Compound %102 %106
OpStore %c1 %107
%109 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%111 = OpLoad %v4float %109
%112 = OpCompositeExtract %float %111 1
%113 = OpCompositeConstruct %v4float %112 %float_2 %float_3 %float_4
%114 = OpCompositeConstruct %Compound %113 %106
OpStore %c2 %114
%116 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%117 = OpLoad %v4float %116
%118 = OpCompositeExtract %float %117 0
%119 = OpCompositeConstruct %v4float %118 %float_2 %float_3 %float_4
%120 = OpCompositeConstruct %Compound %119 %106
OpStore %c3 %120
%124 = OpLoad %float %x
%125 = OpFOrdEqual %bool %124 %float_3
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%129 = OpLoad %float %128
%130 = OpFOrdEqual %bool %129 %float_2
OpBranch %127
%127 = OpLabel
%131 = OpPhi %bool %false %66 %130 %126
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%135 = OpLoad %int %134
%136 = OpIEqual %bool %135 %int_3
OpBranch %133
%133 = OpLabel
%137 = OpPhi %bool %false %127 %136 %132
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%140 = OpLoad %S %s_0
%141 = OpLoad %S %expected
%142 = OpCompositeExtract %float %140 0
%143 = OpCompositeExtract %float %141 0
%144 = OpFOrdEqual %bool %142 %143
%145 = OpCompositeExtract %int %140 1
%146 = OpCompositeExtract %int %141 1
%147 = OpIEqual %bool %145 %146
%148 = OpLogicalAnd %bool %147 %144
OpBranch %139
%139 = OpLabel
%149 = OpPhi %bool %false %133 %148 %138
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%152 = OpLoad %S %s_0
%153 = OpCompositeExtract %float %152 0
%154 = OpCompositeExtract %float %45 0
%155 = OpFOrdEqual %bool %153 %154
%156 = OpCompositeExtract %int %152 1
%157 = OpCompositeExtract %int %45 1
%158 = OpIEqual %bool %156 %157
%159 = OpLogicalAnd %bool %158 %155
OpBranch %151
%151 = OpLabel
%160 = OpPhi %bool %false %139 %159 %150
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%163 = OpLoad %S %s_0
%164 = OpFunctionCall %S %returns_a_struct_S
%165 = OpCompositeExtract %float %163 0
%166 = OpCompositeExtract %float %164 0
%167 = OpFUnordNotEqual %bool %165 %166
%168 = OpCompositeExtract %int %163 1
%169 = OpCompositeExtract %int %164 1
%170 = OpINotEqual %bool %168 %169
%171 = OpLogicalOr %bool %170 %167
OpBranch %162
%162 = OpLabel
%172 = OpPhi %bool %false %151 %171 %161
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%175 = OpLoad %Nested %n1
%176 = OpLoad %Nested %n2
%177 = OpCompositeExtract %S %175 0
%178 = OpCompositeExtract %S %176 0
%179 = OpCompositeExtract %float %177 0
%180 = OpCompositeExtract %float %178 0
%181 = OpFOrdEqual %bool %179 %180
%182 = OpCompositeExtract %int %177 1
%183 = OpCompositeExtract %int %178 1
%184 = OpIEqual %bool %182 %183
%185 = OpLogicalAnd %bool %184 %181
%186 = OpCompositeExtract %S %175 1
%187 = OpCompositeExtract %S %176 1
%188 = OpCompositeExtract %float %186 0
%189 = OpCompositeExtract %float %187 0
%190 = OpFOrdEqual %bool %188 %189
%191 = OpCompositeExtract %int %186 1
%192 = OpCompositeExtract %int %187 1
%193 = OpIEqual %bool %191 %192
%194 = OpLogicalAnd %bool %193 %190
%195 = OpLogicalAnd %bool %194 %185
OpBranch %174
%174 = OpLabel
%196 = OpPhi %bool %false %162 %195 %173
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%199 = OpLoad %Nested %n1
%200 = OpLoad %Nested %n3
%201 = OpCompositeExtract %S %199 0
%202 = OpCompositeExtract %S %200 0
%203 = OpCompositeExtract %float %201 0
%204 = OpCompositeExtract %float %202 0
%205 = OpFUnordNotEqual %bool %203 %204
%206 = OpCompositeExtract %int %201 1
%207 = OpCompositeExtract %int %202 1
%208 = OpINotEqual %bool %206 %207
%209 = OpLogicalOr %bool %208 %205
%210 = OpCompositeExtract %S %199 1
%211 = OpCompositeExtract %S %200 1
%212 = OpCompositeExtract %float %210 0
%213 = OpCompositeExtract %float %211 0
%214 = OpFUnordNotEqual %bool %212 %213
%215 = OpCompositeExtract %int %210 1
%216 = OpCompositeExtract %int %211 1
%217 = OpINotEqual %bool %215 %216
%218 = OpLogicalOr %bool %217 %214
%219 = OpLogicalOr %bool %218 %209
OpBranch %198
%198 = OpLabel
%220 = OpPhi %bool %false %174 %219 %197
OpSelectionMerge %222 None
OpBranchConditional %220 %221 %222
%221 = OpLabel
%223 = OpLoad %Nested %n3
%225 = OpCompositeConstruct %Nested %224 %45
%226 = OpCompositeExtract %S %223 0
%227 = OpCompositeExtract %S %225 0
%228 = OpCompositeExtract %float %226 0
%229 = OpCompositeExtract %float %227 0
%230 = OpFOrdEqual %bool %228 %229
%231 = OpCompositeExtract %int %226 1
%232 = OpCompositeExtract %int %227 1
%233 = OpIEqual %bool %231 %232
%234 = OpLogicalAnd %bool %233 %230
%235 = OpCompositeExtract %S %223 1
%236 = OpCompositeExtract %S %225 1
%237 = OpCompositeExtract %float %235 0
%238 = OpCompositeExtract %float %236 0
%239 = OpFOrdEqual %bool %237 %238
%240 = OpCompositeExtract %int %235 1
%241 = OpCompositeExtract %int %236 1
%242 = OpIEqual %bool %240 %241
%243 = OpLogicalAnd %bool %242 %239
%244 = OpLogicalAnd %bool %243 %234
OpBranch %222
%222 = OpLabel
%245 = OpPhi %bool %false %198 %244 %221
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%248 = OpLoad %Compound %c1
%249 = OpLoad %Compound %c2
%250 = OpCompositeExtract %v4float %248 0
%251 = OpCompositeExtract %v4float %249 0
%252 = OpFOrdEqual %v4bool %250 %251
%254 = OpAll %bool %252
%255 = OpCompositeExtract %v3int %248 1
%256 = OpCompositeExtract %v3int %249 1
%257 = OpIEqual %v3bool %255 %256
%259 = OpAll %bool %257
%260 = OpLogicalAnd %bool %259 %254
OpBranch %247
%247 = OpLabel
%261 = OpPhi %bool %false %222 %260 %246
OpSelectionMerge %263 None
OpBranchConditional %261 %262 %263
%262 = OpLabel
%264 = OpLoad %Compound %c2
%265 = OpLoad %Compound %c3
%266 = OpCompositeExtract %v4float %264 0
%267 = OpCompositeExtract %v4float %265 0
%268 = OpFUnordNotEqual %v4bool %266 %267
%269 = OpAny %bool %268
%270 = OpCompositeExtract %v3int %264 1
%271 = OpCompositeExtract %v3int %265 1
%272 = OpINotEqual %v3bool %270 %271
%273 = OpAny %bool %272
%274 = OpLogicalOr %bool %273 %269
OpBranch %263
%263 = OpLabel
%275 = OpPhi %bool %false %247 %274 %262
OpStore %valid %275
%276 = OpLoad %bool %valid
OpSelectionMerge %281 None
OpBranchConditional %276 %279 %280
%279 = OpLabel
%282 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%283 = OpLoad %v4float %282
OpStore %277 %283
OpBranch %281
%280 = OpLabel
%284 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%285 = OpLoad %v4float %284
OpStore %277 %285
OpBranch %281
%281 = OpLabel
%286 = OpLoad %v4float %277
OpReturnValue %286
OpFunctionEnd
