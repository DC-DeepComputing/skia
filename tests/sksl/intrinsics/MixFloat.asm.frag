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
OpMemberName %_UniformBuffer 2 "colorBlack"
OpMemberName %_UniformBuffer 3 "colorWhite"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedBW "expectedBW"
OpName %expectedWT "expectedWT"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %expectedBW RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %expectedWT RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_5 = OpConstant %float 0.5
%float_1 = OpConstant %float 1
%32 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
%float_2_25 = OpConstant %float 2.25
%35 = OpConstantComposite %v4float %float_1 %float_0_5 %float_1 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%47 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%v4bool = OpTypeVector %bool 4
%float_0_25 = OpConstant %float 0.25
%float_0_75 = OpConstant %float 0.75
%61 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
%73 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
%85 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%157 = OpConstantComposite %v2float %float_0_5 %float_0_5
%165 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%int_4 = OpConstant %int 4
%200 = OpConstantComposite %v2float %float_0 %float_0_5
%215 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%228 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
%241 = OpConstantComposite %v2float %float_1 %float_0_5
%249 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1
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
%expectedBW = OpVariable %_ptr_Function_v4float Function
%expectedWT = OpVariable %_ptr_Function_v4float Function
%261 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedBW %32
OpStore %expectedWT %35
%38 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%42 = OpLoad %v4float %38
%43 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%45 = OpLoad %v4float %43
%46 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%37 = OpExtInst %v4float %1 FMix %42 %45 %46
%48 = OpFOrdEqual %v4bool %37 %47
%50 = OpAll %bool %48
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%54 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%55 = OpLoad %v4float %54
%56 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%57 = OpLoad %v4float %56
%59 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%53 = OpExtInst %v4float %1 FMix %55 %57 %59
%62 = OpFOrdEqual %v4bool %53 %61
%63 = OpAll %bool %62
OpBranch %52
%52 = OpLabel
%64 = OpPhi %bool %false %27 %63 %51
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%69 = OpLoad %v4float %68
%70 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%71 = OpLoad %v4float %70
%72 = OpCompositeConstruct %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%67 = OpExtInst %v4float %1 FMix %69 %71 %72
%74 = OpFOrdEqual %v4bool %67 %73
%75 = OpAll %bool %74
OpBranch %66
%66 = OpLabel
%76 = OpPhi %bool %false %52 %75 %65
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%81 = OpLoad %v4float %80
%82 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%83 = OpLoad %v4float %82
%84 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%79 = OpExtInst %v4float %1 FMix %81 %83 %84
%86 = OpFOrdEqual %v4bool %79 %85
%87 = OpAll %bool %86
OpBranch %78
%78 = OpLabel
%88 = OpPhi %bool %false %66 %87 %77
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%94 = OpLoad %v4float %92
%95 = OpCompositeExtract %float %94 0
%96 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%98 = OpLoad %v4float %96
%99 = OpCompositeExtract %float %98 0
%91 = OpExtInst %float %1 FMix %95 %99 %float_0_5
%100 = OpLoad %v4float %expectedBW
%101 = OpCompositeExtract %float %100 0
%102 = OpFOrdEqual %bool %91 %101
OpBranch %90
%90 = OpLabel
%103 = OpPhi %bool %false %78 %102 %89
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%108 = OpLoad %v4float %107
%109 = OpVectorShuffle %v2float %108 %108 0 1
%110 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%111 = OpLoad %v4float %110
%112 = OpVectorShuffle %v2float %111 %111 0 1
%113 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%106 = OpExtInst %v2float %1 FMix %109 %112 %113
%114 = OpLoad %v4float %expectedBW
%115 = OpVectorShuffle %v2float %114 %114 0 1
%116 = OpFOrdEqual %v2bool %106 %115
%118 = OpAll %bool %116
OpBranch %105
%105 = OpLabel
%119 = OpPhi %bool %false %90 %118 %104
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%124 = OpLoad %v4float %123
%125 = OpVectorShuffle %v3float %124 %124 0 1 2
%127 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%128 = OpLoad %v4float %127
%129 = OpVectorShuffle %v3float %128 %128 0 1 2
%130 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%122 = OpExtInst %v3float %1 FMix %125 %129 %130
%131 = OpLoad %v4float %expectedBW
%132 = OpVectorShuffle %v3float %131 %131 0 1 2
%133 = OpFOrdEqual %v3bool %122 %132
%135 = OpAll %bool %133
OpBranch %121
%121 = OpLabel
%136 = OpPhi %bool %false %105 %135 %120
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%141 = OpLoad %v4float %140
%142 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%143 = OpLoad %v4float %142
%144 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%139 = OpExtInst %v4float %1 FMix %141 %143 %144
%145 = OpLoad %v4float %expectedBW
%146 = OpFOrdEqual %v4bool %139 %145
%147 = OpAll %bool %146
OpBranch %138
%138 = OpLabel
%148 = OpPhi %bool %false %121 %147 %137
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%151 = OpLoad %v4float %expectedBW
%152 = OpCompositeExtract %float %151 0
%153 = OpFOrdEqual %bool %float_0_5 %152
OpBranch %150
%150 = OpLabel
%154 = OpPhi %bool %false %138 %153 %149
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%158 = OpLoad %v4float %expectedBW
%159 = OpVectorShuffle %v2float %158 %158 0 1
%160 = OpFOrdEqual %v2bool %157 %159
%161 = OpAll %bool %160
OpBranch %156
%156 = OpLabel
%162 = OpPhi %bool %false %150 %161 %155
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%166 = OpLoad %v4float %expectedBW
%167 = OpVectorShuffle %v3float %166 %166 0 1 2
%168 = OpFOrdEqual %v3bool %165 %167
%169 = OpAll %bool %168
OpBranch %164
%164 = OpLabel
%170 = OpPhi %bool %false %156 %169 %163
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpLoad %v4float %expectedBW
%174 = OpFOrdEqual %v4bool %32 %173
%175 = OpAll %bool %174
OpBranch %172
%172 = OpLabel
%176 = OpPhi %bool %false %164 %175 %171
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%180 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%181 = OpLoad %v4float %180
%182 = OpCompositeExtract %float %181 0
%183 = OpAccessChain %_ptr_Uniform_v4float %12 %int_4
%185 = OpLoad %v4float %183
%186 = OpCompositeExtract %float %185 0
%179 = OpExtInst %float %1 FMix %182 %186 %float_0
%187 = OpLoad %v4float %expectedWT
%188 = OpCompositeExtract %float %187 0
%189 = OpFOrdEqual %bool %179 %188
OpBranch %178
%178 = OpLabel
%190 = OpPhi %bool %false %172 %189 %177
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%194 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%195 = OpLoad %v4float %194
%196 = OpVectorShuffle %v2float %195 %195 0 1
%197 = OpAccessChain %_ptr_Uniform_v4float %12 %int_4
%198 = OpLoad %v4float %197
%199 = OpVectorShuffle %v2float %198 %198 0 1
%193 = OpExtInst %v2float %1 FMix %196 %199 %200
%201 = OpLoad %v4float %expectedWT
%202 = OpVectorShuffle %v2float %201 %201 0 1
%203 = OpFOrdEqual %v2bool %193 %202
%204 = OpAll %bool %203
OpBranch %192
%192 = OpLabel
%205 = OpPhi %bool %false %178 %204 %191
OpSelectionMerge %207 None
OpBranchConditional %205 %206 %207
%206 = OpLabel
%209 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%210 = OpLoad %v4float %209
%211 = OpVectorShuffle %v3float %210 %210 0 1 2
%212 = OpAccessChain %_ptr_Uniform_v4float %12 %int_4
%213 = OpLoad %v4float %212
%214 = OpVectorShuffle %v3float %213 %213 0 1 2
%208 = OpExtInst %v3float %1 FMix %211 %214 %215
%216 = OpLoad %v4float %expectedWT
%217 = OpVectorShuffle %v3float %216 %216 0 1 2
%218 = OpFOrdEqual %v3bool %208 %217
%219 = OpAll %bool %218
OpBranch %207
%207 = OpLabel
%220 = OpPhi %bool %false %192 %219 %206
OpSelectionMerge %222 None
OpBranchConditional %220 %221 %222
%221 = OpLabel
%224 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%225 = OpLoad %v4float %224
%226 = OpAccessChain %_ptr_Uniform_v4float %12 %int_4
%227 = OpLoad %v4float %226
%223 = OpExtInst %v4float %1 FMix %225 %227 %228
%229 = OpLoad %v4float %expectedWT
%230 = OpFOrdEqual %v4bool %223 %229
%231 = OpAll %bool %230
OpBranch %222
%222 = OpLabel
%232 = OpPhi %bool %false %207 %231 %221
OpSelectionMerge %234 None
OpBranchConditional %232 %233 %234
%233 = OpLabel
%235 = OpLoad %v4float %expectedWT
%236 = OpCompositeExtract %float %235 0
%237 = OpFOrdEqual %bool %float_1 %236
OpBranch %234
%234 = OpLabel
%238 = OpPhi %bool %false %222 %237 %233
OpSelectionMerge %240 None
OpBranchConditional %238 %239 %240
%239 = OpLabel
%242 = OpLoad %v4float %expectedWT
%243 = OpVectorShuffle %v2float %242 %242 0 1
%244 = OpFOrdEqual %v2bool %241 %243
%245 = OpAll %bool %244
OpBranch %240
%240 = OpLabel
%246 = OpPhi %bool %false %234 %245 %239
OpSelectionMerge %248 None
OpBranchConditional %246 %247 %248
%247 = OpLabel
%250 = OpLoad %v4float %expectedWT
%251 = OpVectorShuffle %v3float %250 %250 0 1 2
%252 = OpFOrdEqual %v3bool %249 %251
%253 = OpAll %bool %252
OpBranch %248
%248 = OpLabel
%254 = OpPhi %bool %false %240 %253 %247
OpSelectionMerge %256 None
OpBranchConditional %254 %255 %256
%255 = OpLabel
%257 = OpLoad %v4float %expectedWT
%258 = OpFOrdEqual %v4bool %35 %257
%259 = OpAll %bool %258
OpBranch %256
%256 = OpLabel
%260 = OpPhi %bool %false %248 %259 %255
OpSelectionMerge %264 None
OpBranchConditional %260 %262 %263
%262 = OpLabel
%265 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%266 = OpLoad %v4float %265
OpStore %261 %266
OpBranch %264
%263 = OpLabel
%267 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%268 = OpLoad %v4float %267
OpStore %261 %268
OpBranch %264
%264 = OpLabel
%269 = OpLoad %v4float %261
OpReturnValue %269
OpFunctionEnd
