OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "colorBlack"
OpMemberName %_UniformBuffer 3 "colorWhite"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %FTFT "FTFT"
OpName %TFTF "TFTF"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
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
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%false = OpConstantFalse %bool
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%v2bool = OpTypeVector %bool 2
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%int_4 = OpConstant %int 4
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
%FTFT = OpVariable %_ptr_Function_v4bool Function
%TFTF = OpVariable %_ptr_Function_v4bool Function
%243 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %29
%34 = OpCompositeExtract %float %33 0
%35 = OpFUnordNotEqual %bool %34 %float_0
%36 = OpCompositeExtract %float %33 1
%37 = OpFUnordNotEqual %bool %36 %float_0
%38 = OpCompositeExtract %float %33 2
%39 = OpFUnordNotEqual %bool %38 %float_0
%40 = OpCompositeExtract %float %33 3
%41 = OpFUnordNotEqual %bool %40 %float_0
%42 = OpCompositeConstruct %v4bool %35 %37 %39 %41
OpStore %FTFT %42
%45 = OpCompositeExtract %bool %42 0
%44 = OpLogicalNot %bool %45
%47 = OpCompositeExtract %bool %42 1
%46 = OpLogicalNot %bool %47
%49 = OpCompositeExtract %bool %42 2
%48 = OpLogicalNot %bool %49
%51 = OpCompositeExtract %bool %42 3
%50 = OpLogicalNot %bool %51
%52 = OpCompositeConstruct %v4bool %44 %46 %48 %50
OpStore %TFTF %52
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%57 = OpLoad %v4float %55
%58 = OpCompositeExtract %float %57 0
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%61 = OpLoad %v4float %59
%62 = OpCompositeExtract %float %61 0
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%64 = OpLoad %v4float %63
%65 = OpCompositeExtract %float %64 0
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%67 = OpLoad %v4float %66
%68 = OpCompositeExtract %float %67 0
%54 = OpSelect %float %45 %68 %65
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%70 = OpLoad %v4float %69
%71 = OpCompositeExtract %float %70 0
%72 = OpFOrdEqual %bool %54 %71
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%77 = OpLoad %v4float %76
%78 = OpVectorShuffle %v2float %77 %77 0 1
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%80 = OpLoad %v4float %79
%81 = OpVectorShuffle %v2float %80 %80 0 1
%82 = OpVectorShuffle %v2bool %42 %42 0 1
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%85 = OpLoad %v4float %84
%86 = OpVectorShuffle %v2float %85 %85 0 1
%87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%88 = OpLoad %v4float %87
%89 = OpVectorShuffle %v2float %88 %88 0 1
%90 = OpVectorShuffle %v2bool %42 %42 0 1
%75 = OpSelect %v2float %90 %89 %86
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%92 = OpLoad %v4float %91
%93 = OpCompositeExtract %float %92 0
%95 = OpCompositeConstruct %v2float %93 %float_1
%96 = OpFOrdEqual %v2bool %75 %95
%97 = OpAll %bool %96
OpBranch %74
%74 = OpLabel
%98 = OpPhi %bool %false %25 %97 %73
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%103 = OpLoad %v4float %102
%104 = OpVectorShuffle %v3float %103 %103 0 1 2
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%107 = OpLoad %v4float %106
%108 = OpVectorShuffle %v3float %107 %107 0 1 2
%109 = OpVectorShuffle %v3bool %42 %42 0 1 2
%111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%112 = OpLoad %v4float %111
%113 = OpVectorShuffle %v3float %112 %112 0 1 2
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%115 = OpLoad %v4float %114
%116 = OpVectorShuffle %v3float %115 %115 0 1 2
%117 = OpVectorShuffle %v3bool %42 %42 0 1 2
%101 = OpSelect %v3float %117 %116 %113
%118 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%119 = OpLoad %v4float %118
%120 = OpCompositeExtract %float %119 0
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%122 = OpLoad %v4float %121
%123 = OpCompositeExtract %float %122 2
%124 = OpCompositeConstruct %v3float %120 %float_1 %123
%125 = OpFOrdEqual %v3bool %101 %124
%126 = OpAll %bool %125
OpBranch %100
%100 = OpLabel
%127 = OpPhi %bool %false %74 %126 %99
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%132 = OpLoad %v4float %131
%133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%134 = OpLoad %v4float %133
%135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%136 = OpLoad %v4float %135
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%138 = OpLoad %v4float %137
%130 = OpSelect %v4float %42 %138 %136
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%140 = OpLoad %v4float %139
%141 = OpCompositeExtract %float %140 0
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%143 = OpLoad %v4float %142
%144 = OpCompositeExtract %float %143 2
%145 = OpCompositeConstruct %v4float %141 %float_1 %144 %float_1
%146 = OpFOrdEqual %v4bool %130 %145
%147 = OpAll %bool %146
OpBranch %129
%129 = OpLabel
%148 = OpPhi %bool %false %100 %147 %128
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%153 = OpLoad %v4float %152
%154 = OpCompositeExtract %float %153 0
%155 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%157 = OpLoad %v4float %155
%158 = OpCompositeExtract %float %157 0
%159 = OpCompositeExtract %bool %52 0
%160 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%161 = OpLoad %v4float %160
%162 = OpCompositeExtract %float %161 0
%163 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%164 = OpLoad %v4float %163
%165 = OpCompositeExtract %float %164 0
%151 = OpSelect %float %159 %165 %162
%166 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%167 = OpLoad %v4float %166
%168 = OpCompositeExtract %float %167 0
%169 = OpFOrdEqual %bool %151 %168
OpBranch %150
%150 = OpLabel
%170 = OpPhi %bool %false %129 %169 %149
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%174 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%175 = OpLoad %v4float %174
%176 = OpVectorShuffle %v2float %175 %175 0 1
%177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%178 = OpLoad %v4float %177
%179 = OpVectorShuffle %v2float %178 %178 0 1
%180 = OpVectorShuffle %v2bool %52 %52 0 1
%181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%182 = OpLoad %v4float %181
%183 = OpVectorShuffle %v2float %182 %182 0 1
%184 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%185 = OpLoad %v4float %184
%186 = OpVectorShuffle %v2float %185 %185 0 1
%187 = OpVectorShuffle %v2bool %52 %52 0 1
%173 = OpSelect %v2float %187 %186 %183
%188 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%189 = OpLoad %v4float %188
%190 = OpCompositeExtract %float %189 0
%191 = OpCompositeConstruct %v2float %190 %float_1
%192 = OpFOrdEqual %v2bool %173 %191
%193 = OpAll %bool %192
OpBranch %172
%172 = OpLabel
%194 = OpPhi %bool %false %150 %193 %171
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
%198 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%199 = OpLoad %v4float %198
%200 = OpVectorShuffle %v3float %199 %199 0 1 2
%201 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%202 = OpLoad %v4float %201
%203 = OpVectorShuffle %v3float %202 %202 0 1 2
%204 = OpVectorShuffle %v3bool %52 %52 0 1 2
%205 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%206 = OpLoad %v4float %205
%207 = OpVectorShuffle %v3float %206 %206 0 1 2
%208 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%209 = OpLoad %v4float %208
%210 = OpVectorShuffle %v3float %209 %209 0 1 2
%211 = OpVectorShuffle %v3bool %52 %52 0 1 2
%197 = OpSelect %v3float %211 %210 %207
%212 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%213 = OpLoad %v4float %212
%214 = OpCompositeExtract %float %213 0
%215 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%216 = OpLoad %v4float %215
%217 = OpCompositeExtract %float %216 2
%218 = OpCompositeConstruct %v3float %214 %float_1 %217
%219 = OpFOrdEqual %v3bool %197 %218
%220 = OpAll %bool %219
OpBranch %196
%196 = OpLabel
%221 = OpPhi %bool %false %172 %220 %195
OpSelectionMerge %223 None
OpBranchConditional %221 %222 %223
%222 = OpLabel
%225 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%226 = OpLoad %v4float %225
%227 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%228 = OpLoad %v4float %227
%229 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%230 = OpLoad %v4float %229
%231 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%232 = OpLoad %v4float %231
%224 = OpSelect %v4float %52 %232 %230
%233 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%234 = OpLoad %v4float %233
%235 = OpCompositeExtract %float %234 0
%236 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%237 = OpLoad %v4float %236
%238 = OpCompositeExtract %float %237 2
%239 = OpCompositeConstruct %v4float %235 %float_1 %238 %float_1
%240 = OpFOrdEqual %v4bool %224 %239
%241 = OpAll %bool %240
OpBranch %223
%223 = OpLabel
%242 = OpPhi %bool %false %196 %241 %222
OpSelectionMerge %247 None
OpBranchConditional %242 %245 %246
%245 = OpLabel
%248 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%249 = OpLoad %v4float %248
OpStore %243 %249
OpBranch %247
%246 = OpLabel
%250 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%252 = OpLoad %v4float %250
OpStore %243 %252
OpBranch %247
%247 = OpLabel
%253 = OpLoad %v4float %243
OpReturnValue %253
OpFunctionEnd
