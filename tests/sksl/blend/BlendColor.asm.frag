OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_color_luminance_Qhh3 "blend_color_luminance_Qhh3"
OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"
OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
OpName %guarded_divide_Qh3h3h "guarded_divide_Qh3h3h"
OpName %blend_set_color_luminance_Qh3h3hh3 "blend_set_color_luminance_Qh3h3hh3"
OpName %lum "lum"
OpName %result "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %blend_set_color_saturation_Qh3h3h3 "blend_set_color_saturation_Qh3h3h3"
OpName %mn "mn"
OpName %mx "mx"
OpName %blend_hslc_h4h2h4h4 "blend_hslc_h4h2h4h4"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %main "main"
OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %23 Binding 0
OpDecorate %23 DescriptorSet 0
OpDecorate %31 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %lum RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %minComp RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %mn RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %mx RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
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
OpDecorate %262 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
%float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
%bool = OpTypeBool
%false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
%float_0 = OpConstant %float 0
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%23 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%28 = OpTypeFunction %float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%35 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%_ptr_Function_float = OpTypePointer Function %float
%57 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%66 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_1 = OpConstant %float 1
%77 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%138 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%175 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%180 = OpTypeFunction %v4float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%254 = OpTypeFunction %void
%256 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_color_luminance_Qhh3 = OpFunction %float None %28
%29 = OpFunctionParameter %_ptr_Function_v3float
%30 = OpLabel
%36 = OpLoad %v3float %29
%31 = OpDot %float %35 %36
OpReturnValue %31
OpFunctionEnd
%blend_color_saturation_Qhh3 = OpFunction %float None %28
%37 = OpFunctionParameter %_ptr_Function_v3float
%38 = OpLabel
%41 = OpLoad %v3float %37
%42 = OpCompositeExtract %float %41 0
%43 = OpLoad %v3float %37
%44 = OpCompositeExtract %float %43 1
%40 = OpExtInst %float %1 FMax %42 %44
%45 = OpLoad %v3float %37
%46 = OpCompositeExtract %float %45 2
%39 = OpExtInst %float %1 FMax %40 %46
%49 = OpLoad %v3float %37
%50 = OpCompositeExtract %float %49 0
%51 = OpLoad %v3float %37
%52 = OpCompositeExtract %float %51 1
%48 = OpExtInst %float %1 FMin %50 %52
%53 = OpLoad %v3float %37
%54 = OpCompositeExtract %float %53 2
%47 = OpExtInst %float %1 FMin %48 %54
%55 = OpFSub %float %39 %47
OpReturnValue %55
OpFunctionEnd
%guarded_divide_Qhhh = OpFunction %float None %57
%58 = OpFunctionParameter %_ptr_Function_float
%59 = OpFunctionParameter %_ptr_Function_float
%60 = OpLabel
%61 = OpLoad %float %58
%62 = OpLoad %float %59
%63 = OpLoad %float %_kGuardedDivideEpsilon
%64 = OpFAdd %float %62 %63
%65 = OpFDiv %float %61 %64
OpReturnValue %65
OpFunctionEnd
%guarded_divide_Qh3h3h = OpFunction %v3float None %66
%67 = OpFunctionParameter %_ptr_Function_v3float
%68 = OpFunctionParameter %_ptr_Function_float
%69 = OpLabel
%70 = OpLoad %v3float %67
%71 = OpLoad %float %68
%72 = OpLoad %float %_kGuardedDivideEpsilon
%73 = OpFAdd %float %71 %72
%75 = OpFDiv %float %float_1 %73
%76 = OpVectorTimesScalar %v3float %70 %75
OpReturnValue %76
OpFunctionEnd
%blend_set_color_luminance_Qh3h3hh3 = OpFunction %v3float None %77
%78 = OpFunctionParameter %_ptr_Function_v3float
%79 = OpFunctionParameter %_ptr_Function_float
%80 = OpFunctionParameter %_ptr_Function_v3float
%81 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%84 = OpVariable %_ptr_Function_v3float Function
%result = OpVariable %_ptr_Function_v3float Function
%88 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%112 = OpVariable %_ptr_Function_float Function
%114 = OpVariable %_ptr_Function_float Function
%132 = OpVariable %_ptr_Function_v3float Function
%134 = OpVariable %_ptr_Function_float Function
%83 = OpLoad %v3float %80
OpStore %84 %83
%85 = OpFunctionCall %float %blend_color_luminance_Qhh3 %84
OpStore %lum %85
%87 = OpLoad %v3float %78
OpStore %88 %87
%89 = OpFunctionCall %float %blend_color_luminance_Qhh3 %88
%90 = OpFSub %float %85 %89
%91 = OpLoad %v3float %78
%92 = OpCompositeConstruct %v3float %90 %90 %90
%93 = OpFAdd %v3float %92 %91
OpStore %result %93
%97 = OpCompositeExtract %float %93 0
%98 = OpCompositeExtract %float %93 1
%96 = OpExtInst %float %1 FMin %97 %98
%99 = OpCompositeExtract %float %93 2
%95 = OpExtInst %float %1 FMin %96 %99
OpStore %minComp %95
%102 = OpExtInst %float %1 FMax %97 %98
%101 = OpExtInst %float %1 FMax %102 %99
OpStore %maxComp %101
%103 = OpFOrdLessThan %bool %95 %float_0
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpFUnordNotEqual %bool %85 %95
OpBranch %105
%105 = OpLabel
%107 = OpPhi %bool %false %81 %106 %104
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpCompositeConstruct %v3float %85 %85 %85
%111 = OpFSub %v3float %93 %110
OpStore %112 %85
%113 = OpFSub %float %85 %95
OpStore %114 %113
%115 = OpFunctionCall %float %guarded_divide_Qhhh %112 %114
%116 = OpVectorTimesScalar %v3float %111 %115
%117 = OpFAdd %v3float %110 %116
OpStore %result %117
OpBranch %109
%109 = OpLabel
%118 = OpLoad %float %79
%119 = OpFOrdGreaterThan %bool %101 %118
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpFUnordNotEqual %bool %101 %85
OpBranch %121
%121 = OpLabel
%123 = OpPhi %bool %false %109 %122 %120
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpLoad %v3float %result
%127 = OpCompositeConstruct %v3float %85 %85 %85
%128 = OpFSub %v3float %126 %127
%129 = OpLoad %float %79
%130 = OpFSub %float %129 %85
%131 = OpVectorTimesScalar %v3float %128 %130
OpStore %132 %131
%133 = OpFSub %float %101 %85
OpStore %134 %133
%135 = OpFunctionCall %v3float %guarded_divide_Qh3h3h %132 %134
%136 = OpFAdd %v3float %127 %135
OpStore %result %136
OpBranch %125
%125 = OpLabel
%137 = OpLoad %v3float %result
OpReturnValue %137
OpFunctionEnd
%blend_set_color_saturation_Qh3h3h3 = OpFunction %v3float None %138
%139 = OpFunctionParameter %_ptr_Function_v3float
%140 = OpFunctionParameter %_ptr_Function_v3float
%141 = OpLabel
%mn = OpVariable %_ptr_Function_float Function
%mx = OpVariable %_ptr_Function_float Function
%161 = OpVariable %_ptr_Function_v3float Function
%169 = OpVariable %_ptr_Function_v3float Function
%145 = OpLoad %v3float %139
%146 = OpCompositeExtract %float %145 0
%147 = OpLoad %v3float %139
%148 = OpCompositeExtract %float %147 1
%144 = OpExtInst %float %1 FMin %146 %148
%149 = OpLoad %v3float %139
%150 = OpCompositeExtract %float %149 2
%143 = OpExtInst %float %1 FMin %144 %150
OpStore %mn %143
%154 = OpLoad %v3float %139
%155 = OpCompositeExtract %float %154 0
%156 = OpLoad %v3float %139
%157 = OpCompositeExtract %float %156 1
%153 = OpExtInst %float %1 FMax %155 %157
%158 = OpLoad %v3float %139
%159 = OpCompositeExtract %float %158 2
%152 = OpExtInst %float %1 FMax %153 %159
OpStore %mx %152
%160 = OpFOrdGreaterThan %bool %152 %143
OpSelectionMerge %164 None
OpBranchConditional %160 %162 %163
%162 = OpLabel
%165 = OpLoad %v3float %139
%166 = OpCompositeConstruct %v3float %143 %143 %143
%167 = OpFSub %v3float %165 %166
%168 = OpLoad %v3float %140
OpStore %169 %168
%170 = OpFunctionCall %float %blend_color_saturation_Qhh3 %169
%171 = OpVectorTimesScalar %v3float %167 %170
%172 = OpFSub %float %152 %143
%173 = OpFDiv %float %float_1 %172
%174 = OpVectorTimesScalar %v3float %171 %173
OpStore %161 %174
OpBranch %164
%163 = OpLabel
OpStore %161 %175
OpBranch %164
%164 = OpLabel
%176 = OpLoad %v3float %161
OpReturnValue %176
OpFunctionEnd
%blend_hslc_h4h2h4h4 = OpFunction %v4float None %180
%181 = OpFunctionParameter %_ptr_Function_v2float
%182 = OpFunctionParameter %_ptr_Function_v4float
%183 = OpFunctionParameter %_ptr_Function_v4float
%184 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%207 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%216 = OpVariable %_ptr_Function_v3float Function
%226 = OpVariable %_ptr_Function_v3float Function
%227 = OpVariable %_ptr_Function_v3float Function
%230 = OpVariable %_ptr_Function_v3float Function
%231 = OpVariable %_ptr_Function_float Function
%233 = OpVariable %_ptr_Function_v3float Function
%186 = OpLoad %v4float %183
%187 = OpCompositeExtract %float %186 3
%188 = OpLoad %v4float %182
%189 = OpCompositeExtract %float %188 3
%190 = OpFMul %float %187 %189
OpStore %alpha %190
%192 = OpLoad %v4float %182
%193 = OpVectorShuffle %v3float %192 %192 0 1 2
%194 = OpLoad %v4float %183
%195 = OpCompositeExtract %float %194 3
%196 = OpVectorTimesScalar %v3float %193 %195
OpStore %sda %196
%198 = OpLoad %v4float %183
%199 = OpVectorShuffle %v3float %198 %198 0 1 2
%200 = OpLoad %v4float %182
%201 = OpCompositeExtract %float %200 3
%202 = OpVectorTimesScalar %v3float %199 %201
OpStore %dsa %202
%204 = OpLoad %v2float %181
%205 = OpCompositeExtract %float %204 0
%206 = OpFUnordNotEqual %bool %205 %float_0
OpSelectionMerge %210 None
OpBranchConditional %206 %208 %209
%208 = OpLabel
OpStore %207 %202
OpBranch %210
%209 = OpLabel
OpStore %207 %196
OpBranch %210
%210 = OpLabel
%211 = OpLoad %v3float %207
OpStore %l %211
%213 = OpLoad %v2float %181
%214 = OpCompositeExtract %float %213 0
%215 = OpFUnordNotEqual %bool %214 %float_0
OpSelectionMerge %219 None
OpBranchConditional %215 %217 %218
%217 = OpLabel
OpStore %216 %196
OpBranch %219
%218 = OpLabel
OpStore %216 %202
OpBranch %219
%219 = OpLabel
%220 = OpLoad %v3float %216
OpStore %r %220
%221 = OpLoad %v2float %181
%222 = OpCompositeExtract %float %221 1
%223 = OpFUnordNotEqual %bool %222 %float_0
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
OpStore %226 %211
OpStore %227 %220
%228 = OpFunctionCall %v3float %blend_set_color_saturation_Qh3h3h3 %226 %227
OpStore %l %228
OpStore %r %202
OpBranch %225
%225 = OpLabel
%229 = OpLoad %v3float %l
OpStore %230 %229
OpStore %231 %190
%232 = OpLoad %v3float %r
OpStore %233 %232
%234 = OpFunctionCall %v3float %blend_set_color_luminance_Qh3h3hh3 %230 %231 %233
%235 = OpLoad %v4float %183
%236 = OpVectorShuffle %v3float %235 %235 0 1 2
%237 = OpFAdd %v3float %234 %236
%238 = OpFSub %v3float %237 %202
%239 = OpLoad %v4float %182
%240 = OpVectorShuffle %v3float %239 %239 0 1 2
%241 = OpFAdd %v3float %238 %240
%242 = OpFSub %v3float %241 %196
%243 = OpCompositeExtract %float %242 0
%244 = OpCompositeExtract %float %242 1
%245 = OpCompositeExtract %float %242 2
%246 = OpLoad %v4float %182
%247 = OpCompositeExtract %float %246 3
%248 = OpLoad %v4float %183
%249 = OpCompositeExtract %float %248 3
%250 = OpFAdd %float %247 %249
%251 = OpFSub %float %250 %190
%252 = OpCompositeConstruct %v4float %243 %244 %245 %251
OpReturnValue %252
OpFunctionEnd
%main = OpFunction %void None %254
%255 = OpLabel
%257 = OpVariable %_ptr_Function_v2float Function
%263 = OpVariable %_ptr_Function_v4float Function
%267 = OpVariable %_ptr_Function_v4float Function
%15 = OpSelect %float %false %float_9_99999994en09 %float_0
OpStore %_kGuardedDivideEpsilon %15
OpStore %257 %256
%258 = OpAccessChain %_ptr_Uniform_v4float %23 %int_0
%262 = OpLoad %v4float %258
OpStore %263 %262
%264 = OpAccessChain %_ptr_Uniform_v4float %23 %int_1
%266 = OpLoad %v4float %264
OpStore %267 %266
%268 = OpFunctionCall %v4float %blend_hslc_h4h2h4h4 %257 %263 %267
OpStore %sk_FragColor %268
OpReturn
OpFunctionEnd
