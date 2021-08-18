OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %returns_a_struct_S "returns_a_struct_S"
OpName %s "s"
OpName %constructs_a_struct_S "constructs_a_struct_S"
OpName %accepts_a_struct_fS "accepts_a_struct_fS"
OpName %modifies_a_struct_vS "modifies_a_struct_vS"
OpName %main "main"
OpName %s_0 "s"
OpName %x "x"
OpName %expected "expected"
OpName %Nested "Nested"
OpMemberName %Nested 0 "a"
OpMemberName %Nested 1 "b"
OpName %n1 "n1"
OpName %n2 "n2"
OpName %n3 "n3"
OpName %valid "valid"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %16 Binding 0
OpDecorate %16 DescriptorSet 0
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %43 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpMemberDecorate %Nested 0 Offset 0
OpMemberDecorate %Nested 0 RelaxedPrecision
OpMemberDecorate %Nested 1 Offset 16
OpMemberDecorate %Nested 1 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%21 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%S = OpTypeStruct %float %int
%31 = OpTypeFunction %S
%_ptr_Function_S = OpTypePointer Function %S
%float_1 = OpConstant %float 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%48 = OpTypeFunction %float %_ptr_Function_S
%57 = OpTypeFunction %void %_ptr_Function_S
%66 = OpTypeFunction %v4float %_ptr_Function_v2float
%Nested = OpTypeStruct %S %S
%_ptr_Function_Nested = OpTypePointer Function %Nested
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %21
%22 = OpLabel
%26 = OpVariable %_ptr_Function_v2float Function
OpStore %26 %25
%28 = OpFunctionCall %v4float %main %26
OpStore %sk_FragColor %28
OpReturn
OpFunctionEnd
%returns_a_struct_S = OpFunction %S None %31
%32 = OpLabel
%s = OpVariable %_ptr_Function_S Function
%37 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %37 %float_1
%41 = OpAccessChain %_ptr_Function_int %s %int_1
OpStore %41 %int_2
%43 = OpLoad %S %s
OpReturnValue %43
OpFunctionEnd
%constructs_a_struct_S = OpFunction %S None %31
%44 = OpLabel
%47 = OpCompositeConstruct %S %float_2 %int_3
OpReturnValue %47
OpFunctionEnd
%accepts_a_struct_fS = OpFunction %float None %48
%49 = OpFunctionParameter %_ptr_Function_S
%50 = OpLabel
%51 = OpAccessChain %_ptr_Function_float %49 %int_0
%52 = OpLoad %float %51
%53 = OpAccessChain %_ptr_Function_int %49 %int_1
%54 = OpLoad %int %53
%55 = OpConvertSToF %float %54
%56 = OpFAdd %float %52 %55
OpReturnValue %56
OpFunctionEnd
%modifies_a_struct_vS = OpFunction %void None %57
%58 = OpFunctionParameter %_ptr_Function_S
%59 = OpLabel
%60 = OpAccessChain %_ptr_Function_float %58 %int_0
%61 = OpLoad %float %60
%62 = OpFAdd %float %61 %float_1
OpStore %60 %62
%63 = OpAccessChain %_ptr_Function_int %58 %int_1
%64 = OpLoad %int %63
%65 = OpIAdd %int %64 %int_1
OpStore %63 %65
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %66
%67 = OpFunctionParameter %_ptr_Function_v2float
%68 = OpLabel
%s_0 = OpVariable %_ptr_Function_S Function
%x = OpVariable %_ptr_Function_float Function
%73 = OpVariable %_ptr_Function_S Function
%expected = OpVariable %_ptr_Function_S Function
%n1 = OpVariable %_ptr_Function_Nested Function
%n2 = OpVariable %_ptr_Function_Nested Function
%n3 = OpVariable %_ptr_Function_Nested Function
%92 = OpVariable %_ptr_Function_S Function
%valid = OpVariable %_ptr_Function_bool Function
%224 = OpVariable %_ptr_Function_v4float Function
%70 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %70
%72 = OpLoad %S %s_0
OpStore %73 %72
%74 = OpFunctionCall %float %accepts_a_struct_fS %73
OpStore %x %74
%75 = OpFunctionCall %void %modifies_a_struct_vS %s_0
%77 = OpFunctionCall %S %constructs_a_struct_S
OpStore %expected %77
%83 = OpFunctionCall %S %returns_a_struct_S
%84 = OpAccessChain %_ptr_Function_S %n1 %int_0
OpStore %84 %83
%85 = OpAccessChain %_ptr_Function_S %n1 %int_0
%86 = OpLoad %S %85
%87 = OpAccessChain %_ptr_Function_S %n1 %int_1
OpStore %87 %86
%88 = OpLoad %Nested %n1
OpStore %n2 %88
%89 = OpLoad %Nested %n2
OpStore %n3 %89
%90 = OpAccessChain %_ptr_Function_S %n3 %int_1
%91 = OpLoad %S %90
OpStore %92 %91
%93 = OpFunctionCall %void %modifies_a_struct_vS %92
%94 = OpLoad %S %92
OpStore %90 %94
%98 = OpLoad %float %x
%100 = OpFOrdEqual %bool %98 %float_3
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%104 = OpLoad %float %103
%105 = OpFOrdEqual %bool %104 %float_2
OpBranch %102
%102 = OpLabel
%106 = OpPhi %bool %false %68 %105 %101
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%110 = OpLoad %int %109
%111 = OpIEqual %bool %110 %int_3
OpBranch %108
%108 = OpLabel
%112 = OpPhi %bool %false %102 %111 %107
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpLoad %S %s_0
%116 = OpLoad %S %expected
%117 = OpCompositeExtract %float %115 0
%118 = OpCompositeExtract %float %116 0
%119 = OpFOrdEqual %bool %117 %118
%120 = OpCompositeExtract %int %115 1
%121 = OpCompositeExtract %int %116 1
%122 = OpIEqual %bool %120 %121
%123 = OpLogicalAnd %bool %122 %119
OpBranch %114
%114 = OpLabel
%124 = OpPhi %bool %false %108 %123 %113
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%127 = OpLoad %S %s_0
%128 = OpCompositeConstruct %S %float_2 %int_3
%129 = OpCompositeExtract %float %127 0
%130 = OpCompositeExtract %float %128 0
%131 = OpFOrdEqual %bool %129 %130
%132 = OpCompositeExtract %int %127 1
%133 = OpCompositeExtract %int %128 1
%134 = OpIEqual %bool %132 %133
%135 = OpLogicalAnd %bool %134 %131
OpBranch %126
%126 = OpLabel
%136 = OpPhi %bool %false %114 %135 %125
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%139 = OpLoad %S %s_0
%140 = OpFunctionCall %S %returns_a_struct_S
%141 = OpCompositeExtract %float %139 0
%142 = OpCompositeExtract %float %140 0
%143 = OpFOrdNotEqual %bool %141 %142
%144 = OpCompositeExtract %int %139 1
%145 = OpCompositeExtract %int %140 1
%146 = OpINotEqual %bool %144 %145
%147 = OpLogicalOr %bool %146 %143
OpBranch %138
%138 = OpLabel
%148 = OpPhi %bool %false %126 %147 %137
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%151 = OpLoad %Nested %n1
%152 = OpLoad %Nested %n2
%153 = OpCompositeExtract %S %151 0
%154 = OpCompositeExtract %S %152 0
%155 = OpCompositeExtract %float %153 0
%156 = OpCompositeExtract %float %154 0
%157 = OpFOrdEqual %bool %155 %156
%158 = OpCompositeExtract %int %153 1
%159 = OpCompositeExtract %int %154 1
%160 = OpIEqual %bool %158 %159
%161 = OpLogicalAnd %bool %160 %157
%162 = OpCompositeExtract %S %151 1
%163 = OpCompositeExtract %S %152 1
%164 = OpCompositeExtract %float %162 0
%165 = OpCompositeExtract %float %163 0
%166 = OpFOrdEqual %bool %164 %165
%167 = OpCompositeExtract %int %162 1
%168 = OpCompositeExtract %int %163 1
%169 = OpIEqual %bool %167 %168
%170 = OpLogicalAnd %bool %169 %166
%171 = OpLogicalAnd %bool %170 %161
OpBranch %150
%150 = OpLabel
%172 = OpPhi %bool %false %138 %171 %149
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%175 = OpLoad %Nested %n1
%176 = OpLoad %Nested %n3
%177 = OpCompositeExtract %S %175 0
%178 = OpCompositeExtract %S %176 0
%179 = OpCompositeExtract %float %177 0
%180 = OpCompositeExtract %float %178 0
%181 = OpFOrdNotEqual %bool %179 %180
%182 = OpCompositeExtract %int %177 1
%183 = OpCompositeExtract %int %178 1
%184 = OpINotEqual %bool %182 %183
%185 = OpLogicalOr %bool %184 %181
%186 = OpCompositeExtract %S %175 1
%187 = OpCompositeExtract %S %176 1
%188 = OpCompositeExtract %float %186 0
%189 = OpCompositeExtract %float %187 0
%190 = OpFOrdNotEqual %bool %188 %189
%191 = OpCompositeExtract %int %186 1
%192 = OpCompositeExtract %int %187 1
%193 = OpINotEqual %bool %191 %192
%194 = OpLogicalOr %bool %193 %190
%195 = OpLogicalOr %bool %194 %185
OpBranch %174
%174 = OpLabel
%196 = OpPhi %bool %false %150 %195 %173
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%199 = OpLoad %Nested %n3
%200 = OpCompositeConstruct %S %float_1 %int_2
%201 = OpCompositeConstruct %S %float_2 %int_3
%202 = OpCompositeConstruct %Nested %200 %201
%203 = OpCompositeExtract %S %199 0
%204 = OpCompositeExtract %S %202 0
%205 = OpCompositeExtract %float %203 0
%206 = OpCompositeExtract %float %204 0
%207 = OpFOrdEqual %bool %205 %206
%208 = OpCompositeExtract %int %203 1
%209 = OpCompositeExtract %int %204 1
%210 = OpIEqual %bool %208 %209
%211 = OpLogicalAnd %bool %210 %207
%212 = OpCompositeExtract %S %199 1
%213 = OpCompositeExtract %S %202 1
%214 = OpCompositeExtract %float %212 0
%215 = OpCompositeExtract %float %213 0
%216 = OpFOrdEqual %bool %214 %215
%217 = OpCompositeExtract %int %212 1
%218 = OpCompositeExtract %int %213 1
%219 = OpIEqual %bool %217 %218
%220 = OpLogicalAnd %bool %219 %216
%221 = OpLogicalAnd %bool %220 %211
OpBranch %198
%198 = OpLabel
%222 = OpPhi %bool %false %174 %221 %197
OpStore %valid %222
%223 = OpLoad %bool %valid
OpSelectionMerge %228 None
OpBranchConditional %223 %226 %227
%226 = OpLabel
%229 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%231 = OpLoad %v4float %229
OpStore %224 %231
OpBranch %228
%227 = OpLabel
%232 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%233 = OpLoad %v4float %232
OpStore %224 %233
OpBranch %228
%228 = OpLabel
%234 = OpLoad %v4float %224
OpReturnValue %234
OpFunctionEnd
