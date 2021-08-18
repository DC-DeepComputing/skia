OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 "check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3"
OpName %main "main"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %v3 "v3"
OpName %v4 "v4"
OpName %v5 "v5"
OpName %v6 "v6"
OpName %v7 "v7"
OpName %v8 "v8"
OpName %v9 "v9"
OpName %v10 "v10"
OpName %v11 "v11"
OpName %v12 "v12"
OpName %v13 "v13"
OpName %v14 "v14"
OpName %v15 "v15"
OpName %v16 "v16"
OpName %v17 "v17"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%v3float = OpTypeVector %float 3
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%v4bool = OpTypeVector %bool 4
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%32 = OpTypeFunction %bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v3float %_ptr_Function_v2int %_ptr_Function_v2int %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v2int %_ptr_Function_v4bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2bool %_ptr_Function_v2bool %_ptr_Function_v3bool
%float_1 = OpConstant %float 1
%float_17 = OpConstant %float 17
%117 = OpTypeFunction %v4float %_ptr_Function_v2float
%121 = OpConstantComposite %v2float %float_1 %float_1
%float_2 = OpConstant %float 2
%124 = OpConstantComposite %v2float %float_1 %float_2
%127 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int_1 = OpConstant %int 1
%130 = OpConstantComposite %v2int %int_1 %int_1
%int_2 = OpConstant %int 2
%133 = OpConstantComposite %v2int %int_1 %int_2
%_ptr_Uniform_float = OpTypePointer Uniform %float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_3 = OpConstant %int 3
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%161 = OpConstantComposite %v4bool %true %false %true %false
%163 = OpConstantComposite %v2float %float_1 %float_0
%167 = OpConstantComposite %v2bool %true %true
%170 = OpConstantComposite %v3bool %true %true %true
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 = OpFunction %bool None %32
%39 = OpFunctionParameter %_ptr_Function_v2float
%40 = OpFunctionParameter %_ptr_Function_v2float
%41 = OpFunctionParameter %_ptr_Function_v2float
%42 = OpFunctionParameter %_ptr_Function_v3float
%43 = OpFunctionParameter %_ptr_Function_v2int
%44 = OpFunctionParameter %_ptr_Function_v2int
%45 = OpFunctionParameter %_ptr_Function_v2float
%46 = OpFunctionParameter %_ptr_Function_v2float
%47 = OpFunctionParameter %_ptr_Function_v4float
%48 = OpFunctionParameter %_ptr_Function_v2int
%49 = OpFunctionParameter %_ptr_Function_v4bool
%50 = OpFunctionParameter %_ptr_Function_v2float
%51 = OpFunctionParameter %_ptr_Function_v2float
%52 = OpFunctionParameter %_ptr_Function_v2float
%53 = OpFunctionParameter %_ptr_Function_v2bool
%54 = OpFunctionParameter %_ptr_Function_v2bool
%55 = OpFunctionParameter %_ptr_Function_v3bool
%56 = OpLabel
%57 = OpLoad %v2float %39
%58 = OpCompositeExtract %float %57 0
%59 = OpLoad %v2float %40
%60 = OpCompositeExtract %float %59 0
%61 = OpFAdd %float %58 %60
%62 = OpLoad %v2float %41
%63 = OpCompositeExtract %float %62 0
%64 = OpFAdd %float %61 %63
%65 = OpLoad %v3float %42
%66 = OpCompositeExtract %float %65 0
%67 = OpFAdd %float %64 %66
%68 = OpLoad %v2int %43
%69 = OpCompositeExtract %int %68 0
%70 = OpConvertSToF %float %69
%71 = OpFAdd %float %67 %70
%72 = OpLoad %v2int %44
%73 = OpCompositeExtract %int %72 0
%74 = OpConvertSToF %float %73
%75 = OpFAdd %float %71 %74
%76 = OpLoad %v2float %45
%77 = OpCompositeExtract %float %76 0
%78 = OpFAdd %float %75 %77
%79 = OpLoad %v2float %46
%80 = OpCompositeExtract %float %79 0
%81 = OpFAdd %float %78 %80
%82 = OpLoad %v4float %47
%83 = OpCompositeExtract %float %82 0
%84 = OpFAdd %float %81 %83
%85 = OpLoad %v2int %48
%86 = OpCompositeExtract %int %85 0
%87 = OpConvertSToF %float %86
%88 = OpFAdd %float %84 %87
%89 = OpLoad %v4bool %49
%90 = OpCompositeExtract %bool %89 0
%91 = OpSelect %float %90 %float_1 %float_0
%93 = OpFAdd %float %88 %91
%94 = OpLoad %v2float %50
%95 = OpCompositeExtract %float %94 0
%96 = OpFAdd %float %93 %95
%97 = OpLoad %v2float %51
%98 = OpCompositeExtract %float %97 0
%99 = OpFAdd %float %96 %98
%100 = OpLoad %v2float %52
%101 = OpCompositeExtract %float %100 0
%102 = OpFAdd %float %99 %101
%103 = OpLoad %v2bool %53
%104 = OpCompositeExtract %bool %103 0
%105 = OpSelect %float %104 %float_1 %float_0
%106 = OpFAdd %float %102 %105
%107 = OpLoad %v2bool %54
%108 = OpCompositeExtract %bool %107 0
%109 = OpSelect %float %108 %float_1 %float_0
%110 = OpFAdd %float %106 %109
%111 = OpLoad %v3bool %55
%112 = OpCompositeExtract %bool %111 0
%113 = OpSelect %float %112 %float_1 %float_0
%114 = OpFAdd %float %110 %113
%116 = OpFOrdEqual %bool %114 %float_17
OpReturnValue %116
OpFunctionEnd
%main = OpFunction %v4float None %117
%118 = OpFunctionParameter %_ptr_Function_v2float
%119 = OpLabel
%v1 = OpVariable %_ptr_Function_v2float Function
%v2 = OpVariable %_ptr_Function_v2float Function
%v3 = OpVariable %_ptr_Function_v2float Function
%v4 = OpVariable %_ptr_Function_v3float Function
%v5 = OpVariable %_ptr_Function_v2int Function
%v6 = OpVariable %_ptr_Function_v2int Function
%v7 = OpVariable %_ptr_Function_v2float Function
%v8 = OpVariable %_ptr_Function_v2float Function
%v9 = OpVariable %_ptr_Function_v4float Function
%v10 = OpVariable %_ptr_Function_v2int Function
%v11 = OpVariable %_ptr_Function_v4bool Function
%v12 = OpVariable %_ptr_Function_v2float Function
%v13 = OpVariable %_ptr_Function_v2float Function
%v14 = OpVariable %_ptr_Function_v2float Function
%v15 = OpVariable %_ptr_Function_v2bool Function
%v16 = OpVariable %_ptr_Function_v2bool Function
%v17 = OpVariable %_ptr_Function_v3bool Function
%172 = OpVariable %_ptr_Function_v2float Function
%174 = OpVariable %_ptr_Function_v2float Function
%176 = OpVariable %_ptr_Function_v2float Function
%178 = OpVariable %_ptr_Function_v3float Function
%180 = OpVariable %_ptr_Function_v2int Function
%182 = OpVariable %_ptr_Function_v2int Function
%184 = OpVariable %_ptr_Function_v2float Function
%186 = OpVariable %_ptr_Function_v2float Function
%188 = OpVariable %_ptr_Function_v4float Function
%190 = OpVariable %_ptr_Function_v2int Function
%192 = OpVariable %_ptr_Function_v4bool Function
%194 = OpVariable %_ptr_Function_v2float Function
%196 = OpVariable %_ptr_Function_v2float Function
%198 = OpVariable %_ptr_Function_v2float Function
%200 = OpVariable %_ptr_Function_v2bool Function
%202 = OpVariable %_ptr_Function_v2bool Function
%204 = OpVariable %_ptr_Function_v3bool Function
%206 = OpVariable %_ptr_Function_v4float Function
OpStore %v1 %121
OpStore %v2 %124
OpStore %v3 %121
OpStore %v4 %127
OpStore %v5 %130
OpStore %v6 %133
OpStore %v7 %124
%136 = OpLoad %v2int %v5
%137 = OpCompositeExtract %int %136 0
%138 = OpConvertSToF %float %137
%139 = OpCompositeExtract %int %136 1
%140 = OpConvertSToF %float %139
%141 = OpCompositeConstruct %v2float %138 %140
OpStore %v8 %141
%143 = OpLoad %v2int %v6
%144 = OpCompositeExtract %int %143 0
%145 = OpConvertSToF %float %144
%146 = OpAccessChain %_ptr_Uniform_float %13 %int_2
%148 = OpLoad %float %146
%151 = OpCompositeConstruct %v4float %145 %148 %float_3 %float_4
OpStore %v9 %151
%154 = OpLoad %v2float %v1
%155 = OpCompositeExtract %float %154 0
%156 = OpConvertFToS %int %155
%157 = OpCompositeConstruct %v2int %int_3 %156
OpStore %v10 %157
OpStore %v11 %161
OpStore %v12 %163
OpStore %v13 %22
OpStore %v14 %22
OpStore %v15 %167
OpStore %v16 %167
OpStore %v17 %170
%171 = OpLoad %v2float %v1
OpStore %172 %171
%173 = OpLoad %v2float %v2
OpStore %174 %173
%175 = OpLoad %v2float %v3
OpStore %176 %175
%177 = OpLoad %v3float %v4
OpStore %178 %177
%179 = OpLoad %v2int %v5
OpStore %180 %179
%181 = OpLoad %v2int %v6
OpStore %182 %181
%183 = OpLoad %v2float %v7
OpStore %184 %183
%185 = OpLoad %v2float %v8
OpStore %186 %185
%187 = OpLoad %v4float %v9
OpStore %188 %187
%189 = OpLoad %v2int %v10
OpStore %190 %189
%191 = OpLoad %v4bool %v11
OpStore %192 %191
%193 = OpLoad %v2float %v12
OpStore %194 %193
%195 = OpLoad %v2float %v13
OpStore %196 %195
%197 = OpLoad %v2float %v14
OpStore %198 %197
%199 = OpLoad %v2bool %v15
OpStore %200 %199
%201 = OpLoad %v2bool %v16
OpStore %202 %201
%203 = OpLoad %v3bool %v17
OpStore %204 %203
%205 = OpFunctionCall %bool %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 %172 %174 %176 %178 %180 %182 %184 %186 %188 %190 %192 %194 %196 %198 %200 %202 %204
OpSelectionMerge %209 None
OpBranchConditional %205 %207 %208
%207 = OpLabel
%210 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%213 = OpLoad %v4float %210
OpStore %206 %213
OpBranch %209
%208 = OpLabel
%214 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%215 = OpLoad %v4float %214
OpStore %206 %215
OpBranch %209
%209 = OpLabel
%216 = OpLoad %v4float %206
OpReturnValue %216
OpFunctionEnd
