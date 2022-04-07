OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_set_color_saturation_helper_Qh3h3h3 "blend_set_color_saturation_helper_Qh3h3h3"
OpName %sat "sat"
OpName %blend_hslc_h4h4h4bb "blend_hslc_h4h4h4bb"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_2_blend_set_color_saturation "_2_blend_set_color_saturation"
OpName %_3_lum "_3_lum"
OpName %_4_result "_4_result"
OpName %_5_minComp "_5_minComp"
OpName %_6_maxComp "_6_maxComp"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %sat RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %_2_blend_set_color_saturation RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %_3_lum RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %_4_result RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %_5_minComp RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %_6_maxComp RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%16 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%64 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%65 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_bool %_ptr_Function_bool
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%200 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%303 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_set_color_saturation_helper_Qh3h3h3 = OpFunction %v3float None %16
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_v3float
%20 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%21 = OpLoad %v3float %18
%22 = OpVectorShuffle %v2float %21 %21 1 2
%24 = OpLoad %v3float %18
%25 = OpVectorShuffle %v2float %24 %24 0 0
%26 = OpFSub %v2float %22 %25
%27 = OpLoad %v3float %18
%28 = OpVectorShuffle %v3float %27 %26 0 3 4
OpStore %18 %28
%29 = OpLoad %v3float %18
%30 = OpCompositeExtract %float %29 2
%32 = OpFOrdGreaterThan %bool %30 %float_0
OpSelectionMerge %35 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
%40 = OpLoad %v3float %19
%41 = OpCompositeExtract %float %40 0
%42 = OpLoad %v3float %19
%43 = OpCompositeExtract %float %42 1
%39 = OpExtInst %float %1 FMax %41 %43
%44 = OpLoad %v3float %19
%45 = OpCompositeExtract %float %44 2
%38 = OpExtInst %float %1 FMax %39 %45
%48 = OpLoad %v3float %19
%49 = OpCompositeExtract %float %48 0
%50 = OpLoad %v3float %19
%51 = OpCompositeExtract %float %50 1
%47 = OpExtInst %float %1 FMin %49 %51
%52 = OpLoad %v3float %19
%53 = OpCompositeExtract %float %52 2
%46 = OpExtInst %float %1 FMin %47 %53
%54 = OpFSub %float %38 %46
OpStore %sat %54
%55 = OpLoad %float %sat
%56 = OpLoad %v3float %18
%57 = OpCompositeExtract %float %56 1
%58 = OpLoad %v3float %18
%59 = OpCompositeExtract %float %58 2
%60 = OpFDiv %float %57 %59
%61 = OpFMul %float %55 %60
%62 = OpLoad %float %sat
%63 = OpCompositeConstruct %v3float %float_0 %61 %62
OpReturnValue %63
%34 = OpLabel
OpReturnValue %64
%35 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_hslc_h4h4h4bb = OpFunction %v4float None %65
%68 = OpFunctionParameter %_ptr_Function_v4float
%69 = OpFunctionParameter %_ptr_Function_v4float
%70 = OpFunctionParameter %_ptr_Function_bool
%71 = OpFunctionParameter %_ptr_Function_bool
%72 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%93 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%102 = OpVariable %_ptr_Function_v3float Function
%_2_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%130 = OpVariable %_ptr_Function_v3float Function
%132 = OpVariable %_ptr_Function_v3float Function
%144 = OpVariable %_ptr_Function_v3float Function
%146 = OpVariable %_ptr_Function_v3float Function
%151 = OpVariable %_ptr_Function_v3float Function
%153 = OpVariable %_ptr_Function_v3float Function
%166 = OpVariable %_ptr_Function_v3float Function
%168 = OpVariable %_ptr_Function_v3float Function
%181 = OpVariable %_ptr_Function_v3float Function
%183 = OpVariable %_ptr_Function_v3float Function
%188 = OpVariable %_ptr_Function_v3float Function
%190 = OpVariable %_ptr_Function_v3float Function
%_3_lum = OpVariable %_ptr_Function_float Function
%_4_result = OpVariable %_ptr_Function_v3float Function
%_5_minComp = OpVariable %_ptr_Function_float Function
%_6_maxComp = OpVariable %_ptr_Function_float Function
%74 = OpLoad %v4float %69
%75 = OpCompositeExtract %float %74 3
%76 = OpLoad %v4float %68
%77 = OpCompositeExtract %float %76 3
%78 = OpFMul %float %75 %77
OpStore %alpha %78
%80 = OpLoad %v4float %68
%81 = OpVectorShuffle %v3float %80 %80 0 1 2
%82 = OpLoad %v4float %69
%83 = OpCompositeExtract %float %82 3
%84 = OpVectorTimesScalar %v3float %81 %83
OpStore %sda %84
%86 = OpLoad %v4float %69
%87 = OpVectorShuffle %v3float %86 %86 0 1 2
%88 = OpLoad %v4float %68
%89 = OpCompositeExtract %float %88 3
%90 = OpVectorTimesScalar %v3float %87 %89
OpStore %dsa %90
%92 = OpLoad %bool %70
OpSelectionMerge %96 None
OpBranchConditional %92 %94 %95
%94 = OpLabel
%97 = OpLoad %v3float %dsa
OpStore %93 %97
OpBranch %96
%95 = OpLabel
%98 = OpLoad %v3float %sda
OpStore %93 %98
OpBranch %96
%96 = OpLabel
%99 = OpLoad %v3float %93
OpStore %l %99
%101 = OpLoad %bool %70
OpSelectionMerge %105 None
OpBranchConditional %101 %103 %104
%103 = OpLabel
%106 = OpLoad %v3float %sda
OpStore %102 %106
OpBranch %105
%104 = OpLabel
%107 = OpLoad %v3float %dsa
OpStore %102 %107
OpBranch %105
%105 = OpLabel
%108 = OpLoad %v3float %102
OpStore %r %108
%109 = OpLoad %bool %71
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpLoad %v3float %l
%114 = OpCompositeExtract %float %113 0
%115 = OpLoad %v3float %l
%116 = OpCompositeExtract %float %115 1
%117 = OpFOrdLessThanEqual %bool %114 %116
OpSelectionMerge %120 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpLoad %v3float %l
%122 = OpCompositeExtract %float %121 1
%123 = OpLoad %v3float %l
%124 = OpCompositeExtract %float %123 2
%125 = OpFOrdLessThanEqual %bool %122 %124
OpSelectionMerge %128 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpLoad %v3float %l
OpStore %130 %129
%131 = OpLoad %v3float %r
OpStore %132 %131
%133 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %130 %132
OpStore %_2_blend_set_color_saturation %133
OpBranch %128
%127 = OpLabel
%134 = OpLoad %v3float %l
%135 = OpCompositeExtract %float %134 0
%136 = OpLoad %v3float %l
%137 = OpCompositeExtract %float %136 2
%138 = OpFOrdLessThanEqual %bool %135 %137
OpSelectionMerge %141 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpLoad %v3float %l
%143 = OpVectorShuffle %v3float %142 %142 0 2 1
OpStore %144 %143
%145 = OpLoad %v3float %r
OpStore %146 %145
%147 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %144 %146
%148 = OpVectorShuffle %v3float %147 %147 0 2 1
OpStore %_2_blend_set_color_saturation %148
OpBranch %141
%140 = OpLabel
%149 = OpLoad %v3float %l
%150 = OpVectorShuffle %v3float %149 %149 2 0 1
OpStore %151 %150
%152 = OpLoad %v3float %r
OpStore %153 %152
%154 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %151 %153
%155 = OpVectorShuffle %v3float %154 %154 1 2 0
OpStore %_2_blend_set_color_saturation %155
OpBranch %141
%141 = OpLabel
OpBranch %128
%128 = OpLabel
OpBranch %120
%119 = OpLabel
%156 = OpLoad %v3float %l
%157 = OpCompositeExtract %float %156 0
%158 = OpLoad %v3float %l
%159 = OpCompositeExtract %float %158 2
%160 = OpFOrdLessThanEqual %bool %157 %159
OpSelectionMerge %163 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%164 = OpLoad %v3float %l
%165 = OpVectorShuffle %v3float %164 %164 1 0 2
OpStore %166 %165
%167 = OpLoad %v3float %r
OpStore %168 %167
%169 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %166 %168
%170 = OpVectorShuffle %v3float %169 %169 1 0 2
OpStore %_2_blend_set_color_saturation %170
OpBranch %163
%162 = OpLabel
%171 = OpLoad %v3float %l
%172 = OpCompositeExtract %float %171 1
%173 = OpLoad %v3float %l
%174 = OpCompositeExtract %float %173 2
%175 = OpFOrdLessThanEqual %bool %172 %174
OpSelectionMerge %178 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%179 = OpLoad %v3float %l
%180 = OpVectorShuffle %v3float %179 %179 1 2 0
OpStore %181 %180
%182 = OpLoad %v3float %r
OpStore %183 %182
%184 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %181 %183
%185 = OpVectorShuffle %v3float %184 %184 2 0 1
OpStore %_2_blend_set_color_saturation %185
OpBranch %178
%177 = OpLabel
%186 = OpLoad %v3float %l
%187 = OpVectorShuffle %v3float %186 %186 2 1 0
OpStore %188 %187
%189 = OpLoad %v3float %r
OpStore %190 %189
%191 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h3 %188 %190
%192 = OpVectorShuffle %v3float %191 %191 2 1 0
OpStore %_2_blend_set_color_saturation %192
OpBranch %178
%178 = OpLabel
OpBranch %163
%163 = OpLabel
OpBranch %120
%120 = OpLabel
%193 = OpLoad %v3float %_2_blend_set_color_saturation
OpStore %l %193
%194 = OpLoad %v3float %dsa
OpStore %r %194
OpBranch %111
%111 = OpLabel
%201 = OpLoad %v3float %r
%196 = OpDot %float %200 %201
OpStore %_3_lum %196
%203 = OpLoad %float %_3_lum
%205 = OpLoad %v3float %l
%204 = OpDot %float %200 %205
%206 = OpFSub %float %203 %204
%207 = OpLoad %v3float %l
%208 = OpCompositeConstruct %v3float %206 %206 %206
%209 = OpFAdd %v3float %208 %207
OpStore %_4_result %209
%213 = OpLoad %v3float %_4_result
%214 = OpCompositeExtract %float %213 0
%215 = OpLoad %v3float %_4_result
%216 = OpCompositeExtract %float %215 1
%212 = OpExtInst %float %1 FMin %214 %216
%217 = OpLoad %v3float %_4_result
%218 = OpCompositeExtract %float %217 2
%211 = OpExtInst %float %1 FMin %212 %218
OpStore %_5_minComp %211
%222 = OpLoad %v3float %_4_result
%223 = OpCompositeExtract %float %222 0
%224 = OpLoad %v3float %_4_result
%225 = OpCompositeExtract %float %224 1
%221 = OpExtInst %float %1 FMax %223 %225
%226 = OpLoad %v3float %_4_result
%227 = OpCompositeExtract %float %226 2
%220 = OpExtInst %float %1 FMax %221 %227
OpStore %_6_maxComp %220
%229 = OpLoad %float %_5_minComp
%230 = OpFOrdLessThan %bool %229 %float_0
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %float %_3_lum
%234 = OpLoad %float %_5_minComp
%235 = OpFUnordNotEqual %bool %233 %234
OpBranch %232
%232 = OpLabel
%236 = OpPhi %bool %false %111 %235 %231
OpSelectionMerge %238 None
OpBranchConditional %236 %237 %238
%237 = OpLabel
%239 = OpLoad %float %_3_lum
%240 = OpLoad %v3float %_4_result
%241 = OpLoad %float %_3_lum
%242 = OpCompositeConstruct %v3float %241 %241 %241
%243 = OpFSub %v3float %240 %242
%244 = OpLoad %float %_3_lum
%245 = OpLoad %float %_3_lum
%246 = OpLoad %float %_5_minComp
%247 = OpFSub %float %245 %246
%248 = OpFDiv %float %244 %247
%249 = OpVectorTimesScalar %v3float %243 %248
%250 = OpCompositeConstruct %v3float %239 %239 %239
%251 = OpFAdd %v3float %250 %249
OpStore %_4_result %251
OpBranch %238
%238 = OpLabel
%252 = OpLoad %float %_6_maxComp
%253 = OpLoad %float %alpha
%254 = OpFOrdGreaterThan %bool %252 %253
OpSelectionMerge %256 None
OpBranchConditional %254 %255 %256
%255 = OpLabel
%257 = OpLoad %float %_6_maxComp
%258 = OpLoad %float %_3_lum
%259 = OpFUnordNotEqual %bool %257 %258
OpBranch %256
%256 = OpLabel
%260 = OpPhi %bool %false %238 %259 %255
OpSelectionMerge %262 None
OpBranchConditional %260 %261 %262
%261 = OpLabel
%263 = OpLoad %float %_3_lum
%264 = OpLoad %v3float %_4_result
%265 = OpLoad %float %_3_lum
%266 = OpCompositeConstruct %v3float %265 %265 %265
%267 = OpFSub %v3float %264 %266
%268 = OpLoad %float %alpha
%269 = OpLoad %float %_3_lum
%270 = OpFSub %float %268 %269
%271 = OpVectorTimesScalar %v3float %267 %270
%272 = OpLoad %float %_6_maxComp
%273 = OpLoad %float %_3_lum
%274 = OpFSub %float %272 %273
%276 = OpFDiv %float %float_1 %274
%277 = OpVectorTimesScalar %v3float %271 %276
%278 = OpCompositeConstruct %v3float %263 %263 %263
%279 = OpFAdd %v3float %278 %277
OpStore %_4_result %279
OpBranch %262
%262 = OpLabel
%280 = OpLoad %v3float %_4_result
%281 = OpLoad %v4float %69
%282 = OpVectorShuffle %v3float %281 %281 0 1 2
%283 = OpFAdd %v3float %280 %282
%284 = OpLoad %v3float %dsa
%285 = OpFSub %v3float %283 %284
%286 = OpLoad %v4float %68
%287 = OpVectorShuffle %v3float %286 %286 0 1 2
%288 = OpFAdd %v3float %285 %287
%289 = OpLoad %v3float %sda
%290 = OpFSub %v3float %288 %289
%291 = OpCompositeExtract %float %290 0
%292 = OpCompositeExtract %float %290 1
%293 = OpCompositeExtract %float %290 2
%294 = OpLoad %v4float %68
%295 = OpCompositeExtract %float %294 3
%296 = OpLoad %v4float %69
%297 = OpCompositeExtract %float %296 3
%298 = OpFAdd %float %295 %297
%299 = OpLoad %float %alpha
%300 = OpFSub %float %298 %299
%301 = OpCompositeConstruct %v4float %291 %292 %293 %300
OpReturnValue %301
OpFunctionEnd
%main = OpFunction %void None %303
%304 = OpLabel
%310 = OpVariable %_ptr_Function_v4float Function
%314 = OpVariable %_ptr_Function_v4float Function
%315 = OpVariable %_ptr_Function_bool Function
%316 = OpVariable %_ptr_Function_bool Function
%305 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%309 = OpLoad %v4float %305
OpStore %310 %309
%311 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%313 = OpLoad %v4float %311
OpStore %314 %313
OpStore %315 %false
OpStore %316 %false
%317 = OpFunctionCall %v4float %blend_hslc_h4h4h4bb %310 %314 %315 %316
OpStore %sk_FragColor %317
OpReturn
OpFunctionEnd
