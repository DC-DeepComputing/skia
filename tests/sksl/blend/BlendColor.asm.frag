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
OpName %blend_hslc_h4h4h4hb "blend_hslc_h4h4h4hb"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_2_blend_set_color_saturation "_2_blend_set_color_saturation"
OpName %_3_hueLumColor "_3_hueLumColor"
OpName %_4_sat "_4_sat"
OpName %_5_unpermute "_5_unpermute"
OpName %_6_blend_set_color_luminance "_6_blend_set_color_luminance"
OpName %_7_lum "_7_lum"
OpName %_8_result "_8_result"
OpName %_9_minComp "_9_minComp"
OpName %_10_maxComp "_10_maxComp"
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %alpha RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %_2_blend_set_color_saturation RelaxedPrecision
OpDecorate %_3_hueLumColor RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %_4_sat RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %_5_unpermute RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %_6_blend_set_color_luminance RelaxedPrecision
OpDecorate %_7_lum RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %_8_result RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %_9_minComp RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %_10_maxComp RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
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
OpDecorate %275 RelaxedPrecision
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
OpDecorate %299 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_bool = OpTypePointer Function %bool
%14 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_float %_ptr_Function_bool
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%182 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%191 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%void = OpTypeVoid
%295 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%blend_hslc_h4h4h4hb = OpFunction %v4float None %14
%18 = OpFunctionParameter %_ptr_Function_v4float
%19 = OpFunctionParameter %_ptr_Function_v4float
%20 = OpFunctionParameter %_ptr_Function_float
%21 = OpFunctionParameter %_ptr_Function_bool
%22 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%_2_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_3_hueLumColor = OpVariable %_ptr_Function_v3float Function
%_4_sat = OpVariable %_ptr_Function_float Function
%_5_unpermute = OpVariable %_ptr_Function_mat3v3float Function
%_6_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_7_lum = OpVariable %_ptr_Function_float Function
%_8_result = OpVariable %_ptr_Function_v3float Function
%_9_minComp = OpVariable %_ptr_Function_float Function
%_10_maxComp = OpVariable %_ptr_Function_float Function
%24 = OpLoad %v4float %19
%25 = OpCompositeExtract %float %24 3
%26 = OpLoad %v4float %18
%27 = OpCompositeExtract %float %26 3
%28 = OpFMul %float %25 %27
OpStore %alpha %28
%32 = OpLoad %v4float %18
%33 = OpVectorShuffle %v3float %32 %32 0 1 2
%34 = OpLoad %v4float %19
%35 = OpCompositeExtract %float %34 3
%36 = OpVectorTimesScalar %v3float %33 %35
OpStore %sda %36
%38 = OpLoad %v4float %19
%39 = OpVectorShuffle %v3float %38 %38 0 1 2
%40 = OpLoad %v4float %18
%41 = OpCompositeExtract %float %40 3
%42 = OpVectorTimesScalar %v3float %39 %41
OpStore %dsa %42
%45 = OpLoad %v3float %sda
%46 = OpLoad %v3float %dsa
%47 = OpLoad %float %20
%48 = OpCompositeConstruct %v3float %47 %47 %47
%44 = OpExtInst %v3float %1 FMix %45 %46 %48
OpStore %l %44
%51 = OpLoad %v3float %dsa
%52 = OpLoad %v3float %sda
%53 = OpLoad %float %20
%54 = OpCompositeConstruct %v3float %53 %53 %53
%50 = OpExtInst %v3float %1 FMix %51 %52 %54
OpStore %r %50
%55 = OpLoad %bool %21
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%60 = OpLoad %v3float %l
OpStore %_3_hueLumColor %60
%64 = OpLoad %v3float %r
%65 = OpCompositeExtract %float %64 0
%66 = OpLoad %v3float %r
%67 = OpCompositeExtract %float %66 1
%63 = OpExtInst %float %1 FMax %65 %67
%68 = OpLoad %v3float %r
%69 = OpCompositeExtract %float %68 2
%62 = OpExtInst %float %1 FMax %63 %69
%72 = OpLoad %v3float %r
%73 = OpCompositeExtract %float %72 0
%74 = OpLoad %v3float %r
%75 = OpCompositeExtract %float %74 1
%71 = OpExtInst %float %1 FMin %73 %75
%76 = OpLoad %v3float %r
%77 = OpCompositeExtract %float %76 2
%70 = OpExtInst %float %1 FMin %71 %77
%78 = OpFSub %float %62 %70
OpStore %_4_sat %78
%85 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%86 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%87 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%83 = OpCompositeConstruct %mat3v3float %85 %86 %87
OpStore %_5_unpermute %83
%88 = OpLoad %v3float %_3_hueLumColor
%89 = OpCompositeExtract %float %88 0
%90 = OpLoad %v3float %_3_hueLumColor
%91 = OpCompositeExtract %float %90 1
%92 = OpFOrdGreaterThan %bool %89 %91
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpLoad %v3float %_3_hueLumColor
%96 = OpVectorShuffle %v2float %95 %95 1 0
%98 = OpLoad %v3float %_3_hueLumColor
%99 = OpVectorShuffle %v3float %98 %96 3 4 2
OpStore %_3_hueLumColor %99
%102 = OpAccessChain %_ptr_Function_v3float %_5_unpermute %int_0
%103 = OpLoad %v3float %102
%104 = OpVectorShuffle %v3float %103 %103 1 0 2
%106 = OpAccessChain %_ptr_Function_v3float %_5_unpermute %int_1
%107 = OpLoad %v3float %106
%108 = OpVectorShuffle %v3float %107 %107 1 0 2
%110 = OpAccessChain %_ptr_Function_v3float %_5_unpermute %int_2
%111 = OpLoad %v3float %110
%112 = OpVectorShuffle %v3float %111 %111 1 0 2
%113 = OpCompositeConstruct %mat3v3float %104 %108 %112
OpStore %_5_unpermute %113
OpBranch %94
%94 = OpLabel
%114 = OpLoad %v3float %_3_hueLumColor
%115 = OpCompositeExtract %float %114 0
%116 = OpLoad %v3float %_3_hueLumColor
%117 = OpCompositeExtract %float %116 2
%118 = OpFOrdGreaterThan %bool %115 %117
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%121 = OpLoad %v3float %_3_hueLumColor
%122 = OpVectorShuffle %v2float %121 %121 2 0
%123 = OpLoad %v3float %_3_hueLumColor
%124 = OpVectorShuffle %v3float %123 %122 3 1 4
OpStore %_3_hueLumColor %124
%125 = OpAccessChain %_ptr_Function_v3float %_5_unpermute %int_0
%126 = OpLoad %v3float %125
%127 = OpVectorShuffle %v3float %126 %126 2 1 0
%128 = OpAccessChain %_ptr_Function_v3float %_5_unpermute %int_1
%129 = OpLoad %v3float %128
%130 = OpVectorShuffle %v3float %129 %129 2 1 0
%131 = OpAccessChain %_ptr_Function_v3float %_5_unpermute %int_2
%132 = OpLoad %v3float %131
%133 = OpVectorShuffle %v3float %132 %132 2 1 0
%134 = OpCompositeConstruct %mat3v3float %127 %130 %133
OpStore %_5_unpermute %134
OpBranch %120
%120 = OpLabel
%135 = OpLoad %v3float %_3_hueLumColor
%136 = OpCompositeExtract %float %135 1
%137 = OpLoad %v3float %_3_hueLumColor
%138 = OpCompositeExtract %float %137 2
%139 = OpFOrdGreaterThan %bool %136 %138
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%142 = OpLoad %v3float %_3_hueLumColor
%143 = OpVectorShuffle %v2float %142 %142 2 1
%144 = OpLoad %v3float %_3_hueLumColor
%145 = OpVectorShuffle %v3float %144 %143 0 3 4
OpStore %_3_hueLumColor %145
%146 = OpAccessChain %_ptr_Function_v3float %_5_unpermute %int_0
%147 = OpLoad %v3float %146
%148 = OpVectorShuffle %v3float %147 %147 0 2 1
%149 = OpAccessChain %_ptr_Function_v3float %_5_unpermute %int_1
%150 = OpLoad %v3float %149
%151 = OpVectorShuffle %v3float %150 %150 0 2 1
%152 = OpAccessChain %_ptr_Function_v3float %_5_unpermute %int_2
%153 = OpLoad %v3float %152
%154 = OpVectorShuffle %v3float %153 %153 0 2 1
%155 = OpCompositeConstruct %mat3v3float %148 %151 %154
OpStore %_5_unpermute %155
OpBranch %141
%141 = OpLabel
%156 = OpLoad %v3float %_3_hueLumColor
%157 = OpCompositeExtract %float %156 2
%158 = OpLoad %v3float %_3_hueLumColor
%159 = OpCompositeExtract %float %158 0
%160 = OpFOrdGreaterThan %bool %157 %159
OpSelectionMerge %163 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%164 = OpLoad %v3float %_3_hueLumColor
%165 = OpVectorShuffle %v2float %164 %164 1 2
%166 = OpLoad %v3float %_3_hueLumColor
%167 = OpVectorShuffle %v2float %166 %166 0 0
%168 = OpFSub %v2float %165 %167
%169 = OpLoad %v3float %_3_hueLumColor
%170 = OpVectorShuffle %v3float %169 %168 0 3 4
OpStore %_3_hueLumColor %170
%171 = OpLoad %float %_4_sat
%172 = OpLoad %v3float %_3_hueLumColor
%173 = OpCompositeExtract %float %172 1
%174 = OpFMul %float %171 %173
%175 = OpLoad %v3float %_3_hueLumColor
%176 = OpCompositeExtract %float %175 2
%177 = OpFDiv %float %174 %176
%178 = OpLoad %float %_4_sat
%179 = OpCompositeConstruct %v3float %float_0 %177 %178
%180 = OpLoad %mat3v3float %_5_unpermute
%181 = OpVectorTimesMatrix %v3float %179 %180
OpStore %_2_blend_set_color_saturation %181
OpBranch %163
%162 = OpLabel
OpStore %_2_blend_set_color_saturation %182
OpBranch %163
%163 = OpLabel
%183 = OpLoad %v3float %_2_blend_set_color_saturation
OpStore %l %183
%184 = OpLoad %v3float %dsa
OpStore %r %184
OpBranch %57
%57 = OpLabel
%192 = OpLoad %v3float %r
%187 = OpDot %float %191 %192
OpStore %_7_lum %187
%194 = OpLoad %float %_7_lum
%196 = OpLoad %v3float %l
%195 = OpDot %float %191 %196
%197 = OpFSub %float %194 %195
%198 = OpLoad %v3float %l
%199 = OpCompositeConstruct %v3float %197 %197 %197
%200 = OpFAdd %v3float %199 %198
OpStore %_8_result %200
%204 = OpLoad %v3float %_8_result
%205 = OpCompositeExtract %float %204 0
%206 = OpLoad %v3float %_8_result
%207 = OpCompositeExtract %float %206 1
%203 = OpExtInst %float %1 FMin %205 %207
%208 = OpLoad %v3float %_8_result
%209 = OpCompositeExtract %float %208 2
%202 = OpExtInst %float %1 FMin %203 %209
OpStore %_9_minComp %202
%213 = OpLoad %v3float %_8_result
%214 = OpCompositeExtract %float %213 0
%215 = OpLoad %v3float %_8_result
%216 = OpCompositeExtract %float %215 1
%212 = OpExtInst %float %1 FMax %214 %216
%217 = OpLoad %v3float %_8_result
%218 = OpCompositeExtract %float %217 2
%211 = OpExtInst %float %1 FMax %212 %218
OpStore %_10_maxComp %211
%220 = OpLoad %float %_9_minComp
%221 = OpFOrdLessThan %bool %220 %float_0
OpSelectionMerge %223 None
OpBranchConditional %221 %222 %223
%222 = OpLabel
%224 = OpLoad %float %_7_lum
%225 = OpLoad %float %_9_minComp
%226 = OpFUnordNotEqual %bool %224 %225
OpBranch %223
%223 = OpLabel
%227 = OpPhi %bool %false %57 %226 %222
OpSelectionMerge %229 None
OpBranchConditional %227 %228 %229
%228 = OpLabel
%230 = OpLoad %float %_7_lum
%231 = OpLoad %v3float %_8_result
%232 = OpLoad %float %_7_lum
%233 = OpCompositeConstruct %v3float %232 %232 %232
%234 = OpFSub %v3float %231 %233
%235 = OpLoad %float %_7_lum
%236 = OpLoad %float %_7_lum
%237 = OpLoad %float %_9_minComp
%238 = OpFSub %float %236 %237
%239 = OpFDiv %float %235 %238
%240 = OpVectorTimesScalar %v3float %234 %239
%241 = OpCompositeConstruct %v3float %230 %230 %230
%242 = OpFAdd %v3float %241 %240
OpStore %_8_result %242
OpBranch %229
%229 = OpLabel
%243 = OpLoad %float %_10_maxComp
%244 = OpLoad %float %alpha
%245 = OpFOrdGreaterThan %bool %243 %244
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%248 = OpLoad %float %_10_maxComp
%249 = OpLoad %float %_7_lum
%250 = OpFUnordNotEqual %bool %248 %249
OpBranch %247
%247 = OpLabel
%251 = OpPhi %bool %false %229 %250 %246
OpSelectionMerge %254 None
OpBranchConditional %251 %252 %253
%252 = OpLabel
%255 = OpLoad %float %_7_lum
%256 = OpLoad %v3float %_8_result
%257 = OpLoad %float %_7_lum
%258 = OpCompositeConstruct %v3float %257 %257 %257
%259 = OpFSub %v3float %256 %258
%260 = OpLoad %float %alpha
%261 = OpLoad %float %_7_lum
%262 = OpFSub %float %260 %261
%263 = OpVectorTimesScalar %v3float %259 %262
%264 = OpLoad %float %_10_maxComp
%265 = OpLoad %float %_7_lum
%266 = OpFSub %float %264 %265
%267 = OpFDiv %float %float_1 %266
%268 = OpVectorTimesScalar %v3float %263 %267
%269 = OpCompositeConstruct %v3float %255 %255 %255
%270 = OpFAdd %v3float %269 %268
OpStore %_6_blend_set_color_luminance %270
OpBranch %254
%253 = OpLabel
%271 = OpLoad %v3float %_8_result
OpStore %_6_blend_set_color_luminance %271
OpBranch %254
%254 = OpLabel
%272 = OpLoad %v3float %_6_blend_set_color_luminance
%273 = OpLoad %v4float %19
%274 = OpVectorShuffle %v3float %273 %273 0 1 2
%275 = OpFAdd %v3float %272 %274
%276 = OpLoad %v3float %dsa
%277 = OpFSub %v3float %275 %276
%278 = OpLoad %v4float %18
%279 = OpVectorShuffle %v3float %278 %278 0 1 2
%280 = OpFAdd %v3float %277 %279
%281 = OpLoad %v3float %sda
%282 = OpFSub %v3float %280 %281
%283 = OpCompositeExtract %float %282 0
%284 = OpCompositeExtract %float %282 1
%285 = OpCompositeExtract %float %282 2
%286 = OpLoad %v4float %18
%287 = OpCompositeExtract %float %286 3
%288 = OpLoad %v4float %19
%289 = OpCompositeExtract %float %288 3
%290 = OpFAdd %float %287 %289
%291 = OpLoad %float %alpha
%292 = OpFSub %float %290 %291
%293 = OpCompositeConstruct %v4float %283 %284 %285 %292
OpReturnValue %293
OpFunctionEnd
%main = OpFunction %void None %295
%296 = OpLabel
%300 = OpVariable %_ptr_Function_v4float Function
%303 = OpVariable %_ptr_Function_v4float Function
%304 = OpVariable %_ptr_Function_float Function
%305 = OpVariable %_ptr_Function_bool Function
%297 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%299 = OpLoad %v4float %297
OpStore %300 %299
%301 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%302 = OpLoad %v4float %301
OpStore %303 %302
OpStore %304 %float_0
OpStore %305 %false
%306 = OpFunctionCall %v4float %blend_hslc_h4h4h4hb %300 %303 %304 %305
OpStore %sk_FragColor %306
OpReturn
OpFunctionEnd
