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
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
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
%108 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%124 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%137 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%int_4 = OpConstant %int 4
%183 = OpConstantComposite %v2float %float_0 %float_0_5
%197 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%209 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
%220 = OpConstantComposite %v2float %float_1 %float_0_5
%227 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1
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
%237 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedBW %30
OpStore %expectedWT %33
%35 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %35
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%42 = OpLoad %v4float %40
%44 = OpExtInst %v4float %1 FMix %39 %42 %43
%47 = OpFOrdEqual %v4bool %44 %45
%48 = OpAll %bool %47
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
%98 = OpFOrdEqual %bool %97 %float_0_5
OpBranch %88
%88 = OpLabel
%99 = OpPhi %bool %false %76 %98 %87
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%103 = OpLoad %v4float %102
%104 = OpVectorShuffle %v2float %103 %103 0 1
%105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%106 = OpLoad %v4float %105
%107 = OpVectorShuffle %v2float %106 %106 0 1
%109 = OpExtInst %v2float %1 FMix %104 %107 %108
%110 = OpVectorShuffle %v2float %30 %30 0 1
%112 = OpFOrdEqual %v2bool %109 %110
%113 = OpAll %bool %112
OpBranch %101
%101 = OpLabel
%114 = OpPhi %bool %false %88 %113 %100
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%118 = OpLoad %v4float %117
%119 = OpVectorShuffle %v3float %118 %118 0 1 2
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%122 = OpLoad %v4float %121
%123 = OpVectorShuffle %v3float %122 %122 0 1 2
%125 = OpExtInst %v3float %1 FMix %119 %123 %124
%126 = OpVectorShuffle %v3float %30 %30 0 1 2
%128 = OpFOrdEqual %v3bool %125 %126
%129 = OpAll %bool %128
OpBranch %116
%116 = OpLabel
%130 = OpPhi %bool %false %101 %129 %115
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%134 = OpLoad %v4float %133
%135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%136 = OpLoad %v4float %135
%138 = OpExtInst %v4float %1 FMix %134 %136 %137
%139 = OpFOrdEqual %v4bool %138 %30
%140 = OpAll %bool %139
OpBranch %132
%132 = OpLabel
%141 = OpPhi %bool %false %116 %140 %131
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%144 = OpFOrdEqual %bool %float_0_5 %float_0_5
OpBranch %143
%143 = OpLabel
%145 = OpPhi %bool %false %132 %144 %142
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%148 = OpVectorShuffle %v2float %30 %30 0 1
%149 = OpFOrdEqual %v2bool %108 %148
%150 = OpAll %bool %149
OpBranch %147
%147 = OpLabel
%151 = OpPhi %bool %false %143 %150 %146
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%154 = OpVectorShuffle %v3float %30 %30 0 1 2
%155 = OpFOrdEqual %v3bool %124 %154
%156 = OpAll %bool %155
OpBranch %153
%153 = OpLabel
%157 = OpPhi %bool %false %147 %156 %152
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%160 = OpFOrdEqual %v4bool %30 %30
%161 = OpAll %bool %160
OpBranch %159
%159 = OpLabel
%162 = OpPhi %bool %false %153 %161 %158
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%165 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%166 = OpLoad %v4float %165
%167 = OpCompositeExtract %float %166 0
%168 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%170 = OpLoad %v4float %168
%171 = OpCompositeExtract %float %170 0
%172 = OpExtInst %float %1 FMix %167 %171 %float_0
%173 = OpFOrdEqual %bool %172 %float_1
OpBranch %164
%164 = OpLabel
%174 = OpPhi %bool %false %159 %173 %163
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%178 = OpLoad %v4float %177
%179 = OpVectorShuffle %v2float %178 %178 0 1
%180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%181 = OpLoad %v4float %180
%182 = OpVectorShuffle %v2float %181 %181 0 1
%184 = OpExtInst %v2float %1 FMix %179 %182 %183
%185 = OpVectorShuffle %v2float %33 %33 0 1
%186 = OpFOrdEqual %v2bool %184 %185
%187 = OpAll %bool %186
OpBranch %176
%176 = OpLabel
%188 = OpPhi %bool %false %164 %187 %175
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%192 = OpLoad %v4float %191
%193 = OpVectorShuffle %v3float %192 %192 0 1 2
%194 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%195 = OpLoad %v4float %194
%196 = OpVectorShuffle %v3float %195 %195 0 1 2
%198 = OpExtInst %v3float %1 FMix %193 %196 %197
%199 = OpVectorShuffle %v3float %33 %33 0 1 2
%200 = OpFOrdEqual %v3bool %198 %199
%201 = OpAll %bool %200
OpBranch %190
%190 = OpLabel
%202 = OpPhi %bool %false %176 %201 %189
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%206 = OpLoad %v4float %205
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%208 = OpLoad %v4float %207
%210 = OpExtInst %v4float %1 FMix %206 %208 %209
%211 = OpFOrdEqual %v4bool %210 %33
%212 = OpAll %bool %211
OpBranch %204
%204 = OpLabel
%213 = OpPhi %bool %false %190 %212 %203
OpSelectionMerge %215 None
OpBranchConditional %213 %214 %215
%214 = OpLabel
%216 = OpFOrdEqual %bool %float_1 %float_1
OpBranch %215
%215 = OpLabel
%217 = OpPhi %bool %false %204 %216 %214
OpSelectionMerge %219 None
OpBranchConditional %217 %218 %219
%218 = OpLabel
%221 = OpVectorShuffle %v2float %33 %33 0 1
%222 = OpFOrdEqual %v2bool %220 %221
%223 = OpAll %bool %222
OpBranch %219
%219 = OpLabel
%224 = OpPhi %bool %false %215 %223 %218
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
%228 = OpVectorShuffle %v3float %33 %33 0 1 2
%229 = OpFOrdEqual %v3bool %227 %228
%230 = OpAll %bool %229
OpBranch %226
%226 = OpLabel
%231 = OpPhi %bool %false %219 %230 %225
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%234 = OpFOrdEqual %v4bool %33 %33
%235 = OpAll %bool %234
OpBranch %233
%233 = OpLabel
%236 = OpPhi %bool %false %226 %235 %232
OpSelectionMerge %240 None
OpBranchConditional %236 %238 %239
%238 = OpLabel
%241 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%242 = OpLoad %v4float %241
OpStore %237 %242
OpBranch %240
%239 = OpLabel
%243 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%244 = OpLoad %v4float %243
OpStore %237 %244
OpBranch %240
%240 = OpLabel
%245 = OpLoad %v4float %237
OpReturnValue %245
OpFunctionEnd
