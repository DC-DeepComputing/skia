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
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
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
%48 = OpConstantComposite %v2float %float_n1_25 %float_0
%49 = OpConstantComposite %v2float %float_0_75 %float_2_25
%50 = OpConstantComposite %mat2v2float %48 %49
%v2bool = OpTypeVector %bool 2
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%88 = OpConstantComposite %v2float %float_0 %float_1
%89 = OpConstantComposite %mat2v2float %88 %88
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
%207 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
OpSelectionMerge %31 None
OpBranchConditional %true %30 %31
%30 = OpLabel
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 0
%38 = OpCompositeExtract %float %36 1
%39 = OpCompositeExtract %float %36 2
%40 = OpCompositeExtract %float %36 3
%41 = OpCompositeConstruct %v2float %37 %38
%42 = OpCompositeConstruct %v2float %39 %40
%44 = OpCompositeConstruct %mat2v2float %41 %42
%52 = OpFOrdEqual %v2bool %41 %48
%53 = OpAll %bool %52
%54 = OpFOrdEqual %v2bool %42 %49
%55 = OpAll %bool %54
%56 = OpLogicalAnd %bool %53 %55
OpBranch %31
%31 = OpLabel
%57 = OpPhi %bool %false %25 %56 %30
OpStore %ok %57
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%61 = OpLoad %v4float %60
%62 = OpCompositeExtract %float %61 0
%63 = OpCompositeExtract %float %61 1
%64 = OpCompositeExtract %float %61 2
%65 = OpCompositeExtract %float %61 3
%66 = OpCompositeConstruct %v2float %62 %63
%67 = OpCompositeConstruct %v2float %64 %65
%68 = OpCompositeConstruct %mat2v2float %66 %67
%69 = OpFOrdEqual %v2bool %66 %48
%70 = OpAll %bool %69
%71 = OpFOrdEqual %v2bool %67 %49
%72 = OpAll %bool %71
%73 = OpLogicalAnd %bool %70 %72
OpBranch %59
%59 = OpLabel
%74 = OpPhi %bool %false %31 %73 %58
OpStore %ok %74
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%79 = OpLoad %v4float %77
%80 = OpCompositeExtract %float %79 0
%81 = OpCompositeExtract %float %79 1
%82 = OpCompositeExtract %float %79 2
%83 = OpCompositeExtract %float %79 3
%84 = OpCompositeConstruct %v2float %80 %81
%85 = OpCompositeConstruct %v2float %82 %83
%86 = OpCompositeConstruct %mat2v2float %84 %85
%90 = OpFOrdEqual %v2bool %84 %88
%91 = OpAll %bool %90
%92 = OpFOrdEqual %v2bool %85 %88
%93 = OpAll %bool %92
%94 = OpLogicalAnd %bool %91 %93
OpBranch %76
%76 = OpLabel
%95 = OpPhi %bool %false %59 %94 %75
OpStore %ok %95
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%99 = OpLoad %v4float %98
%100 = OpCompositeExtract %float %99 0
%101 = OpCompositeExtract %float %99 1
%102 = OpCompositeExtract %float %99 2
%103 = OpCompositeExtract %float %99 3
%104 = OpCompositeConstruct %v2float %100 %101
%105 = OpCompositeConstruct %v2float %102 %103
%106 = OpCompositeConstruct %mat2v2float %104 %105
%107 = OpFOrdEqual %v2bool %104 %88
%108 = OpAll %bool %107
%109 = OpFOrdEqual %v2bool %105 %88
%110 = OpAll %bool %109
%111 = OpLogicalAnd %bool %108 %110
OpBranch %97
%97 = OpLabel
%112 = OpPhi %bool %false %76 %111 %96
OpStore %ok %112
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%116 = OpLoad %v4float %115
%117 = OpCompositeExtract %float %116 0
%118 = OpConvertFToS %int %117
%119 = OpCompositeExtract %float %116 1
%120 = OpConvertFToS %int %119
%121 = OpCompositeExtract %float %116 2
%122 = OpConvertFToS %int %121
%123 = OpCompositeExtract %float %116 3
%124 = OpConvertFToS %int %123
%126 = OpCompositeConstruct %v4int %118 %120 %122 %124
%127 = OpConvertSToF %float %118
%128 = OpConvertSToF %float %120
%129 = OpConvertSToF %float %122
%130 = OpConvertSToF %float %124
%131 = OpCompositeConstruct %v4float %127 %128 %129 %130
%132 = OpCompositeConstruct %v2float %127 %128
%133 = OpCompositeConstruct %v2float %129 %130
%134 = OpCompositeConstruct %mat2v2float %132 %133
%135 = OpFOrdEqual %v2bool %132 %88
%136 = OpAll %bool %135
%137 = OpFOrdEqual %v2bool %133 %88
%138 = OpAll %bool %137
%139 = OpLogicalAnd %bool %136 %138
OpBranch %114
%114 = OpLabel
%140 = OpPhi %bool %false %97 %139 %113
OpStore %ok %140
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%144 = OpLoad %v4float %143
%145 = OpCompositeExtract %float %144 0
%146 = OpCompositeExtract %float %144 1
%147 = OpCompositeExtract %float %144 2
%148 = OpCompositeExtract %float %144 3
%149 = OpCompositeConstruct %v2float %145 %146
%150 = OpCompositeConstruct %v2float %147 %148
%151 = OpCompositeConstruct %mat2v2float %149 %150
%152 = OpFOrdEqual %v2bool %149 %88
%153 = OpAll %bool %152
%154 = OpFOrdEqual %v2bool %150 %88
%155 = OpAll %bool %154
%156 = OpLogicalAnd %bool %153 %155
OpBranch %142
%142 = OpLabel
%157 = OpPhi %bool %false %114 %156 %141
OpStore %ok %157
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%161 = OpLoad %v4float %160
%162 = OpCompositeExtract %float %161 0
%163 = OpCompositeExtract %float %161 1
%164 = OpCompositeExtract %float %161 2
%165 = OpCompositeExtract %float %161 3
%166 = OpCompositeConstruct %v2float %162 %163
%167 = OpCompositeConstruct %v2float %164 %165
%168 = OpCompositeConstruct %mat2v2float %166 %167
%169 = OpFOrdEqual %v2bool %166 %88
%170 = OpAll %bool %169
%171 = OpFOrdEqual %v2bool %167 %88
%172 = OpAll %bool %171
%173 = OpLogicalAnd %bool %170 %172
OpBranch %159
%159 = OpLabel
%174 = OpPhi %bool %false %142 %173 %158
OpStore %ok %174
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%178 = OpLoad %v4float %177
%179 = OpCompositeExtract %float %178 0
%180 = OpFUnordNotEqual %bool %179 %float_0
%181 = OpCompositeExtract %float %178 1
%182 = OpFUnordNotEqual %bool %181 %float_0
%183 = OpCompositeExtract %float %178 2
%184 = OpFUnordNotEqual %bool %183 %float_0
%185 = OpCompositeExtract %float %178 3
%186 = OpFUnordNotEqual %bool %185 %float_0
%188 = OpCompositeConstruct %v4bool %180 %182 %184 %186
%189 = OpCompositeExtract %bool %188 0
%190 = OpSelect %float %189 %float_1 %float_0
%191 = OpCompositeExtract %bool %188 1
%192 = OpSelect %float %191 %float_1 %float_0
%193 = OpCompositeExtract %bool %188 2
%194 = OpSelect %float %193 %float_1 %float_0
%195 = OpCompositeExtract %bool %188 3
%196 = OpSelect %float %195 %float_1 %float_0
%197 = OpCompositeConstruct %v4float %190 %192 %194 %196
%198 = OpCompositeConstruct %v2float %190 %192
%199 = OpCompositeConstruct %v2float %194 %196
%200 = OpCompositeConstruct %mat2v2float %198 %199
%201 = OpFOrdEqual %v2bool %198 %88
%202 = OpAll %bool %201
%203 = OpFOrdEqual %v2bool %199 %88
%204 = OpAll %bool %203
%205 = OpLogicalAnd %bool %202 %204
OpBranch %176
%176 = OpLabel
%206 = OpPhi %bool %false %159 %205 %175
OpStore %ok %206
OpSelectionMerge %211 None
OpBranchConditional %206 %209 %210
%209 = OpLabel
%212 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%213 = OpLoad %v4float %212
OpStore %207 %213
OpBranch %211
%210 = OpLabel
%214 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%216 = OpLoad %v4float %214
OpStore %207 %216
OpBranch %211
%211 = OpLabel
%217 = OpLoad %v4float %207
OpReturnValue %217
OpFunctionEnd
