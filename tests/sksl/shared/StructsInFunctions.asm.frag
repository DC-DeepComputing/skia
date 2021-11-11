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
OpName %Vector "Vector"
OpMemberName %Vector 0 "f4"
OpMemberName %Vector 1 "i4"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %v3 "v3"
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
OpMemberDecorate %Vector 0 Offset 0
OpMemberDecorate %Vector 1 Offset 16
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
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
%v4int = OpTypeVector %int 4
%Vector = OpTypeStruct %v4float %v4int
%_ptr_Function_Vector = OpTypePointer Function %Vector
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%99 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%int_8 = OpConstant %int 8
%104 = OpConstantComposite %v4int %int_5 %int_6 %int_7 %int_8
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
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
%v1 = OpVariable %_ptr_Function_Vector Function
%v2 = OpVariable %_ptr_Function_Vector Function
%v3 = OpVariable %_ptr_Function_Vector Function
%valid = OpVariable %_ptr_Function_bool Function
%276 = OpVariable %_ptr_Function_v4float Function
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
%105 = OpCompositeConstruct %Vector %99 %104
OpStore %v1 %105
%107 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%109 = OpLoad %v4float %107
%110 = OpCompositeExtract %float %109 1
%111 = OpCompositeConstruct %v4float %110 %float_2 %float_3 %float_4
%112 = OpCompositeConstruct %Vector %111 %104
OpStore %v2 %112
%114 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%115 = OpLoad %v4float %114
%116 = OpCompositeExtract %float %115 0
%117 = OpCompositeConstruct %v4float %116 %float_2 %float_3 %float_4
%118 = OpCompositeConstruct %Vector %117 %104
OpStore %v3 %118
%122 = OpLoad %float %x
%123 = OpFOrdEqual %bool %122 %float_3
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%127 = OpLoad %float %126
%128 = OpFOrdEqual %bool %127 %float_2
OpBranch %125
%125 = OpLabel
%129 = OpPhi %bool %false %66 %128 %124
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%132 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%133 = OpLoad %int %132
%134 = OpIEqual %bool %133 %int_3
OpBranch %131
%131 = OpLabel
%135 = OpPhi %bool %false %125 %134 %130
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpLoad %S %s_0
%139 = OpLoad %S %expected
%140 = OpCompositeExtract %float %138 0
%141 = OpCompositeExtract %float %139 0
%142 = OpFOrdEqual %bool %140 %141
%143 = OpCompositeExtract %int %138 1
%144 = OpCompositeExtract %int %139 1
%145 = OpIEqual %bool %143 %144
%146 = OpLogicalAnd %bool %145 %142
OpBranch %137
%137 = OpLabel
%147 = OpPhi %bool %false %131 %146 %136
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%150 = OpLoad %S %s_0
%151 = OpCompositeConstruct %S %float_2 %int_3
%152 = OpCompositeExtract %float %150 0
%153 = OpCompositeExtract %float %151 0
%154 = OpFOrdEqual %bool %152 %153
%155 = OpCompositeExtract %int %150 1
%156 = OpCompositeExtract %int %151 1
%157 = OpIEqual %bool %155 %156
%158 = OpLogicalAnd %bool %157 %154
OpBranch %149
%149 = OpLabel
%159 = OpPhi %bool %false %137 %158 %148
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpLoad %S %s_0
%163 = OpFunctionCall %S %returns_a_struct_S
%164 = OpCompositeExtract %float %162 0
%165 = OpCompositeExtract %float %163 0
%166 = OpFOrdNotEqual %bool %164 %165
%167 = OpCompositeExtract %int %162 1
%168 = OpCompositeExtract %int %163 1
%169 = OpINotEqual %bool %167 %168
%170 = OpLogicalOr %bool %169 %166
OpBranch %161
%161 = OpLabel
%171 = OpPhi %bool %false %149 %170 %160
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%174 = OpLoad %Nested %n1
%175 = OpLoad %Nested %n2
%176 = OpCompositeExtract %S %174 0
%177 = OpCompositeExtract %S %175 0
%178 = OpCompositeExtract %float %176 0
%179 = OpCompositeExtract %float %177 0
%180 = OpFOrdEqual %bool %178 %179
%181 = OpCompositeExtract %int %176 1
%182 = OpCompositeExtract %int %177 1
%183 = OpIEqual %bool %181 %182
%184 = OpLogicalAnd %bool %183 %180
%185 = OpCompositeExtract %S %174 1
%186 = OpCompositeExtract %S %175 1
%187 = OpCompositeExtract %float %185 0
%188 = OpCompositeExtract %float %186 0
%189 = OpFOrdEqual %bool %187 %188
%190 = OpCompositeExtract %int %185 1
%191 = OpCompositeExtract %int %186 1
%192 = OpIEqual %bool %190 %191
%193 = OpLogicalAnd %bool %192 %189
%194 = OpLogicalAnd %bool %193 %184
OpBranch %173
%173 = OpLabel
%195 = OpPhi %bool %false %161 %194 %172
OpSelectionMerge %197 None
OpBranchConditional %195 %196 %197
%196 = OpLabel
%198 = OpLoad %Nested %n1
%199 = OpLoad %Nested %n3
%200 = OpCompositeExtract %S %198 0
%201 = OpCompositeExtract %S %199 0
%202 = OpCompositeExtract %float %200 0
%203 = OpCompositeExtract %float %201 0
%204 = OpFOrdNotEqual %bool %202 %203
%205 = OpCompositeExtract %int %200 1
%206 = OpCompositeExtract %int %201 1
%207 = OpINotEqual %bool %205 %206
%208 = OpLogicalOr %bool %207 %204
%209 = OpCompositeExtract %S %198 1
%210 = OpCompositeExtract %S %199 1
%211 = OpCompositeExtract %float %209 0
%212 = OpCompositeExtract %float %210 0
%213 = OpFOrdNotEqual %bool %211 %212
%214 = OpCompositeExtract %int %209 1
%215 = OpCompositeExtract %int %210 1
%216 = OpINotEqual %bool %214 %215
%217 = OpLogicalOr %bool %216 %213
%218 = OpLogicalOr %bool %217 %208
OpBranch %197
%197 = OpLabel
%219 = OpPhi %bool %false %173 %218 %196
OpSelectionMerge %221 None
OpBranchConditional %219 %220 %221
%220 = OpLabel
%222 = OpLoad %Nested %n3
%223 = OpCompositeConstruct %S %float_1 %int_2
%224 = OpCompositeConstruct %S %float_2 %int_3
%225 = OpCompositeConstruct %Nested %223 %224
%226 = OpCompositeExtract %S %222 0
%227 = OpCompositeExtract %S %225 0
%228 = OpCompositeExtract %float %226 0
%229 = OpCompositeExtract %float %227 0
%230 = OpFOrdEqual %bool %228 %229
%231 = OpCompositeExtract %int %226 1
%232 = OpCompositeExtract %int %227 1
%233 = OpIEqual %bool %231 %232
%234 = OpLogicalAnd %bool %233 %230
%235 = OpCompositeExtract %S %222 1
%236 = OpCompositeExtract %S %225 1
%237 = OpCompositeExtract %float %235 0
%238 = OpCompositeExtract %float %236 0
%239 = OpFOrdEqual %bool %237 %238
%240 = OpCompositeExtract %int %235 1
%241 = OpCompositeExtract %int %236 1
%242 = OpIEqual %bool %240 %241
%243 = OpLogicalAnd %bool %242 %239
%244 = OpLogicalAnd %bool %243 %234
OpBranch %221
%221 = OpLabel
%245 = OpPhi %bool %false %197 %244 %220
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%248 = OpLoad %Vector %v1
%249 = OpLoad %Vector %v2
%250 = OpCompositeExtract %v4float %248 0
%251 = OpCompositeExtract %v4float %249 0
%252 = OpFOrdEqual %v4bool %250 %251
%254 = OpAll %bool %252
%255 = OpCompositeExtract %v4int %248 1
%256 = OpCompositeExtract %v4int %249 1
%257 = OpIEqual %v4bool %255 %256
%258 = OpAll %bool %257
%259 = OpLogicalAnd %bool %258 %254
OpBranch %247
%247 = OpLabel
%260 = OpPhi %bool %false %221 %259 %246
OpSelectionMerge %262 None
OpBranchConditional %260 %261 %262
%261 = OpLabel
%263 = OpLoad %Vector %v2
%264 = OpLoad %Vector %v3
%265 = OpCompositeExtract %v4float %263 0
%266 = OpCompositeExtract %v4float %264 0
%267 = OpFOrdNotEqual %v4bool %265 %266
%268 = OpAny %bool %267
%269 = OpCompositeExtract %v4int %263 1
%270 = OpCompositeExtract %v4int %264 1
%271 = OpINotEqual %v4bool %269 %270
%272 = OpAny %bool %271
%273 = OpLogicalOr %bool %272 %268
OpBranch %262
%262 = OpLabel
%274 = OpPhi %bool %false %247 %273 %261
OpStore %valid %274
%275 = OpLoad %bool %valid
OpSelectionMerge %280 None
OpBranchConditional %275 %278 %279
%278 = OpLabel
%281 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%282 = OpLoad %v4float %281
OpStore %276 %282
OpBranch %280
%279 = OpLabel
%283 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%284 = OpLoad %v4float %283
OpStore %276 %284
OpBranch %280
%280 = OpLabel
%285 = OpLoad %v4float %276
OpReturnValue %285
OpFunctionEnd
