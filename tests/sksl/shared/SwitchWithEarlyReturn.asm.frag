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
OpName %_entrypoint_v "_entrypoint_v"
OpName %return_in_one_case_bi "return_in_one_case_bi"
OpName %val "val"
OpName %return_in_default_bi "return_in_default_bi"
OpName %return_in_every_case_bi "return_in_every_case_bi"
OpName %return_in_every_case_no_default_bi "return_in_every_case_no_default_bi"
OpName %val_0 "val"
OpName %no_return_in_default_bi "no_return_in_default_bi"
OpName %val_1 "val"
OpName %return_with_fallthrough_bi "return_with_fallthrough_bi"
OpName %fallthrough_ends_in_break_bi "fallthrough_ends_in_break_bi"
OpName %val_2 "val"
OpName %fallthrough_to_default_with_break_bi "fallthrough_to_default_with_break_bi"
OpName %val_3 "val"
OpName %fallthrough_to_default_with_return_bi "fallthrough_to_default_with_return_bi"
OpName %fallthrough_with_loop_break_bi "fallthrough_with_loop_break_bi"
OpName %val_4 "val"
OpName %i "i"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %20 Binding 0
OpDecorate %20 DescriptorSet 0
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%20 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%25 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%29 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%35 = OpTypeFunction %bool %_ptr_Function_int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%int_5 = OpConstant %int 5
%137 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %25
%26 = OpLabel
%30 = OpVariable %_ptr_Function_v2float Function
OpStore %30 %29
%32 = OpFunctionCall %v4float %main %30
OpStore %sk_FragColor %32
OpReturn
OpFunctionEnd
%return_in_one_case_bi = OpFunction %bool None %35
%36 = OpFunctionParameter %_ptr_Function_int
%37 = OpLabel
%val = OpVariable %_ptr_Function_int Function
OpStore %val %int_0
%40 = OpLoad %int %36
OpSelectionMerge %41 None
OpSwitch %40 %43 1 %42
%42 = OpLabel
%45 = OpIAdd %int %int_0 %int_1
OpStore %val %45
OpReturnValue %false
%43 = OpLabel
%47 = OpLoad %int %val
%48 = OpIAdd %int %47 %int_1
OpStore %val %48
OpBranch %41
%41 = OpLabel
%49 = OpLoad %int %val
%50 = OpIEqual %bool %49 %int_1
OpReturnValue %50
OpFunctionEnd
%return_in_default_bi = OpFunction %bool None %35
%51 = OpFunctionParameter %_ptr_Function_int
%52 = OpLabel
%53 = OpLoad %int %51
OpSelectionMerge %54 None
OpSwitch %53 %55
%55 = OpLabel
OpReturnValue %true
%54 = OpLabel
OpUnreachable
OpFunctionEnd
%return_in_every_case_bi = OpFunction %bool None %35
%57 = OpFunctionParameter %_ptr_Function_int
%58 = OpLabel
%59 = OpLoad %int %57
OpSelectionMerge %60 None
OpSwitch %59 %62 1 %61
%61 = OpLabel
OpReturnValue %false
%62 = OpLabel
OpReturnValue %true
%60 = OpLabel
OpUnreachable
OpFunctionEnd
%return_in_every_case_no_default_bi = OpFunction %bool None %35
%63 = OpFunctionParameter %_ptr_Function_int
%64 = OpLabel
%val_0 = OpVariable %_ptr_Function_int Function
OpStore %val_0 %int_0
%66 = OpLoad %int %63
OpSelectionMerge %67 None
OpSwitch %66 %67 1 %68 2 %69
%68 = OpLabel
OpReturnValue %false
%69 = OpLabel
OpReturnValue %true
%67 = OpLabel
%70 = OpIAdd %int %int_0 %int_1
OpStore %val_0 %70
%71 = OpIEqual %bool %70 %int_1
OpReturnValue %71
OpFunctionEnd
%no_return_in_default_bi = OpFunction %bool None %35
%72 = OpFunctionParameter %_ptr_Function_int
%73 = OpLabel
%val_1 = OpVariable %_ptr_Function_int Function
OpStore %val_1 %int_0
%75 = OpLoad %int %72
OpSelectionMerge %76 None
OpSwitch %75 %79 1 %77 2 %78
%77 = OpLabel
OpReturnValue %false
%78 = OpLabel
OpReturnValue %true
%79 = OpLabel
OpBranch %76
%76 = OpLabel
%80 = OpIAdd %int %int_0 %int_1
OpStore %val_1 %80
%81 = OpIEqual %bool %80 %int_1
OpReturnValue %81
OpFunctionEnd
%return_with_fallthrough_bi = OpFunction %bool None %35
%82 = OpFunctionParameter %_ptr_Function_int
%83 = OpLabel
%84 = OpLoad %int %82
OpSelectionMerge %85 None
OpSwitch %84 %88 1 %86 2 %87
%86 = OpLabel
OpBranch %87
%87 = OpLabel
OpReturnValue %true
%88 = OpLabel
OpReturnValue %false
%85 = OpLabel
OpUnreachable
OpFunctionEnd
%fallthrough_ends_in_break_bi = OpFunction %bool None %35
%89 = OpFunctionParameter %_ptr_Function_int
%90 = OpLabel
%val_2 = OpVariable %_ptr_Function_int Function
OpStore %val_2 %int_0
%92 = OpLoad %int %89
OpSelectionMerge %93 None
OpSwitch %92 %96 1 %94 2 %95
%94 = OpLabel
OpBranch %95
%95 = OpLabel
OpBranch %93
%96 = OpLabel
OpReturnValue %false
%93 = OpLabel
%97 = OpIAdd %int %int_0 %int_1
OpStore %val_2 %97
%98 = OpIEqual %bool %97 %int_1
OpReturnValue %98
OpFunctionEnd
%fallthrough_to_default_with_break_bi = OpFunction %bool None %35
%99 = OpFunctionParameter %_ptr_Function_int
%100 = OpLabel
%val_3 = OpVariable %_ptr_Function_int Function
OpStore %val_3 %int_0
%102 = OpLoad %int %99
OpSelectionMerge %103 None
OpSwitch %102 %106 1 %104 2 %105
%104 = OpLabel
OpBranch %105
%105 = OpLabel
OpBranch %106
%106 = OpLabel
OpBranch %103
%103 = OpLabel
%107 = OpIAdd %int %int_0 %int_1
OpStore %val_3 %107
%108 = OpIEqual %bool %107 %int_1
OpReturnValue %108
OpFunctionEnd
%fallthrough_to_default_with_return_bi = OpFunction %bool None %35
%109 = OpFunctionParameter %_ptr_Function_int
%110 = OpLabel
%111 = OpLoad %int %109
OpSelectionMerge %112 None
OpSwitch %111 %115 1 %113 2 %114
%113 = OpLabel
OpBranch %114
%114 = OpLabel
OpBranch %115
%115 = OpLabel
OpReturnValue %true
%112 = OpLabel
OpUnreachable
OpFunctionEnd
%fallthrough_with_loop_break_bi = OpFunction %bool None %35
%116 = OpFunctionParameter %_ptr_Function_int
%117 = OpLabel
%val_4 = OpVariable %_ptr_Function_int Function
%i = OpVariable %_ptr_Function_int Function
OpStore %val_4 %int_0
%119 = OpLoad %int %116
OpSelectionMerge %120 None
OpSwitch %119 %123 1 %121 2 %122
%121 = OpLabel
OpStore %i %int_0
OpBranch %125
%125 = OpLabel
OpLoopMerge %129 %128 None
OpBranch %126
%126 = OpLabel
%130 = OpLoad %int %i
%132 = OpSLessThan %bool %130 %int_5
OpBranchConditional %132 %127 %129
%127 = OpLabel
%133 = OpLoad %int %val_4
%134 = OpIAdd %int %133 %int_1
OpStore %val_4 %134
OpBranch %129
%128 = OpLabel
%135 = OpLoad %int %i
%136 = OpIAdd %int %135 %int_1
OpStore %i %136
OpBranch %125
%129 = OpLabel
OpBranch %122
%122 = OpLabel
OpBranch %123
%123 = OpLabel
OpReturnValue %true
%120 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %137
%138 = OpFunctionParameter %_ptr_Function_v2float
%139 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%146 = OpVariable %_ptr_Function_int Function
%150 = OpVariable %_ptr_Function_int Function
%155 = OpVariable %_ptr_Function_int Function
%160 = OpVariable %_ptr_Function_int Function
%165 = OpVariable %_ptr_Function_int Function
%170 = OpVariable %_ptr_Function_int Function
%175 = OpVariable %_ptr_Function_int Function
%180 = OpVariable %_ptr_Function_int Function
%185 = OpVariable %_ptr_Function_int Function
%190 = OpVariable %_ptr_Function_int Function
%193 = OpVariable %_ptr_Function_v4float Function
%141 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%143 = OpLoad %v4float %141
%144 = OpCompositeExtract %float %143 1
%145 = OpConvertFToS %int %144
OpStore %x %145
OpStore %146 %145
%147 = OpFunctionCall %bool %return_in_one_case_bi %146
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
OpStore %150 %145
%151 = OpFunctionCall %bool %return_in_default_bi %150
OpBranch %149
%149 = OpLabel
%152 = OpPhi %bool %false %139 %151 %148
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
OpStore %155 %145
%156 = OpFunctionCall %bool %return_in_every_case_bi %155
OpBranch %154
%154 = OpLabel
%157 = OpPhi %bool %false %149 %156 %153
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
OpStore %160 %145
%161 = OpFunctionCall %bool %return_in_every_case_no_default_bi %160
OpBranch %159
%159 = OpLabel
%162 = OpPhi %bool %false %154 %161 %158
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
OpStore %165 %145
%166 = OpFunctionCall %bool %no_return_in_default_bi %165
OpBranch %164
%164 = OpLabel
%167 = OpPhi %bool %false %159 %166 %163
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
OpStore %170 %145
%171 = OpFunctionCall %bool %return_with_fallthrough_bi %170
OpBranch %169
%169 = OpLabel
%172 = OpPhi %bool %false %164 %171 %168
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
OpStore %175 %145
%176 = OpFunctionCall %bool %fallthrough_ends_in_break_bi %175
OpBranch %174
%174 = OpLabel
%177 = OpPhi %bool %false %169 %176 %173
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
OpStore %180 %145
%181 = OpFunctionCall %bool %fallthrough_to_default_with_break_bi %180
OpBranch %179
%179 = OpLabel
%182 = OpPhi %bool %false %174 %181 %178
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
OpStore %185 %145
%186 = OpFunctionCall %bool %fallthrough_to_default_with_return_bi %185
OpBranch %184
%184 = OpLabel
%187 = OpPhi %bool %false %179 %186 %183
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
OpStore %190 %145
%191 = OpFunctionCall %bool %fallthrough_with_loop_break_bi %190
OpBranch %189
%189 = OpLabel
%192 = OpPhi %bool %false %184 %191 %188
OpSelectionMerge %197 None
OpBranchConditional %192 %195 %196
%195 = OpLabel
%198 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%199 = OpLoad %v4float %198
OpStore %193 %199
OpBranch %197
%196 = OpLabel
%200 = OpAccessChain %_ptr_Uniform_v4float %20 %int_1
%201 = OpLoad %v4float %200
OpStore %193 %201
OpBranch %197
%197 = OpLabel
%202 = OpLoad %v4float %193
OpReturnValue %202
OpFunctionEnd
