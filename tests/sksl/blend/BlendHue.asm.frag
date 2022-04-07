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
OpName %blend_hslc_h4h4h4bb "blend_hslc_h4h4h4bb"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_2_hueLumColor "_2_hueLumColor"
OpName %_3_midPt "_3_midPt"
OpName %_4_satPt "_4_satPt"
OpName %_5_sat "_5_sat"
OpName %_6_mid "_6_mid"
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
OpDecorate %44 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %_2_hueLumColor RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %_3_midPt RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %_4_satPt RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %_5_sat RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %_6_mid RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %_7_lum RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %_8_result RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %_9_minComp RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %_10_maxComp RelaxedPrecision
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
OpDecorate %306 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%14 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_bool %_ptr_Function_bool
%_ptr_Function_float = OpTypePointer Function %float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_0 = OpConstant %float 0
%68 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_1 = OpConstant %float 1
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%200 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%void = OpTypeVoid
%302 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%true = OpConstantTrue %bool
%blend_hslc_h4h4h4bb = OpFunction %v4float None %14
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpFunctionParameter %_ptr_Function_v4float
%19 = OpFunctionParameter %_ptr_Function_bool
%20 = OpFunctionParameter %_ptr_Function_bool
%21 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%45 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%54 = OpVariable %_ptr_Function_v3float Function
%_2_hueLumColor = OpVariable %_ptr_Function_v3float Function
%_3_midPt = OpVariable %_ptr_Function_v3float Function
%_4_satPt = OpVariable %_ptr_Function_v3float Function
%_5_sat = OpVariable %_ptr_Function_float Function
%_6_mid = OpVariable %_ptr_Function_float Function
%_7_lum = OpVariable %_ptr_Function_float Function
%_8_result = OpVariable %_ptr_Function_v3float Function
%_9_minComp = OpVariable %_ptr_Function_float Function
%_10_maxComp = OpVariable %_ptr_Function_float Function
%24 = OpLoad %v4float %18
%25 = OpCompositeExtract %float %24 3
%26 = OpLoad %v4float %17
%27 = OpCompositeExtract %float %26 3
%28 = OpFMul %float %25 %27
OpStore %alpha %28
%32 = OpLoad %v4float %17
%33 = OpVectorShuffle %v3float %32 %32 0 1 2
%34 = OpLoad %v4float %18
%35 = OpCompositeExtract %float %34 3
%36 = OpVectorTimesScalar %v3float %33 %35
OpStore %sda %36
%38 = OpLoad %v4float %18
%39 = OpVectorShuffle %v3float %38 %38 0 1 2
%40 = OpLoad %v4float %17
%41 = OpCompositeExtract %float %40 3
%42 = OpVectorTimesScalar %v3float %39 %41
OpStore %dsa %42
%44 = OpLoad %bool %19
OpSelectionMerge %48 None
OpBranchConditional %44 %46 %47
%46 = OpLabel
%49 = OpLoad %v3float %dsa
OpStore %45 %49
OpBranch %48
%47 = OpLabel
%50 = OpLoad %v3float %sda
OpStore %45 %50
OpBranch %48
%48 = OpLabel
%51 = OpLoad %v3float %45
OpStore %l %51
%53 = OpLoad %bool %19
OpSelectionMerge %57 None
OpBranchConditional %53 %55 %56
%55 = OpLabel
%58 = OpLoad %v3float %sda
OpStore %54 %58
OpBranch %57
%56 = OpLabel
%59 = OpLoad %v3float %dsa
OpStore %54 %59
OpBranch %57
%57 = OpLabel
%60 = OpLoad %v3float %54
OpStore %r %60
%61 = OpLoad %bool %20
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpLoad %v3float %l
OpStore %_2_hueLumColor %65
OpStore %_3_midPt %68
OpStore %_4_satPt %68
%70 = OpLoad %v3float %_2_hueLumColor
%71 = OpCompositeExtract %float %70 0
%72 = OpLoad %v3float %_2_hueLumColor
%73 = OpCompositeExtract %float %72 1
%74 = OpFOrdLessThanEqual %bool %71 %73
OpSelectionMerge %77 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpLoad %v3float %_2_hueLumColor
%79 = OpCompositeExtract %float %78 1
%80 = OpLoad %v3float %_2_hueLumColor
%81 = OpCompositeExtract %float %80 2
%82 = OpFOrdLessThanEqual %bool %79 %81
OpSelectionMerge %85 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpLoad %v3float %_2_hueLumColor
%87 = OpLoad %v3float %_2_hueLumColor
%88 = OpVectorShuffle %v3float %87 %87 0 0 0
%89 = OpFSub %v3float %86 %88
OpStore %_2_hueLumColor %89
%91 = OpAccessChain %_ptr_Function_float %_4_satPt %int_2
OpStore %91 %float_1
%94 = OpAccessChain %_ptr_Function_float %_3_midPt %int_1
OpStore %94 %float_1
OpBranch %85
%84 = OpLabel
%96 = OpLoad %v3float %_2_hueLumColor
%97 = OpCompositeExtract %float %96 0
%98 = OpLoad %v3float %_2_hueLumColor
%99 = OpCompositeExtract %float %98 2
%100 = OpFOrdLessThanEqual %bool %97 %99
OpSelectionMerge %103 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpLoad %v3float %_2_hueLumColor
%105 = OpVectorShuffle %v3float %104 %104 0 2 1
%106 = OpLoad %v3float %_2_hueLumColor
%107 = OpVectorShuffle %v3float %106 %106 0 0 0
%108 = OpFSub %v3float %105 %107
OpStore %_2_hueLumColor %108
%109 = OpAccessChain %_ptr_Function_float %_4_satPt %int_1
OpStore %109 %float_1
%110 = OpAccessChain %_ptr_Function_float %_3_midPt %int_2
OpStore %110 %float_1
OpBranch %103
%102 = OpLabel
%111 = OpLoad %v3float %_2_hueLumColor
%112 = OpVectorShuffle %v3float %111 %111 2 0 1
%113 = OpLoad %v3float %_2_hueLumColor
%114 = OpVectorShuffle %v3float %113 %113 2 2 2
%115 = OpFSub %v3float %112 %114
OpStore %_2_hueLumColor %115
%116 = OpAccessChain %_ptr_Function_float %_4_satPt %int_1
OpStore %116 %float_1
%117 = OpAccessChain %_ptr_Function_float %_3_midPt %int_0
OpStore %117 %float_1
OpBranch %103
%103 = OpLabel
OpBranch %85
%85 = OpLabel
OpBranch %77
%76 = OpLabel
%119 = OpLoad %v3float %_2_hueLumColor
%120 = OpCompositeExtract %float %119 0
%121 = OpLoad %v3float %_2_hueLumColor
%122 = OpCompositeExtract %float %121 2
%123 = OpFOrdLessThanEqual %bool %120 %122
OpSelectionMerge %126 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%127 = OpLoad %v3float %_2_hueLumColor
%128 = OpVectorShuffle %v3float %127 %127 1 0 2
%129 = OpLoad %v3float %_2_hueLumColor
%130 = OpVectorShuffle %v3float %129 %129 1 1 1
%131 = OpFSub %v3float %128 %130
OpStore %_2_hueLumColor %131
%132 = OpAccessChain %_ptr_Function_float %_4_satPt %int_2
OpStore %132 %float_1
%133 = OpAccessChain %_ptr_Function_float %_3_midPt %int_0
OpStore %133 %float_1
OpBranch %126
%125 = OpLabel
%134 = OpLoad %v3float %_2_hueLumColor
%135 = OpCompositeExtract %float %134 1
%136 = OpLoad %v3float %_2_hueLumColor
%137 = OpCompositeExtract %float %136 2
%138 = OpFOrdLessThanEqual %bool %135 %137
OpSelectionMerge %141 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpLoad %v3float %_2_hueLumColor
%143 = OpVectorShuffle %v3float %142 %142 1 2 0
%144 = OpLoad %v3float %_2_hueLumColor
%145 = OpVectorShuffle %v3float %144 %144 1 1 1
%146 = OpFSub %v3float %143 %145
OpStore %_2_hueLumColor %146
%147 = OpAccessChain %_ptr_Function_float %_4_satPt %int_0
OpStore %147 %float_1
%148 = OpAccessChain %_ptr_Function_float %_3_midPt %int_2
OpStore %148 %float_1
OpBranch %141
%140 = OpLabel
%149 = OpLoad %v3float %_2_hueLumColor
%150 = OpVectorShuffle %v3float %149 %149 2 1 0
%151 = OpLoad %v3float %_2_hueLumColor
%152 = OpVectorShuffle %v3float %151 %151 2 2 2
%153 = OpFSub %v3float %150 %152
OpStore %_2_hueLumColor %153
%154 = OpAccessChain %_ptr_Function_float %_4_satPt %int_0
OpStore %154 %float_1
%155 = OpAccessChain %_ptr_Function_float %_3_midPt %int_1
OpStore %155 %float_1
OpBranch %141
%141 = OpLabel
OpBranch %126
%126 = OpLabel
OpBranch %77
%77 = OpLabel
%156 = OpLoad %v3float %_2_hueLumColor
%157 = OpCompositeExtract %float %156 2
%158 = OpFOrdGreaterThan %bool %157 %float_0
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%164 = OpLoad %v3float %r
%165 = OpCompositeExtract %float %164 0
%166 = OpLoad %v3float %r
%167 = OpCompositeExtract %float %166 1
%163 = OpExtInst %float %1 FMax %165 %167
%168 = OpLoad %v3float %r
%169 = OpCompositeExtract %float %168 2
%162 = OpExtInst %float %1 FMax %163 %169
%172 = OpLoad %v3float %r
%173 = OpCompositeExtract %float %172 0
%174 = OpLoad %v3float %r
%175 = OpCompositeExtract %float %174 1
%171 = OpExtInst %float %1 FMin %173 %175
%176 = OpLoad %v3float %r
%177 = OpCompositeExtract %float %176 2
%170 = OpExtInst %float %1 FMin %171 %177
%178 = OpFSub %float %162 %170
OpStore %_5_sat %178
%180 = OpLoad %v3float %_2_hueLumColor
%181 = OpCompositeExtract %float %180 1
%182 = OpLoad %v3float %_2_hueLumColor
%183 = OpCompositeExtract %float %182 2
%184 = OpFDiv %float %181 %183
OpStore %_6_mid %184
%185 = OpLoad %float %_5_sat
%186 = OpLoad %float %_6_mid
%187 = OpCompositeConstruct %v3float %186 %186 %186
%188 = OpLoad %v3float %_3_midPt
%189 = OpFMul %v3float %187 %188
%190 = OpLoad %v3float %_4_satPt
%191 = OpFAdd %v3float %189 %190
%192 = OpVectorTimesScalar %v3float %191 %185
OpStore %_2_hueLumColor %192
OpBranch %160
%160 = OpLabel
%193 = OpLoad %v3float %_2_hueLumColor
OpStore %l %193
%194 = OpLoad %v3float %dsa
OpStore %r %194
OpBranch %63
%63 = OpLabel
%201 = OpLoad %v3float %r
%196 = OpDot %float %200 %201
OpStore %_7_lum %196
%203 = OpLoad %float %_7_lum
%205 = OpLoad %v3float %l
%204 = OpDot %float %200 %205
%206 = OpFSub %float %203 %204
%207 = OpLoad %v3float %l
%208 = OpCompositeConstruct %v3float %206 %206 %206
%209 = OpFAdd %v3float %208 %207
OpStore %_8_result %209
%213 = OpLoad %v3float %_8_result
%214 = OpCompositeExtract %float %213 0
%215 = OpLoad %v3float %_8_result
%216 = OpCompositeExtract %float %215 1
%212 = OpExtInst %float %1 FMin %214 %216
%217 = OpLoad %v3float %_8_result
%218 = OpCompositeExtract %float %217 2
%211 = OpExtInst %float %1 FMin %212 %218
OpStore %_9_minComp %211
%222 = OpLoad %v3float %_8_result
%223 = OpCompositeExtract %float %222 0
%224 = OpLoad %v3float %_8_result
%225 = OpCompositeExtract %float %224 1
%221 = OpExtInst %float %1 FMax %223 %225
%226 = OpLoad %v3float %_8_result
%227 = OpCompositeExtract %float %226 2
%220 = OpExtInst %float %1 FMax %221 %227
OpStore %_10_maxComp %220
%229 = OpLoad %float %_9_minComp
%230 = OpFOrdLessThan %bool %229 %float_0
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %float %_7_lum
%234 = OpLoad %float %_9_minComp
%235 = OpFUnordNotEqual %bool %233 %234
OpBranch %232
%232 = OpLabel
%236 = OpPhi %bool %false %63 %235 %231
OpSelectionMerge %238 None
OpBranchConditional %236 %237 %238
%237 = OpLabel
%239 = OpLoad %float %_7_lum
%240 = OpLoad %v3float %_8_result
%241 = OpLoad %float %_7_lum
%242 = OpCompositeConstruct %v3float %241 %241 %241
%243 = OpFSub %v3float %240 %242
%244 = OpLoad %float %_7_lum
%245 = OpLoad %float %_7_lum
%246 = OpLoad %float %_9_minComp
%247 = OpFSub %float %245 %246
%248 = OpFDiv %float %244 %247
%249 = OpVectorTimesScalar %v3float %243 %248
%250 = OpCompositeConstruct %v3float %239 %239 %239
%251 = OpFAdd %v3float %250 %249
OpStore %_8_result %251
OpBranch %238
%238 = OpLabel
%252 = OpLoad %float %_10_maxComp
%253 = OpLoad %float %alpha
%254 = OpFOrdGreaterThan %bool %252 %253
OpSelectionMerge %256 None
OpBranchConditional %254 %255 %256
%255 = OpLabel
%257 = OpLoad %float %_10_maxComp
%258 = OpLoad %float %_7_lum
%259 = OpFUnordNotEqual %bool %257 %258
OpBranch %256
%256 = OpLabel
%260 = OpPhi %bool %false %238 %259 %255
OpSelectionMerge %262 None
OpBranchConditional %260 %261 %262
%261 = OpLabel
%263 = OpLoad %float %_7_lum
%264 = OpLoad %v3float %_8_result
%265 = OpLoad %float %_7_lum
%266 = OpCompositeConstruct %v3float %265 %265 %265
%267 = OpFSub %v3float %264 %266
%268 = OpLoad %float %alpha
%269 = OpLoad %float %_7_lum
%270 = OpFSub %float %268 %269
%271 = OpVectorTimesScalar %v3float %267 %270
%272 = OpLoad %float %_10_maxComp
%273 = OpLoad %float %_7_lum
%274 = OpFSub %float %272 %273
%275 = OpFDiv %float %float_1 %274
%276 = OpVectorTimesScalar %v3float %271 %275
%277 = OpCompositeConstruct %v3float %263 %263 %263
%278 = OpFAdd %v3float %277 %276
OpStore %_8_result %278
OpBranch %262
%262 = OpLabel
%279 = OpLoad %v3float %_8_result
%280 = OpLoad %v4float %18
%281 = OpVectorShuffle %v3float %280 %280 0 1 2
%282 = OpFAdd %v3float %279 %281
%283 = OpLoad %v3float %dsa
%284 = OpFSub %v3float %282 %283
%285 = OpLoad %v4float %17
%286 = OpVectorShuffle %v3float %285 %285 0 1 2
%287 = OpFAdd %v3float %284 %286
%288 = OpLoad %v3float %sda
%289 = OpFSub %v3float %287 %288
%290 = OpCompositeExtract %float %289 0
%291 = OpCompositeExtract %float %289 1
%292 = OpCompositeExtract %float %289 2
%293 = OpLoad %v4float %17
%294 = OpCompositeExtract %float %293 3
%295 = OpLoad %v4float %18
%296 = OpCompositeExtract %float %295 3
%297 = OpFAdd %float %294 %296
%298 = OpLoad %float %alpha
%299 = OpFSub %float %297 %298
%300 = OpCompositeConstruct %v4float %290 %291 %292 %299
OpReturnValue %300
OpFunctionEnd
%main = OpFunction %void None %302
%303 = OpLabel
%307 = OpVariable %_ptr_Function_v4float Function
%310 = OpVariable %_ptr_Function_v4float Function
%311 = OpVariable %_ptr_Function_bool Function
%313 = OpVariable %_ptr_Function_bool Function
%304 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%306 = OpLoad %v4float %304
OpStore %307 %306
%308 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%309 = OpLoad %v4float %308
OpStore %310 %309
OpStore %311 %false
OpStore %313 %true
%314 = OpFunctionCall %v4float %blend_hslc_h4h4h4bb %307 %310 %311 %313
OpStore %sk_FragColor %314
OpReturn
OpFunctionEnd
