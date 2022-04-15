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
OpDecorate %62 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
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
%89 = OpConstantComposite %v2float %float_0 %float_1
%90 = OpConstantComposite %mat2v2float %89 %89
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
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%62 = OpLoad %v4float %61
%63 = OpCompositeExtract %float %62 0
%64 = OpCompositeExtract %float %62 1
%65 = OpCompositeExtract %float %62 2
%66 = OpCompositeExtract %float %62 3
%67 = OpCompositeConstruct %v2float %63 %64
%68 = OpCompositeConstruct %v2float %65 %66
%69 = OpCompositeConstruct %mat2v2float %67 %68
%70 = OpFOrdEqual %v2bool %67 %49
%71 = OpAll %bool %70
%72 = OpFOrdEqual %v2bool %68 %50
%73 = OpAll %bool %72
%74 = OpLogicalAnd %bool %71 %73
OpBranch %60
%60 = OpLabel
%75 = OpPhi %bool %false %32 %74 %59
OpStore %ok %75
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%80 = OpLoad %v4float %78
%81 = OpCompositeExtract %float %80 0
%82 = OpCompositeExtract %float %80 1
%83 = OpCompositeExtract %float %80 2
%84 = OpCompositeExtract %float %80 3
%85 = OpCompositeConstruct %v2float %81 %82
%86 = OpCompositeConstruct %v2float %83 %84
%87 = OpCompositeConstruct %mat2v2float %85 %86
%91 = OpFOrdEqual %v2bool %85 %89
%92 = OpAll %bool %91
%93 = OpFOrdEqual %v2bool %86 %89
%94 = OpAll %bool %93
%95 = OpLogicalAnd %bool %92 %94
OpBranch %77
%77 = OpLabel
%96 = OpPhi %bool %false %60 %95 %76
OpStore %ok %96
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%100 = OpLoad %v4float %99
%101 = OpCompositeExtract %float %100 0
%102 = OpCompositeExtract %float %100 1
%103 = OpCompositeExtract %float %100 2
%104 = OpCompositeExtract %float %100 3
%105 = OpCompositeConstruct %v2float %101 %102
%106 = OpCompositeConstruct %v2float %103 %104
%107 = OpCompositeConstruct %mat2v2float %105 %106
%108 = OpFOrdEqual %v2bool %105 %89
%109 = OpAll %bool %108
%110 = OpFOrdEqual %v2bool %106 %89
%111 = OpAll %bool %110
%112 = OpLogicalAnd %bool %109 %111
OpBranch %98
%98 = OpLabel
%113 = OpPhi %bool %false %77 %112 %97
OpStore %ok %113
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%117 = OpLoad %v4float %116
%118 = OpCompositeExtract %float %117 0
%119 = OpConvertFToS %int %118
%120 = OpCompositeExtract %float %117 1
%121 = OpConvertFToS %int %120
%122 = OpCompositeExtract %float %117 2
%123 = OpConvertFToS %int %122
%124 = OpCompositeExtract %float %117 3
%125 = OpConvertFToS %int %124
%127 = OpCompositeConstruct %v4int %119 %121 %123 %125
%128 = OpCompositeExtract %int %127 0
%129 = OpConvertSToF %float %128
%130 = OpCompositeExtract %int %127 1
%131 = OpConvertSToF %float %130
%132 = OpCompositeExtract %int %127 2
%133 = OpConvertSToF %float %132
%134 = OpCompositeExtract %int %127 3
%135 = OpConvertSToF %float %134
%136 = OpCompositeConstruct %v4float %129 %131 %133 %135
%137 = OpCompositeExtract %float %136 0
%138 = OpCompositeExtract %float %136 1
%139 = OpCompositeExtract %float %136 2
%140 = OpCompositeExtract %float %136 3
%141 = OpCompositeConstruct %v2float %137 %138
%142 = OpCompositeConstruct %v2float %139 %140
%143 = OpCompositeConstruct %mat2v2float %141 %142
%144 = OpFOrdEqual %v2bool %141 %89
%145 = OpAll %bool %144
%146 = OpFOrdEqual %v2bool %142 %89
%147 = OpAll %bool %146
%148 = OpLogicalAnd %bool %145 %147
OpBranch %115
%115 = OpLabel
%149 = OpPhi %bool %false %98 %148 %114
OpStore %ok %149
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%153 = OpLoad %v4float %152
%154 = OpCompositeExtract %float %153 0
%155 = OpCompositeExtract %float %153 1
%156 = OpCompositeExtract %float %153 2
%157 = OpCompositeExtract %float %153 3
%158 = OpCompositeConstruct %v2float %154 %155
%159 = OpCompositeConstruct %v2float %156 %157
%160 = OpCompositeConstruct %mat2v2float %158 %159
%161 = OpFOrdEqual %v2bool %158 %89
%162 = OpAll %bool %161
%163 = OpFOrdEqual %v2bool %159 %89
%164 = OpAll %bool %163
%165 = OpLogicalAnd %bool %162 %164
OpBranch %151
%151 = OpLabel
%166 = OpPhi %bool %false %115 %165 %150
OpStore %ok %166
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%169 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%170 = OpLoad %v4float %169
%171 = OpCompositeExtract %float %170 0
%172 = OpCompositeExtract %float %170 1
%173 = OpCompositeExtract %float %170 2
%174 = OpCompositeExtract %float %170 3
%175 = OpCompositeConstruct %v2float %171 %172
%176 = OpCompositeConstruct %v2float %173 %174
%177 = OpCompositeConstruct %mat2v2float %175 %176
%178 = OpFOrdEqual %v2bool %175 %89
%179 = OpAll %bool %178
%180 = OpFOrdEqual %v2bool %176 %89
%181 = OpAll %bool %180
%182 = OpLogicalAnd %bool %179 %181
OpBranch %168
%168 = OpLabel
%183 = OpPhi %bool %false %151 %182 %167
OpStore %ok %183
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%187 = OpLoad %v4float %186
%188 = OpCompositeExtract %float %187 0
%189 = OpFUnordNotEqual %bool %188 %float_0
%190 = OpCompositeExtract %float %187 1
%191 = OpFUnordNotEqual %bool %190 %float_0
%192 = OpCompositeExtract %float %187 2
%193 = OpFUnordNotEqual %bool %192 %float_0
%194 = OpCompositeExtract %float %187 3
%195 = OpFUnordNotEqual %bool %194 %float_0
%197 = OpCompositeConstruct %v4bool %189 %191 %193 %195
%198 = OpCompositeExtract %bool %197 0
%199 = OpSelect %float %198 %float_1 %float_0
%200 = OpCompositeExtract %bool %197 1
%201 = OpSelect %float %200 %float_1 %float_0
%202 = OpCompositeExtract %bool %197 2
%203 = OpSelect %float %202 %float_1 %float_0
%204 = OpCompositeExtract %bool %197 3
%205 = OpSelect %float %204 %float_1 %float_0
%206 = OpCompositeConstruct %v4float %199 %201 %203 %205
%207 = OpCompositeExtract %float %206 0
%208 = OpCompositeExtract %float %206 1
%209 = OpCompositeExtract %float %206 2
%210 = OpCompositeExtract %float %206 3
%211 = OpCompositeConstruct %v2float %207 %208
%212 = OpCompositeConstruct %v2float %209 %210
%213 = OpCompositeConstruct %mat2v2float %211 %212
%214 = OpFOrdEqual %v2bool %211 %89
%215 = OpAll %bool %214
%216 = OpFOrdEqual %v2bool %212 %89
%217 = OpAll %bool %216
%218 = OpLogicalAnd %bool %215 %217
OpBranch %185
%185 = OpLabel
%219 = OpPhi %bool %false %168 %218 %184
OpStore %ok %219
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
