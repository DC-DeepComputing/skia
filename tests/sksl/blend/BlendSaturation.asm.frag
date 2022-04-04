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
OpName %delta "delta"
OpName %mid "mid"
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
OpDecorate %delta RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %mid RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %sat RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
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
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %_0_alpha RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %_1_sda RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %_2_dsa RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%147 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%149 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%void = OpTypeVoid
%252 = OpTypeFunction %void
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
%delta = OpVariable %_ptr_Function_v2float Function
%mid = OpVariable %_ptr_Function_float Function
%140 = OpVariable %_ptr_Function_v3float Function
%119 = OpLoad %v3float %113
%120 = OpVectorShuffle %v2float %119 %119 1 2
%121 = OpLoad %v3float %113
%122 = OpVectorShuffle %v2float %121 %121 0 0
%123 = OpFSub %v2float %120 %122
OpStore %delta %123
%125 = OpLoad %float %114
%126 = OpLoad %v2float %delta
%127 = OpCompositeExtract %float %126 0
%128 = OpFMul %float %125 %127
%129 = OpLoad %v2float %delta
%130 = OpCompositeExtract %float %129 1
%131 = OpFDiv %float %128 %130
OpStore %mid %131
%132 = OpLoad %v2float %delta
%133 = OpCompositeExtract %float %132 1
%134 = OpFOrdGreaterThan %bool %133 %float_0
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpLoad %float %mid
%138 = OpFOrdLessThanEqual %bool %137 %float_1
OpBranch %136
%136 = OpLabel
%139 = OpPhi %bool %false %115 %138 %135
OpSelectionMerge %143 None
OpBranchConditional %139 %141 %142
%141 = OpLabel
%144 = OpLoad %float %mid
%145 = OpLoad %float %114
%146 = OpCompositeConstruct %v3float %float_0 %144 %145
OpStore %140 %146
OpBranch %143
%142 = OpLabel
OpStore %140 %147
OpBranch %143
%143 = OpLabel
%148 = OpLoad %v3float %140
OpReturnValue %148
OpFunctionEnd
%blend_set_color_saturation_Qh3h3h3 = OpFunction %v3float None %149
%150 = OpFunctionParameter %_ptr_Function_v3float
%151 = OpFunctionParameter %_ptr_Function_v3float
%152 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%188 = OpVariable %_ptr_Function_v3float Function
%190 = OpVariable %_ptr_Function_float Function
%202 = OpVariable %_ptr_Function_v3float Function
%204 = OpVariable %_ptr_Function_float Function
%209 = OpVariable %_ptr_Function_v3float Function
%211 = OpVariable %_ptr_Function_float Function
%224 = OpVariable %_ptr_Function_v3float Function
%226 = OpVariable %_ptr_Function_float Function
%239 = OpVariable %_ptr_Function_v3float Function
%241 = OpVariable %_ptr_Function_float Function
%246 = OpVariable %_ptr_Function_v3float Function
%248 = OpVariable %_ptr_Function_float Function
%156 = OpLoad %v3float %151
%157 = OpCompositeExtract %float %156 0
%158 = OpLoad %v3float %151
%159 = OpCompositeExtract %float %158 1
%155 = OpExtInst %float %1 FMax %157 %159
%160 = OpLoad %v3float %151
%161 = OpCompositeExtract %float %160 2
%154 = OpExtInst %float %1 FMax %155 %161
%164 = OpLoad %v3float %151
%165 = OpCompositeExtract %float %164 0
%166 = OpLoad %v3float %151
%167 = OpCompositeExtract %float %166 1
%163 = OpExtInst %float %1 FMin %165 %167
%168 = OpLoad %v3float %151
%169 = OpCompositeExtract %float %168 2
%162 = OpExtInst %float %1 FMin %163 %169
%170 = OpFSub %float %154 %162
OpStore %sat %170
%171 = OpLoad %v3float %150
%172 = OpCompositeExtract %float %171 0
%173 = OpLoad %v3float %150
%174 = OpCompositeExtract %float %173 1
%175 = OpFOrdLessThanEqual %bool %172 %174
OpSelectionMerge %178 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%179 = OpLoad %v3float %150
%180 = OpCompositeExtract %float %179 1
%181 = OpLoad %v3float %150
%182 = OpCompositeExtract %float %181 2
%183 = OpFOrdLessThanEqual %bool %180 %182
OpSelectionMerge %186 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%187 = OpLoad %v3float %150
OpStore %188 %187
%189 = OpLoad %float %sat
OpStore %190 %189
%191 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %188 %190
OpReturnValue %191
%185 = OpLabel
%192 = OpLoad %v3float %150
%193 = OpCompositeExtract %float %192 0
%194 = OpLoad %v3float %150
%195 = OpCompositeExtract %float %194 2
%196 = OpFOrdLessThanEqual %bool %193 %195
OpSelectionMerge %199 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%200 = OpLoad %v3float %150
%201 = OpVectorShuffle %v3float %200 %200 0 2 1
OpStore %202 %201
%203 = OpLoad %float %sat
OpStore %204 %203
%205 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %202 %204
%206 = OpVectorShuffle %v3float %205 %205 0 2 1
OpReturnValue %206
%198 = OpLabel
%207 = OpLoad %v3float %150
%208 = OpVectorShuffle %v3float %207 %207 2 0 1
OpStore %209 %208
%210 = OpLoad %float %sat
OpStore %211 %210
%212 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %209 %211
%213 = OpVectorShuffle %v3float %212 %212 1 2 0
OpReturnValue %213
%199 = OpLabel
OpBranch %186
%186 = OpLabel
OpBranch %178
%177 = OpLabel
%214 = OpLoad %v3float %150
%215 = OpCompositeExtract %float %214 0
%216 = OpLoad %v3float %150
%217 = OpCompositeExtract %float %216 2
%218 = OpFOrdLessThanEqual %bool %215 %217
OpSelectionMerge %221 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%222 = OpLoad %v3float %150
%223 = OpVectorShuffle %v3float %222 %222 1 0 2
OpStore %224 %223
%225 = OpLoad %float %sat
OpStore %226 %225
%227 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %224 %226
%228 = OpVectorShuffle %v3float %227 %227 1 0 2
OpReturnValue %228
%220 = OpLabel
%229 = OpLoad %v3float %150
%230 = OpCompositeExtract %float %229 1
%231 = OpLoad %v3float %150
%232 = OpCompositeExtract %float %231 2
%233 = OpFOrdLessThanEqual %bool %230 %232
OpSelectionMerge %236 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
%237 = OpLoad %v3float %150
%238 = OpVectorShuffle %v3float %237 %237 1 2 0
OpStore %239 %238
%240 = OpLoad %float %sat
OpStore %241 %240
%242 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %239 %241
%243 = OpVectorShuffle %v3float %242 %242 2 0 1
OpReturnValue %243
%235 = OpLabel
%244 = OpLoad %v3float %150
%245 = OpVectorShuffle %v3float %244 %244 2 1 0
OpStore %246 %245
%247 = OpLoad %float %sat
OpStore %248 %247
%249 = OpFunctionCall %v3float %blend_set_color_saturation_helper_Qh3h3h %246 %248
%250 = OpVectorShuffle %v3float %249 %249 2 1 0
OpReturnValue %250
%236 = OpLabel
OpBranch %221
%221 = OpLabel
OpBranch %178
%178 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %252
%253 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%283 = OpVariable %_ptr_Function_v3float Function
%285 = OpVariable %_ptr_Function_v3float Function
%287 = OpVariable %_ptr_Function_v3float Function
%289 = OpVariable %_ptr_Function_float Function
%291 = OpVariable %_ptr_Function_v3float Function
%255 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%259 = OpLoad %v4float %255
%260 = OpCompositeExtract %float %259 3
%261 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%263 = OpLoad %v4float %261
%264 = OpCompositeExtract %float %263 3
%265 = OpFMul %float %260 %264
OpStore %_0_alpha %265
%267 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%268 = OpLoad %v4float %267
%269 = OpVectorShuffle %v3float %268 %268 0 1 2
%270 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%271 = OpLoad %v4float %270
%272 = OpCompositeExtract %float %271 3
%273 = OpVectorTimesScalar %v3float %269 %272
OpStore %_1_sda %273
%275 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%276 = OpLoad %v4float %275
%277 = OpVectorShuffle %v3float %276 %276 0 1 2
%278 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%279 = OpLoad %v4float %278
%280 = OpCompositeExtract %float %279 3
%281 = OpVectorTimesScalar %v3float %277 %280
OpStore %_2_dsa %281
%282 = OpLoad %v3float %_2_dsa
OpStore %283 %282
%284 = OpLoad %v3float %_1_sda
OpStore %285 %284
%286 = OpFunctionCall %v3float %blend_set_color_saturation_Qh3h3h3 %283 %285
OpStore %287 %286
%288 = OpLoad %float %_0_alpha
OpStore %289 %288
%290 = OpLoad %v3float %_2_dsa
OpStore %291 %290
%292 = OpFunctionCall %v3float %blend_set_color_luminance_Qh3h3hh3 %287 %289 %291
%293 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%294 = OpLoad %v4float %293
%295 = OpVectorShuffle %v3float %294 %294 0 1 2
%296 = OpFAdd %v3float %292 %295
%297 = OpLoad %v3float %_2_dsa
%298 = OpFSub %v3float %296 %297
%299 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%300 = OpLoad %v4float %299
%301 = OpVectorShuffle %v3float %300 %300 0 1 2
%302 = OpFAdd %v3float %298 %301
%303 = OpLoad %v3float %_1_sda
%304 = OpFSub %v3float %302 %303
%305 = OpCompositeExtract %float %304 0
%306 = OpCompositeExtract %float %304 1
%307 = OpCompositeExtract %float %304 2
%308 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%309 = OpLoad %v4float %308
%310 = OpCompositeExtract %float %309 3
%311 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%312 = OpLoad %v4float %311
%313 = OpCompositeExtract %float %312 3
%314 = OpFAdd %float %310 %313
%315 = OpLoad %float %_0_alpha
%316 = OpFSub %float %314 %315
%317 = OpCompositeConstruct %v4float %305 %306 %307 %316
OpStore %sk_FragColor %317
OpReturn
OpFunctionEnd
