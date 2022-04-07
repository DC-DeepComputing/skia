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
OpName %_3_swizzle "_3_swizzle"
OpName %_4_sat "_4_sat"
OpName %_5_mid "_5_mid"
OpName %_6_m "_6_m"
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
OpDecorate %_3_swizzle RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %_4_sat RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %_5_mid RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %_6_m RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %_7_lum RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %_8_result RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %_9_minComp RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %_10_maxComp RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
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
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
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
OpDecorate %274 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%70 = OpConstantComposite %v3float %float_n1 %float_0 %float_1
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%166 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%void = OpTypeVoid
%268 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%_3_swizzle = OpVariable %_ptr_Function_v3float Function
%_4_sat = OpVariable %_ptr_Function_float Function
%_5_mid = OpVariable %_ptr_Function_float Function
%_6_m = OpVariable %_ptr_Function_mat3v3float Function
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
OpStore %_3_swizzle %70
%71 = OpLoad %v3float %_2_hueLumColor
%72 = OpCompositeExtract %float %71 0
%73 = OpLoad %v3float %_2_hueLumColor
%74 = OpCompositeExtract %float %73 1
%75 = OpFOrdGreaterThan %bool %72 %74
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpLoad %v3float %_2_hueLumColor
%79 = OpVectorShuffle %v3float %78 %78 1 0 2
OpStore %_2_hueLumColor %79
%80 = OpLoad %v3float %_3_swizzle
%81 = OpVectorShuffle %v3float %80 %80 1 0 2
OpStore %_3_swizzle %81
OpBranch %77
%77 = OpLabel
%82 = OpLoad %v3float %_2_hueLumColor
%83 = OpCompositeExtract %float %82 0
%84 = OpLoad %v3float %_2_hueLumColor
%85 = OpCompositeExtract %float %84 2
%86 = OpFOrdGreaterThan %bool %83 %85
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpLoad %v3float %_2_hueLumColor
%90 = OpVectorShuffle %v3float %89 %89 2 1 0
OpStore %_2_hueLumColor %90
%91 = OpLoad %v3float %_3_swizzle
%92 = OpVectorShuffle %v3float %91 %91 2 1 0
OpStore %_3_swizzle %92
OpBranch %88
%88 = OpLabel
%93 = OpLoad %v3float %_2_hueLumColor
%94 = OpCompositeExtract %float %93 1
%95 = OpLoad %v3float %_2_hueLumColor
%96 = OpCompositeExtract %float %95 2
%97 = OpFOrdGreaterThan %bool %94 %96
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpLoad %v3float %_2_hueLumColor
%101 = OpVectorShuffle %v3float %100 %100 0 2 1
OpStore %_2_hueLumColor %101
%102 = OpLoad %v3float %_3_swizzle
%103 = OpVectorShuffle %v3float %102 %102 0 2 1
OpStore %_3_swizzle %103
OpBranch %99
%99 = OpLabel
%104 = OpLoad %v3float %_2_hueLumColor
%105 = OpLoad %v3float %_2_hueLumColor
%106 = OpVectorShuffle %v3float %105 %105 0 0 0
%107 = OpFSub %v3float %104 %106
OpStore %_2_hueLumColor %107
%108 = OpLoad %v3float %_2_hueLumColor
%109 = OpCompositeExtract %float %108 2
%110 = OpFOrdGreaterThan %bool %109 %float_0
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%116 = OpLoad %v3float %r
%117 = OpCompositeExtract %float %116 0
%118 = OpLoad %v3float %r
%119 = OpCompositeExtract %float %118 1
%115 = OpExtInst %float %1 FMax %117 %119
%120 = OpLoad %v3float %r
%121 = OpCompositeExtract %float %120 2
%114 = OpExtInst %float %1 FMax %115 %121
%124 = OpLoad %v3float %r
%125 = OpCompositeExtract %float %124 0
%126 = OpLoad %v3float %r
%127 = OpCompositeExtract %float %126 1
%123 = OpExtInst %float %1 FMin %125 %127
%128 = OpLoad %v3float %r
%129 = OpCompositeExtract %float %128 2
%122 = OpExtInst %float %1 FMin %123 %129
%130 = OpFSub %float %114 %122
OpStore %_4_sat %130
%132 = OpLoad %float %_4_sat
%133 = OpLoad %v3float %_2_hueLumColor
%134 = OpCompositeExtract %float %133 1
%135 = OpLoad %v3float %_2_hueLumColor
%136 = OpCompositeExtract %float %135 2
%137 = OpFDiv %float %134 %136
%138 = OpFMul %float %132 %137
OpStore %_5_mid %138
%143 = OpLoad %v3float %_3_swizzle
%144 = OpFNegate %v3float %143
%145 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%142 = OpExtInst %v3float %1 FMax %144 %145
%147 = OpLoad %v3float %_3_swizzle
%146 = OpExtInst %v3float %1 FAbs %147
%148 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%149 = OpFSub %v3float %148 %146
%151 = OpLoad %v3float %_3_swizzle
%152 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%150 = OpExtInst %v3float %1 FMax %151 %152
%153 = OpCompositeConstruct %mat3v3float %142 %149 %150
OpStore %_6_m %153
%154 = OpLoad %float %_5_mid
%155 = OpLoad %float %_4_sat
%156 = OpCompositeConstruct %v3float %float_0 %154 %155
%157 = OpLoad %mat3v3float %_6_m
%158 = OpVectorTimesMatrix %v3float %156 %157
OpStore %_2_hueLumColor %158
OpBranch %112
%112 = OpLabel
%159 = OpLoad %v3float %_2_hueLumColor
OpStore %l %159
%160 = OpLoad %v3float %dsa
OpStore %r %160
OpBranch %63
%63 = OpLabel
%167 = OpLoad %v3float %r
%162 = OpDot %float %166 %167
OpStore %_7_lum %162
%169 = OpLoad %float %_7_lum
%171 = OpLoad %v3float %l
%170 = OpDot %float %166 %171
%172 = OpFSub %float %169 %170
%173 = OpLoad %v3float %l
%174 = OpCompositeConstruct %v3float %172 %172 %172
%175 = OpFAdd %v3float %174 %173
OpStore %_8_result %175
%179 = OpLoad %v3float %_8_result
%180 = OpCompositeExtract %float %179 0
%181 = OpLoad %v3float %_8_result
%182 = OpCompositeExtract %float %181 1
%178 = OpExtInst %float %1 FMin %180 %182
%183 = OpLoad %v3float %_8_result
%184 = OpCompositeExtract %float %183 2
%177 = OpExtInst %float %1 FMin %178 %184
OpStore %_9_minComp %177
%188 = OpLoad %v3float %_8_result
%189 = OpCompositeExtract %float %188 0
%190 = OpLoad %v3float %_8_result
%191 = OpCompositeExtract %float %190 1
%187 = OpExtInst %float %1 FMax %189 %191
%192 = OpLoad %v3float %_8_result
%193 = OpCompositeExtract %float %192 2
%186 = OpExtInst %float %1 FMax %187 %193
OpStore %_10_maxComp %186
%195 = OpLoad %float %_9_minComp
%196 = OpFOrdLessThan %bool %195 %float_0
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%199 = OpLoad %float %_7_lum
%200 = OpLoad %float %_9_minComp
%201 = OpFUnordNotEqual %bool %199 %200
OpBranch %198
%198 = OpLabel
%202 = OpPhi %bool %false %63 %201 %197
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpLoad %float %_7_lum
%206 = OpLoad %v3float %_8_result
%207 = OpLoad %float %_7_lum
%208 = OpCompositeConstruct %v3float %207 %207 %207
%209 = OpFSub %v3float %206 %208
%210 = OpLoad %float %_7_lum
%211 = OpLoad %float %_7_lum
%212 = OpLoad %float %_9_minComp
%213 = OpFSub %float %211 %212
%214 = OpFDiv %float %210 %213
%215 = OpVectorTimesScalar %v3float %209 %214
%216 = OpCompositeConstruct %v3float %205 %205 %205
%217 = OpFAdd %v3float %216 %215
OpStore %_8_result %217
OpBranch %204
%204 = OpLabel
%218 = OpLoad %float %_10_maxComp
%219 = OpLoad %float %alpha
%220 = OpFOrdGreaterThan %bool %218 %219
OpSelectionMerge %222 None
OpBranchConditional %220 %221 %222
%221 = OpLabel
%223 = OpLoad %float %_10_maxComp
%224 = OpLoad %float %_7_lum
%225 = OpFUnordNotEqual %bool %223 %224
OpBranch %222
%222 = OpLabel
%226 = OpPhi %bool %false %204 %225 %221
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%229 = OpLoad %float %_7_lum
%230 = OpLoad %v3float %_8_result
%231 = OpLoad %float %_7_lum
%232 = OpCompositeConstruct %v3float %231 %231 %231
%233 = OpFSub %v3float %230 %232
%234 = OpLoad %float %alpha
%235 = OpLoad %float %_7_lum
%236 = OpFSub %float %234 %235
%237 = OpVectorTimesScalar %v3float %233 %236
%238 = OpLoad %float %_10_maxComp
%239 = OpLoad %float %_7_lum
%240 = OpFSub %float %238 %239
%241 = OpFDiv %float %float_1 %240
%242 = OpVectorTimesScalar %v3float %237 %241
%243 = OpCompositeConstruct %v3float %229 %229 %229
%244 = OpFAdd %v3float %243 %242
OpStore %_8_result %244
OpBranch %228
%228 = OpLabel
%245 = OpLoad %v3float %_8_result
%246 = OpLoad %v4float %18
%247 = OpVectorShuffle %v3float %246 %246 0 1 2
%248 = OpFAdd %v3float %245 %247
%249 = OpLoad %v3float %dsa
%250 = OpFSub %v3float %248 %249
%251 = OpLoad %v4float %17
%252 = OpVectorShuffle %v3float %251 %251 0 1 2
%253 = OpFAdd %v3float %250 %252
%254 = OpLoad %v3float %sda
%255 = OpFSub %v3float %253 %254
%256 = OpCompositeExtract %float %255 0
%257 = OpCompositeExtract %float %255 1
%258 = OpCompositeExtract %float %255 2
%259 = OpLoad %v4float %17
%260 = OpCompositeExtract %float %259 3
%261 = OpLoad %v4float %18
%262 = OpCompositeExtract %float %261 3
%263 = OpFAdd %float %260 %262
%264 = OpLoad %float %alpha
%265 = OpFSub %float %263 %264
%266 = OpCompositeConstruct %v4float %256 %257 %258 %265
OpReturnValue %266
OpFunctionEnd
%main = OpFunction %void None %268
%269 = OpLabel
%275 = OpVariable %_ptr_Function_v4float Function
%279 = OpVariable %_ptr_Function_v4float Function
%280 = OpVariable %_ptr_Function_bool Function
%281 = OpVariable %_ptr_Function_bool Function
%270 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%274 = OpLoad %v4float %270
OpStore %275 %274
%276 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%278 = OpLoad %v4float %276
OpStore %279 %278
OpStore %280 %false
OpStore %281 %false
%282 = OpFunctionCall %v4float %blend_hslc_h4h4h4bb %275 %279 %280 %281
OpStore %sk_FragColor %282
OpReturn
OpFunctionEnd
