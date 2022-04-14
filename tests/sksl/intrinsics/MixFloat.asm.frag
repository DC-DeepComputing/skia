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
OpMemberName %_UniformBuffer 2 "colorBlack"
OpMemberName %_UniformBuffer 3 "colorWhite"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedBW "expectedBW"
OpName %expectedWT "expectedWT"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expectedBW RelaxedPrecision
OpDecorate %expectedWT RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_5 = OpConstant %float 0.5
%float_1 = OpConstant %float 1
%30 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
%float_2_25 = OpConstant %float 2.25
%33 = OpConstantComposite %v4float %float_1 %float_0_5 %float_1 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%43 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%45 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%v4bool = OpTypeVector %bool 4
%float_0_25 = OpConstant %float 0.25
%56 = OpConstantComposite %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%float_0_75 = OpConstant %float 0.75
%59 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
%69 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%71 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
%81 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%83 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%110 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%127 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%141 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%int_4 = OpConstant %int 4
%195 = OpConstantComposite %v2float %float_0 %float_0_5
%210 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%223 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
%237 = OpConstantComposite %v2float %float_1 %float_0_5
%245 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1
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
%expectedBW = OpVariable %_ptr_Function_v4float Function
%expectedWT = OpVariable %_ptr_Function_v4float Function
%257 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedBW %30
OpStore %expectedWT %33
%35 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %35
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%42 = OpLoad %v4float %40
%44 = OpExtInst %v4float %1 FMix %39 %42 %43
%46 = OpFOrdEqual %v4bool %44 %45
%48 = OpAll %bool %46
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%52 = OpLoad %v4float %51
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%54 = OpLoad %v4float %53
%57 = OpExtInst %v4float %1 FMix %52 %54 %56
%60 = OpFOrdEqual %v4bool %57 %59
%61 = OpAll %bool %60
OpBranch %50
%50 = OpLabel
%62 = OpPhi %bool %false %25 %61 %49
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%65 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%66 = OpLoad %v4float %65
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%68 = OpLoad %v4float %67
%70 = OpExtInst %v4float %1 FMix %66 %68 %69
%72 = OpFOrdEqual %v4bool %70 %71
%73 = OpAll %bool %72
OpBranch %64
%64 = OpLabel
%74 = OpPhi %bool %false %50 %73 %63
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%78 = OpLoad %v4float %77
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%80 = OpLoad %v4float %79
%82 = OpExtInst %v4float %1 FMix %78 %80 %81
%84 = OpFOrdEqual %v4bool %82 %83
%85 = OpAll %bool %84
OpBranch %76
%76 = OpLabel
%86 = OpPhi %bool %false %64 %85 %75
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%91 = OpLoad %v4float %89
%92 = OpCompositeExtract %float %91 0
%93 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%95 = OpLoad %v4float %93
%96 = OpCompositeExtract %float %95 0
%97 = OpExtInst %float %1 FMix %92 %96 %float_0_5
%98 = OpLoad %v4float %expectedBW
%99 = OpCompositeExtract %float %98 0
%100 = OpFOrdEqual %bool %97 %99
OpBranch %88
%88 = OpLabel
%101 = OpPhi %bool %false %76 %100 %87
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%105 = OpLoad %v4float %104
%106 = OpVectorShuffle %v2float %105 %105 0 1
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%108 = OpLoad %v4float %107
%109 = OpVectorShuffle %v2float %108 %108 0 1
%111 = OpExtInst %v2float %1 FMix %106 %109 %110
%112 = OpLoad %v4float %expectedBW
%113 = OpVectorShuffle %v2float %112 %112 0 1
%114 = OpFOrdEqual %v2bool %111 %113
%116 = OpAll %bool %114
OpBranch %103
%103 = OpLabel
%117 = OpPhi %bool %false %88 %116 %102
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%121 = OpLoad %v4float %120
%122 = OpVectorShuffle %v3float %121 %121 0 1 2
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%125 = OpLoad %v4float %124
%126 = OpVectorShuffle %v3float %125 %125 0 1 2
%128 = OpExtInst %v3float %1 FMix %122 %126 %127
%129 = OpLoad %v4float %expectedBW
%130 = OpVectorShuffle %v3float %129 %129 0 1 2
%131 = OpFOrdEqual %v3bool %128 %130
%133 = OpAll %bool %131
OpBranch %119
%119 = OpLabel
%134 = OpPhi %bool %false %103 %133 %118
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%138 = OpLoad %v4float %137
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%140 = OpLoad %v4float %139
%142 = OpExtInst %v4float %1 FMix %138 %140 %141
%143 = OpLoad %v4float %expectedBW
%144 = OpFOrdEqual %v4bool %142 %143
%145 = OpAll %bool %144
OpBranch %136
%136 = OpLabel
%146 = OpPhi %bool %false %119 %145 %135
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%149 = OpLoad %v4float %expectedBW
%150 = OpCompositeExtract %float %149 0
%151 = OpFOrdEqual %bool %float_0_5 %150
OpBranch %148
%148 = OpLabel
%152 = OpPhi %bool %false %136 %151 %147
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpLoad %v4float %expectedBW
%156 = OpVectorShuffle %v2float %155 %155 0 1
%157 = OpFOrdEqual %v2bool %110 %156
%158 = OpAll %bool %157
OpBranch %154
%154 = OpLabel
%159 = OpPhi %bool %false %148 %158 %153
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpLoad %v4float %expectedBW
%163 = OpVectorShuffle %v3float %162 %162 0 1 2
%164 = OpFOrdEqual %v3bool %127 %163
%165 = OpAll %bool %164
OpBranch %161
%161 = OpLabel
%166 = OpPhi %bool %false %154 %165 %160
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%169 = OpLoad %v4float %expectedBW
%170 = OpFOrdEqual %v4bool %30 %169
%171 = OpAll %bool %170
OpBranch %168
%168 = OpLabel
%172 = OpPhi %bool %false %161 %171 %167
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%175 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%176 = OpLoad %v4float %175
%177 = OpCompositeExtract %float %176 0
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%180 = OpLoad %v4float %178
%181 = OpCompositeExtract %float %180 0
%182 = OpExtInst %float %1 FMix %177 %181 %float_0
%183 = OpLoad %v4float %expectedWT
%184 = OpCompositeExtract %float %183 0
%185 = OpFOrdEqual %bool %182 %184
OpBranch %174
%174 = OpLabel
%186 = OpPhi %bool %false %168 %185 %173
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%190 = OpLoad %v4float %189
%191 = OpVectorShuffle %v2float %190 %190 0 1
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%193 = OpLoad %v4float %192
%194 = OpVectorShuffle %v2float %193 %193 0 1
%196 = OpExtInst %v2float %1 FMix %191 %194 %195
%197 = OpLoad %v4float %expectedWT
%198 = OpVectorShuffle %v2float %197 %197 0 1
%199 = OpFOrdEqual %v2bool %196 %198
%200 = OpAll %bool %199
OpBranch %188
%188 = OpLabel
%201 = OpPhi %bool %false %174 %200 %187
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
%204 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%205 = OpLoad %v4float %204
%206 = OpVectorShuffle %v3float %205 %205 0 1 2
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%208 = OpLoad %v4float %207
%209 = OpVectorShuffle %v3float %208 %208 0 1 2
%211 = OpExtInst %v3float %1 FMix %206 %209 %210
%212 = OpLoad %v4float %expectedWT
%213 = OpVectorShuffle %v3float %212 %212 0 1 2
%214 = OpFOrdEqual %v3bool %211 %213
%215 = OpAll %bool %214
OpBranch %203
%203 = OpLabel
%216 = OpPhi %bool %false %188 %215 %202
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%220 = OpLoad %v4float %219
%221 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%222 = OpLoad %v4float %221
%224 = OpExtInst %v4float %1 FMix %220 %222 %223
%225 = OpLoad %v4float %expectedWT
%226 = OpFOrdEqual %v4bool %224 %225
%227 = OpAll %bool %226
OpBranch %218
%218 = OpLabel
%228 = OpPhi %bool %false %203 %227 %217
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%231 = OpLoad %v4float %expectedWT
%232 = OpCompositeExtract %float %231 0
%233 = OpFOrdEqual %bool %float_1 %232
OpBranch %230
%230 = OpLabel
%234 = OpPhi %bool %false %218 %233 %229
OpSelectionMerge %236 None
OpBranchConditional %234 %235 %236
%235 = OpLabel
%238 = OpLoad %v4float %expectedWT
%239 = OpVectorShuffle %v2float %238 %238 0 1
%240 = OpFOrdEqual %v2bool %237 %239
%241 = OpAll %bool %240
OpBranch %236
%236 = OpLabel
%242 = OpPhi %bool %false %230 %241 %235
OpSelectionMerge %244 None
OpBranchConditional %242 %243 %244
%243 = OpLabel
%246 = OpLoad %v4float %expectedWT
%247 = OpVectorShuffle %v3float %246 %246 0 1 2
%248 = OpFOrdEqual %v3bool %245 %247
%249 = OpAll %bool %248
OpBranch %244
%244 = OpLabel
%250 = OpPhi %bool %false %236 %249 %243
OpSelectionMerge %252 None
OpBranchConditional %250 %251 %252
%251 = OpLabel
%253 = OpLoad %v4float %expectedWT
%254 = OpFOrdEqual %v4bool %33 %253
%255 = OpAll %bool %254
OpBranch %252
%252 = OpLabel
%256 = OpPhi %bool %false %244 %255 %251
OpSelectionMerge %260 None
OpBranchConditional %256 %258 %259
%258 = OpLabel
%261 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%262 = OpLoad %v4float %261
OpStore %257 %262
OpBranch %260
%259 = OpLabel
%263 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%264 = OpLoad %v4float %263
OpStore %257 %264
OpBranch %260
%260 = OpLabel
%265 = OpLoad %v4float %257
OpReturnValue %265
OpFunctionEnd
