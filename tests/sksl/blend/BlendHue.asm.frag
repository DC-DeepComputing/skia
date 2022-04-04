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
OpName %blend_set_color_luminance_Qh3h3hh3 "blend_set_color_luminance_Qh3h3hh3"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %blend_set_color_saturation_helper_Qh3h3h "blend_set_color_saturation_helper_Qh3h3h"
OpName %blend_set_color_saturation_Qh3h3h3 "blend_set_color_saturation_Qh3h3h3"
OpName %sat "sat"
OpName %main "main"
OpName %_0_alpha "_0_alpha"
OpName %_1_sda "_1_sda"
OpName %_2_dsa "_2_dsa"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %lum RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %minComp RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %sat RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %_0_alpha RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %_1_sda RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %_2_dsa RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_float = OpTypePointer Function %float
%17 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%29 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%112 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_0_00100000005 = OpConstant %float 0.00100000005
%141 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%142 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%void = OpTypeVoid
%245 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%blend_set_color_luminance_Qh3h3hh3 = OpFunction %v3float None %17
%20 = OpFunctionParameter %_ptr_Function_v3float
%21 = OpFunctionParameter %_ptr_Function_float
%22 = OpFunctionParameter %_ptr_Function_v3float
%23 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%30 = OpLoad %v3float %22
%25 = OpDot %float %29 %30
OpStore %lum %25
%32 = OpLoad %float %lum
%34 = OpLoad %v3float %20
%33 = OpDot %float %29 %34
%35 = OpFSub %float %32 %33
%36 = OpLoad %v3float %20
%37 = OpCompositeConstruct %v3float %35 %35 %35
%38 = OpFAdd %v3float %37 %36
OpStore %result %38
%42 = OpLoad %v3float %result
%43 = OpCompositeExtract %float %42 0
%44 = OpLoad %v3float %result
%45 = OpCompositeExtract %float %44 1
%41 = OpExtInst %float %1 FMin %43 %45
%46 = OpLoad %v3float %result
%47 = OpCompositeExtract %float %46 2
%40 = OpExtInst %float %1 FMin %41 %47
OpStore %minComp %40
%51 = OpLoad %v3float %result
%52 = OpCompositeExtract %float %51 0
%53 = OpLoad %v3float %result
%54 = OpCompositeExtract %float %53 1
%50 = OpExtInst %float %1 FMax %52 %54
%55 = OpLoad %v3float %result
%56 = OpCompositeExtract %float %55 2
%49 = OpExtInst %float %1 FMax %50 %56
OpStore %maxComp %49
%58 = OpLoad %float %minComp
%60 = OpFOrdLessThan %bool %58 %float_0
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%63 = OpLoad %float %lum
%64 = OpLoad %float %minComp
%65 = OpFUnordNotEqual %bool %63 %64
OpBranch %62
%62 = OpLabel
%66 = OpPhi %bool %false %23 %65 %61
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%69 = OpLoad %float %lum
%70 = OpLoad %v3float %result
%71 = OpLoad %float %lum
%72 = OpCompositeConstruct %v3float %71 %71 %71
%73 = OpFSub %v3float %70 %72
%74 = OpLoad %float %lum
%75 = OpLoad %float %lum
%76 = OpLoad %float %minComp
%77 = OpFSub %float %75 %76
%78 = OpFDiv %float %74 %77
%79 = OpVectorTimesScalar %v3float %73 %78
%80 = OpCompositeConstruct %v3float %69 %69 %69
%81 = OpFAdd %v3float %80 %79
OpStore %result %81
OpBranch %68
%68 = OpLabel
%82 = OpLoad %float %maxComp
%83 = OpLoad %float %21
%84 = OpFOrdGreaterThan %bool %82 %83
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %float %maxComp
%88 = OpLoad %float %lum
%89 = OpFUnordNotEqual %bool %87 %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %68 %89 %85
OpSelectionMerge %93 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpLoad %float %lum
%95 = OpLoad %v3float %result
%96 = OpLoad %float %lum
%97 = OpCompositeConstruct %v3float %96 %96 %96
%98 = OpFSub %v3float %95 %97
%99 = OpLoad %float %21
%100 = OpLoad %float %lum
%101 = OpFSub %float %99 %100
%102 = OpVectorTimesScalar %v3float %98 %101
%103 = OpLoad %float %maxComp
%104 = OpLoad %float %lum
%105 = OpFSub %float %103 %104
%107 = OpFDiv %float %float_1 %105
%108 = OpVectorTimesScalar %v3float %102 %107
%109 = OpCompositeConstruct %v3float %94 %94 %94
%110 = OpFAdd %v3float %109 %108
OpReturnValue %110
%92 = OpLabel
%111 = OpLoad %v3float %result
OpReturnValue %111
%93 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_set_color_saturation_helper_Qh3h3h = OpFunction %v3float None %112
%113 = OpFunctionParameter %_ptr_Function_v3float
%114 = OpFunctionParameter %_ptr_Function_float
%115 = OpLabel
%116 = OpLoad %v3float %113
%117 = OpCompositeExtract %float %116 2
%118 = OpLoad %v3float %113
%119 = OpCompositeExtract %float %118 0
%120 = OpFSub %float %117 %119
%122 = OpFOrdGreaterThanEqual %bool %120 %float_0_00100000005
OpSelectionMerge %125 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpLoad %float %114
%127 = OpLoad %v3float %113
%128 = OpCompositeExtract %float %127 1
%129 = OpLoad %v3float %113
%130 = OpCompositeExtract %float %129 0
%131 = OpFSub %float %128 %130
%132 = OpFMul %float %126 %131
%133 = OpLoad %v3float %113
%134 = OpCompositeExtract %float %133 2
%135 = OpLoad %v3float %113
%136 = OpCompositeExtract %float %135 0
%137 = OpFSub %float %134 %136
%138 = OpFDiv %float %132 %137
%139 = OpLoad %float %114
%140 = OpCompositeConstruct %v3float %float_0 %138 %139
OpReturnValue %140
%124 = OpLabel
OpReturnValue %141
%125 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_set_color_saturation_Qh3h3h3 = OpFunction %v3float None %142
%143 = OpFunctionParameter %_ptr_Function_v3float
%144 = OpFunctionParameter %_ptr_Function_v3float
%145 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%181 = OpVariable %_ptr_Function_v3float Function
%183 = OpVariable %_ptr_Function_float Function
%195 = OpVariable %_ptr_Function_v3float Function
%197 = OpVariable %_ptr_Function_float Function
%202 = OpVariable %_ptr_Function_v3float Function
%204 = OpVariable %_ptr_Function_float Function
%217 = OpVariable %_ptr_Function_v3float Function
%219 = OpVariable %_ptr_Function_float Function
%232 = OpVariable %_ptr_Function_v3float Function
%234 = OpVariable %_ptr_Function_float Function
%239 = OpVariable %_ptr_Function_v3float Function
%241 = OpVariable %_ptr_Function_float Function
%149 = OpLoad %v3float %144
%150 = OpCompositeExtract %float %149 0
%151 = OpLoad %v3float %144
%152 = OpCompositeExtract %float %151 1
%148 = OpExtInst %float %1 FMax %150 %152
%153 = OpLoad %v3float %144
%154 = OpCompositeExtract %float %153 2
%147 = OpExtInst %float %1 FMax %148 %154
%157 = OpLoad %v3float %144
%158 = OpCompositeExtract %float %157 0
%159 = OpLoad %v3float %144
%160 = OpCompositeExtract %float %159 1
%156 = OpExtInst %float %1 FMin %158 %160
%161 = OpLoad %v3float %144
%162 = OpCompositeExtract %float %161 2
%155 = OpExtInst %float %1 FMin %156 %162
%163 = OpFSub %float %147 %155
OpStore %sat %163
%164 = OpLoad %v3float %143
%165 = OpCompositeExtract %float %164 0
%166 = OpLoad %v3float %143
%167 = OpCompositeExtract %float %166 1
%168 = OpFOrdLessThanEqual %bool %165 %167
OpSelectionMerge %171 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%172 = OpLoad %v3float %143
%173 = OpCompositeExtract %float %172 1
%174 = OpLoad %v3float %143
%175 = OpCompositeExtract %float %174 2
%176 = OpFOrdLessThanEqual %bool %173 %175
OpSelectionMerge %179 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%180 = OpLoad %v3float %143
OpStore %181 %180
%182 = OpLoad %float %sat
OpStore %183 %182
%184 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %181 %183
OpReturnValue %184
%178 = OpLabel
%185 = OpLoad %v3float %143
%186 = OpCompositeExtract %float %185 0
%187 = OpLoad %v3float %143
%188 = OpCompositeExtract %float %187 2
%189 = OpFOrdLessThanEqual %bool %186 %188
OpSelectionMerge %192 None
OpBranchConditional %189 %190 %191
%190 = OpLabel
%193 = OpLoad %v3float %143
%194 = OpVectorShuffle %v3float %193 %193 0 2 1
OpStore %195 %194
%196 = OpLoad %float %sat
OpStore %197 %196
%198 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %195 %197
%199 = OpVectorShuffle %v3float %198 %198 0 2 1
OpReturnValue %199
%191 = OpLabel
%200 = OpLoad %v3float %143
%201 = OpVectorShuffle %v3float %200 %200 2 0 1
OpStore %202 %201
%203 = OpLoad %float %sat
OpStore %204 %203
%205 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %202 %204
%206 = OpVectorShuffle %v3float %205 %205 1 2 0
OpReturnValue %206
%192 = OpLabel
OpBranch %179
%179 = OpLabel
OpBranch %171
%170 = OpLabel
%207 = OpLoad %v3float %143
%208 = OpCompositeExtract %float %207 0
%209 = OpLoad %v3float %143
%210 = OpCompositeExtract %float %209 2
%211 = OpFOrdLessThanEqual %bool %208 %210
OpSelectionMerge %214 None
OpBranchConditional %211 %212 %213
%212 = OpLabel
%215 = OpLoad %v3float %143
%216 = OpVectorShuffle %v3float %215 %215 1 0 2
OpStore %217 %216
%218 = OpLoad %float %sat
OpStore %219 %218
%220 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %217 %219
%221 = OpVectorShuffle %v3float %220 %220 1 0 2
OpReturnValue %221
%213 = OpLabel
%222 = OpLoad %v3float %143
%223 = OpCompositeExtract %float %222 1
%224 = OpLoad %v3float %143
%225 = OpCompositeExtract %float %224 2
%226 = OpFOrdLessThanEqual %bool %223 %225
OpSelectionMerge %229 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%230 = OpLoad %v3float %143
%231 = OpVectorShuffle %v3float %230 %230 1 2 0
OpStore %232 %231
%233 = OpLoad %float %sat
OpStore %234 %233
%235 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %232 %234
%236 = OpVectorShuffle %v3float %235 %235 2 0 1
OpReturnValue %236
%228 = OpLabel
%237 = OpLoad %v3float %143
%238 = OpVectorShuffle %v3float %237 %237 2 1 0
OpStore %239 %238
%240 = OpLoad %float %sat
OpStore %241 %240
%242 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %239 %241
%243 = OpVectorShuffle %v3float %242 %242 2 1 0
OpReturnValue %243
%229 = OpLabel
OpBranch %214
%214 = OpLabel
OpBranch %171
%171 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %245
%246 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%276 = OpVariable %_ptr_Function_v3float Function
%278 = OpVariable %_ptr_Function_v3float Function
%280 = OpVariable %_ptr_Function_v3float Function
%282 = OpVariable %_ptr_Function_float Function
%284 = OpVariable %_ptr_Function_v3float Function
%248 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%252 = OpLoad %v4float %248
%253 = OpCompositeExtract %float %252 3
%254 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%256 = OpLoad %v4float %254
%257 = OpCompositeExtract %float %256 3
%258 = OpFMul %float %253 %257
OpStore %_0_alpha %258
%260 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%261 = OpLoad %v4float %260
%262 = OpVectorShuffle %v3float %261 %261 0 1 2
%263 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%264 = OpLoad %v4float %263
%265 = OpCompositeExtract %float %264 3
%266 = OpVectorTimesScalar %v3float %262 %265
OpStore %_1_sda %266
%268 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%269 = OpLoad %v4float %268
%270 = OpVectorShuffle %v3float %269 %269 0 1 2
%271 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%272 = OpLoad %v4float %271
%273 = OpCompositeExtract %float %272 3
%274 = OpVectorTimesScalar %v3float %270 %273
OpStore %_2_dsa %274
%275 = OpLoad %v3float %_1_sda
OpStore %276 %275
%277 = OpLoad %v3float %_2_dsa
OpStore %278 %277
%279 = OpFunctionCall %v3float %blend_set_color_saturation_Qh3h3h3 %276 %278
OpStore %280 %279
%281 = OpLoad %float %_0_alpha
OpStore %282 %281
%283 = OpLoad %v3float %_2_dsa
OpStore %284 %283
%285 = OpFunctionCall %v3float %blend_set_color_luminance_Qh3h3hh3 %280 %282 %284
%286 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%287 = OpLoad %v4float %286
%288 = OpVectorShuffle %v3float %287 %287 0 1 2
%289 = OpFAdd %v3float %285 %288
%290 = OpLoad %v3float %_2_dsa
%291 = OpFSub %v3float %289 %290
%292 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%293 = OpLoad %v4float %292
%294 = OpVectorShuffle %v3float %293 %293 0 1 2
%295 = OpFAdd %v3float %291 %294
%296 = OpLoad %v3float %_1_sda
%297 = OpFSub %v3float %295 %296
%298 = OpCompositeExtract %float %297 0
%299 = OpCompositeExtract %float %297 1
%300 = OpCompositeExtract %float %297 2
%301 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%302 = OpLoad %v4float %301
%303 = OpCompositeExtract %float %302 3
%304 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%305 = OpLoad %v4float %304
%306 = OpCompositeExtract %float %305 3
%307 = OpFAdd %float %303 %306
%308 = OpLoad %float %_0_alpha
%309 = OpFSub %float %307 %308
%310 = OpCompositeConstruct %v4float %298 %299 %300 %309
OpStore %sk_FragColor %310
OpReturn
OpFunctionEnd
