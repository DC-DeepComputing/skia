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
OpDecorate %expectedA RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
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
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%51 = OpConstantComposite %v3float %float_0 %float_0 %float_0_84375
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%119 = OpConstantComposite %v2float %float_n1_25 %float_0
%136 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_75
%166 = OpConstantComposite %v2float %float_1 %float_0
%174 = OpConstantComposite %v3float %float_1 %float_0 %float_1
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
%239 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal %31
OpStore %expectedA %35
OpStore %expectedB %37
%39 = OpFOrdEqual %bool %float_0 %float_0
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpLoad %v4float %expectedA
%43 = OpVectorShuffle %v2float %42 %42 0 1
%44 = OpFOrdEqual %v2bool %19 %43
%46 = OpAll %bool %44
OpBranch %41
%41 = OpLabel
%47 = OpPhi %bool %false %25 %46 %40
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%52 = OpLoad %v4float %expectedA
%53 = OpVectorShuffle %v3float %52 %52 0 1 2
%54 = OpFOrdEqual %v3bool %51 %53
%56 = OpAll %bool %54
OpBranch %49
%49 = OpLabel
%57 = OpPhi %bool %false %41 %56 %48
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%60 = OpLoad %v4float %expectedA
%61 = OpFOrdEqual %v4bool %35 %60
%63 = OpAll %bool %61
OpBranch %59
%59 = OpLabel
%64 = OpPhi %bool %false %49 %63 %58
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%67 = OpLoad %v4float %expectedA
%68 = OpCompositeExtract %float %67 0
%69 = OpFOrdEqual %bool %float_0 %68
OpBranch %66
%66 = OpLabel
%70 = OpPhi %bool %false %59 %69 %65
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%73 = OpLoad %v4float %expectedA
%74 = OpVectorShuffle %v2float %73 %73 0 1
%75 = OpFOrdEqual %v2bool %19 %74
%76 = OpAll %bool %75
OpBranch %72
%72 = OpLabel
%77 = OpPhi %bool %false %66 %76 %71
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpLoad %v4float %expectedA
%81 = OpVectorShuffle %v3float %80 %80 0 1 2
%82 = OpFOrdEqual %v3bool %51 %81
%83 = OpAll %bool %82
OpBranch %79
%79 = OpLabel
%84 = OpPhi %bool %false %72 %83 %78
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %v4float %expectedA
%88 = OpFOrdEqual %v4bool %35 %87
%89 = OpAll %bool %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %79 %89 %85
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%98 = OpLoad %v4float %94
%99 = OpCompositeExtract %float %98 1
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%102 = OpLoad %v4float %100
%103 = OpCompositeExtract %float %102 1
%93 = OpExtInst %float %1 SmoothStep %99 %103 %float_n1_25
%104 = OpLoad %v4float %expectedA
%105 = OpCompositeExtract %float %104 0
%106 = OpFOrdEqual %bool %93 %105
OpBranch %92
%92 = OpLabel
%107 = OpPhi %bool %false %86 %106 %91
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%112 = OpLoad %v4float %111
%113 = OpCompositeExtract %float %112 1
%114 = OpCompositeConstruct %v2float %113 %113
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%116 = OpLoad %v4float %115
%117 = OpCompositeExtract %float %116 1
%118 = OpCompositeConstruct %v2float %117 %117
%110 = OpExtInst %v2float %1 SmoothStep %114 %118 %119
%120 = OpLoad %v4float %expectedA
%121 = OpVectorShuffle %v2float %120 %120 0 1
%122 = OpFOrdEqual %v2bool %110 %121
%123 = OpAll %bool %122
OpBranch %109
%109 = OpLabel
%124 = OpPhi %bool %false %92 %123 %108
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%129 = OpLoad %v4float %128
%130 = OpCompositeExtract %float %129 1
%131 = OpCompositeConstruct %v3float %130 %130 %130
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%133 = OpLoad %v4float %132
%134 = OpCompositeExtract %float %133 1
%135 = OpCompositeConstruct %v3float %134 %134 %134
%127 = OpExtInst %v3float %1 SmoothStep %131 %135 %136
%137 = OpLoad %v4float %expectedA
%138 = OpVectorShuffle %v3float %137 %137 0 1 2
%139 = OpFOrdEqual %v3bool %127 %138
%140 = OpAll %bool %139
OpBranch %126
%126 = OpLabel
%141 = OpPhi %bool %false %109 %140 %125
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%146 = OpLoad %v4float %145
%147 = OpCompositeExtract %float %146 1
%148 = OpCompositeConstruct %v4float %147 %147 %147 %147
%149 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%150 = OpLoad %v4float %149
%151 = OpCompositeExtract %float %150 1
%152 = OpCompositeConstruct %v4float %151 %151 %151 %151
%153 = OpLoad %v4float %constVal
%144 = OpExtInst %v4float %1 SmoothStep %148 %152 %153
%154 = OpLoad %v4float %expectedA
%155 = OpFOrdEqual %v4bool %144 %154
%156 = OpAll %bool %155
OpBranch %143
%143 = OpLabel
%157 = OpPhi %bool %false %126 %156 %142
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%160 = OpLoad %v4float %expectedB
%161 = OpCompositeExtract %float %160 0
%162 = OpFOrdEqual %bool %float_1 %161
OpBranch %159
%159 = OpLabel
%163 = OpPhi %bool %false %143 %162 %158
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%167 = OpLoad %v4float %expectedB
%168 = OpVectorShuffle %v2float %167 %167 0 1
%169 = OpFOrdEqual %v2bool %166 %168
%170 = OpAll %bool %169
OpBranch %165
%165 = OpLabel
%171 = OpPhi %bool %false %159 %170 %164
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%175 = OpLoad %v4float %expectedB
%176 = OpVectorShuffle %v3float %175 %175 0 1 2
%177 = OpFOrdEqual %v3bool %174 %176
%178 = OpAll %bool %177
OpBranch %173
%173 = OpLabel
%179 = OpPhi %bool %false %165 %178 %172
OpSelectionMerge %181 None
OpBranchConditional %179 %180 %181
%180 = OpLabel
%182 = OpLoad %v4float %expectedB
%183 = OpFOrdEqual %v4bool %37 %182
%184 = OpAll %bool %183
OpBranch %181
%181 = OpLabel
%185 = OpPhi %bool %false %173 %184 %180
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%189 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%190 = OpLoad %v4float %189
%191 = OpCompositeExtract %float %190 0
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%193 = OpLoad %v4float %192
%194 = OpCompositeExtract %float %193 0
%188 = OpExtInst %float %1 SmoothStep %191 %194 %float_n1_25
%195 = OpLoad %v4float %expectedB
%196 = OpCompositeExtract %float %195 0
%197 = OpFOrdEqual %bool %188 %196
OpBranch %187
%187 = OpLabel
%198 = OpPhi %bool %false %181 %197 %186
OpSelectionMerge %200 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%202 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%203 = OpLoad %v4float %202
%204 = OpVectorShuffle %v2float %203 %203 0 1
%205 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%206 = OpLoad %v4float %205
%207 = OpVectorShuffle %v2float %206 %206 0 1
%201 = OpExtInst %v2float %1 SmoothStep %204 %207 %119
%208 = OpLoad %v4float %expectedB
%209 = OpVectorShuffle %v2float %208 %208 0 1
%210 = OpFOrdEqual %v2bool %201 %209
%211 = OpAll %bool %210
OpBranch %200
%200 = OpLabel
%212 = OpPhi %bool %false %187 %211 %199
OpSelectionMerge %214 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
%216 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%217 = OpLoad %v4float %216
%218 = OpVectorShuffle %v3float %217 %217 0 1 2
%219 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%220 = OpLoad %v4float %219
%221 = OpVectorShuffle %v3float %220 %220 0 1 2
%215 = OpExtInst %v3float %1 SmoothStep %218 %221 %136
%222 = OpLoad %v4float %expectedB
%223 = OpVectorShuffle %v3float %222 %222 0 1 2
%224 = OpFOrdEqual %v3bool %215 %223
%225 = OpAll %bool %224
OpBranch %214
%214 = OpLabel
%226 = OpPhi %bool %false %200 %225 %213
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%230 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%231 = OpLoad %v4float %230
%232 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%233 = OpLoad %v4float %232
%234 = OpLoad %v4float %constVal
%229 = OpExtInst %v4float %1 SmoothStep %231 %233 %234
%235 = OpLoad %v4float %expectedB
%236 = OpFOrdEqual %v4bool %229 %235
%237 = OpAll %bool %236
OpBranch %228
%228 = OpLabel
%238 = OpPhi %bool %false %214 %237 %227
OpSelectionMerge %242 None
OpBranchConditional %238 %240 %241
%240 = OpLabel
%243 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%244 = OpLoad %v4float %243
OpStore %239 %244
OpBranch %242
%241 = OpLabel
%245 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%246 = OpLoad %v4float %245
OpStore %239 %246
OpBranch %242
%242 = OpLabel
%247 = OpLoad %v4float %239
OpReturnValue %247
OpFunctionEnd
