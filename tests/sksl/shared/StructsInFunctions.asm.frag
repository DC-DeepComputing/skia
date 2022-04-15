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
OpDecorate %91 RelaxedPrecision
OpMemberDecorate %Compound 0 Offset 0
OpMemberDecorate %Compound 1 Offset 16
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
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
%101 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%105 = OpConstantComposite %v3int %int_5 %int_6 %int_7
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
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
%74 = OpVariable %_ptr_Function_S Function
%expected = OpVariable %_ptr_Function_S Function
%n1 = OpVariable %_ptr_Function_Nested Function
%n2 = OpVariable %_ptr_Function_Nested Function
%n3 = OpVariable %_ptr_Function_Nested Function
%92 = OpVariable %_ptr_Function_S Function
%c1 = OpVariable %_ptr_Function_Compound Function
%c2 = OpVariable %_ptr_Function_Compound Function
%c3 = OpVariable %_ptr_Function_Compound Function
%valid = OpVariable %_ptr_Function_bool Function
%270 = OpVariable %_ptr_Function_v4float Function
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
OpStore %n3 %89
%90 = OpAccessChain %_ptr_Function_S %n3 %int_1
%91 = OpLoad %S %90
OpStore %92 %91
%93 = OpFunctionCall %void %modifies_a_struct_vS %92
%94 = OpLoad %S %92
OpStore %90 %94
%106 = OpCompositeConstruct %Compound %101 %105
OpStore %c1 %106
%108 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%110 = OpLoad %v4float %108
%111 = OpCompositeExtract %float %110 1
%112 = OpCompositeConstruct %v4float %111 %float_2 %float_3 %float_4
%113 = OpCompositeConstruct %Compound %112 %105
OpStore %c2 %113
%115 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%116 = OpLoad %v4float %115
%117 = OpCompositeExtract %float %116 0
%118 = OpCompositeConstruct %v4float %117 %float_2 %float_3 %float_4
%119 = OpCompositeConstruct %Compound %118 %105
OpStore %c3 %119
%123 = OpLoad %float %x
%124 = OpFOrdEqual %bool %123 %float_3
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%127 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%128 = OpLoad %float %127
%129 = OpFOrdEqual %bool %128 %float_2
OpBranch %126
%126 = OpLabel
%130 = OpPhi %bool %false %66 %129 %125
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%134 = OpLoad %int %133
%135 = OpIEqual %bool %134 %int_3
OpBranch %132
%132 = OpLabel
%136 = OpPhi %bool %false %126 %135 %131
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%139 = OpLoad %S %s_0
%140 = OpLoad %S %expected
%141 = OpCompositeExtract %float %139 0
%142 = OpCompositeExtract %float %140 0
%143 = OpFOrdEqual %bool %141 %142
%144 = OpCompositeExtract %int %139 1
%145 = OpCompositeExtract %int %140 1
%146 = OpIEqual %bool %144 %145
%147 = OpLogicalAnd %bool %146 %143
OpBranch %138
%138 = OpLabel
%148 = OpPhi %bool %false %132 %147 %137
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%151 = OpLoad %S %s_0
%152 = OpCompositeConstruct %S %float_2 %int_3
%153 = OpCompositeExtract %float %151 0
%154 = OpFOrdEqual %bool %153 %float_2
%155 = OpCompositeExtract %int %151 1
%156 = OpIEqual %bool %155 %int_3
%157 = OpLogicalAnd %bool %156 %154
OpBranch %150
%150 = OpLabel
%158 = OpPhi %bool %false %138 %157 %149
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%161 = OpLoad %S %s_0
%162 = OpFunctionCall %S %returns_a_struct_S
%163 = OpCompositeExtract %float %161 0
%164 = OpCompositeExtract %float %162 0
%165 = OpFUnordNotEqual %bool %163 %164
%166 = OpCompositeExtract %int %161 1
%167 = OpCompositeExtract %int %162 1
%168 = OpINotEqual %bool %166 %167
%169 = OpLogicalOr %bool %168 %165
OpBranch %160
%160 = OpLabel
%170 = OpPhi %bool %false %150 %169 %159
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpLoad %Nested %n1
%174 = OpLoad %Nested %n2
%175 = OpCompositeExtract %S %173 0
%176 = OpCompositeExtract %S %174 0
%177 = OpCompositeExtract %float %175 0
%178 = OpCompositeExtract %float %176 0
%179 = OpFOrdEqual %bool %177 %178
%180 = OpCompositeExtract %int %175 1
%181 = OpCompositeExtract %int %176 1
%182 = OpIEqual %bool %180 %181
%183 = OpLogicalAnd %bool %182 %179
%184 = OpCompositeExtract %S %173 1
%185 = OpCompositeExtract %S %174 1
%186 = OpCompositeExtract %float %184 0
%187 = OpCompositeExtract %float %185 0
%188 = OpFOrdEqual %bool %186 %187
%189 = OpCompositeExtract %int %184 1
%190 = OpCompositeExtract %int %185 1
%191 = OpIEqual %bool %189 %190
%192 = OpLogicalAnd %bool %191 %188
%193 = OpLogicalAnd %bool %192 %183
OpBranch %172
%172 = OpLabel
%194 = OpPhi %bool %false %160 %193 %171
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
%197 = OpLoad %Nested %n1
%198 = OpLoad %Nested %n3
%199 = OpCompositeExtract %S %197 0
%200 = OpCompositeExtract %S %198 0
%201 = OpCompositeExtract %float %199 0
%202 = OpCompositeExtract %float %200 0
%203 = OpFUnordNotEqual %bool %201 %202
%204 = OpCompositeExtract %int %199 1
%205 = OpCompositeExtract %int %200 1
%206 = OpINotEqual %bool %204 %205
%207 = OpLogicalOr %bool %206 %203
%208 = OpCompositeExtract %S %197 1
%209 = OpCompositeExtract %S %198 1
%210 = OpCompositeExtract %float %208 0
%211 = OpCompositeExtract %float %209 0
%212 = OpFUnordNotEqual %bool %210 %211
%213 = OpCompositeExtract %int %208 1
%214 = OpCompositeExtract %int %209 1
%215 = OpINotEqual %bool %213 %214
%216 = OpLogicalOr %bool %215 %212
%217 = OpLogicalOr %bool %216 %207
OpBranch %196
%196 = OpLabel
%218 = OpPhi %bool %false %172 %217 %195
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpLoad %Nested %n3
%222 = OpCompositeConstruct %S %float_1 %int_2
%223 = OpCompositeConstruct %S %float_2 %int_3
%224 = OpCompositeConstruct %Nested %222 %223
%225 = OpCompositeExtract %S %221 0
%226 = OpCompositeExtract %float %225 0
%227 = OpFOrdEqual %bool %226 %float_1
%228 = OpCompositeExtract %int %225 1
%229 = OpIEqual %bool %228 %int_2
%230 = OpLogicalAnd %bool %229 %227
%231 = OpCompositeExtract %S %221 1
%232 = OpCompositeExtract %float %231 0
%233 = OpFOrdEqual %bool %232 %float_2
%234 = OpCompositeExtract %int %231 1
%235 = OpIEqual %bool %234 %int_3
%236 = OpLogicalAnd %bool %235 %233
%237 = OpLogicalAnd %bool %236 %230
OpBranch %220
%220 = OpLabel
%238 = OpPhi %bool %false %196 %237 %219
OpSelectionMerge %240 None
OpBranchConditional %238 %239 %240
%239 = OpLabel
%241 = OpLoad %Compound %c1
%242 = OpLoad %Compound %c2
%243 = OpCompositeExtract %v4float %241 0
%244 = OpCompositeExtract %v4float %242 0
%245 = OpFOrdEqual %v4bool %243 %244
%247 = OpAll %bool %245
%248 = OpCompositeExtract %v3int %241 1
%249 = OpCompositeExtract %v3int %242 1
%250 = OpIEqual %v3bool %248 %249
%252 = OpAll %bool %250
%253 = OpLogicalAnd %bool %252 %247
OpBranch %240
%240 = OpLabel
%254 = OpPhi %bool %false %220 %253 %239
OpSelectionMerge %256 None
OpBranchConditional %254 %255 %256
%255 = OpLabel
%257 = OpLoad %Compound %c2
%258 = OpLoad %Compound %c3
%259 = OpCompositeExtract %v4float %257 0
%260 = OpCompositeExtract %v4float %258 0
%261 = OpFUnordNotEqual %v4bool %259 %260
%262 = OpAny %bool %261
%263 = OpCompositeExtract %v3int %257 1
%264 = OpCompositeExtract %v3int %258 1
%265 = OpINotEqual %v3bool %263 %264
%266 = OpAny %bool %265
%267 = OpLogicalOr %bool %266 %262
OpBranch %256
%256 = OpLabel
%268 = OpPhi %bool %false %240 %267 %255
OpStore %valid %268
%269 = OpLoad %bool %valid
OpSelectionMerge %274 None
OpBranchConditional %269 %272 %273
%272 = OpLabel
%275 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%276 = OpLoad %v4float %275
OpStore %270 %276
OpBranch %274
%273 = OpLabel
%277 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%278 = OpLoad %v4float %277
OpStore %270 %278
OpBranch %274
%274 = OpLabel
%279 = OpLoad %v4float %270
OpReturnValue %279
OpFunctionEnd
