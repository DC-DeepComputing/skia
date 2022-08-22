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
OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"
OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
OpName %blend_hslc_h4h4h4h2 "blend_hslc_h4h4h4h2"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_4_mn "_4_mn"
OpName %_5_mx "_5_mx"
OpName %_6_lum "_6_lum"
OpName %_7_result "_7_result"
OpName %_8_minComp "_8_minComp"
OpName %_9_maxComp "_9_maxComp"
OpName %_10_n "_10_n"
OpName %_11_d "_11_d"
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
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %_4_mn RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %_5_mx RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %_6_lum RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %_7_result RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %_8_minComp RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %_9_maxComp RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %_10_n RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %_11_d RelaxedPrecision
OpDecorate %184 RelaxedPrecision
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
OpDecorate %222 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%18 = OpTypeFunction %float %_ptr_Function_v3float
%_ptr_Function_float = OpTypePointer Function %float
%39 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%60 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_v2float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%130 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%137 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%void = OpTypeVoid
%216 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%228 = OpConstantComposite %v2float %float_0 %float_0
%blend_color_saturation_Qhh3 = OpFunction %float None %18
%19 = OpFunctionParameter %_ptr_Function_v3float
%20 = OpLabel
%23 = OpLoad %v3float %19
%24 = OpCompositeExtract %float %23 0
%25 = OpLoad %v3float %19
%26 = OpCompositeExtract %float %25 1
%22 = OpExtInst %float %1 FMax %24 %26
%27 = OpLoad %v3float %19
%28 = OpCompositeExtract %float %27 2
%21 = OpExtInst %float %1 FMax %22 %28
%31 = OpLoad %v3float %19
%32 = OpCompositeExtract %float %31 0
%33 = OpLoad %v3float %19
%34 = OpCompositeExtract %float %33 1
%30 = OpExtInst %float %1 FMin %32 %34
%35 = OpLoad %v3float %19
%36 = OpCompositeExtract %float %35 2
%29 = OpExtInst %float %1 FMin %30 %36
%37 = OpFSub %float %21 %29
OpReturnValue %37
OpFunctionEnd
%guarded_divide_Qhhh = OpFunction %float None %39
%40 = OpFunctionParameter %_ptr_Function_float
%41 = OpFunctionParameter %_ptr_Function_float
%42 = OpLabel
%44 = OpVariable %_ptr_Function_float Function
OpSelectionMerge %47 None
OpBranchConditional %false %45 %46
%45 = OpLabel
%48 = OpLoad %float %40
%49 = OpLoad %float %41
%51 = OpFAdd %float %49 %float_9_99999994en09
%52 = OpFDiv %float %48 %51
OpStore %44 %52
OpBranch %47
%46 = OpLabel
%53 = OpLoad %float %40
%54 = OpLoad %float %41
%55 = OpFDiv %float %53 %54
OpStore %44 %55
OpBranch %47
%47 = OpLabel
%56 = OpLoad %float %44
OpReturnValue %56
OpFunctionEnd
%blend_hslc_h4h4h4h2 = OpFunction %v4float None %60
%61 = OpFunctionParameter %_ptr_Function_v4float
%62 = OpFunctionParameter %_ptr_Function_v4float
%63 = OpFunctionParameter %_ptr_Function_v2float
%64 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%88 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%97 = OpVariable %_ptr_Function_v3float Function
%_4_mn = OpVariable %_ptr_Function_float Function
%_5_mx = OpVariable %_ptr_Function_float Function
%117 = OpVariable %_ptr_Function_v3float Function
%123 = OpVariable %_ptr_Function_v3float Function
%_6_lum = OpVariable %_ptr_Function_float Function
%_7_result = OpVariable %_ptr_Function_v3float Function
%_8_minComp = OpVariable %_ptr_Function_float Function
%_9_maxComp = OpVariable %_ptr_Function_float Function
%164 = OpVariable %_ptr_Function_float Function
%166 = OpVariable %_ptr_Function_float Function
%_10_n = OpVariable %_ptr_Function_v3float Function
%_11_d = OpVariable %_ptr_Function_float Function
%185 = OpVariable %_ptr_Function_v3float Function
%66 = OpLoad %v4float %62
%67 = OpCompositeExtract %float %66 3
%68 = OpLoad %v4float %61
%69 = OpCompositeExtract %float %68 3
%70 = OpFMul %float %67 %69
OpStore %alpha %70
%72 = OpLoad %v4float %61
%73 = OpVectorShuffle %v3float %72 %72 0 1 2
%74 = OpLoad %v4float %62
%75 = OpCompositeExtract %float %74 3
%76 = OpVectorTimesScalar %v3float %73 %75
OpStore %sda %76
%78 = OpLoad %v4float %62
%79 = OpVectorShuffle %v3float %78 %78 0 1 2
%80 = OpLoad %v4float %61
%81 = OpCompositeExtract %float %80 3
%82 = OpVectorTimesScalar %v3float %79 %81
OpStore %dsa %82
%84 = OpLoad %v2float %63
%85 = OpCompositeExtract %float %84 0
%86 = OpFUnordNotEqual %bool %85 %float_0
OpSelectionMerge %91 None
OpBranchConditional %86 %89 %90
%89 = OpLabel
OpStore %88 %82
OpBranch %91
%90 = OpLabel
OpStore %88 %76
OpBranch %91
%91 = OpLabel
%92 = OpLoad %v3float %88
OpStore %l %92
%94 = OpLoad %v2float %63
%95 = OpCompositeExtract %float %94 0
%96 = OpFUnordNotEqual %bool %95 %float_0
OpSelectionMerge %100 None
OpBranchConditional %96 %98 %99
%98 = OpLabel
OpStore %97 %76
OpBranch %100
%99 = OpLabel
OpStore %97 %82
OpBranch %100
%100 = OpLabel
%101 = OpLoad %v3float %97
OpStore %r %101
%102 = OpLoad %v2float %63
%103 = OpCompositeExtract %float %102 1
%104 = OpFUnordNotEqual %bool %103 %float_0
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%110 = OpCompositeExtract %float %92 0
%111 = OpCompositeExtract %float %92 1
%109 = OpExtInst %float %1 FMin %110 %111
%112 = OpCompositeExtract %float %92 2
%108 = OpExtInst %float %1 FMin %109 %112
OpStore %_4_mn %108
%115 = OpExtInst %float %1 FMax %110 %111
%114 = OpExtInst %float %1 FMax %115 %112
OpStore %_5_mx %114
%116 = OpFOrdGreaterThan %bool %114 %108
OpSelectionMerge %120 None
OpBranchConditional %116 %118 %119
%118 = OpLabel
%121 = OpCompositeConstruct %v3float %108 %108 %108
%122 = OpFSub %v3float %92 %121
OpStore %123 %101
%124 = OpFunctionCall %float %blend_color_saturation_Qhh3 %123
%125 = OpVectorTimesScalar %v3float %122 %124
%126 = OpFSub %float %114 %108
%128 = OpFDiv %float %float_1 %126
%129 = OpVectorTimesScalar %v3float %125 %128
OpStore %117 %129
OpBranch %120
%119 = OpLabel
OpStore %117 %130
OpBranch %120
%120 = OpLabel
%131 = OpLoad %v3float %117
OpStore %l %131
OpStore %r %82
OpBranch %106
%106 = OpLabel
%138 = OpLoad %v3float %r
%133 = OpDot %float %137 %138
OpStore %_6_lum %133
%141 = OpLoad %v3float %l
%140 = OpDot %float %137 %141
%142 = OpFSub %float %133 %140
%143 = OpLoad %v3float %l
%144 = OpCompositeConstruct %v3float %142 %142 %142
%145 = OpFAdd %v3float %144 %143
OpStore %_7_result %145
%149 = OpCompositeExtract %float %145 0
%150 = OpCompositeExtract %float %145 1
%148 = OpExtInst %float %1 FMin %149 %150
%151 = OpCompositeExtract %float %145 2
%147 = OpExtInst %float %1 FMin %148 %151
OpStore %_8_minComp %147
%154 = OpExtInst %float %1 FMax %149 %150
%153 = OpExtInst %float %1 FMax %154 %151
OpStore %_9_maxComp %153
%155 = OpFOrdLessThan %bool %147 %float_0
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpFUnordNotEqual %bool %133 %147
OpBranch %157
%157 = OpLabel
%159 = OpPhi %bool %false %106 %158 %156
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpCompositeConstruct %v3float %133 %133 %133
%163 = OpFSub %v3float %145 %162
OpStore %164 %133
%165 = OpFSub %float %133 %147
OpStore %166 %165
%167 = OpFunctionCall %float %guarded_divide_Qhhh %164 %166
%168 = OpVectorTimesScalar %v3float %163 %167
%169 = OpFAdd %v3float %162 %168
OpStore %_7_result %169
OpBranch %161
%161 = OpLabel
%170 = OpFOrdGreaterThan %bool %153 %70
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpFUnordNotEqual %bool %153 %133
OpBranch %172
%172 = OpLabel
%174 = OpPhi %bool %false %161 %173 %171
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%178 = OpLoad %v3float %_7_result
%179 = OpCompositeConstruct %v3float %133 %133 %133
%180 = OpFSub %v3float %178 %179
%181 = OpFSub %float %70 %133
%182 = OpVectorTimesScalar %v3float %180 %181
OpStore %_10_n %182
%184 = OpFSub %float %153 %133
OpStore %_11_d %184
OpSelectionMerge %188 None
OpBranchConditional %false %186 %187
%186 = OpLabel
%189 = OpFAdd %float %184 %float_9_99999994en09
%190 = OpFDiv %float %float_1 %189
%191 = OpVectorTimesScalar %v3float %182 %190
OpStore %185 %191
OpBranch %188
%187 = OpLabel
%192 = OpFDiv %float %float_1 %184
%193 = OpVectorTimesScalar %v3float %182 %192
OpStore %185 %193
OpBranch %188
%188 = OpLabel
%194 = OpLoad %v3float %185
%195 = OpFAdd %v3float %179 %194
OpStore %_7_result %195
OpBranch %176
%176 = OpLabel
%196 = OpLoad %v3float %_7_result
%197 = OpLoad %v4float %62
%198 = OpVectorShuffle %v3float %197 %197 0 1 2
%199 = OpFAdd %v3float %196 %198
%200 = OpFSub %v3float %199 %82
%201 = OpLoad %v4float %61
%202 = OpVectorShuffle %v3float %201 %201 0 1 2
%203 = OpFAdd %v3float %200 %202
%204 = OpFSub %v3float %203 %76
%205 = OpCompositeExtract %float %204 0
%206 = OpCompositeExtract %float %204 1
%207 = OpCompositeExtract %float %204 2
%208 = OpLoad %v4float %61
%209 = OpCompositeExtract %float %208 3
%210 = OpLoad %v4float %62
%211 = OpCompositeExtract %float %210 3
%212 = OpFAdd %float %209 %211
%213 = OpFSub %float %212 %70
%214 = OpCompositeConstruct %v4float %205 %206 %207 %213
OpReturnValue %214
OpFunctionEnd
%main = OpFunction %void None %216
%217 = OpLabel
%223 = OpVariable %_ptr_Function_v4float Function
%227 = OpVariable %_ptr_Function_v4float Function
%229 = OpVariable %_ptr_Function_v2float Function
%218 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%222 = OpLoad %v4float %218
OpStore %223 %222
%224 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%226 = OpLoad %v4float %224
OpStore %227 %226
OpStore %229 %228
%230 = OpFunctionCall %v4float %blend_hslc_h4h4h4h2 %223 %227 %229
OpStore %sk_FragColor %230
OpReturn
OpFunctionEnd
