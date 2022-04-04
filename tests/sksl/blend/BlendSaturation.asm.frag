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
OpName %blend_set_color_saturation_helper_Qh3h3h "blend_set_color_saturation_helper_Qh3h3h"
OpName %blend_hslc_h4h4h4hb "blend_hslc_h4h4h4hb"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_1_blend_set_color_saturation "_1_blend_set_color_saturation"
OpName %_2_sat "_2_sat"
OpName %_3_blend_set_color_luminance "_3_blend_set_color_luminance"
OpName %_4_lum "_4_lum"
OpName %_5_result "_5_result"
OpName %_6_minComp "_6_minComp"
OpName %_7_maxComp "_7_maxComp"
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
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %_1_blend_set_color_saturation RelaxedPrecision
OpDecorate %_2_sat RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %_3_blend_set_color_luminance RelaxedPrecision
OpDecorate %_4_lum RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %_5_result RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %_6_minComp RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %_7_maxComp RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
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
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
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
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%16 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_0_00100000005 = OpConstant %float 0.00100000005
%float_0 = OpConstant %float 0
%48 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%49 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_float %_ptr_Function_bool
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%197 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%302 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%true = OpConstantTrue %bool
%blend_set_color_saturation_helper_Qh3h3h = OpFunction %v3float None %16
%19 = OpFunctionParameter %_ptr_Function_v3float
%20 = OpFunctionParameter %_ptr_Function_float
%21 = OpLabel
%22 = OpLoad %v3float %19
%23 = OpCompositeExtract %float %22 2
%24 = OpLoad %v3float %19
%25 = OpCompositeExtract %float %24 0
%26 = OpFSub %float %23 %25
%28 = OpFOrdGreaterThanEqual %bool %26 %float_0_00100000005
OpSelectionMerge %31 None
OpBranchConditional %28 %29 %30
%29 = OpLabel
%33 = OpLoad %float %20
%34 = OpLoad %v3float %19
%35 = OpCompositeExtract %float %34 1
%36 = OpLoad %v3float %19
%37 = OpCompositeExtract %float %36 0
%38 = OpFSub %float %35 %37
%39 = OpFMul %float %33 %38
%40 = OpLoad %v3float %19
%41 = OpCompositeExtract %float %40 2
%42 = OpLoad %v3float %19
%43 = OpCompositeExtract %float %42 0
%44 = OpFSub %float %41 %43
%45 = OpFDiv %float %39 %44
%46 = OpLoad %float %20
%47 = OpCompositeConstruct %v3float %float_0 %45 %46
OpReturnValue %47
%30 = OpLabel
OpReturnValue %48
%31 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_hslc_h4h4h4hb = OpFunction %v4float None %49
%52 = OpFunctionParameter %_ptr_Function_v4float
%53 = OpFunctionParameter %_ptr_Function_v4float
%54 = OpFunctionParameter %_ptr_Function_float
%55 = OpFunctionParameter %_ptr_Function_bool
%56 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%_1_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_2_sat = OpVariable %_ptr_Function_float Function
%126 = OpVariable %_ptr_Function_v3float Function
%128 = OpVariable %_ptr_Function_float Function
%140 = OpVariable %_ptr_Function_v3float Function
%142 = OpVariable %_ptr_Function_float Function
%147 = OpVariable %_ptr_Function_v3float Function
%149 = OpVariable %_ptr_Function_float Function
%162 = OpVariable %_ptr_Function_v3float Function
%164 = OpVariable %_ptr_Function_float Function
%177 = OpVariable %_ptr_Function_v3float Function
%179 = OpVariable %_ptr_Function_float Function
%184 = OpVariable %_ptr_Function_v3float Function
%186 = OpVariable %_ptr_Function_float Function
%_3_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_4_lum = OpVariable %_ptr_Function_float Function
%_5_result = OpVariable %_ptr_Function_v3float Function
%_6_minComp = OpVariable %_ptr_Function_float Function
%_7_maxComp = OpVariable %_ptr_Function_float Function
%58 = OpLoad %v4float %53
%59 = OpCompositeExtract %float %58 3
%60 = OpLoad %v4float %52
%61 = OpCompositeExtract %float %60 3
%62 = OpFMul %float %59 %61
OpStore %alpha %62
%64 = OpLoad %v4float %52
%65 = OpVectorShuffle %v3float %64 %64 0 1 2
%66 = OpLoad %v4float %53
%67 = OpCompositeExtract %float %66 3
%68 = OpVectorTimesScalar %v3float %65 %67
OpStore %sda %68
%70 = OpLoad %v4float %53
%71 = OpVectorShuffle %v3float %70 %70 0 1 2
%72 = OpLoad %v4float %52
%73 = OpCompositeExtract %float %72 3
%74 = OpVectorTimesScalar %v3float %71 %73
OpStore %dsa %74
%77 = OpLoad %v3float %sda
%78 = OpLoad %v3float %dsa
%79 = OpLoad %float %54
%80 = OpCompositeConstruct %v3float %79 %79 %79
%76 = OpExtInst %v3float %1 FMix %77 %78 %80
OpStore %l %76
%83 = OpLoad %v3float %dsa
%84 = OpLoad %v3float %sda
%85 = OpLoad %float %54
%86 = OpCompositeConstruct %v3float %85 %85 %85
%82 = OpExtInst %v3float %1 FMix %83 %84 %86
OpStore %r %82
%87 = OpLoad %bool %55
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%94 = OpLoad %v3float %r
%95 = OpCompositeExtract %float %94 0
%96 = OpLoad %v3float %r
%97 = OpCompositeExtract %float %96 1
%93 = OpExtInst %float %1 FMax %95 %97
%98 = OpLoad %v3float %r
%99 = OpCompositeExtract %float %98 2
%92 = OpExtInst %float %1 FMax %93 %99
%102 = OpLoad %v3float %r
%103 = OpCompositeExtract %float %102 0
%104 = OpLoad %v3float %r
%105 = OpCompositeExtract %float %104 1
%101 = OpExtInst %float %1 FMin %103 %105
%106 = OpLoad %v3float %r
%107 = OpCompositeExtract %float %106 2
%100 = OpExtInst %float %1 FMin %101 %107
%108 = OpFSub %float %92 %100
OpStore %_2_sat %108
%109 = OpLoad %v3float %l
%110 = OpCompositeExtract %float %109 0
%111 = OpLoad %v3float %l
%112 = OpCompositeExtract %float %111 1
%113 = OpFOrdLessThanEqual %bool %110 %112
OpSelectionMerge %116 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpLoad %v3float %l
%118 = OpCompositeExtract %float %117 1
%119 = OpLoad %v3float %l
%120 = OpCompositeExtract %float %119 2
%121 = OpFOrdLessThanEqual %bool %118 %120
OpSelectionMerge %124 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%125 = OpLoad %v3float %l
OpStore %126 %125
%127 = OpLoad %float %_2_sat
OpStore %128 %127
%129 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %126 %128
OpStore %_1_blend_set_color_saturation %129
OpBranch %124
%123 = OpLabel
%130 = OpLoad %v3float %l
%131 = OpCompositeExtract %float %130 0
%132 = OpLoad %v3float %l
%133 = OpCompositeExtract %float %132 2
%134 = OpFOrdLessThanEqual %bool %131 %133
OpSelectionMerge %137 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%138 = OpLoad %v3float %l
%139 = OpVectorShuffle %v3float %138 %138 0 2 1
OpStore %140 %139
%141 = OpLoad %float %_2_sat
OpStore %142 %141
%143 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %140 %142
%144 = OpVectorShuffle %v3float %143 %143 0 2 1
OpStore %_1_blend_set_color_saturation %144
OpBranch %137
%136 = OpLabel
%145 = OpLoad %v3float %l
%146 = OpVectorShuffle %v3float %145 %145 2 0 1
OpStore %147 %146
%148 = OpLoad %float %_2_sat
OpStore %149 %148
%150 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %147 %149
%151 = OpVectorShuffle %v3float %150 %150 1 2 0
OpStore %_1_blend_set_color_saturation %151
OpBranch %137
%137 = OpLabel
OpBranch %124
%124 = OpLabel
OpBranch %116
%115 = OpLabel
%152 = OpLoad %v3float %l
%153 = OpCompositeExtract %float %152 0
%154 = OpLoad %v3float %l
%155 = OpCompositeExtract %float %154 2
%156 = OpFOrdLessThanEqual %bool %153 %155
OpSelectionMerge %159 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%160 = OpLoad %v3float %l
%161 = OpVectorShuffle %v3float %160 %160 1 0 2
OpStore %162 %161
%163 = OpLoad %float %_2_sat
OpStore %164 %163
%165 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %162 %164
%166 = OpVectorShuffle %v3float %165 %165 1 0 2
OpStore %_1_blend_set_color_saturation %166
OpBranch %159
%158 = OpLabel
%167 = OpLoad %v3float %l
%168 = OpCompositeExtract %float %167 1
%169 = OpLoad %v3float %l
%170 = OpCompositeExtract %float %169 2
%171 = OpFOrdLessThanEqual %bool %168 %170
OpSelectionMerge %174 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%175 = OpLoad %v3float %l
%176 = OpVectorShuffle %v3float %175 %175 1 2 0
OpStore %177 %176
%178 = OpLoad %float %_2_sat
OpStore %179 %178
%180 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %177 %179
%181 = OpVectorShuffle %v3float %180 %180 2 0 1
OpStore %_1_blend_set_color_saturation %181
OpBranch %174
%173 = OpLabel
%182 = OpLoad %v3float %l
%183 = OpVectorShuffle %v3float %182 %182 2 1 0
OpStore %184 %183
%185 = OpLoad %float %_2_sat
OpStore %186 %185
%187 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %184 %186
%188 = OpVectorShuffle %v3float %187 %187 2 1 0
OpStore %_1_blend_set_color_saturation %188
OpBranch %174
%174 = OpLabel
OpBranch %159
%159 = OpLabel
OpBranch %116
%116 = OpLabel
%189 = OpLoad %v3float %_1_blend_set_color_saturation
OpStore %l %189
%190 = OpLoad %v3float %dsa
OpStore %r %190
OpBranch %89
%89 = OpLabel
%198 = OpLoad %v3float %r
%193 = OpDot %float %197 %198
OpStore %_4_lum %193
%200 = OpLoad %float %_4_lum
%202 = OpLoad %v3float %l
%201 = OpDot %float %197 %202
%203 = OpFSub %float %200 %201
%204 = OpLoad %v3float %l
%205 = OpCompositeConstruct %v3float %203 %203 %203
%206 = OpFAdd %v3float %205 %204
OpStore %_5_result %206
%210 = OpLoad %v3float %_5_result
%211 = OpCompositeExtract %float %210 0
%212 = OpLoad %v3float %_5_result
%213 = OpCompositeExtract %float %212 1
%209 = OpExtInst %float %1 FMin %211 %213
%214 = OpLoad %v3float %_5_result
%215 = OpCompositeExtract %float %214 2
%208 = OpExtInst %float %1 FMin %209 %215
OpStore %_6_minComp %208
%219 = OpLoad %v3float %_5_result
%220 = OpCompositeExtract %float %219 0
%221 = OpLoad %v3float %_5_result
%222 = OpCompositeExtract %float %221 1
%218 = OpExtInst %float %1 FMax %220 %222
%223 = OpLoad %v3float %_5_result
%224 = OpCompositeExtract %float %223 2
%217 = OpExtInst %float %1 FMax %218 %224
OpStore %_7_maxComp %217
%226 = OpLoad %float %_6_minComp
%227 = OpFOrdLessThan %bool %226 %float_0
OpSelectionMerge %229 None
OpBranchConditional %227 %228 %229
%228 = OpLabel
%230 = OpLoad %float %_4_lum
%231 = OpLoad %float %_6_minComp
%232 = OpFUnordNotEqual %bool %230 %231
OpBranch %229
%229 = OpLabel
%233 = OpPhi %bool %false %89 %232 %228
OpSelectionMerge %235 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
%236 = OpLoad %float %_4_lum
%237 = OpLoad %v3float %_5_result
%238 = OpLoad %float %_4_lum
%239 = OpCompositeConstruct %v3float %238 %238 %238
%240 = OpFSub %v3float %237 %239
%241 = OpLoad %float %_4_lum
%242 = OpLoad %float %_4_lum
%243 = OpLoad %float %_6_minComp
%244 = OpFSub %float %242 %243
%245 = OpFDiv %float %241 %244
%246 = OpVectorTimesScalar %v3float %240 %245
%247 = OpCompositeConstruct %v3float %236 %236 %236
%248 = OpFAdd %v3float %247 %246
OpStore %_5_result %248
OpBranch %235
%235 = OpLabel
%249 = OpLoad %float %_7_maxComp
%250 = OpLoad %float %alpha
%251 = OpFOrdGreaterThan %bool %249 %250
OpSelectionMerge %253 None
OpBranchConditional %251 %252 %253
%252 = OpLabel
%254 = OpLoad %float %_7_maxComp
%255 = OpLoad %float %_4_lum
%256 = OpFUnordNotEqual %bool %254 %255
OpBranch %253
%253 = OpLabel
%257 = OpPhi %bool %false %235 %256 %252
OpSelectionMerge %260 None
OpBranchConditional %257 %258 %259
%258 = OpLabel
%261 = OpLoad %float %_4_lum
%262 = OpLoad %v3float %_5_result
%263 = OpLoad %float %_4_lum
%264 = OpCompositeConstruct %v3float %263 %263 %263
%265 = OpFSub %v3float %262 %264
%266 = OpLoad %float %alpha
%267 = OpLoad %float %_4_lum
%268 = OpFSub %float %266 %267
%269 = OpVectorTimesScalar %v3float %265 %268
%270 = OpLoad %float %_7_maxComp
%271 = OpLoad %float %_4_lum
%272 = OpFSub %float %270 %271
%274 = OpFDiv %float %float_1 %272
%275 = OpVectorTimesScalar %v3float %269 %274
%276 = OpCompositeConstruct %v3float %261 %261 %261
%277 = OpFAdd %v3float %276 %275
OpStore %_3_blend_set_color_luminance %277
OpBranch %260
%259 = OpLabel
%278 = OpLoad %v3float %_5_result
OpStore %_3_blend_set_color_luminance %278
OpBranch %260
%260 = OpLabel
%279 = OpLoad %v3float %_3_blend_set_color_luminance
%280 = OpLoad %v4float %53
%281 = OpVectorShuffle %v3float %280 %280 0 1 2
%282 = OpFAdd %v3float %279 %281
%283 = OpLoad %v3float %dsa
%284 = OpFSub %v3float %282 %283
%285 = OpLoad %v4float %52
%286 = OpVectorShuffle %v3float %285 %285 0 1 2
%287 = OpFAdd %v3float %284 %286
%288 = OpLoad %v3float %sda
%289 = OpFSub %v3float %287 %288
%290 = OpCompositeExtract %float %289 0
%291 = OpCompositeExtract %float %289 1
%292 = OpCompositeExtract %float %289 2
%293 = OpLoad %v4float %52
%294 = OpCompositeExtract %float %293 3
%295 = OpLoad %v4float %53
%296 = OpCompositeExtract %float %295 3
%297 = OpFAdd %float %294 %296
%298 = OpLoad %float %alpha
%299 = OpFSub %float %297 %298
%300 = OpCompositeConstruct %v4float %290 %291 %292 %299
OpReturnValue %300
OpFunctionEnd
%main = OpFunction %void None %302
%303 = OpLabel
%309 = OpVariable %_ptr_Function_v4float Function
%313 = OpVariable %_ptr_Function_v4float Function
%314 = OpVariable %_ptr_Function_float Function
%316 = OpVariable %_ptr_Function_bool Function
%304 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%308 = OpLoad %v4float %304
OpStore %309 %308
%310 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%312 = OpLoad %v4float %310
OpStore %313 %312
OpStore %314 %float_1
OpStore %316 %true
%317 = OpFunctionCall %v4float %blend_hslc_h4h4h4hb %309 %313 %314 %316
OpStore %sk_FragColor %317
OpReturn
OpFunctionEnd
