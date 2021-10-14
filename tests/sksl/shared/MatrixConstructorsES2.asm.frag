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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f4 "f4"
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
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %70 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%mat3v3float = OpTypeMatrix %v3float 3
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%f4 = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_bool Function
%180 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %f4 %38
%41 = OpLoad %v4float %f4
%42 = OpVectorShuffle %v3float %41 %41 0 1 2
%44 = OpLoad %v4float %f4
%45 = OpCompositeExtract %float %44 3
%46 = OpCompositeExtract %float %42 0
%47 = OpCompositeExtract %float %42 1
%48 = OpCompositeConstruct %v2float %46 %47
%49 = OpCompositeExtract %float %42 2
%50 = OpCompositeConstruct %v2float %49 %45
%51 = OpCompositeConstruct %mat2v2float %48 %50
%56 = OpCompositeConstruct %v2float %float_1 %float_2
%57 = OpCompositeConstruct %v2float %float_3 %float_4
%58 = OpCompositeConstruct %mat2v2float %56 %57
%60 = OpCompositeExtract %v2float %51 0
%61 = OpCompositeExtract %v2float %58 0
%62 = OpFOrdEqual %v2bool %60 %61
%63 = OpAll %bool %62
%64 = OpCompositeExtract %v2float %51 1
%65 = OpCompositeExtract %v2float %58 1
%66 = OpFOrdEqual %v2bool %64 %65
%67 = OpAll %bool %66
%68 = OpLogicalAnd %bool %63 %67
OpStore %ok %68
%70 = OpLoad %bool %ok
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%73 = OpLoad %v4float %f4
%74 = OpVectorShuffle %v2float %73 %73 0 1
%75 = OpLoad %v4float %f4
%76 = OpVectorShuffle %v2float %75 %75 2 3
%77 = OpLoad %v4float %f4
%78 = OpCompositeExtract %float %77 0
%79 = OpLoad %v4float %f4
%80 = OpCompositeExtract %float %79 1
%81 = OpLoad %v4float %f4
%82 = OpCompositeExtract %float %81 2
%83 = OpLoad %v4float %f4
%84 = OpCompositeExtract %float %83 3
%85 = OpLoad %v4float %f4
%86 = OpCompositeExtract %float %85 0
%87 = OpCompositeExtract %float %74 0
%88 = OpCompositeExtract %float %74 1
%89 = OpCompositeExtract %float %76 0
%90 = OpCompositeConstruct %v3float %87 %88 %89
%91 = OpCompositeExtract %float %76 1
%92 = OpCompositeConstruct %v3float %91 %78 %80
%93 = OpCompositeConstruct %v3float %82 %84 %86
%94 = OpCompositeConstruct %mat3v3float %90 %92 %93
%96 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%97 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%98 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%99 = OpCompositeConstruct %mat3v3float %96 %97 %98
%101 = OpCompositeExtract %v3float %94 0
%102 = OpCompositeExtract %v3float %99 0
%103 = OpFOrdEqual %v3bool %101 %102
%104 = OpAll %bool %103
%105 = OpCompositeExtract %v3float %94 1
%106 = OpCompositeExtract %v3float %99 1
%107 = OpFOrdEqual %v3bool %105 %106
%108 = OpAll %bool %107
%109 = OpLogicalAnd %bool %104 %108
%110 = OpCompositeExtract %v3float %94 2
%111 = OpCompositeExtract %v3float %99 2
%112 = OpFOrdEqual %v3bool %110 %111
%113 = OpAll %bool %112
%114 = OpLogicalAnd %bool %109 %113
OpBranch %72
%72 = OpLabel
%115 = OpPhi %bool %false %26 %114 %71
OpStore %ok %115
%116 = OpLoad %bool %ok
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%119 = OpLoad %v4float %f4
%120 = OpCompositeExtract %float %119 0
%121 = OpLoad %v4float %f4
%122 = OpVectorShuffle %v3float %121 %121 1 2 3
%123 = OpLoad %v4float %f4
%124 = OpVectorShuffle %v3float %123 %123 0 1 2
%125 = OpLoad %v4float %f4
%126 = OpVectorShuffle %v4float %125 %125 3 0 1 2
%127 = OpLoad %v4float %f4
%128 = OpVectorShuffle %v2float %127 %127 3 0
%129 = OpLoad %v4float %f4
%130 = OpVectorShuffle %v2float %129 %129 1 2
%131 = OpLoad %v4float %f4
%132 = OpCompositeExtract %float %131 3
%133 = OpCompositeExtract %float %122 0
%134 = OpCompositeExtract %float %122 1
%135 = OpCompositeExtract %float %122 2
%136 = OpCompositeConstruct %v4float %120 %133 %134 %135
%137 = OpCompositeExtract %float %124 0
%138 = OpCompositeExtract %float %124 1
%139 = OpCompositeExtract %float %124 2
%140 = OpCompositeExtract %float %126 0
%141 = OpCompositeConstruct %v4float %137 %138 %139 %140
%142 = OpCompositeExtract %float %126 1
%143 = OpCompositeExtract %float %126 2
%144 = OpCompositeExtract %float %126 3
%145 = OpCompositeExtract %float %128 0
%146 = OpCompositeConstruct %v4float %142 %143 %144 %145
%147 = OpCompositeExtract %float %128 1
%148 = OpCompositeExtract %float %130 0
%149 = OpCompositeExtract %float %130 1
%150 = OpCompositeConstruct %v4float %147 %148 %149 %132
%151 = OpCompositeConstruct %mat4v4float %136 %141 %146 %150
%153 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%154 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%155 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%156 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%157 = OpCompositeConstruct %mat4v4float %153 %154 %155 %156
%159 = OpCompositeExtract %v4float %151 0
%160 = OpCompositeExtract %v4float %157 0
%161 = OpFOrdEqual %v4bool %159 %160
%162 = OpAll %bool %161
%163 = OpCompositeExtract %v4float %151 1
%164 = OpCompositeExtract %v4float %157 1
%165 = OpFOrdEqual %v4bool %163 %164
%166 = OpAll %bool %165
%167 = OpLogicalAnd %bool %162 %166
%168 = OpCompositeExtract %v4float %151 2
%169 = OpCompositeExtract %v4float %157 2
%170 = OpFOrdEqual %v4bool %168 %169
%171 = OpAll %bool %170
%172 = OpLogicalAnd %bool %167 %171
%173 = OpCompositeExtract %v4float %151 3
%174 = OpCompositeExtract %v4float %157 3
%175 = OpFOrdEqual %v4bool %173 %174
%176 = OpAll %bool %175
%177 = OpLogicalAnd %bool %172 %176
OpBranch %118
%118 = OpLabel
%178 = OpPhi %bool %false %72 %177 %117
OpStore %ok %178
%179 = OpLoad %bool %ok
OpSelectionMerge %183 None
OpBranchConditional %179 %181 %182
%181 = OpLabel
%184 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%187 = OpLoad %v4float %184
OpStore %180 %187
OpBranch %183
%182 = OpLabel
%188 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%190 = OpLoad %v4float %188
OpStore %180 %190
OpBranch %183
%183 = OpLabel
%191 = OpLoad %v4float %180
OpReturnValue %191
OpFunctionEnd
