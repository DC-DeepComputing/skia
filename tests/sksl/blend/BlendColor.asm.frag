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
OpName %blend_hslc_h4h2h4h4 "blend_hslc_h4h2h4h4"
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
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %27 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %_2_mn RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %_3_mx RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %_4_lum RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %_5_result RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %_6_minComp RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %_7_maxComp RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
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
OpDecorate %209 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
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
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%24 = OpTypeFunction %float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%31 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%55 = OpTypeFunction %v4float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%125 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%void = OpTypeVoid
%201 = OpTypeFunction %void
%203 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_color_luminance_Qhh3 = OpFunction %float None %24
%25 = OpFunctionParameter %_ptr_Function_v3float
%26 = OpLabel
%32 = OpLoad %v3float %25
%27 = OpDot %float %31 %32
OpReturnValue %27
OpFunctionEnd
%blend_color_saturation_Qhh3 = OpFunction %float None %24
%33 = OpFunctionParameter %_ptr_Function_v3float
%34 = OpLabel
%37 = OpLoad %v3float %33
%38 = OpCompositeExtract %float %37 0
%39 = OpLoad %v3float %33
%40 = OpCompositeExtract %float %39 1
%36 = OpExtInst %float %1 FMax %38 %40
%41 = OpLoad %v3float %33
%42 = OpCompositeExtract %float %41 2
%35 = OpExtInst %float %1 FMax %36 %42
%45 = OpLoad %v3float %33
%46 = OpCompositeExtract %float %45 0
%47 = OpLoad %v3float %33
%48 = OpCompositeExtract %float %47 1
%44 = OpExtInst %float %1 FMin %46 %48
%49 = OpLoad %v3float %33
%50 = OpCompositeExtract %float %49 2
%43 = OpExtInst %float %1 FMin %44 %50
%51 = OpFSub %float %35 %43
OpReturnValue %51
OpFunctionEnd
%blend_hslc_h4h2h4h4 = OpFunction %v4float None %55
%56 = OpFunctionParameter %_ptr_Function_v2float
%57 = OpFunctionParameter %_ptr_Function_v4float
%58 = OpFunctionParameter %_ptr_Function_v4float
%59 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%83 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%92 = OpVariable %_ptr_Function_v3float Function
%_2_mn = OpVariable %_ptr_Function_float Function
%_3_mx = OpVariable %_ptr_Function_float Function
%112 = OpVariable %_ptr_Function_v3float Function
%118 = OpVariable %_ptr_Function_v3float Function
%_4_lum = OpVariable %_ptr_Function_float Function
%129 = OpVariable %_ptr_Function_v3float Function
%_5_result = OpVariable %_ptr_Function_v3float Function
%133 = OpVariable %_ptr_Function_v3float Function
%_6_minComp = OpVariable %_ptr_Function_float Function
%_7_maxComp = OpVariable %_ptr_Function_float Function
%62 = OpLoad %v4float %58
%63 = OpCompositeExtract %float %62 3
%64 = OpLoad %v4float %57
%65 = OpCompositeExtract %float %64 3
%66 = OpFMul %float %63 %65
OpStore %alpha %66
%68 = OpLoad %v4float %57
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%70 = OpLoad %v4float %58
%71 = OpCompositeExtract %float %70 3
%72 = OpVectorTimesScalar %v3float %69 %71
OpStore %sda %72
%74 = OpLoad %v4float %58
%75 = OpVectorShuffle %v3float %74 %74 0 1 2
%76 = OpLoad %v4float %57
%77 = OpCompositeExtract %float %76 3
%78 = OpVectorTimesScalar %v3float %75 %77
OpStore %dsa %78
%80 = OpLoad %v2float %56
%81 = OpCompositeExtract %float %80 0
%82 = OpFUnordNotEqual %bool %81 %float_0
OpSelectionMerge %86 None
OpBranchConditional %82 %84 %85
%84 = OpLabel
OpStore %83 %78
OpBranch %86
%85 = OpLabel
OpStore %83 %72
OpBranch %86
%86 = OpLabel
%87 = OpLoad %v3float %83
OpStore %l %87
%89 = OpLoad %v2float %56
%90 = OpCompositeExtract %float %89 0
%91 = OpFUnordNotEqual %bool %90 %float_0
OpSelectionMerge %95 None
OpBranchConditional %91 %93 %94
%93 = OpLabel
OpStore %92 %72
OpBranch %95
%94 = OpLabel
OpStore %92 %78
OpBranch %95
%95 = OpLabel
%96 = OpLoad %v3float %92
OpStore %r %96
%97 = OpLoad %v2float %56
%98 = OpCompositeExtract %float %97 1
%99 = OpFUnordNotEqual %bool %98 %float_0
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%105 = OpCompositeExtract %float %87 0
%106 = OpCompositeExtract %float %87 1
%104 = OpExtInst %float %1 FMin %105 %106
%107 = OpCompositeExtract %float %87 2
%103 = OpExtInst %float %1 FMin %104 %107
OpStore %_2_mn %103
%110 = OpExtInst %float %1 FMax %105 %106
%109 = OpExtInst %float %1 FMax %110 %107
OpStore %_3_mx %109
%111 = OpFOrdGreaterThan %bool %109 %103
OpSelectionMerge %115 None
OpBranchConditional %111 %113 %114
%113 = OpLabel
%116 = OpCompositeConstruct %v3float %103 %103 %103
%117 = OpFSub %v3float %87 %116
OpStore %118 %96
%119 = OpFunctionCall %float %blend_color_saturation_Qhh3 %118
%120 = OpVectorTimesScalar %v3float %117 %119
%121 = OpFSub %float %109 %103
%123 = OpFDiv %float %float_1 %121
%124 = OpVectorTimesScalar %v3float %120 %123
OpStore %112 %124
OpBranch %115
%114 = OpLabel
OpStore %112 %125
OpBranch %115
%115 = OpLabel
%126 = OpLoad %v3float %112
OpStore %l %126
OpStore %r %78
OpBranch %101
%101 = OpLabel
%128 = OpLoad %v3float %r
OpStore %129 %128
%130 = OpFunctionCall %float %blend_color_luminance_Qhh3 %129
OpStore %_4_lum %130
%132 = OpLoad %v3float %l
OpStore %133 %132
%134 = OpFunctionCall %float %blend_color_luminance_Qhh3 %133
%135 = OpFSub %float %130 %134
%136 = OpLoad %v3float %l
%137 = OpCompositeConstruct %v3float %135 %135 %135
%138 = OpFAdd %v3float %137 %136
OpStore %_5_result %138
%142 = OpCompositeExtract %float %138 0
%143 = OpCompositeExtract %float %138 1
%141 = OpExtInst %float %1 FMin %142 %143
%144 = OpCompositeExtract %float %138 2
%140 = OpExtInst %float %1 FMin %141 %144
OpStore %_6_minComp %140
%147 = OpExtInst %float %1 FMax %142 %143
%146 = OpExtInst %float %1 FMax %147 %144
OpStore %_7_maxComp %146
%148 = OpFOrdLessThan %bool %140 %float_0
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%151 = OpFUnordNotEqual %bool %130 %140
OpBranch %150
%150 = OpLabel
%152 = OpPhi %bool %false %101 %151 %149
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpCompositeConstruct %v3float %130 %130 %130
%156 = OpFSub %v3float %138 %155
%157 = OpFSub %float %130 %140
%158 = OpLoad %float %_kGuardedDivideEpsilon
%159 = OpFAdd %float %157 %158
%160 = OpFDiv %float %130 %159
%161 = OpVectorTimesScalar %v3float %156 %160
%162 = OpFAdd %v3float %155 %161
OpStore %_5_result %162
OpBranch %154
%154 = OpLabel
%163 = OpFOrdGreaterThan %bool %146 %66
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%166 = OpFUnordNotEqual %bool %146 %130
OpBranch %165
%165 = OpLabel
%167 = OpPhi %bool %false %154 %166 %164
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%170 = OpLoad %v3float %_5_result
%171 = OpCompositeConstruct %v3float %130 %130 %130
%172 = OpFSub %v3float %170 %171
%173 = OpFSub %float %66 %130
%174 = OpVectorTimesScalar %v3float %172 %173
%175 = OpFSub %float %146 %130
%176 = OpLoad %float %_kGuardedDivideEpsilon
%177 = OpFAdd %float %175 %176
%178 = OpFDiv %float %float_1 %177
%179 = OpVectorTimesScalar %v3float %174 %178
%180 = OpFAdd %v3float %171 %179
OpStore %_5_result %180
OpBranch %169
%169 = OpLabel
%181 = OpLoad %v3float %_5_result
%182 = OpLoad %v4float %58
%183 = OpVectorShuffle %v3float %182 %182 0 1 2
%184 = OpFAdd %v3float %181 %183
%185 = OpFSub %v3float %184 %78
%186 = OpLoad %v4float %57
%187 = OpVectorShuffle %v3float %186 %186 0 1 2
%188 = OpFAdd %v3float %185 %187
%189 = OpFSub %v3float %188 %72
%190 = OpCompositeExtract %float %189 0
%191 = OpCompositeExtract %float %189 1
%192 = OpCompositeExtract %float %189 2
%193 = OpLoad %v4float %57
%194 = OpCompositeExtract %float %193 3
%195 = OpLoad %v4float %58
%196 = OpCompositeExtract %float %195 3
%197 = OpFAdd %float %194 %196
%198 = OpFSub %float %197 %66
%199 = OpCompositeConstruct %v4float %190 %191 %192 %198
OpReturnValue %199
OpFunctionEnd
%main = OpFunction %void None %201
%202 = OpLabel
%204 = OpVariable %_ptr_Function_v2float Function
%210 = OpVariable %_ptr_Function_v4float Function
%214 = OpVariable %_ptr_Function_v4float Function
%11 = OpSelect %float %false %float_9_99999994en09 %float_0
OpStore %_kGuardedDivideEpsilon %11
OpStore %204 %203
%205 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%209 = OpLoad %v4float %205
OpStore %210 %209
%211 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%213 = OpLoad %v4float %211
OpStore %214 %213
%215 = OpFunctionCall %v4float %blend_hslc_h4h2h4h4 %204 %210 %214
OpStore %sk_FragColor %215
OpReturn
OpFunctionEnd
