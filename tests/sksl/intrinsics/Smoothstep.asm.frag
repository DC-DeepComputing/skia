OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %constVal "constVal"
OpName %expectedA "expectedA"
OpName %expectedB "expectedB"
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
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %constVal RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %expectedA RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
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
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%33 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%float_0_84375 = OpConstant %float 0.84375
%float_1 = OpConstant %float 1
%37 = OpConstantComposite %v4float %float_0 %float_0 %float_0_84375 %float_1
%39 = OpConstantComposite %v4float %float_1 %float_0 %float_1 %float_1
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%55 = OpConstantComposite %v3float %float_0 %float_0 %float_0_84375
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%123 = OpConstantComposite %v2float %float_n1_25 %float_0
%140 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_75
%170 = OpConstantComposite %v2float %float_1 %float_0
%178 = OpConstantComposite %v3float %float_1 %float_0 %float_1
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
%constVal = OpVariable %_ptr_Function_v4float Function
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%243 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal %33
OpStore %expectedA %37
OpStore %expectedB %39
%41 = OpLoad %v4float %expectedA
%42 = OpCompositeExtract %float %41 0
%43 = OpFOrdEqual %bool %float_0 %42
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%46 = OpLoad %v4float %expectedA
%47 = OpVectorShuffle %v2float %46 %46 0 1
%48 = OpFOrdEqual %v2bool %21 %47
%50 = OpAll %bool %48
OpBranch %45
%45 = OpLabel
%51 = OpPhi %bool %false %27 %50 %44
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%56 = OpLoad %v4float %expectedA
%57 = OpVectorShuffle %v3float %56 %56 0 1 2
%58 = OpFOrdEqual %v3bool %55 %57
%60 = OpAll %bool %58
OpBranch %53
%53 = OpLabel
%61 = OpPhi %bool %false %45 %60 %52
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpLoad %v4float %expectedA
%65 = OpFOrdEqual %v4bool %37 %64
%67 = OpAll %bool %65
OpBranch %63
%63 = OpLabel
%68 = OpPhi %bool %false %53 %67 %62
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpLoad %v4float %expectedA
%72 = OpCompositeExtract %float %71 0
%73 = OpFOrdEqual %bool %float_0 %72
OpBranch %70
%70 = OpLabel
%74 = OpPhi %bool %false %63 %73 %69
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %v4float %expectedA
%78 = OpVectorShuffle %v2float %77 %77 0 1
%79 = OpFOrdEqual %v2bool %21 %78
%80 = OpAll %bool %79
OpBranch %76
%76 = OpLabel
%81 = OpPhi %bool %false %70 %80 %75
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpLoad %v4float %expectedA
%85 = OpVectorShuffle %v3float %84 %84 0 1 2
%86 = OpFOrdEqual %v3bool %55 %85
%87 = OpAll %bool %86
OpBranch %83
%83 = OpLabel
%88 = OpPhi %bool %false %76 %87 %82
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpLoad %v4float %expectedA
%92 = OpFOrdEqual %v4bool %37 %91
%93 = OpAll %bool %92
OpBranch %90
%90 = OpLabel
%94 = OpPhi %bool %false %83 %93 %89
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%102 = OpLoad %v4float %98
%103 = OpCompositeExtract %float %102 1
%104 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%106 = OpLoad %v4float %104
%107 = OpCompositeExtract %float %106 1
%97 = OpExtInst %float %1 SmoothStep %103 %107 %float_n1_25
%108 = OpLoad %v4float %expectedA
%109 = OpCompositeExtract %float %108 0
%110 = OpFOrdEqual %bool %97 %109
OpBranch %96
%96 = OpLabel
%111 = OpPhi %bool %false %90 %110 %95
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%116 = OpLoad %v4float %115
%117 = OpCompositeExtract %float %116 1
%118 = OpCompositeConstruct %v2float %117 %117
%119 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%120 = OpLoad %v4float %119
%121 = OpCompositeExtract %float %120 1
%122 = OpCompositeConstruct %v2float %121 %121
%114 = OpExtInst %v2float %1 SmoothStep %118 %122 %123
%124 = OpLoad %v4float %expectedA
%125 = OpVectorShuffle %v2float %124 %124 0 1
%126 = OpFOrdEqual %v2bool %114 %125
%127 = OpAll %bool %126
OpBranch %113
%113 = OpLabel
%128 = OpPhi %bool %false %96 %127 %112
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%133 = OpLoad %v4float %132
%134 = OpCompositeExtract %float %133 1
%135 = OpCompositeConstruct %v3float %134 %134 %134
%136 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%137 = OpLoad %v4float %136
%138 = OpCompositeExtract %float %137 1
%139 = OpCompositeConstruct %v3float %138 %138 %138
%131 = OpExtInst %v3float %1 SmoothStep %135 %139 %140
%141 = OpLoad %v4float %expectedA
%142 = OpVectorShuffle %v3float %141 %141 0 1 2
%143 = OpFOrdEqual %v3bool %131 %142
%144 = OpAll %bool %143
OpBranch %130
%130 = OpLabel
%145 = OpPhi %bool %false %113 %144 %129
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%149 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%150 = OpLoad %v4float %149
%151 = OpCompositeExtract %float %150 1
%152 = OpCompositeConstruct %v4float %151 %151 %151 %151
%153 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%154 = OpLoad %v4float %153
%155 = OpCompositeExtract %float %154 1
%156 = OpCompositeConstruct %v4float %155 %155 %155 %155
%157 = OpLoad %v4float %constVal
%148 = OpExtInst %v4float %1 SmoothStep %152 %156 %157
%158 = OpLoad %v4float %expectedA
%159 = OpFOrdEqual %v4bool %148 %158
%160 = OpAll %bool %159
OpBranch %147
%147 = OpLabel
%161 = OpPhi %bool %false %130 %160 %146
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpLoad %v4float %expectedB
%165 = OpCompositeExtract %float %164 0
%166 = OpFOrdEqual %bool %float_1 %165
OpBranch %163
%163 = OpLabel
%167 = OpPhi %bool %false %147 %166 %162
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%171 = OpLoad %v4float %expectedB
%172 = OpVectorShuffle %v2float %171 %171 0 1
%173 = OpFOrdEqual %v2bool %170 %172
%174 = OpAll %bool %173
OpBranch %169
%169 = OpLabel
%175 = OpPhi %bool %false %163 %174 %168
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%179 = OpLoad %v4float %expectedB
%180 = OpVectorShuffle %v3float %179 %179 0 1 2
%181 = OpFOrdEqual %v3bool %178 %180
%182 = OpAll %bool %181
OpBranch %177
%177 = OpLabel
%183 = OpPhi %bool %false %169 %182 %176
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpLoad %v4float %expectedB
%187 = OpFOrdEqual %v4bool %39 %186
%188 = OpAll %bool %187
OpBranch %185
%185 = OpLabel
%189 = OpPhi %bool %false %177 %188 %184
OpSelectionMerge %191 None
OpBranchConditional %189 %190 %191
%190 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%194 = OpLoad %v4float %193
%195 = OpCompositeExtract %float %194 0
%196 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%197 = OpLoad %v4float %196
%198 = OpCompositeExtract %float %197 0
%192 = OpExtInst %float %1 SmoothStep %195 %198 %float_n1_25
%199 = OpLoad %v4float %expectedB
%200 = OpCompositeExtract %float %199 0
%201 = OpFOrdEqual %bool %192 %200
OpBranch %191
%191 = OpLabel
%202 = OpPhi %bool %false %185 %201 %190
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%206 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%207 = OpLoad %v4float %206
%208 = OpVectorShuffle %v2float %207 %207 0 1
%209 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%210 = OpLoad %v4float %209
%211 = OpVectorShuffle %v2float %210 %210 0 1
%205 = OpExtInst %v2float %1 SmoothStep %208 %211 %123
%212 = OpLoad %v4float %expectedB
%213 = OpVectorShuffle %v2float %212 %212 0 1
%214 = OpFOrdEqual %v2bool %205 %213
%215 = OpAll %bool %214
OpBranch %204
%204 = OpLabel
%216 = OpPhi %bool %false %191 %215 %203
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%220 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%221 = OpLoad %v4float %220
%222 = OpVectorShuffle %v3float %221 %221 0 1 2
%223 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%224 = OpLoad %v4float %223
%225 = OpVectorShuffle %v3float %224 %224 0 1 2
%219 = OpExtInst %v3float %1 SmoothStep %222 %225 %140
%226 = OpLoad %v4float %expectedB
%227 = OpVectorShuffle %v3float %226 %226 0 1 2
%228 = OpFOrdEqual %v3bool %219 %227
%229 = OpAll %bool %228
OpBranch %218
%218 = OpLabel
%230 = OpPhi %bool %false %204 %229 %217
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%234 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%235 = OpLoad %v4float %234
%236 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%237 = OpLoad %v4float %236
%238 = OpLoad %v4float %constVal
%233 = OpExtInst %v4float %1 SmoothStep %235 %237 %238
%239 = OpLoad %v4float %expectedB
%240 = OpFOrdEqual %v4bool %233 %239
%241 = OpAll %bool %240
OpBranch %232
%232 = OpLabel
%242 = OpPhi %bool %false %218 %241 %231
OpSelectionMerge %246 None
OpBranchConditional %242 %244 %245
%244 = OpLabel
%247 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%248 = OpLoad %v4float %247
OpStore %243 %248
OpBranch %246
%245 = OpLabel
%249 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%250 = OpLoad %v4float %249
OpStore %243 %250
OpBranch %246
%246 = OpLabel
%251 = OpLoad %v4float %243
OpReturnValue %251
OpFunctionEnd
