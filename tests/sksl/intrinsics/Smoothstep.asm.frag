OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
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
OpDecorate %constVal RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %expectedA RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%float_0_84375 = OpConstant %float 0.84375
%float_1 = OpConstant %float 1
%35 = OpConstantComposite %v4float %float_0 %float_0 %float_0_84375 %float_1
%37 = OpConstantComposite %v4float %float_1 %float_0 %float_1 %float_1
%false = OpConstantFalse %bool
%44 = OpConstantComposite %v2float %float_0 %float_0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%54 = OpConstantComposite %v3float %float_0 %float_0 %float_0_84375
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%122 = OpConstantComposite %v2float %float_n1_25 %float_0
%139 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_75
%169 = OpConstantComposite %v2float %float_1 %float_0
%177 = OpConstantComposite %v3float %float_1 %float_0 %float_1
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
%constVal = OpVariable %_ptr_Function_v4float Function
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%242 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal %31
OpStore %expectedA %35
OpStore %expectedB %37
%39 = OpLoad %v4float %expectedA
%40 = OpCompositeExtract %float %39 0
%41 = OpFOrdEqual %bool %float_0 %40
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%45 = OpLoad %v4float %expectedA
%46 = OpVectorShuffle %v2float %45 %45 0 1
%47 = OpFOrdEqual %v2bool %44 %46
%49 = OpAll %bool %47
OpBranch %43
%43 = OpLabel
%50 = OpPhi %bool %false %25 %49 %42
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%55 = OpLoad %v4float %expectedA
%56 = OpVectorShuffle %v3float %55 %55 0 1 2
%57 = OpFOrdEqual %v3bool %54 %56
%59 = OpAll %bool %57
OpBranch %52
%52 = OpLabel
%60 = OpPhi %bool %false %43 %59 %51
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%63 = OpLoad %v4float %expectedA
%64 = OpFOrdEqual %v4bool %35 %63
%66 = OpAll %bool %64
OpBranch %62
%62 = OpLabel
%67 = OpPhi %bool %false %52 %66 %61
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %v4float %expectedA
%71 = OpCompositeExtract %float %70 0
%72 = OpFOrdEqual %bool %float_0 %71
OpBranch %69
%69 = OpLabel
%73 = OpPhi %bool %false %62 %72 %68
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %v4float %expectedA
%77 = OpVectorShuffle %v2float %76 %76 0 1
%78 = OpFOrdEqual %v2bool %44 %77
%79 = OpAll %bool %78
OpBranch %75
%75 = OpLabel
%80 = OpPhi %bool %false %69 %79 %74
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpLoad %v4float %expectedA
%84 = OpVectorShuffle %v3float %83 %83 0 1 2
%85 = OpFOrdEqual %v3bool %54 %84
%86 = OpAll %bool %85
OpBranch %82
%82 = OpLabel
%87 = OpPhi %bool %false %75 %86 %81
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %v4float %expectedA
%91 = OpFOrdEqual %v4bool %35 %90
%92 = OpAll %bool %91
OpBranch %89
%89 = OpLabel
%93 = OpPhi %bool %false %82 %92 %88
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%101 = OpLoad %v4float %97
%102 = OpCompositeExtract %float %101 1
%103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%105 = OpLoad %v4float %103
%106 = OpCompositeExtract %float %105 1
%96 = OpExtInst %float %1 SmoothStep %102 %106 %float_n1_25
%107 = OpLoad %v4float %expectedA
%108 = OpCompositeExtract %float %107 0
%109 = OpFOrdEqual %bool %96 %108
OpBranch %95
%95 = OpLabel
%110 = OpPhi %bool %false %89 %109 %94
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%115 = OpLoad %v4float %114
%116 = OpCompositeExtract %float %115 1
%117 = OpCompositeConstruct %v2float %116 %116
%118 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%119 = OpLoad %v4float %118
%120 = OpCompositeExtract %float %119 1
%121 = OpCompositeConstruct %v2float %120 %120
%113 = OpExtInst %v2float %1 SmoothStep %117 %121 %122
%123 = OpLoad %v4float %expectedA
%124 = OpVectorShuffle %v2float %123 %123 0 1
%125 = OpFOrdEqual %v2bool %113 %124
%126 = OpAll %bool %125
OpBranch %112
%112 = OpLabel
%127 = OpPhi %bool %false %95 %126 %111
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%132 = OpLoad %v4float %131
%133 = OpCompositeExtract %float %132 1
%134 = OpCompositeConstruct %v3float %133 %133 %133
%135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%136 = OpLoad %v4float %135
%137 = OpCompositeExtract %float %136 1
%138 = OpCompositeConstruct %v3float %137 %137 %137
%130 = OpExtInst %v3float %1 SmoothStep %134 %138 %139
%140 = OpLoad %v4float %expectedA
%141 = OpVectorShuffle %v3float %140 %140 0 1 2
%142 = OpFOrdEqual %v3bool %130 %141
%143 = OpAll %bool %142
OpBranch %129
%129 = OpLabel
%144 = OpPhi %bool %false %112 %143 %128
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%149 = OpLoad %v4float %148
%150 = OpCompositeExtract %float %149 1
%151 = OpCompositeConstruct %v4float %150 %150 %150 %150
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%153 = OpLoad %v4float %152
%154 = OpCompositeExtract %float %153 1
%155 = OpCompositeConstruct %v4float %154 %154 %154 %154
%156 = OpLoad %v4float %constVal
%147 = OpExtInst %v4float %1 SmoothStep %151 %155 %156
%157 = OpLoad %v4float %expectedA
%158 = OpFOrdEqual %v4bool %147 %157
%159 = OpAll %bool %158
OpBranch %146
%146 = OpLabel
%160 = OpPhi %bool %false %129 %159 %145
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%163 = OpLoad %v4float %expectedB
%164 = OpCompositeExtract %float %163 0
%165 = OpFOrdEqual %bool %float_1 %164
OpBranch %162
%162 = OpLabel
%166 = OpPhi %bool %false %146 %165 %161
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%170 = OpLoad %v4float %expectedB
%171 = OpVectorShuffle %v2float %170 %170 0 1
%172 = OpFOrdEqual %v2bool %169 %171
%173 = OpAll %bool %172
OpBranch %168
%168 = OpLabel
%174 = OpPhi %bool %false %162 %173 %167
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%178 = OpLoad %v4float %expectedB
%179 = OpVectorShuffle %v3float %178 %178 0 1 2
%180 = OpFOrdEqual %v3bool %177 %179
%181 = OpAll %bool %180
OpBranch %176
%176 = OpLabel
%182 = OpPhi %bool %false %168 %181 %175
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpLoad %v4float %expectedB
%186 = OpFOrdEqual %v4bool %37 %185
%187 = OpAll %bool %186
OpBranch %184
%184 = OpLabel
%188 = OpPhi %bool %false %176 %187 %183
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%193 = OpLoad %v4float %192
%194 = OpCompositeExtract %float %193 0
%195 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%196 = OpLoad %v4float %195
%197 = OpCompositeExtract %float %196 0
%191 = OpExtInst %float %1 SmoothStep %194 %197 %float_n1_25
%198 = OpLoad %v4float %expectedB
%199 = OpCompositeExtract %float %198 0
%200 = OpFOrdEqual %bool %191 %199
OpBranch %190
%190 = OpLabel
%201 = OpPhi %bool %false %184 %200 %189
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
%205 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%206 = OpLoad %v4float %205
%207 = OpVectorShuffle %v2float %206 %206 0 1
%208 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%209 = OpLoad %v4float %208
%210 = OpVectorShuffle %v2float %209 %209 0 1
%204 = OpExtInst %v2float %1 SmoothStep %207 %210 %122
%211 = OpLoad %v4float %expectedB
%212 = OpVectorShuffle %v2float %211 %211 0 1
%213 = OpFOrdEqual %v2bool %204 %212
%214 = OpAll %bool %213
OpBranch %203
%203 = OpLabel
%215 = OpPhi %bool %false %190 %214 %202
OpSelectionMerge %217 None
OpBranchConditional %215 %216 %217
%216 = OpLabel
%219 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%220 = OpLoad %v4float %219
%221 = OpVectorShuffle %v3float %220 %220 0 1 2
%222 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%223 = OpLoad %v4float %222
%224 = OpVectorShuffle %v3float %223 %223 0 1 2
%218 = OpExtInst %v3float %1 SmoothStep %221 %224 %139
%225 = OpLoad %v4float %expectedB
%226 = OpVectorShuffle %v3float %225 %225 0 1 2
%227 = OpFOrdEqual %v3bool %218 %226
%228 = OpAll %bool %227
OpBranch %217
%217 = OpLabel
%229 = OpPhi %bool %false %203 %228 %216
OpSelectionMerge %231 None
OpBranchConditional %229 %230 %231
%230 = OpLabel
%233 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%234 = OpLoad %v4float %233
%235 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%236 = OpLoad %v4float %235
%237 = OpLoad %v4float %constVal
%232 = OpExtInst %v4float %1 SmoothStep %234 %236 %237
%238 = OpLoad %v4float %expectedB
%239 = OpFOrdEqual %v4bool %232 %238
%240 = OpAll %bool %239
OpBranch %231
%231 = OpLabel
%241 = OpPhi %bool %false %217 %240 %230
OpSelectionMerge %245 None
OpBranchConditional %241 %243 %244
%243 = OpLabel
%246 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%247 = OpLoad %v4float %246
OpStore %242 %247
OpBranch %245
%244 = OpLabel
%248 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%249 = OpLoad %v4float %248
OpStore %242 %249
OpBranch %245
%245 = OpLabel
%250 = OpLoad %v4float %242
OpReturnValue %250
OpFunctionEnd
