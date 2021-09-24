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
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %return_on_both_sides_b "return_on_both_sides_b"
OpName %for_inside_body_b "for_inside_body_b"
OpName %x "x"
OpName %after_for_body_b "after_for_body_b"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return_b "for_with_double_sided_conditional_return_b"
OpName %x_1 "x"
OpName %if_else_chain_b "if_else_chain_b"
OpName %conditional_inside_while_loop_b "conditional_inside_while_loop_b"
OpName %inside_do_loop_b "inside_do_loop_b"
OpName %inside_while_loop_b "inside_while_loop_b"
OpName %after_do_loop_b "after_do_loop_b"
OpName %after_while_loop_b "after_while_loop_b"
OpName %main "main"
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
OpDecorate %20 Binding 0
OpDecorate %20 DescriptorSet 0
OpDecorate %39 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%20 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%25 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%29 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%33 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_123 = OpConstant %float 123
%152 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %25
%26 = OpLabel
%30 = OpVariable %_ptr_Function_v2float Function
OpStore %30 %29
%32 = OpFunctionCall %v4float %main %30
OpStore %sk_FragColor %32
OpReturn
OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %33
%34 = OpLabel
%35 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%39 = OpLoad %float %35
%41 = OpFOrdEqual %bool %39 %float_1
OpSelectionMerge %44 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
OpReturnValue %true
%43 = OpLabel
OpReturnValue %true
%44 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body_b = OpFunction %bool None %33
%46 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %50
%50 = OpLabel
OpLoopMerge %54 %53 None
OpBranch %51
%51 = OpLabel
%55 = OpLoad %int %x
%57 = OpSLessThanEqual %bool %55 %int_10
OpBranchConditional %57 %52 %54
%52 = OpLabel
OpReturnValue %true
%53 = OpLabel
%59 = OpLoad %int %x
%60 = OpIAdd %int %59 %int_1
OpStore %x %60
OpBranch %50
%54 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body_b = OpFunction %bool None %33
%61 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %63
%63 = OpLabel
OpLoopMerge %67 %66 None
OpBranch %64
%64 = OpLabel
%68 = OpLoad %int %x_0
%69 = OpSLessThanEqual %bool %68 %int_10
OpBranchConditional %69 %65 %67
%65 = OpLabel
OpBranch %66
%66 = OpLabel
%70 = OpLoad %int %x_0
%71 = OpIAdd %int %70 %int_1
OpStore %x_0 %71
OpBranch %63
%67 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return_b = OpFunction %bool None %33
%72 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %74
%74 = OpLabel
OpLoopMerge %78 %77 None
OpBranch %75
%75 = OpLabel
%79 = OpLoad %int %x_1
%80 = OpSLessThanEqual %bool %79 %int_10
OpBranchConditional %80 %76 %78
%76 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%82 = OpLoad %float %81
%83 = OpFOrdEqual %bool %82 %float_1
OpSelectionMerge %86 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
OpReturnValue %true
%85 = OpLabel
OpReturnValue %true
%86 = OpLabel
OpBranch %77
%77 = OpLabel
%87 = OpLoad %int %x_1
%88 = OpIAdd %int %87 %int_1
OpStore %x_1 %88
OpBranch %74
%78 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain_b = OpFunction %bool None %33
%89 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%91 = OpLoad %float %90
%92 = OpFOrdEqual %bool %91 %float_1
OpSelectionMerge %95 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
OpReturnValue %true
%94 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%97 = OpLoad %float %96
%99 = OpFOrdEqual %bool %97 %float_2
OpSelectionMerge %102 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
OpReturnValue %false
%101 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%105 = OpLoad %float %104
%107 = OpFOrdEqual %bool %105 %float_3
OpSelectionMerge %110 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
OpReturnValue %true
%109 = OpLabel
%111 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%112 = OpLoad %float %111
%114 = OpFOrdEqual %bool %112 %float_4
OpSelectionMerge %117 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
OpReturnValue %false
%116 = OpLabel
OpReturnValue %true
%117 = OpLabel
OpBranch %110
%110 = OpLabel
OpBranch %102
%102 = OpLabel
OpBranch %95
%95 = OpLabel
OpUnreachable
OpFunctionEnd
%conditional_inside_while_loop_b = OpFunction %bool None %33
%118 = OpLabel
OpBranch %119
%119 = OpLabel
OpLoopMerge %123 %122 None
OpBranch %120
%120 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_float %20 %int_2
%125 = OpLoad %float %124
%127 = OpFOrdEqual %bool %125 %float_123
OpBranchConditional %127 %121 %123
%121 = OpLabel
OpReturnValue %true
%122 = OpLabel
OpBranch %119
%123 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_do_loop_b = OpFunction %bool None %33
%128 = OpLabel
OpBranch %129
%129 = OpLabel
OpLoopMerge %133 %132 None
OpBranch %130
%130 = OpLabel
OpReturnValue %true
%131 = OpLabel
OpBranch %132
%132 = OpLabel
OpBranchConditional %true %129 %133
%133 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_while_loop_b = OpFunction %bool None %33
%134 = OpLabel
OpBranch %135
%135 = OpLabel
OpLoopMerge %139 %138 None
OpBranch %136
%136 = OpLabel
OpBranchConditional %true %137 %139
%137 = OpLabel
OpReturnValue %true
%138 = OpLabel
OpBranch %135
%139 = OpLabel
OpUnreachable
OpFunctionEnd
%after_do_loop_b = OpFunction %bool None %33
%140 = OpLabel
OpBranch %141
%141 = OpLabel
OpLoopMerge %145 %144 None
OpBranch %142
%142 = OpLabel
OpBranch %145
%143 = OpLabel
OpBranch %144
%144 = OpLabel
OpBranchConditional %true %141 %145
%145 = OpLabel
OpReturnValue %true
OpFunctionEnd
%after_while_loop_b = OpFunction %bool None %33
%146 = OpLabel
OpBranch %147
%147 = OpLabel
OpLoopMerge %151 %150 None
OpBranch %148
%148 = OpLabel
OpBranchConditional %true %149 %151
%149 = OpLabel
OpBranch %151
%150 = OpLabel
OpBranch %147
%151 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %152
%153 = OpFunctionParameter %_ptr_Function_v2float
%154 = OpLabel
%195 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %156 None
OpBranchConditional %true %155 %156
%155 = OpLabel
%157 = OpFunctionCall %bool %return_on_both_sides_b
OpBranch %156
%156 = OpLabel
%158 = OpPhi %bool %false %154 %157 %155
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%161 = OpFunctionCall %bool %for_inside_body_b
OpBranch %160
%160 = OpLabel
%162 = OpPhi %bool %false %156 %161 %159
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%165 = OpFunctionCall %bool %after_for_body_b
OpBranch %164
%164 = OpLabel
%166 = OpPhi %bool %false %160 %165 %163
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%169 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
OpBranch %168
%168 = OpLabel
%170 = OpPhi %bool %false %164 %169 %167
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpFunctionCall %bool %if_else_chain_b
OpBranch %172
%172 = OpLabel
%174 = OpPhi %bool %false %168 %173 %171
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpFunctionCall %bool %conditional_inside_while_loop_b
OpBranch %176
%176 = OpLabel
%178 = OpPhi %bool %false %172 %177 %175
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%181 = OpFunctionCall %bool %inside_do_loop_b
OpBranch %180
%180 = OpLabel
%182 = OpPhi %bool %false %176 %181 %179
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpFunctionCall %bool %inside_while_loop_b
OpBranch %184
%184 = OpLabel
%186 = OpPhi %bool %false %180 %185 %183
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpFunctionCall %bool %after_do_loop_b
OpBranch %188
%188 = OpLabel
%190 = OpPhi %bool %false %184 %189 %187
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%193 = OpFunctionCall %bool %after_while_loop_b
OpBranch %192
%192 = OpLabel
%194 = OpPhi %bool %false %188 %193 %191
OpSelectionMerge %199 None
OpBranchConditional %194 %197 %198
%197 = OpLabel
%200 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%202 = OpLoad %v4float %200
OpStore %195 %202
OpBranch %199
%198 = OpLabel
%203 = OpAccessChain %_ptr_Uniform_v4float %20 %int_1
%204 = OpLoad %v4float %203
OpStore %195 %204
OpBranch %199
%199 = OpLabel
%205 = OpLoad %v4float %195
OpReturnValue %205
OpFunctionEnd
