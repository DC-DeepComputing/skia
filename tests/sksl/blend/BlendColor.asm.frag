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
OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"
OpName %blend_hslc_h4h4h4bb "blend_hslc_h4h4h4bb"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_2_mn "_2_mn"
OpName %_3_mx "_3_mx"
OpName %_4_sat "_4_sat"
OpName %_5_lum "_5_lum"
OpName %_6_result "_6_result"
OpName %_7_minComp "_7_minComp"
OpName %_8_maxComp "_8_maxComp"
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
OpDecorate %27 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %_2_mn RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %_3_mx RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %_4_sat RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %_5_lum RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %_6_result RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %_7_minComp RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %_8_maxComp RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
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
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
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
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
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
%16 = OpTypeFunction %float %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%37 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_bool %_ptr_Function_bool
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%126 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%134 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%void = OpTypeVoid
%236 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_color_saturation_Qhh3 = OpFunction %float None %16
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpLabel
%22 = OpLoad %v3float %18
%23 = OpCompositeExtract %float %22 0
%24 = OpLoad %v3float %18
%25 = OpCompositeExtract %float %24 1
%21 = OpExtInst %float %1 FMax %23 %25
%26 = OpLoad %v3float %18
%27 = OpCompositeExtract %float %26 2
%20 = OpExtInst %float %1 FMax %21 %27
%30 = OpLoad %v3float %18
%31 = OpCompositeExtract %float %30 0
%32 = OpLoad %v3float %18
%33 = OpCompositeExtract %float %32 1
%29 = OpExtInst %float %1 FMin %31 %33
%34 = OpLoad %v3float %18
%35 = OpCompositeExtract %float %34 2
%28 = OpExtInst %float %1 FMin %29 %35
%36 = OpFSub %float %20 %28
OpReturnValue %36
OpFunctionEnd
%blend_hslc_h4h4h4bb = OpFunction %v4float None %37
%40 = OpFunctionParameter %_ptr_Function_v4float
%41 = OpFunctionParameter %_ptr_Function_v4float
%42 = OpFunctionParameter %_ptr_Function_bool
%43 = OpFunctionParameter %_ptr_Function_bool
%44 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%66 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%75 = OpVariable %_ptr_Function_v3float Function
%_2_mn = OpVariable %_ptr_Function_float Function
%_3_mx = OpVariable %_ptr_Function_float Function
%_4_sat = OpVariable %_ptr_Function_float Function
%110 = OpVariable %_ptr_Function_v3float Function
%119 = OpVariable %_ptr_Function_v3float Function
%_5_lum = OpVariable %_ptr_Function_float Function
%_6_result = OpVariable %_ptr_Function_v3float Function
%_7_minComp = OpVariable %_ptr_Function_float Function
%_8_maxComp = OpVariable %_ptr_Function_float Function
%47 = OpLoad %v4float %41
%48 = OpCompositeExtract %float %47 3
%49 = OpLoad %v4float %40
%50 = OpCompositeExtract %float %49 3
%51 = OpFMul %float %48 %50
OpStore %alpha %51
%53 = OpLoad %v4float %40
%54 = OpVectorShuffle %v3float %53 %53 0 1 2
%55 = OpLoad %v4float %41
%56 = OpCompositeExtract %float %55 3
%57 = OpVectorTimesScalar %v3float %54 %56
OpStore %sda %57
%59 = OpLoad %v4float %41
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%61 = OpLoad %v4float %40
%62 = OpCompositeExtract %float %61 3
%63 = OpVectorTimesScalar %v3float %60 %62
OpStore %dsa %63
%65 = OpLoad %bool %42
OpSelectionMerge %69 None
OpBranchConditional %65 %67 %68
%67 = OpLabel
%70 = OpLoad %v3float %dsa
OpStore %66 %70
OpBranch %69
%68 = OpLabel
%71 = OpLoad %v3float %sda
OpStore %66 %71
OpBranch %69
%69 = OpLabel
%72 = OpLoad %v3float %66
OpStore %l %72
%74 = OpLoad %bool %42
OpSelectionMerge %78 None
OpBranchConditional %74 %76 %77
%76 = OpLabel
%79 = OpLoad %v3float %sda
OpStore %75 %79
OpBranch %78
%77 = OpLabel
%80 = OpLoad %v3float %dsa
OpStore %75 %80
OpBranch %78
%78 = OpLabel
%81 = OpLoad %v3float %75
OpStore %r %81
%82 = OpLoad %bool %43
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%88 = OpLoad %v3float %l
%89 = OpCompositeExtract %float %88 0
%90 = OpLoad %v3float %l
%91 = OpCompositeExtract %float %90 1
%87 = OpExtInst %float %1 FMin %89 %91
%92 = OpLoad %v3float %l
%93 = OpCompositeExtract %float %92 2
%86 = OpExtInst %float %1 FMin %87 %93
OpStore %_2_mn %86
%97 = OpLoad %v3float %l
%98 = OpCompositeExtract %float %97 0
%99 = OpLoad %v3float %l
%100 = OpCompositeExtract %float %99 1
%96 = OpExtInst %float %1 FMax %98 %100
%101 = OpLoad %v3float %l
%102 = OpCompositeExtract %float %101 2
%95 = OpExtInst %float %1 FMax %96 %102
OpStore %_3_mx %95
%104 = OpLoad %float %_3_mx
%105 = OpLoad %float %_2_mn
%106 = OpFSub %float %104 %105
OpStore %_4_sat %106
%107 = OpLoad %float %_4_sat
%109 = OpFOrdGreaterThan %bool %107 %float_0
OpSelectionMerge %113 None
OpBranchConditional %109 %111 %112
%111 = OpLabel
%114 = OpLoad %v3float %l
%115 = OpLoad %float %_2_mn
%116 = OpCompositeConstruct %v3float %115 %115 %115
%117 = OpFSub %v3float %114 %116
%118 = OpLoad %v3float %r
OpStore %119 %118
%120 = OpFunctionCall %float %blend_color_saturation_Qhh3 %119
%121 = OpVectorTimesScalar %v3float %117 %120
%122 = OpLoad %float %_4_sat
%124 = OpFDiv %float %float_1 %122
%125 = OpVectorTimesScalar %v3float %121 %124
OpStore %110 %125
OpBranch %113
%112 = OpLabel
OpStore %110 %126
OpBranch %113
%113 = OpLabel
%127 = OpLoad %v3float %110
OpStore %l %127
%128 = OpLoad %v3float %dsa
OpStore %r %128
OpBranch %84
%84 = OpLabel
%135 = OpLoad %v3float %r
%130 = OpDot %float %134 %135
OpStore %_5_lum %130
%137 = OpLoad %float %_5_lum
%139 = OpLoad %v3float %l
%138 = OpDot %float %134 %139
%140 = OpFSub %float %137 %138
%141 = OpLoad %v3float %l
%142 = OpCompositeConstruct %v3float %140 %140 %140
%143 = OpFAdd %v3float %142 %141
OpStore %_6_result %143
%147 = OpLoad %v3float %_6_result
%148 = OpCompositeExtract %float %147 0
%149 = OpLoad %v3float %_6_result
%150 = OpCompositeExtract %float %149 1
%146 = OpExtInst %float %1 FMin %148 %150
%151 = OpLoad %v3float %_6_result
%152 = OpCompositeExtract %float %151 2
%145 = OpExtInst %float %1 FMin %146 %152
OpStore %_7_minComp %145
%156 = OpLoad %v3float %_6_result
%157 = OpCompositeExtract %float %156 0
%158 = OpLoad %v3float %_6_result
%159 = OpCompositeExtract %float %158 1
%155 = OpExtInst %float %1 FMax %157 %159
%160 = OpLoad %v3float %_6_result
%161 = OpCompositeExtract %float %160 2
%154 = OpExtInst %float %1 FMax %155 %161
OpStore %_8_maxComp %154
%163 = OpLoad %float %_7_minComp
%164 = OpFOrdLessThan %bool %163 %float_0
OpSelectionMerge %166 None
OpBranchConditional %164 %165 %166
%165 = OpLabel
%167 = OpLoad %float %_5_lum
%168 = OpLoad %float %_7_minComp
%169 = OpFUnordNotEqual %bool %167 %168
OpBranch %166
%166 = OpLabel
%170 = OpPhi %bool %false %84 %169 %165
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpLoad %float %_5_lum
%174 = OpLoad %v3float %_6_result
%175 = OpLoad %float %_5_lum
%176 = OpCompositeConstruct %v3float %175 %175 %175
%177 = OpFSub %v3float %174 %176
%178 = OpLoad %float %_5_lum
%179 = OpLoad %float %_5_lum
%180 = OpLoad %float %_7_minComp
%181 = OpFSub %float %179 %180
%182 = OpFDiv %float %178 %181
%183 = OpVectorTimesScalar %v3float %177 %182
%184 = OpCompositeConstruct %v3float %173 %173 %173
%185 = OpFAdd %v3float %184 %183
OpStore %_6_result %185
OpBranch %172
%172 = OpLabel
%186 = OpLoad %float %_8_maxComp
%187 = OpLoad %float %alpha
%188 = OpFOrdGreaterThan %bool %186 %187
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpLoad %float %_8_maxComp
%192 = OpLoad %float %_5_lum
%193 = OpFUnordNotEqual %bool %191 %192
OpBranch %190
%190 = OpLabel
%194 = OpPhi %bool %false %172 %193 %189
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
%197 = OpLoad %float %_5_lum
%198 = OpLoad %v3float %_6_result
%199 = OpLoad %float %_5_lum
%200 = OpCompositeConstruct %v3float %199 %199 %199
%201 = OpFSub %v3float %198 %200
%202 = OpLoad %float %alpha
%203 = OpLoad %float %_5_lum
%204 = OpFSub %float %202 %203
%205 = OpVectorTimesScalar %v3float %201 %204
%206 = OpLoad %float %_8_maxComp
%207 = OpLoad %float %_5_lum
%208 = OpFSub %float %206 %207
%209 = OpFDiv %float %float_1 %208
%210 = OpVectorTimesScalar %v3float %205 %209
%211 = OpCompositeConstruct %v3float %197 %197 %197
%212 = OpFAdd %v3float %211 %210
OpStore %_6_result %212
OpBranch %196
%196 = OpLabel
%213 = OpLoad %v3float %_6_result
%214 = OpLoad %v4float %41
%215 = OpVectorShuffle %v3float %214 %214 0 1 2
%216 = OpFAdd %v3float %213 %215
%217 = OpLoad %v3float %dsa
%218 = OpFSub %v3float %216 %217
%219 = OpLoad %v4float %40
%220 = OpVectorShuffle %v3float %219 %219 0 1 2
%221 = OpFAdd %v3float %218 %220
%222 = OpLoad %v3float %sda
%223 = OpFSub %v3float %221 %222
%224 = OpCompositeExtract %float %223 0
%225 = OpCompositeExtract %float %223 1
%226 = OpCompositeExtract %float %223 2
%227 = OpLoad %v4float %40
%228 = OpCompositeExtract %float %227 3
%229 = OpLoad %v4float %41
%230 = OpCompositeExtract %float %229 3
%231 = OpFAdd %float %228 %230
%232 = OpLoad %float %alpha
%233 = OpFSub %float %231 %232
%234 = OpCompositeConstruct %v4float %224 %225 %226 %233
OpReturnValue %234
OpFunctionEnd
%main = OpFunction %void None %236
%237 = OpLabel
%243 = OpVariable %_ptr_Function_v4float Function
%247 = OpVariable %_ptr_Function_v4float Function
%248 = OpVariable %_ptr_Function_bool Function
%249 = OpVariable %_ptr_Function_bool Function
%238 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%242 = OpLoad %v4float %238
OpStore %243 %242
%244 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%246 = OpLoad %v4float %244
OpStore %247 %246
OpStore %248 %false
OpStore %249 %false
%250 = OpFunctionCall %v4float %blend_hslc_h4h4h4bb %243 %247 %248 %249
OpStore %sk_FragColor %250
OpReturn
OpFunctionEnd
