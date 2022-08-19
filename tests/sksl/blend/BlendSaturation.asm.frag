OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
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
OpName %blend_hslc_h4h4h4h2 "blend_hslc_h4h4h4h2"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %blend_saturation_h4h4h4 "blend_saturation_h4h4h4"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %18 Binding 0
OpDecorate %18 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %lum RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %minComp RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %mn RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %mx RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
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
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%23 = OpTypeFunction %float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%30 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%_ptr_Function_float = OpTypePointer Function %float
%52 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%59 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_1 = OpConstant %float 1
%68 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%131 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%168 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%173 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_v2float
%246 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%254 = OpConstantComposite %v2float %float_1 %float_1
%void = OpTypeVoid
%258 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_color_luminance_Qhh3 = OpFunction %float None %23
%24 = OpFunctionParameter %_ptr_Function_v3float
%25 = OpLabel
%31 = OpLoad %v3float %24
%26 = OpDot %float %30 %31
OpReturnValue %26
OpFunctionEnd
%blend_color_saturation_Qhh3 = OpFunction %float None %23
%32 = OpFunctionParameter %_ptr_Function_v3float
%33 = OpLabel
%36 = OpLoad %v3float %32
%37 = OpCompositeExtract %float %36 0
%38 = OpLoad %v3float %32
%39 = OpCompositeExtract %float %38 1
%35 = OpExtInst %float %1 FMax %37 %39
%40 = OpLoad %v3float %32
%41 = OpCompositeExtract %float %40 2
%34 = OpExtInst %float %1 FMax %35 %41
%44 = OpLoad %v3float %32
%45 = OpCompositeExtract %float %44 0
%46 = OpLoad %v3float %32
%47 = OpCompositeExtract %float %46 1
%43 = OpExtInst %float %1 FMin %45 %47
%48 = OpLoad %v3float %32
%49 = OpCompositeExtract %float %48 2
%42 = OpExtInst %float %1 FMin %43 %49
%50 = OpFSub %float %34 %42
OpReturnValue %50
OpFunctionEnd
%guarded_divide_Qhhh = OpFunction %float None %52
%53 = OpFunctionParameter %_ptr_Function_float
%54 = OpFunctionParameter %_ptr_Function_float
%55 = OpLabel
%56 = OpLoad %float %53
%57 = OpLoad %float %54
%58 = OpFDiv %float %56 %57
OpReturnValue %58
OpFunctionEnd
%guarded_divide_Qh3h3h = OpFunction %v3float None %59
%60 = OpFunctionParameter %_ptr_Function_v3float
%61 = OpFunctionParameter %_ptr_Function_float
%62 = OpLabel
%63 = OpLoad %v3float %60
%64 = OpLoad %float %61
%66 = OpFDiv %float %float_1 %64
%67 = OpVectorTimesScalar %v3float %63 %66
OpReturnValue %67
OpFunctionEnd
%blend_set_color_luminance_Qh3h3hh3 = OpFunction %v3float None %68
%69 = OpFunctionParameter %_ptr_Function_v3float
%70 = OpFunctionParameter %_ptr_Function_float
%71 = OpFunctionParameter %_ptr_Function_v3float
%72 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%75 = OpVariable %_ptr_Function_v3float Function
%result = OpVariable %_ptr_Function_v3float Function
%79 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%105 = OpVariable %_ptr_Function_float Function
%107 = OpVariable %_ptr_Function_float Function
%125 = OpVariable %_ptr_Function_v3float Function
%127 = OpVariable %_ptr_Function_float Function
%74 = OpLoad %v3float %71
OpStore %75 %74
%76 = OpFunctionCall %float %blend_color_luminance_Qhh3 %75
OpStore %lum %76
%78 = OpLoad %v3float %69
OpStore %79 %78
%80 = OpFunctionCall %float %blend_color_luminance_Qhh3 %79
%81 = OpFSub %float %76 %80
%82 = OpLoad %v3float %69
%83 = OpCompositeConstruct %v3float %81 %81 %81
%84 = OpFAdd %v3float %83 %82
OpStore %result %84
%88 = OpCompositeExtract %float %84 0
%89 = OpCompositeExtract %float %84 1
%87 = OpExtInst %float %1 FMin %88 %89
%90 = OpCompositeExtract %float %84 2
%86 = OpExtInst %float %1 FMin %87 %90
OpStore %minComp %86
%93 = OpExtInst %float %1 FMax %88 %89
%92 = OpExtInst %float %1 FMax %93 %90
OpStore %maxComp %92
%96 = OpFOrdLessThan %bool %86 %float_0
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpFUnordNotEqual %bool %76 %86
OpBranch %98
%98 = OpLabel
%100 = OpPhi %bool %false %72 %99 %97
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpCompositeConstruct %v3float %76 %76 %76
%104 = OpFSub %v3float %84 %103
OpStore %105 %76
%106 = OpFSub %float %76 %86
OpStore %107 %106
%108 = OpFunctionCall %float %guarded_divide_Qhhh %105 %107
%109 = OpVectorTimesScalar %v3float %104 %108
%110 = OpFAdd %v3float %103 %109
OpStore %result %110
OpBranch %102
%102 = OpLabel
%111 = OpLoad %float %70
%112 = OpFOrdGreaterThan %bool %92 %111
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpFUnordNotEqual %bool %92 %76
OpBranch %114
%114 = OpLabel
%116 = OpPhi %bool %false %102 %115 %113
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%119 = OpLoad %v3float %result
%120 = OpCompositeConstruct %v3float %76 %76 %76
%121 = OpFSub %v3float %119 %120
%122 = OpLoad %float %70
%123 = OpFSub %float %122 %76
%124 = OpVectorTimesScalar %v3float %121 %123
OpStore %125 %124
%126 = OpFSub %float %92 %76
OpStore %127 %126
%128 = OpFunctionCall %v3float %guarded_divide_Qh3h3h %125 %127
%129 = OpFAdd %v3float %120 %128
OpStore %result %129
OpBranch %118
%118 = OpLabel
%130 = OpLoad %v3float %result
OpReturnValue %130
OpFunctionEnd
%blend_set_color_saturation_Qh3h3h3 = OpFunction %v3float None %131
%132 = OpFunctionParameter %_ptr_Function_v3float
%133 = OpFunctionParameter %_ptr_Function_v3float
%134 = OpLabel
%mn = OpVariable %_ptr_Function_float Function
%mx = OpVariable %_ptr_Function_float Function
%154 = OpVariable %_ptr_Function_v3float Function
%162 = OpVariable %_ptr_Function_v3float Function
%138 = OpLoad %v3float %132
%139 = OpCompositeExtract %float %138 0
%140 = OpLoad %v3float %132
%141 = OpCompositeExtract %float %140 1
%137 = OpExtInst %float %1 FMin %139 %141
%142 = OpLoad %v3float %132
%143 = OpCompositeExtract %float %142 2
%136 = OpExtInst %float %1 FMin %137 %143
OpStore %mn %136
%147 = OpLoad %v3float %132
%148 = OpCompositeExtract %float %147 0
%149 = OpLoad %v3float %132
%150 = OpCompositeExtract %float %149 1
%146 = OpExtInst %float %1 FMax %148 %150
%151 = OpLoad %v3float %132
%152 = OpCompositeExtract %float %151 2
%145 = OpExtInst %float %1 FMax %146 %152
OpStore %mx %145
%153 = OpFOrdGreaterThan %bool %145 %136
OpSelectionMerge %157 None
OpBranchConditional %153 %155 %156
%155 = OpLabel
%158 = OpLoad %v3float %132
%159 = OpCompositeConstruct %v3float %136 %136 %136
%160 = OpFSub %v3float %158 %159
%161 = OpLoad %v3float %133
OpStore %162 %161
%163 = OpFunctionCall %float %blend_color_saturation_Qhh3 %162
%164 = OpVectorTimesScalar %v3float %160 %163
%165 = OpFSub %float %145 %136
%166 = OpFDiv %float %float_1 %165
%167 = OpVectorTimesScalar %v3float %164 %166
OpStore %154 %167
OpBranch %157
%156 = OpLabel
OpStore %154 %168
OpBranch %157
%157 = OpLabel
%169 = OpLoad %v3float %154
OpReturnValue %169
OpFunctionEnd
%blend_hslc_h4h4h4h2 = OpFunction %v4float None %173
%174 = OpFunctionParameter %_ptr_Function_v4float
%175 = OpFunctionParameter %_ptr_Function_v4float
%176 = OpFunctionParameter %_ptr_Function_v2float
%177 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%200 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%209 = OpVariable %_ptr_Function_v3float Function
%219 = OpVariable %_ptr_Function_v3float Function
%220 = OpVariable %_ptr_Function_v3float Function
%223 = OpVariable %_ptr_Function_v3float Function
%224 = OpVariable %_ptr_Function_float Function
%226 = OpVariable %_ptr_Function_v3float Function
%179 = OpLoad %v4float %175
%180 = OpCompositeExtract %float %179 3
%181 = OpLoad %v4float %174
%182 = OpCompositeExtract %float %181 3
%183 = OpFMul %float %180 %182
OpStore %alpha %183
%185 = OpLoad %v4float %174
%186 = OpVectorShuffle %v3float %185 %185 0 1 2
%187 = OpLoad %v4float %175
%188 = OpCompositeExtract %float %187 3
%189 = OpVectorTimesScalar %v3float %186 %188
OpStore %sda %189
%191 = OpLoad %v4float %175
%192 = OpVectorShuffle %v3float %191 %191 0 1 2
%193 = OpLoad %v4float %174
%194 = OpCompositeExtract %float %193 3
%195 = OpVectorTimesScalar %v3float %192 %194
OpStore %dsa %195
%197 = OpLoad %v2float %176
%198 = OpCompositeExtract %float %197 0
%199 = OpFUnordNotEqual %bool %198 %float_0
OpSelectionMerge %203 None
OpBranchConditional %199 %201 %202
%201 = OpLabel
OpStore %200 %195
OpBranch %203
%202 = OpLabel
OpStore %200 %189
OpBranch %203
%203 = OpLabel
%204 = OpLoad %v3float %200
OpStore %l %204
%206 = OpLoad %v2float %176
%207 = OpCompositeExtract %float %206 0
%208 = OpFUnordNotEqual %bool %207 %float_0
OpSelectionMerge %212 None
OpBranchConditional %208 %210 %211
%210 = OpLabel
OpStore %209 %189
OpBranch %212
%211 = OpLabel
OpStore %209 %195
OpBranch %212
%212 = OpLabel
%213 = OpLoad %v3float %209
OpStore %r %213
%214 = OpLoad %v2float %176
%215 = OpCompositeExtract %float %214 1
%216 = OpFUnordNotEqual %bool %215 %float_0
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
OpStore %219 %204
OpStore %220 %213
%221 = OpFunctionCall %v3float %blend_set_color_saturation_Qh3h3h3 %219 %220
OpStore %l %221
OpStore %r %195
OpBranch %218
%218 = OpLabel
%222 = OpLoad %v3float %l
OpStore %223 %222
OpStore %224 %183
%225 = OpLoad %v3float %r
OpStore %226 %225
%227 = OpFunctionCall %v3float %blend_set_color_luminance_Qh3h3hh3 %223 %224 %226
%228 = OpLoad %v4float %175
%229 = OpVectorShuffle %v3float %228 %228 0 1 2
%230 = OpFAdd %v3float %227 %229
%231 = OpFSub %v3float %230 %195
%232 = OpLoad %v4float %174
%233 = OpVectorShuffle %v3float %232 %232 0 1 2
%234 = OpFAdd %v3float %231 %233
%235 = OpFSub %v3float %234 %189
%236 = OpCompositeExtract %float %235 0
%237 = OpCompositeExtract %float %235 1
%238 = OpCompositeExtract %float %235 2
%239 = OpLoad %v4float %174
%240 = OpCompositeExtract %float %239 3
%241 = OpLoad %v4float %175
%242 = OpCompositeExtract %float %241 3
%243 = OpFAdd %float %240 %242
%244 = OpFSub %float %243 %183
%245 = OpCompositeConstruct %v4float %236 %237 %238 %244
OpReturnValue %245
OpFunctionEnd
%blend_saturation_h4h4h4 = OpFunction %v4float None %246
%247 = OpFunctionParameter %_ptr_Function_v4float
%248 = OpFunctionParameter %_ptr_Function_v4float
%249 = OpLabel
%251 = OpVariable %_ptr_Function_v4float Function
%253 = OpVariable %_ptr_Function_v4float Function
%255 = OpVariable %_ptr_Function_v2float Function
%250 = OpLoad %v4float %247
OpStore %251 %250
%252 = OpLoad %v4float %248
OpStore %253 %252
OpStore %255 %254
%256 = OpFunctionCall %v4float %blend_hslc_h4h4h4h2 %251 %253 %255
OpReturnValue %256
OpFunctionEnd
%main = OpFunction %void None %258
%259 = OpLabel
%265 = OpVariable %_ptr_Function_v4float Function
%269 = OpVariable %_ptr_Function_v4float Function
%260 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%264 = OpLoad %v4float %260
OpStore %265 %264
%266 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%268 = OpLoad %v4float %266
OpStore %269 %268
%270 = OpFunctionCall %v4float %blend_saturation_h4h4h4 %265 %269
OpStore %sk_FragColor %270
OpReturn
OpFunctionEnd
