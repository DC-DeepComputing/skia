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
OpName %blend_hslc_h4h4h4h2 "blend_hslc_h4h4h4h2"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %flip "flip"
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
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %_2_mn RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %_3_mx RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %_4_lum RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %_5_result RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %_6_minComp RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %_7_maxComp RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
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
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
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
OpDecorate %191 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%40 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_bool = OpTypePointer Function %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%110 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%117 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%void = OpTypeVoid
%185 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%197 = OpConstantComposite %v2float %float_1 %float_1
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
%blend_hslc_h4h4h4h2 = OpFunction %v4float None %40
%41 = OpFunctionParameter %_ptr_Function_v4float
%42 = OpFunctionParameter %_ptr_Function_v4float
%43 = OpFunctionParameter %_ptr_Function_v2float
%44 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%flip = OpVariable %_ptr_Function_bool Function
%l = OpVariable %_ptr_Function_v3float Function
%71 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%77 = OpVariable %_ptr_Function_v3float Function
%_2_mn = OpVariable %_ptr_Function_float Function
%_3_mx = OpVariable %_ptr_Function_float Function
%97 = OpVariable %_ptr_Function_v3float Function
%103 = OpVariable %_ptr_Function_v3float Function
%_4_lum = OpVariable %_ptr_Function_float Function
%_5_result = OpVariable %_ptr_Function_v3float Function
%_6_minComp = OpVariable %_ptr_Function_float Function
%_7_maxComp = OpVariable %_ptr_Function_float Function
%47 = OpLoad %v4float %42
%48 = OpCompositeExtract %float %47 3
%49 = OpLoad %v4float %41
%50 = OpCompositeExtract %float %49 3
%51 = OpFMul %float %48 %50
OpStore %alpha %51
%53 = OpLoad %v4float %41
%54 = OpVectorShuffle %v3float %53 %53 0 1 2
%55 = OpLoad %v4float %42
%56 = OpCompositeExtract %float %55 3
%57 = OpVectorTimesScalar %v3float %54 %56
OpStore %sda %57
%59 = OpLoad %v4float %42
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%61 = OpLoad %v4float %41
%62 = OpCompositeExtract %float %61 3
%63 = OpVectorTimesScalar %v3float %60 %62
OpStore %dsa %63
%66 = OpLoad %v2float %43
%67 = OpCompositeExtract %float %66 0
%68 = OpFUnordNotEqual %bool %67 %float_0
OpStore %flip %68
OpSelectionMerge %74 None
OpBranchConditional %68 %72 %73
%72 = OpLabel
OpStore %71 %63
OpBranch %74
%73 = OpLabel
OpStore %71 %57
OpBranch %74
%74 = OpLabel
%75 = OpLoad %v3float %71
OpStore %l %75
OpSelectionMerge %80 None
OpBranchConditional %68 %78 %79
%78 = OpLabel
OpStore %77 %57
OpBranch %80
%79 = OpLabel
OpStore %77 %63
OpBranch %80
%80 = OpLabel
%81 = OpLoad %v3float %77
OpStore %r %81
%82 = OpLoad %v2float %43
%83 = OpCompositeExtract %float %82 1
%84 = OpFUnordNotEqual %bool %83 %float_0
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%90 = OpCompositeExtract %float %75 0
%91 = OpCompositeExtract %float %75 1
%89 = OpExtInst %float %1 FMin %90 %91
%92 = OpCompositeExtract %float %75 2
%88 = OpExtInst %float %1 FMin %89 %92
OpStore %_2_mn %88
%95 = OpExtInst %float %1 FMax %90 %91
%94 = OpExtInst %float %1 FMax %95 %92
OpStore %_3_mx %94
%96 = OpFOrdGreaterThan %bool %94 %88
OpSelectionMerge %100 None
OpBranchConditional %96 %98 %99
%98 = OpLabel
%101 = OpCompositeConstruct %v3float %88 %88 %88
%102 = OpFSub %v3float %75 %101
OpStore %103 %81
%104 = OpFunctionCall %float %blend_color_saturation_Qhh3 %103
%105 = OpVectorTimesScalar %v3float %102 %104
%106 = OpFSub %float %94 %88
%108 = OpFDiv %float %float_1 %106
%109 = OpVectorTimesScalar %v3float %105 %108
OpStore %97 %109
OpBranch %100
%99 = OpLabel
OpStore %97 %110
OpBranch %100
%100 = OpLabel
%111 = OpLoad %v3float %97
OpStore %l %111
OpStore %r %63
OpBranch %86
%86 = OpLabel
%118 = OpLoad %v3float %r
%113 = OpDot %float %117 %118
OpStore %_4_lum %113
%121 = OpLoad %v3float %l
%120 = OpDot %float %117 %121
%122 = OpFSub %float %113 %120
%123 = OpLoad %v3float %l
%124 = OpCompositeConstruct %v3float %122 %122 %122
%125 = OpFAdd %v3float %124 %123
OpStore %_5_result %125
%129 = OpCompositeExtract %float %125 0
%130 = OpCompositeExtract %float %125 1
%128 = OpExtInst %float %1 FMin %129 %130
%131 = OpCompositeExtract %float %125 2
%127 = OpExtInst %float %1 FMin %128 %131
OpStore %_6_minComp %127
%134 = OpExtInst %float %1 FMax %129 %130
%133 = OpExtInst %float %1 FMax %134 %131
OpStore %_7_maxComp %133
%136 = OpFOrdLessThan %bool %127 %float_0
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%139 = OpFUnordNotEqual %bool %113 %127
OpBranch %138
%138 = OpLabel
%140 = OpPhi %bool %false %86 %139 %137
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%143 = OpCompositeConstruct %v3float %113 %113 %113
%144 = OpFSub %v3float %125 %143
%145 = OpFSub %float %113 %127
%146 = OpFDiv %float %113 %145
%147 = OpVectorTimesScalar %v3float %144 %146
%148 = OpFAdd %v3float %143 %147
OpStore %_5_result %148
OpBranch %142
%142 = OpLabel
%149 = OpFOrdGreaterThan %bool %133 %51
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%152 = OpFUnordNotEqual %bool %133 %113
OpBranch %151
%151 = OpLabel
%153 = OpPhi %bool %false %142 %152 %150
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%156 = OpLoad %v3float %_5_result
%157 = OpCompositeConstruct %v3float %113 %113 %113
%158 = OpFSub %v3float %156 %157
%159 = OpFSub %float %51 %113
%160 = OpVectorTimesScalar %v3float %158 %159
%161 = OpFSub %float %133 %113
%162 = OpFDiv %float %float_1 %161
%163 = OpVectorTimesScalar %v3float %160 %162
%164 = OpFAdd %v3float %157 %163
OpStore %_5_result %164
OpBranch %155
%155 = OpLabel
%165 = OpLoad %v3float %_5_result
%166 = OpLoad %v4float %42
%167 = OpVectorShuffle %v3float %166 %166 0 1 2
%168 = OpFAdd %v3float %165 %167
%169 = OpFSub %v3float %168 %63
%170 = OpLoad %v4float %41
%171 = OpVectorShuffle %v3float %170 %170 0 1 2
%172 = OpFAdd %v3float %169 %171
%173 = OpFSub %v3float %172 %57
%174 = OpCompositeExtract %float %173 0
%175 = OpCompositeExtract %float %173 1
%176 = OpCompositeExtract %float %173 2
%177 = OpLoad %v4float %41
%178 = OpCompositeExtract %float %177 3
%179 = OpLoad %v4float %42
%180 = OpCompositeExtract %float %179 3
%181 = OpFAdd %float %178 %180
%182 = OpFSub %float %181 %51
%183 = OpCompositeConstruct %v4float %174 %175 %176 %182
OpReturnValue %183
OpFunctionEnd
%main = OpFunction %void None %185
%186 = OpLabel
%192 = OpVariable %_ptr_Function_v4float Function
%196 = OpVariable %_ptr_Function_v4float Function
%198 = OpVariable %_ptr_Function_v2float Function
%187 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%191 = OpLoad %v4float %187
OpStore %192 %191
%193 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%195 = OpLoad %v4float %193
OpStore %196 %195
OpStore %198 %197
%199 = OpFunctionCall %v4float %blend_hslc_h4h4h4h2 %192 %196 %198
OpStore %sk_FragColor %199
OpReturn
OpFunctionEnd
