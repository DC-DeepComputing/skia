OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %30 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
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
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
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
OpDecorate %219 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%mat2v2float = OpTypeMatrix %v2float 2
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%49 = OpConstantComposite %v2float %float_n1_25 %float_0
%50 = OpConstantComposite %v2float %float_0_75 %float_2_25
%51 = OpConstantComposite %mat2v2float %49 %50
%v2bool = OpTypeVector %bool 2
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%91 = OpConstantComposite %v2float %float_0 %float_1
%92 = OpConstantComposite %mat2v2float %91 %91
%v4int = OpTypeVector %int 4
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%220 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%30 = OpLoad %bool %ok
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%37 = OpLoad %v4float %33
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%40 = OpCompositeExtract %float %37 2
%41 = OpCompositeExtract %float %37 3
%42 = OpCompositeConstruct %v2float %38 %39
%43 = OpCompositeConstruct %v2float %40 %41
%45 = OpCompositeConstruct %mat2v2float %42 %43
%53 = OpFOrdEqual %v2bool %42 %49
%54 = OpAll %bool %53
%55 = OpFOrdEqual %v2bool %43 %50
%56 = OpAll %bool %55
%57 = OpLogicalAnd %bool %54 %56
OpBranch %32
%32 = OpLabel
%58 = OpPhi %bool %false %25 %57 %31
OpStore %ok %58
%59 = OpLoad %bool %ok
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%63 = OpLoad %v4float %62
%64 = OpCompositeExtract %float %63 0
%65 = OpCompositeExtract %float %63 1
%66 = OpCompositeExtract %float %63 2
%67 = OpCompositeExtract %float %63 3
%68 = OpCompositeConstruct %v2float %64 %65
%69 = OpCompositeConstruct %v2float %66 %67
%70 = OpCompositeConstruct %mat2v2float %68 %69
%71 = OpFOrdEqual %v2bool %68 %49
%72 = OpAll %bool %71
%73 = OpFOrdEqual %v2bool %69 %50
%74 = OpAll %bool %73
%75 = OpLogicalAnd %bool %72 %74
OpBranch %61
%61 = OpLabel
%76 = OpPhi %bool %false %32 %75 %60
OpStore %ok %76
%77 = OpLoad %bool %ok
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%82 = OpLoad %v4float %80
%83 = OpCompositeExtract %float %82 0
%84 = OpCompositeExtract %float %82 1
%85 = OpCompositeExtract %float %82 2
%86 = OpCompositeExtract %float %82 3
%87 = OpCompositeConstruct %v2float %83 %84
%88 = OpCompositeConstruct %v2float %85 %86
%89 = OpCompositeConstruct %mat2v2float %87 %88
%93 = OpFOrdEqual %v2bool %87 %91
%94 = OpAll %bool %93
%95 = OpFOrdEqual %v2bool %88 %91
%96 = OpAll %bool %95
%97 = OpLogicalAnd %bool %94 %96
OpBranch %79
%79 = OpLabel
%98 = OpPhi %bool %false %61 %97 %78
OpStore %ok %98
%99 = OpLoad %bool %ok
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%103 = OpLoad %v4float %102
%104 = OpCompositeExtract %float %103 0
%105 = OpCompositeExtract %float %103 1
%106 = OpCompositeExtract %float %103 2
%107 = OpCompositeExtract %float %103 3
%108 = OpCompositeConstruct %v2float %104 %105
%109 = OpCompositeConstruct %v2float %106 %107
%110 = OpCompositeConstruct %mat2v2float %108 %109
%111 = OpFOrdEqual %v2bool %108 %91
%112 = OpAll %bool %111
%113 = OpFOrdEqual %v2bool %109 %91
%114 = OpAll %bool %113
%115 = OpLogicalAnd %bool %112 %114
OpBranch %101
%101 = OpLabel
%116 = OpPhi %bool %false %79 %115 %100
OpStore %ok %116
%117 = OpLoad %bool %ok
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%121 = OpLoad %v4float %120
%122 = OpCompositeExtract %float %121 0
%123 = OpConvertFToS %int %122
%124 = OpCompositeExtract %float %121 1
%125 = OpConvertFToS %int %124
%126 = OpCompositeExtract %float %121 2
%127 = OpConvertFToS %int %126
%128 = OpCompositeExtract %float %121 3
%129 = OpConvertFToS %int %128
%131 = OpCompositeConstruct %v4int %123 %125 %127 %129
%132 = OpConvertSToF %float %123
%133 = OpConvertSToF %float %125
%134 = OpConvertSToF %float %127
%135 = OpConvertSToF %float %129
%136 = OpCompositeConstruct %v4float %132 %133 %134 %135
%137 = OpCompositeExtract %float %136 0
%138 = OpCompositeExtract %float %136 1
%139 = OpCompositeExtract %float %136 2
%140 = OpCompositeExtract %float %136 3
%141 = OpCompositeConstruct %v2float %137 %138
%142 = OpCompositeConstruct %v2float %139 %140
%143 = OpCompositeConstruct %mat2v2float %141 %142
%144 = OpFOrdEqual %v2bool %141 %91
%145 = OpAll %bool %144
%146 = OpFOrdEqual %v2bool %142 %91
%147 = OpAll %bool %146
%148 = OpLogicalAnd %bool %145 %147
OpBranch %119
%119 = OpLabel
%149 = OpPhi %bool %false %101 %148 %118
OpStore %ok %149
%150 = OpLoad %bool %ok
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%154 = OpLoad %v4float %153
%155 = OpCompositeExtract %float %154 0
%156 = OpCompositeExtract %float %154 1
%157 = OpCompositeExtract %float %154 2
%158 = OpCompositeExtract %float %154 3
%159 = OpCompositeConstruct %v2float %155 %156
%160 = OpCompositeConstruct %v2float %157 %158
%161 = OpCompositeConstruct %mat2v2float %159 %160
%162 = OpFOrdEqual %v2bool %159 %91
%163 = OpAll %bool %162
%164 = OpFOrdEqual %v2bool %160 %91
%165 = OpAll %bool %164
%166 = OpLogicalAnd %bool %163 %165
OpBranch %152
%152 = OpLabel
%167 = OpPhi %bool %false %119 %166 %151
OpStore %ok %167
%168 = OpLoad %bool %ok
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%171 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%172 = OpLoad %v4float %171
%173 = OpCompositeExtract %float %172 0
%174 = OpCompositeExtract %float %172 1
%175 = OpCompositeExtract %float %172 2
%176 = OpCompositeExtract %float %172 3
%177 = OpCompositeConstruct %v2float %173 %174
%178 = OpCompositeConstruct %v2float %175 %176
%179 = OpCompositeConstruct %mat2v2float %177 %178
%180 = OpFOrdEqual %v2bool %177 %91
%181 = OpAll %bool %180
%182 = OpFOrdEqual %v2bool %178 %91
%183 = OpAll %bool %182
%184 = OpLogicalAnd %bool %181 %183
OpBranch %170
%170 = OpLabel
%185 = OpPhi %bool %false %152 %184 %169
OpStore %ok %185
%186 = OpLoad %bool %ok
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%190 = OpLoad %v4float %189
%191 = OpCompositeExtract %float %190 0
%192 = OpFUnordNotEqual %bool %191 %float_0
%193 = OpCompositeExtract %float %190 1
%194 = OpFUnordNotEqual %bool %193 %float_0
%195 = OpCompositeExtract %float %190 2
%196 = OpFUnordNotEqual %bool %195 %float_0
%197 = OpCompositeExtract %float %190 3
%198 = OpFUnordNotEqual %bool %197 %float_0
%200 = OpCompositeConstruct %v4bool %192 %194 %196 %198
%201 = OpSelect %float %192 %float_1 %float_0
%202 = OpSelect %float %194 %float_1 %float_0
%203 = OpSelect %float %196 %float_1 %float_0
%204 = OpSelect %float %198 %float_1 %float_0
%205 = OpCompositeConstruct %v4float %201 %202 %203 %204
%206 = OpCompositeExtract %float %205 0
%207 = OpCompositeExtract %float %205 1
%208 = OpCompositeExtract %float %205 2
%209 = OpCompositeExtract %float %205 3
%210 = OpCompositeConstruct %v2float %206 %207
%211 = OpCompositeConstruct %v2float %208 %209
%212 = OpCompositeConstruct %mat2v2float %210 %211
%213 = OpFOrdEqual %v2bool %210 %91
%214 = OpAll %bool %213
%215 = OpFOrdEqual %v2bool %211 %91
%216 = OpAll %bool %215
%217 = OpLogicalAnd %bool %214 %216
OpBranch %188
%188 = OpLabel
%218 = OpPhi %bool %false %170 %217 %187
OpStore %ok %218
%219 = OpLoad %bool %ok
OpSelectionMerge %224 None
OpBranchConditional %219 %222 %223
%222 = OpLabel
%225 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%226 = OpLoad %v4float %225
OpStore %220 %226
OpBranch %224
%223 = OpLabel
%227 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%229 = OpLoad %v4float %227
OpStore %220 %229
OpBranch %224
%224 = OpLabel
%230 = OpLoad %v4float %220
OpReturnValue %230
OpFunctionEnd
