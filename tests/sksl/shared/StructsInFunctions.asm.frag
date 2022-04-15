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
OpMemberDecorate %Nested 0 Offset 0
OpMemberDecorate %Nested 0 RelaxedPrecision
OpMemberDecorate %Nested 1 Offset 16
OpMemberDecorate %Nested 1 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpMemberDecorate %Compound 0 Offset 0
OpMemberDecorate %Compound 1 Offset 16
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
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
%99 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%103 = OpConstantComposite %v3int %int_5 %int_6 %int_7
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
%70 = OpVariable %_ptr_Function_S Function
%72 = OpVariable %_ptr_Function_S Function
%expected = OpVariable %_ptr_Function_S Function
%n1 = OpVariable %_ptr_Function_Nested Function
%n2 = OpVariable %_ptr_Function_Nested Function
%n3 = OpVariable %_ptr_Function_Nested Function
%90 = OpVariable %_ptr_Function_S Function
%c1 = OpVariable %_ptr_Function_Compound Function
%c2 = OpVariable %_ptr_Function_Compound Function
%c3 = OpVariable %_ptr_Function_Compound Function
%valid = OpVariable %_ptr_Function_bool Function
%247 = OpVariable %_ptr_Function_v4float Function
%68 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %68
OpStore %70 %68
%71 = OpFunctionCall %float %accepts_a_struct_fS %70
OpStore %x %71
OpStore %72 %68
%73 = OpFunctionCall %void %modifies_a_struct_vS %72
%74 = OpLoad %S %72
OpStore %s_0 %74
%76 = OpFunctionCall %S %constructs_a_struct_S
OpStore %expected %76
%82 = OpFunctionCall %S %returns_a_struct_S
%83 = OpAccessChain %_ptr_Function_S %n1 %int_0
OpStore %83 %82
%84 = OpAccessChain %_ptr_Function_S %n1 %int_0
%85 = OpLoad %S %84
%86 = OpAccessChain %_ptr_Function_S %n1 %int_1
OpStore %86 %85
%87 = OpLoad %Nested %n1
OpStore %n2 %87
OpStore %n3 %87
%88 = OpAccessChain %_ptr_Function_S %n3 %int_1
%89 = OpLoad %S %88
OpStore %90 %89
%91 = OpFunctionCall %void %modifies_a_struct_vS %90
%92 = OpLoad %S %90
OpStore %88 %92
%104 = OpCompositeConstruct %Compound %99 %103
OpStore %c1 %104
%106 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%108 = OpLoad %v4float %106
%109 = OpCompositeExtract %float %108 1
%110 = OpCompositeConstruct %v4float %109 %float_2 %float_3 %float_4
%111 = OpCompositeConstruct %Compound %110 %103
OpStore %c2 %111
%113 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%114 = OpLoad %v4float %113
%115 = OpCompositeExtract %float %114 0
%116 = OpCompositeConstruct %v4float %115 %float_2 %float_3 %float_4
%117 = OpCompositeConstruct %Compound %116 %103
OpStore %c3 %117
%121 = OpFOrdEqual %bool %71 %float_3
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%125 = OpLoad %float %124
%126 = OpFOrdEqual %bool %125 %float_2
OpBranch %123
%123 = OpLabel
%127 = OpPhi %bool %false %66 %126 %122
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%130 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%131 = OpLoad %int %130
%132 = OpIEqual %bool %131 %int_3
OpBranch %129
%129 = OpLabel
%133 = OpPhi %bool %false %123 %132 %128
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpCompositeExtract %float %74 0
%137 = OpCompositeExtract %float %76 0
%138 = OpFOrdEqual %bool %136 %137
%139 = OpCompositeExtract %int %74 1
%140 = OpCompositeExtract %int %76 1
%141 = OpIEqual %bool %139 %140
%142 = OpLogicalAnd %bool %141 %138
OpBranch %135
%135 = OpLabel
%143 = OpPhi %bool %false %129 %142 %134
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%146 = OpCompositeConstruct %S %float_2 %int_3
%147 = OpCompositeExtract %float %74 0
%148 = OpFOrdEqual %bool %147 %float_2
%149 = OpCompositeExtract %int %74 1
%150 = OpIEqual %bool %149 %int_3
%151 = OpLogicalAnd %bool %150 %148
OpBranch %145
%145 = OpLabel
%152 = OpPhi %bool %false %135 %151 %144
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpFunctionCall %S %returns_a_struct_S
%156 = OpCompositeExtract %float %74 0
%157 = OpCompositeExtract %float %155 0
%158 = OpFUnordNotEqual %bool %156 %157
%159 = OpCompositeExtract %int %74 1
%160 = OpCompositeExtract %int %155 1
%161 = OpINotEqual %bool %159 %160
%162 = OpLogicalOr %bool %161 %158
OpBranch %154
%154 = OpLabel
%163 = OpPhi %bool %false %145 %162 %153
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%166 = OpLoad %Nested %n1
%167 = OpCompositeExtract %S %166 0
%168 = OpCompositeExtract %S %87 0
%169 = OpCompositeExtract %float %167 0
%170 = OpCompositeExtract %float %168 0
%171 = OpFOrdEqual %bool %169 %170
%172 = OpCompositeExtract %int %167 1
%173 = OpCompositeExtract %int %168 1
%174 = OpIEqual %bool %172 %173
%175 = OpLogicalAnd %bool %174 %171
%176 = OpCompositeExtract %S %166 1
%177 = OpCompositeExtract %S %87 1
%178 = OpCompositeExtract %float %176 0
%179 = OpCompositeExtract %float %177 0
%180 = OpFOrdEqual %bool %178 %179
%181 = OpCompositeExtract %int %176 1
%182 = OpCompositeExtract %int %177 1
%183 = OpIEqual %bool %181 %182
%184 = OpLogicalAnd %bool %183 %180
%185 = OpLogicalAnd %bool %184 %175
OpBranch %165
%165 = OpLabel
%186 = OpPhi %bool %false %154 %185 %164
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpLoad %Nested %n1
%190 = OpCompositeExtract %S %189 0
%191 = OpCompositeExtract %S %87 0
%192 = OpCompositeExtract %float %190 0
%193 = OpCompositeExtract %float %191 0
%194 = OpFUnordNotEqual %bool %192 %193
%195 = OpCompositeExtract %int %190 1
%196 = OpCompositeExtract %int %191 1
%197 = OpINotEqual %bool %195 %196
%198 = OpLogicalOr %bool %197 %194
%199 = OpCompositeExtract %S %189 1
%200 = OpCompositeExtract %S %87 1
%201 = OpCompositeExtract %float %199 0
%202 = OpCompositeExtract %float %200 0
%203 = OpFUnordNotEqual %bool %201 %202
%204 = OpCompositeExtract %int %199 1
%205 = OpCompositeExtract %int %200 1
%206 = OpINotEqual %bool %204 %205
%207 = OpLogicalOr %bool %206 %203
%208 = OpLogicalOr %bool %207 %198
OpBranch %188
%188 = OpLabel
%209 = OpPhi %bool %false %165 %208 %187
OpSelectionMerge %211 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%212 = OpCompositeConstruct %S %float_1 %int_2
%213 = OpCompositeConstruct %S %float_2 %int_3
%214 = OpCompositeConstruct %Nested %212 %213
%215 = OpCompositeExtract %S %87 0
%216 = OpCompositeExtract %float %215 0
%217 = OpFOrdEqual %bool %216 %float_1
%218 = OpCompositeExtract %int %215 1
%219 = OpIEqual %bool %218 %int_2
%220 = OpLogicalAnd %bool %219 %217
%221 = OpCompositeExtract %S %87 1
%222 = OpCompositeExtract %float %221 0
%223 = OpFOrdEqual %bool %222 %float_2
%224 = OpCompositeExtract %int %221 1
%225 = OpIEqual %bool %224 %int_3
%226 = OpLogicalAnd %bool %225 %223
%227 = OpLogicalAnd %bool %226 %220
OpBranch %211
%211 = OpLabel
%228 = OpPhi %bool %false %188 %227 %210
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%231 = OpFOrdEqual %v4bool %99 %110
%233 = OpAll %bool %231
%234 = OpIEqual %v3bool %103 %103
%236 = OpAll %bool %234
%237 = OpLogicalAnd %bool %236 %233
OpBranch %230
%230 = OpLabel
%238 = OpPhi %bool %false %211 %237 %229
OpSelectionMerge %240 None
OpBranchConditional %238 %239 %240
%239 = OpLabel
%241 = OpFUnordNotEqual %v4bool %110 %116
%242 = OpAny %bool %241
%243 = OpINotEqual %v3bool %103 %103
%244 = OpAny %bool %243
%245 = OpLogicalOr %bool %244 %242
OpBranch %240
%240 = OpLabel
%246 = OpPhi %bool %false %230 %245 %239
OpStore %valid %246
OpSelectionMerge %251 None
OpBranchConditional %246 %249 %250
%249 = OpLabel
%252 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%253 = OpLoad %v4float %252
OpStore %247 %253
OpBranch %251
%250 = OpLabel
%254 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%255 = OpLoad %v4float %254
OpStore %247 %255
OpBranch %251
%251 = OpLabel
%256 = OpLoad %v4float %247
OpReturnValue %256
OpFunctionEnd
