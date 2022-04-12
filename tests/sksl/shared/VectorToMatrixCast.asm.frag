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
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %30 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%49 = OpConstantComposite %v2float %float_n1_25 %float_0
%50 = OpConstantComposite %v2float %float_0_75 %float_2_25
%v2bool = OpTypeVector %bool 2
%75 = OpConstantComposite %v2float %float_n1_25 %float_0
%76 = OpConstantComposite %v2float %float_0_75 %float_2_25
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%102 = OpConstantComposite %v2float %float_0 %float_1
%v4int = OpTypeVector %int 4
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%268 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%30 = OpLoad %bool %ok
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%37 = OpLoad %v4float %33
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%40 = OpCompositeExtract %float %37 2
%41 = OpCompositeExtract %float %37 3
%42 = OpCompositeConstruct %v2float %38 %39
%43 = OpCompositeConstruct %v2float %40 %41
%45 = OpCompositeConstruct %mat2v2float %42 %43
%51 = OpCompositeConstruct %mat2v2float %49 %50
%53 = OpCompositeExtract %v2float %45 0
%54 = OpCompositeExtract %v2float %51 0
%55 = OpFOrdEqual %v2bool %53 %54
%56 = OpAll %bool %55
%57 = OpCompositeExtract %v2float %45 1
%58 = OpCompositeExtract %v2float %51 1
%59 = OpFOrdEqual %v2bool %57 %58
%60 = OpAll %bool %59
%61 = OpLogicalAnd %bool %56 %60
OpBranch %32
%32 = OpLabel
%62 = OpPhi %bool %false %25 %61 %31
OpStore %ok %62
%63 = OpLoad %bool %ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%67 = OpLoad %v4float %66
%68 = OpCompositeExtract %float %67 0
%69 = OpCompositeExtract %float %67 1
%70 = OpCompositeExtract %float %67 2
%71 = OpCompositeExtract %float %67 3
%72 = OpCompositeConstruct %v2float %68 %69
%73 = OpCompositeConstruct %v2float %70 %71
%74 = OpCompositeConstruct %mat2v2float %72 %73
%77 = OpCompositeConstruct %mat2v2float %75 %76
%78 = OpCompositeExtract %v2float %74 0
%79 = OpCompositeExtract %v2float %77 0
%80 = OpFOrdEqual %v2bool %78 %79
%81 = OpAll %bool %80
%82 = OpCompositeExtract %v2float %74 1
%83 = OpCompositeExtract %v2float %77 1
%84 = OpFOrdEqual %v2bool %82 %83
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %81 %85
OpBranch %65
%65 = OpLabel
%87 = OpPhi %bool %false %32 %86 %64
OpStore %ok %87
%88 = OpLoad %bool %ok
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%93 = OpLoad %v4float %91
%94 = OpCompositeExtract %float %93 0
%95 = OpCompositeExtract %float %93 1
%96 = OpCompositeExtract %float %93 2
%97 = OpCompositeExtract %float %93 3
%98 = OpCompositeConstruct %v2float %94 %95
%99 = OpCompositeConstruct %v2float %96 %97
%100 = OpCompositeConstruct %mat2v2float %98 %99
%103 = OpCompositeConstruct %mat2v2float %102 %102
%104 = OpCompositeExtract %v2float %100 0
%105 = OpCompositeExtract %v2float %103 0
%106 = OpFOrdEqual %v2bool %104 %105
%107 = OpAll %bool %106
%108 = OpCompositeExtract %v2float %100 1
%109 = OpCompositeExtract %v2float %103 1
%110 = OpFOrdEqual %v2bool %108 %109
%111 = OpAll %bool %110
%112 = OpLogicalAnd %bool %107 %111
OpBranch %90
%90 = OpLabel
%113 = OpPhi %bool %false %65 %112 %89
OpStore %ok %113
%114 = OpLoad %bool %ok
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%118 = OpLoad %v4float %117
%119 = OpCompositeExtract %float %118 0
%120 = OpCompositeExtract %float %118 1
%121 = OpCompositeExtract %float %118 2
%122 = OpCompositeExtract %float %118 3
%123 = OpCompositeConstruct %v2float %119 %120
%124 = OpCompositeConstruct %v2float %121 %122
%125 = OpCompositeConstruct %mat2v2float %123 %124
%126 = OpCompositeConstruct %mat2v2float %102 %102
%127 = OpCompositeExtract %v2float %125 0
%128 = OpCompositeExtract %v2float %126 0
%129 = OpFOrdEqual %v2bool %127 %128
%130 = OpAll %bool %129
%131 = OpCompositeExtract %v2float %125 1
%132 = OpCompositeExtract %v2float %126 1
%133 = OpFOrdEqual %v2bool %131 %132
%134 = OpAll %bool %133
%135 = OpLogicalAnd %bool %130 %134
OpBranch %116
%116 = OpLabel
%136 = OpPhi %bool %false %90 %135 %115
OpStore %ok %136
%137 = OpLoad %bool %ok
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%141 = OpLoad %v4float %140
%142 = OpCompositeExtract %float %141 0
%143 = OpConvertFToS %int %142
%144 = OpCompositeExtract %float %141 1
%145 = OpConvertFToS %int %144
%146 = OpCompositeExtract %float %141 2
%147 = OpConvertFToS %int %146
%148 = OpCompositeExtract %float %141 3
%149 = OpConvertFToS %int %148
%151 = OpCompositeConstruct %v4int %143 %145 %147 %149
%152 = OpCompositeExtract %int %151 0
%153 = OpConvertSToF %float %152
%154 = OpCompositeExtract %int %151 1
%155 = OpConvertSToF %float %154
%156 = OpCompositeExtract %int %151 2
%157 = OpConvertSToF %float %156
%158 = OpCompositeExtract %int %151 3
%159 = OpConvertSToF %float %158
%160 = OpCompositeConstruct %v4float %153 %155 %157 %159
%161 = OpCompositeExtract %float %160 0
%162 = OpCompositeExtract %float %160 1
%163 = OpCompositeExtract %float %160 2
%164 = OpCompositeExtract %float %160 3
%165 = OpCompositeConstruct %v2float %161 %162
%166 = OpCompositeConstruct %v2float %163 %164
%167 = OpCompositeConstruct %mat2v2float %165 %166
%168 = OpCompositeConstruct %mat2v2float %102 %102
%169 = OpCompositeExtract %v2float %167 0
%170 = OpCompositeExtract %v2float %168 0
%171 = OpFOrdEqual %v2bool %169 %170
%172 = OpAll %bool %171
%173 = OpCompositeExtract %v2float %167 1
%174 = OpCompositeExtract %v2float %168 1
%175 = OpFOrdEqual %v2bool %173 %174
%176 = OpAll %bool %175
%177 = OpLogicalAnd %bool %172 %176
OpBranch %139
%139 = OpLabel
%178 = OpPhi %bool %false %116 %177 %138
OpStore %ok %178
%179 = OpLoad %bool %ok
OpSelectionMerge %181 None
OpBranchConditional %179 %180 %181
%180 = OpLabel
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%183 = OpLoad %v4float %182
%184 = OpCompositeExtract %float %183 0
%185 = OpCompositeExtract %float %183 1
%186 = OpCompositeExtract %float %183 2
%187 = OpCompositeExtract %float %183 3
%188 = OpCompositeConstruct %v2float %184 %185
%189 = OpCompositeConstruct %v2float %186 %187
%190 = OpCompositeConstruct %mat2v2float %188 %189
%191 = OpCompositeConstruct %mat2v2float %102 %102
%192 = OpCompositeExtract %v2float %190 0
%193 = OpCompositeExtract %v2float %191 0
%194 = OpFOrdEqual %v2bool %192 %193
%195 = OpAll %bool %194
%196 = OpCompositeExtract %v2float %190 1
%197 = OpCompositeExtract %v2float %191 1
%198 = OpFOrdEqual %v2bool %196 %197
%199 = OpAll %bool %198
%200 = OpLogicalAnd %bool %195 %199
OpBranch %181
%181 = OpLabel
%201 = OpPhi %bool %false %139 %200 %180
OpStore %ok %201
%202 = OpLoad %bool %ok
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%206 = OpLoad %v4float %205
%207 = OpCompositeExtract %float %206 0
%208 = OpCompositeExtract %float %206 1
%209 = OpCompositeExtract %float %206 2
%210 = OpCompositeExtract %float %206 3
%211 = OpCompositeConstruct %v2float %207 %208
%212 = OpCompositeConstruct %v2float %209 %210
%213 = OpCompositeConstruct %mat2v2float %211 %212
%214 = OpCompositeConstruct %mat2v2float %102 %102
%215 = OpCompositeExtract %v2float %213 0
%216 = OpCompositeExtract %v2float %214 0
%217 = OpFOrdEqual %v2bool %215 %216
%218 = OpAll %bool %217
%219 = OpCompositeExtract %v2float %213 1
%220 = OpCompositeExtract %v2float %214 1
%221 = OpFOrdEqual %v2bool %219 %220
%222 = OpAll %bool %221
%223 = OpLogicalAnd %bool %218 %222
OpBranch %204
%204 = OpLabel
%224 = OpPhi %bool %false %181 %223 %203
OpStore %ok %224
%225 = OpLoad %bool %ok
OpSelectionMerge %227 None
OpBranchConditional %225 %226 %227
%226 = OpLabel
%228 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%229 = OpLoad %v4float %228
%230 = OpCompositeExtract %float %229 0
%231 = OpFUnordNotEqual %bool %230 %float_0
%232 = OpCompositeExtract %float %229 1
%233 = OpFUnordNotEqual %bool %232 %float_0
%234 = OpCompositeExtract %float %229 2
%235 = OpFUnordNotEqual %bool %234 %float_0
%236 = OpCompositeExtract %float %229 3
%237 = OpFUnordNotEqual %bool %236 %float_0
%239 = OpCompositeConstruct %v4bool %231 %233 %235 %237
%240 = OpCompositeExtract %bool %239 0
%241 = OpSelect %float %240 %float_1 %float_0
%242 = OpCompositeExtract %bool %239 1
%243 = OpSelect %float %242 %float_1 %float_0
%244 = OpCompositeExtract %bool %239 2
%245 = OpSelect %float %244 %float_1 %float_0
%246 = OpCompositeExtract %bool %239 3
%247 = OpSelect %float %246 %float_1 %float_0
%248 = OpCompositeConstruct %v4float %241 %243 %245 %247
%249 = OpCompositeExtract %float %248 0
%250 = OpCompositeExtract %float %248 1
%251 = OpCompositeExtract %float %248 2
%252 = OpCompositeExtract %float %248 3
%253 = OpCompositeConstruct %v2float %249 %250
%254 = OpCompositeConstruct %v2float %251 %252
%255 = OpCompositeConstruct %mat2v2float %253 %254
%256 = OpCompositeConstruct %mat2v2float %102 %102
%257 = OpCompositeExtract %v2float %255 0
%258 = OpCompositeExtract %v2float %256 0
%259 = OpFOrdEqual %v2bool %257 %258
%260 = OpAll %bool %259
%261 = OpCompositeExtract %v2float %255 1
%262 = OpCompositeExtract %v2float %256 1
%263 = OpFOrdEqual %v2bool %261 %262
%264 = OpAll %bool %263
%265 = OpLogicalAnd %bool %260 %264
OpBranch %227
%227 = OpLabel
%266 = OpPhi %bool %false %204 %265 %226
OpStore %ok %266
%267 = OpLoad %bool %ok
OpSelectionMerge %272 None
OpBranchConditional %267 %270 %271
%270 = OpLabel
%273 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%274 = OpLoad %v4float %273
OpStore %268 %274
OpBranch %272
%271 = OpLabel
%275 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%277 = OpLoad %v4float %275
OpStore %268 %277
OpBranch %272
%272 = OpLabel
%278 = OpLoad %v4float %268
OpReturnValue %278
OpFunctionEnd
