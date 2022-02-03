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
OpDecorate %30 RelaxedPrecision
OpDecorate %expectedWT RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
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
%46 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%float_0_00100000005 = OpConstant %float 0.00100000005
%float_0_25 = OpConstant %float 0.25
%float_0_75 = OpConstant %float 0.75
%60 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
%72 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
%84 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%156 = OpConstantComposite %v2float %float_0_5 %float_0_5
%164 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%int_4 = OpConstant %int 4
%199 = OpConstantComposite %v2float %float_0 %float_0_5
%214 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%227 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
%240 = OpConstantComposite %v2float %float_1 %float_0_5
%248 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1
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
%260 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedBW %30
OpStore %expectedWT %33
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %37
%42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%44 = OpLoad %v4float %42
%45 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%36 = OpExtInst %v4float %1 FMix %41 %44 %45
%35 = OpExtInst %float %1 Distance %36 %46
%48 = OpFOrdLessThan %bool %35 %float_0_00100000005
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%54 = OpLoad %v4float %53
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%56 = OpLoad %v4float %55
%58 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%52 = OpExtInst %v4float %1 FMix %54 %56 %58
%51 = OpExtInst %float %1 Distance %52 %60
%61 = OpFOrdLessThan %bool %51 %float_0_00100000005
OpBranch %50
%50 = OpLabel
%62 = OpPhi %bool %false %25 %61 %49
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%68 = OpLoad %v4float %67
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%70 = OpLoad %v4float %69
%71 = OpCompositeConstruct %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%66 = OpExtInst %v4float %1 FMix %68 %70 %71
%65 = OpExtInst %float %1 Distance %66 %72
%73 = OpFOrdLessThan %bool %65 %float_0_00100000005
OpBranch %64
%64 = OpLabel
%74 = OpPhi %bool %false %50 %73 %63
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%80 = OpLoad %v4float %79
%81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%82 = OpLoad %v4float %81
%83 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%78 = OpExtInst %v4float %1 FMix %80 %82 %83
%77 = OpExtInst %float %1 Distance %78 %84
%85 = OpFOrdLessThan %bool %77 %float_0_00100000005
OpBranch %76
%76 = OpLabel
%86 = OpPhi %bool %false %64 %85 %75
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%92 = OpLoad %v4float %90
%93 = OpCompositeExtract %float %92 0
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%96 = OpLoad %v4float %94
%97 = OpCompositeExtract %float %96 0
%89 = OpExtInst %float %1 FMix %93 %97 %float_0_5
%98 = OpLoad %v4float %expectedBW
%99 = OpCompositeExtract %float %98 0
%100 = OpFOrdEqual %bool %89 %99
OpBranch %88
%88 = OpLabel
%101 = OpPhi %bool %false %76 %100 %87
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%106 = OpLoad %v4float %105
%107 = OpVectorShuffle %v2float %106 %106 0 1
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%109 = OpLoad %v4float %108
%110 = OpVectorShuffle %v2float %109 %109 0 1
%111 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%104 = OpExtInst %v2float %1 FMix %107 %110 %111
%112 = OpLoad %v4float %expectedBW
%113 = OpVectorShuffle %v2float %112 %112 0 1
%114 = OpFOrdEqual %v2bool %104 %113
%116 = OpAll %bool %114
OpBranch %103
%103 = OpLabel
%117 = OpPhi %bool %false %88 %116 %102
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%122 = OpLoad %v4float %121
%123 = OpVectorShuffle %v3float %122 %122 0 1 2
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%126 = OpLoad %v4float %125
%127 = OpVectorShuffle %v3float %126 %126 0 1 2
%128 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%120 = OpExtInst %v3float %1 FMix %123 %127 %128
%129 = OpLoad %v4float %expectedBW
%130 = OpVectorShuffle %v3float %129 %129 0 1 2
%131 = OpFOrdEqual %v3bool %120 %130
%133 = OpAll %bool %131
OpBranch %119
%119 = OpLabel
%134 = OpPhi %bool %false %103 %133 %118
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%139 = OpLoad %v4float %138
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%141 = OpLoad %v4float %140
%142 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%137 = OpExtInst %v4float %1 FMix %139 %141 %142
%143 = OpLoad %v4float %expectedBW
%144 = OpFOrdEqual %v4bool %137 %143
%146 = OpAll %bool %144
OpBranch %136
%136 = OpLabel
%147 = OpPhi %bool %false %119 %146 %135
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%150 = OpLoad %v4float %expectedBW
%151 = OpCompositeExtract %float %150 0
%152 = OpFOrdEqual %bool %float_0_5 %151
OpBranch %149
%149 = OpLabel
%153 = OpPhi %bool %false %136 %152 %148
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%157 = OpLoad %v4float %expectedBW
%158 = OpVectorShuffle %v2float %157 %157 0 1
%159 = OpFOrdEqual %v2bool %156 %158
%160 = OpAll %bool %159
OpBranch %155
%155 = OpLabel
%161 = OpPhi %bool %false %149 %160 %154
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%165 = OpLoad %v4float %expectedBW
%166 = OpVectorShuffle %v3float %165 %165 0 1 2
%167 = OpFOrdEqual %v3bool %164 %166
%168 = OpAll %bool %167
OpBranch %163
%163 = OpLabel
%169 = OpPhi %bool %false %155 %168 %162
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpLoad %v4float %expectedBW
%173 = OpFOrdEqual %v4bool %30 %172
%174 = OpAll %bool %173
OpBranch %171
%171 = OpLabel
%175 = OpPhi %bool %false %163 %174 %170
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%179 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%180 = OpLoad %v4float %179
%181 = OpCompositeExtract %float %180 0
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%184 = OpLoad %v4float %182
%185 = OpCompositeExtract %float %184 0
%178 = OpExtInst %float %1 FMix %181 %185 %float_0
%186 = OpLoad %v4float %expectedWT
%187 = OpCompositeExtract %float %186 0
%188 = OpFOrdEqual %bool %178 %187
OpBranch %177
%177 = OpLabel
%189 = OpPhi %bool %false %171 %188 %176
OpSelectionMerge %191 None
OpBranchConditional %189 %190 %191
%190 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%194 = OpLoad %v4float %193
%195 = OpVectorShuffle %v2float %194 %194 0 1
%196 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%197 = OpLoad %v4float %196
%198 = OpVectorShuffle %v2float %197 %197 0 1
%192 = OpExtInst %v2float %1 FMix %195 %198 %199
%200 = OpLoad %v4float %expectedWT
%201 = OpVectorShuffle %v2float %200 %200 0 1
%202 = OpFOrdEqual %v2bool %192 %201
%203 = OpAll %bool %202
OpBranch %191
%191 = OpLabel
%204 = OpPhi %bool %false %177 %203 %190
OpSelectionMerge %206 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%208 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%209 = OpLoad %v4float %208
%210 = OpVectorShuffle %v3float %209 %209 0 1 2
%211 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%212 = OpLoad %v4float %211
%213 = OpVectorShuffle %v3float %212 %212 0 1 2
%207 = OpExtInst %v3float %1 FMix %210 %213 %214
%215 = OpLoad %v4float %expectedWT
%216 = OpVectorShuffle %v3float %215 %215 0 1 2
%217 = OpFOrdEqual %v3bool %207 %216
%218 = OpAll %bool %217
OpBranch %206
%206 = OpLabel
%219 = OpPhi %bool %false %191 %218 %205
OpSelectionMerge %221 None
OpBranchConditional %219 %220 %221
%220 = OpLabel
%223 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%224 = OpLoad %v4float %223
%225 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%226 = OpLoad %v4float %225
%222 = OpExtInst %v4float %1 FMix %224 %226 %227
%228 = OpLoad %v4float %expectedWT
%229 = OpFOrdEqual %v4bool %222 %228
%230 = OpAll %bool %229
OpBranch %221
%221 = OpLabel
%231 = OpPhi %bool %false %206 %230 %220
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%234 = OpLoad %v4float %expectedWT
%235 = OpCompositeExtract %float %234 0
%236 = OpFOrdEqual %bool %float_1 %235
OpBranch %233
%233 = OpLabel
%237 = OpPhi %bool %false %221 %236 %232
OpSelectionMerge %239 None
OpBranchConditional %237 %238 %239
%238 = OpLabel
%241 = OpLoad %v4float %expectedWT
%242 = OpVectorShuffle %v2float %241 %241 0 1
%243 = OpFOrdEqual %v2bool %240 %242
%244 = OpAll %bool %243
OpBranch %239
%239 = OpLabel
%245 = OpPhi %bool %false %233 %244 %238
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%249 = OpLoad %v4float %expectedWT
%250 = OpVectorShuffle %v3float %249 %249 0 1 2
%251 = OpFOrdEqual %v3bool %248 %250
%252 = OpAll %bool %251
OpBranch %247
%247 = OpLabel
%253 = OpPhi %bool %false %239 %252 %246
OpSelectionMerge %255 None
OpBranchConditional %253 %254 %255
%254 = OpLabel
%256 = OpLoad %v4float %expectedWT
%257 = OpFOrdEqual %v4bool %33 %256
%258 = OpAll %bool %257
OpBranch %255
%255 = OpLabel
%259 = OpPhi %bool %false %247 %258 %254
OpSelectionMerge %263 None
OpBranchConditional %259 %261 %262
%261 = OpLabel
%264 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%265 = OpLoad %v4float %264
OpStore %260 %265
OpBranch %263
%262 = OpLabel
%266 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%267 = OpLoad %v4float %266
OpStore %260 %267
OpBranch %263
%263 = OpLabel
%268 = OpLoad %v4float %260
OpReturnValue %268
OpFunctionEnd
