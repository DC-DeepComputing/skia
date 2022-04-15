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
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %_4_lum RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %_5_result RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %_6_minComp RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %_7_maxComp RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
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
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
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
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
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
OpDecorate %228 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
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
%17 = OpTypeFunction %float %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%39 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_bool %_ptr_Function_bool
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%123 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%131 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%void = OpTypeVoid
%222 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%true = OpConstantTrue %bool
%blend_color_saturation_Qhh3 = OpFunction %float None %17
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
%blend_hslc_h4h4h4bb = OpFunction %v4float None %39
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
%104 = OpVariable %_ptr_Function_v3float Function
%113 = OpVariable %_ptr_Function_v3float Function
%_4_lum = OpVariable %_ptr_Function_float Function
%_5_result = OpVariable %_ptr_Function_v3float Function
%_6_minComp = OpVariable %_ptr_Function_float Function
%_7_maxComp = OpVariable %_ptr_Function_float Function
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
%103 = OpFOrdGreaterThan %bool %95 %86
OpSelectionMerge %107 None
OpBranchConditional %103 %105 %106
%105 = OpLabel
%108 = OpLoad %v3float %l
%109 = OpLoad %float %_2_mn
%110 = OpCompositeConstruct %v3float %109 %109 %109
%111 = OpFSub %v3float %108 %110
%112 = OpLoad %v3float %r
OpStore %113 %112
%114 = OpFunctionCall %float %blend_color_saturation_Qhh3 %113
%115 = OpVectorTimesScalar %v3float %111 %114
%116 = OpLoad %float %_3_mx
%117 = OpLoad %float %_2_mn
%118 = OpFSub %float %116 %117
%120 = OpFDiv %float %float_1 %118
%121 = OpVectorTimesScalar %v3float %115 %120
OpStore %104 %121
OpBranch %107
%106 = OpLabel
OpStore %104 %123
OpBranch %107
%107 = OpLabel
%124 = OpLoad %v3float %104
OpStore %l %124
%125 = OpLoad %v3float %dsa
OpStore %r %125
OpBranch %84
%84 = OpLabel
%132 = OpLoad %v3float %r
%127 = OpDot %float %131 %132
OpStore %_4_lum %127
%135 = OpLoad %v3float %l
%134 = OpDot %float %131 %135
%136 = OpFSub %float %127 %134
%137 = OpLoad %v3float %l
%138 = OpCompositeConstruct %v3float %136 %136 %136
%139 = OpFAdd %v3float %138 %137
OpStore %_5_result %139
%143 = OpCompositeExtract %float %139 0
%144 = OpCompositeExtract %float %139 1
%142 = OpExtInst %float %1 FMin %143 %144
%145 = OpCompositeExtract %float %139 2
%141 = OpExtInst %float %1 FMin %142 %145
OpStore %_6_minComp %141
%148 = OpExtInst %float %1 FMax %143 %144
%147 = OpExtInst %float %1 FMax %148 %145
OpStore %_7_maxComp %147
%150 = OpFOrdLessThan %bool %141 %float_0
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%153 = OpLoad %float %_4_lum
%154 = OpLoad %float %_6_minComp
%155 = OpFUnordNotEqual %bool %153 %154
OpBranch %152
%152 = OpLabel
%156 = OpPhi %bool %false %84 %155 %151
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%159 = OpLoad %float %_4_lum
%160 = OpLoad %v3float %_5_result
%161 = OpLoad %float %_4_lum
%162 = OpCompositeConstruct %v3float %161 %161 %161
%163 = OpFSub %v3float %160 %162
%164 = OpLoad %float %_4_lum
%165 = OpLoad %float %_4_lum
%166 = OpLoad %float %_6_minComp
%167 = OpFSub %float %165 %166
%168 = OpFDiv %float %164 %167
%169 = OpVectorTimesScalar %v3float %163 %168
%170 = OpCompositeConstruct %v3float %159 %159 %159
%171 = OpFAdd %v3float %170 %169
OpStore %_5_result %171
OpBranch %158
%158 = OpLabel
%172 = OpLoad %float %_7_maxComp
%173 = OpLoad %float %alpha
%174 = OpFOrdGreaterThan %bool %172 %173
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpLoad %float %_7_maxComp
%178 = OpLoad %float %_4_lum
%179 = OpFUnordNotEqual %bool %177 %178
OpBranch %176
%176 = OpLabel
%180 = OpPhi %bool %false %158 %179 %175
OpSelectionMerge %182 None
OpBranchConditional %180 %181 %182
%181 = OpLabel
%183 = OpLoad %float %_4_lum
%184 = OpLoad %v3float %_5_result
%185 = OpLoad %float %_4_lum
%186 = OpCompositeConstruct %v3float %185 %185 %185
%187 = OpFSub %v3float %184 %186
%188 = OpLoad %float %alpha
%189 = OpLoad %float %_4_lum
%190 = OpFSub %float %188 %189
%191 = OpVectorTimesScalar %v3float %187 %190
%192 = OpLoad %float %_7_maxComp
%193 = OpLoad %float %_4_lum
%194 = OpFSub %float %192 %193
%195 = OpFDiv %float %float_1 %194
%196 = OpVectorTimesScalar %v3float %191 %195
%197 = OpCompositeConstruct %v3float %183 %183 %183
%198 = OpFAdd %v3float %197 %196
OpStore %_5_result %198
OpBranch %182
%182 = OpLabel
%199 = OpLoad %v3float %_5_result
%200 = OpLoad %v4float %41
%201 = OpVectorShuffle %v3float %200 %200 0 1 2
%202 = OpFAdd %v3float %199 %201
%203 = OpLoad %v3float %dsa
%204 = OpFSub %v3float %202 %203
%205 = OpLoad %v4float %40
%206 = OpVectorShuffle %v3float %205 %205 0 1 2
%207 = OpFAdd %v3float %204 %206
%208 = OpLoad %v3float %sda
%209 = OpFSub %v3float %207 %208
%210 = OpCompositeExtract %float %209 0
%211 = OpCompositeExtract %float %209 1
%212 = OpCompositeExtract %float %209 2
%213 = OpLoad %v4float %40
%214 = OpCompositeExtract %float %213 3
%215 = OpLoad %v4float %41
%216 = OpCompositeExtract %float %215 3
%217 = OpFAdd %float %214 %216
%218 = OpLoad %float %alpha
%219 = OpFSub %float %217 %218
%220 = OpCompositeConstruct %v4float %210 %211 %212 %219
OpReturnValue %220
OpFunctionEnd
%main = OpFunction %void None %222
%223 = OpLabel
%229 = OpVariable %_ptr_Function_v4float Function
%233 = OpVariable %_ptr_Function_v4float Function
%234 = OpVariable %_ptr_Function_bool Function
%236 = OpVariable %_ptr_Function_bool Function
%224 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%228 = OpLoad %v4float %224
OpStore %229 %228
%230 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%232 = OpLoad %v4float %230
OpStore %233 %232
OpStore %234 %false
OpStore %236 %true
%237 = OpFunctionCall %v4float %blend_hslc_h4h4h4bb %229 %233 %234 %236
OpStore %sk_FragColor %237
OpReturn
OpFunctionEnd
