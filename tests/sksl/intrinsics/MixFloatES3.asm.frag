OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "colorBlack"
OpMemberName %_UniformBuffer 3 "colorWhite"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %FTFT "FTFT"
OpName %TFTF "TFTF"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%false = OpConstantFalse %bool
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%v2bool = OpTypeVector %bool 2
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%int_4 = OpConstant %int 4
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
%FTFT = OpVariable %_ptr_Function_v4bool Function
%TFTF = OpVariable %_ptr_Function_v4bool Function
%247 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %29
%34 = OpCompositeExtract %float %33 0
%35 = OpFUnordNotEqual %bool %34 %float_0
%36 = OpCompositeExtract %float %33 1
%37 = OpFUnordNotEqual %bool %36 %float_0
%38 = OpCompositeExtract %float %33 2
%39 = OpFUnordNotEqual %bool %38 %float_0
%40 = OpCompositeExtract %float %33 3
%41 = OpFUnordNotEqual %bool %40 %float_0
%42 = OpCompositeConstruct %v4bool %35 %37 %39 %41
OpStore %FTFT %42
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%46 = OpLoad %v4float %45
%47 = OpCompositeExtract %float %46 0
%48 = OpFUnordNotEqual %bool %47 %float_0
%49 = OpCompositeExtract %float %46 1
%50 = OpFUnordNotEqual %bool %49 %float_0
%51 = OpCompositeExtract %float %46 2
%52 = OpFUnordNotEqual %bool %51 %float_0
%53 = OpCompositeExtract %float %46 3
%54 = OpFUnordNotEqual %bool %53 %float_0
%55 = OpCompositeConstruct %v4bool %48 %50 %52 %54
%44 = OpLogicalNot %v4bool %55
OpStore %TFTF %44
%58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%60 = OpLoad %v4float %58
%61 = OpCompositeExtract %float %60 0
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%64 = OpLoad %v4float %62
%65 = OpCompositeExtract %float %64 0
%66 = OpCompositeExtract %bool %42 0
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%68 = OpLoad %v4float %67
%69 = OpCompositeExtract %float %68 0
%70 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%71 = OpLoad %v4float %70
%72 = OpCompositeExtract %float %71 0
%57 = OpSelect %float %66 %72 %69
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%74 = OpLoad %v4float %73
%75 = OpCompositeExtract %float %74 0
%76 = OpFOrdEqual %bool %57 %75
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%81 = OpLoad %v4float %80
%82 = OpVectorShuffle %v2float %81 %81 0 1
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%84 = OpLoad %v4float %83
%85 = OpVectorShuffle %v2float %84 %84 0 1
%86 = OpVectorShuffle %v2bool %42 %42 0 1
%88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%89 = OpLoad %v4float %88
%90 = OpVectorShuffle %v2float %89 %89 0 1
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%92 = OpLoad %v4float %91
%93 = OpVectorShuffle %v2float %92 %92 0 1
%94 = OpVectorShuffle %v2bool %42 %42 0 1
%79 = OpSelect %v2float %94 %93 %90
%95 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%96 = OpLoad %v4float %95
%97 = OpCompositeExtract %float %96 0
%99 = OpCompositeConstruct %v2float %97 %float_1
%100 = OpFOrdEqual %v2bool %79 %99
%101 = OpAll %bool %100
OpBranch %78
%78 = OpLabel
%102 = OpPhi %bool %false %25 %101 %77
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%107 = OpLoad %v4float %106
%108 = OpVectorShuffle %v3float %107 %107 0 1 2
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%111 = OpLoad %v4float %110
%112 = OpVectorShuffle %v3float %111 %111 0 1 2
%113 = OpVectorShuffle %v3bool %42 %42 0 1 2
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%116 = OpLoad %v4float %115
%117 = OpVectorShuffle %v3float %116 %116 0 1 2
%118 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%119 = OpLoad %v4float %118
%120 = OpVectorShuffle %v3float %119 %119 0 1 2
%121 = OpVectorShuffle %v3bool %42 %42 0 1 2
%105 = OpSelect %v3float %121 %120 %117
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%123 = OpLoad %v4float %122
%124 = OpCompositeExtract %float %123 0
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%126 = OpLoad %v4float %125
%127 = OpCompositeExtract %float %126 2
%128 = OpCompositeConstruct %v3float %124 %float_1 %127
%129 = OpFOrdEqual %v3bool %105 %128
%130 = OpAll %bool %129
OpBranch %104
%104 = OpLabel
%131 = OpPhi %bool %false %78 %130 %103
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%136 = OpLoad %v4float %135
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%138 = OpLoad %v4float %137
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%140 = OpLoad %v4float %139
%141 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%142 = OpLoad %v4float %141
%134 = OpSelect %v4float %42 %142 %140
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%144 = OpLoad %v4float %143
%145 = OpCompositeExtract %float %144 0
%146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%147 = OpLoad %v4float %146
%148 = OpCompositeExtract %float %147 2
%149 = OpCompositeConstruct %v4float %145 %float_1 %148 %float_1
%150 = OpFOrdEqual %v4bool %134 %149
%151 = OpAll %bool %150
OpBranch %133
%133 = OpLabel
%152 = OpPhi %bool %false %104 %151 %132
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%156 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%157 = OpLoad %v4float %156
%158 = OpCompositeExtract %float %157 0
%159 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%161 = OpLoad %v4float %159
%162 = OpCompositeExtract %float %161 0
%163 = OpCompositeExtract %bool %44 0
%164 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%165 = OpLoad %v4float %164
%166 = OpCompositeExtract %float %165 0
%167 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%168 = OpLoad %v4float %167
%169 = OpCompositeExtract %float %168 0
%155 = OpSelect %float %163 %169 %166
%170 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%171 = OpLoad %v4float %170
%172 = OpCompositeExtract %float %171 0
%173 = OpFOrdEqual %bool %155 %172
OpBranch %154
%154 = OpLabel
%174 = OpPhi %bool %false %133 %173 %153
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%179 = OpLoad %v4float %178
%180 = OpVectorShuffle %v2float %179 %179 0 1
%181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%182 = OpLoad %v4float %181
%183 = OpVectorShuffle %v2float %182 %182 0 1
%184 = OpVectorShuffle %v2bool %44 %44 0 1
%185 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%186 = OpLoad %v4float %185
%187 = OpVectorShuffle %v2float %186 %186 0 1
%188 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%189 = OpLoad %v4float %188
%190 = OpVectorShuffle %v2float %189 %189 0 1
%191 = OpVectorShuffle %v2bool %44 %44 0 1
%177 = OpSelect %v2float %191 %190 %187
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%193 = OpLoad %v4float %192
%194 = OpCompositeExtract %float %193 0
%195 = OpCompositeConstruct %v2float %194 %float_1
%196 = OpFOrdEqual %v2bool %177 %195
%197 = OpAll %bool %196
OpBranch %176
%176 = OpLabel
%198 = OpPhi %bool %false %154 %197 %175
OpSelectionMerge %200 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%202 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%203 = OpLoad %v4float %202
%204 = OpVectorShuffle %v3float %203 %203 0 1 2
%205 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%206 = OpLoad %v4float %205
%207 = OpVectorShuffle %v3float %206 %206 0 1 2
%208 = OpVectorShuffle %v3bool %44 %44 0 1 2
%209 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%210 = OpLoad %v4float %209
%211 = OpVectorShuffle %v3float %210 %210 0 1 2
%212 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%213 = OpLoad %v4float %212
%214 = OpVectorShuffle %v3float %213 %213 0 1 2
%215 = OpVectorShuffle %v3bool %44 %44 0 1 2
%201 = OpSelect %v3float %215 %214 %211
%216 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%217 = OpLoad %v4float %216
%218 = OpCompositeExtract %float %217 0
%219 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%220 = OpLoad %v4float %219
%221 = OpCompositeExtract %float %220 2
%222 = OpCompositeConstruct %v3float %218 %float_1 %221
%223 = OpFOrdEqual %v3bool %201 %222
%224 = OpAll %bool %223
OpBranch %200
%200 = OpLabel
%225 = OpPhi %bool %false %176 %224 %199
OpSelectionMerge %227 None
OpBranchConditional %225 %226 %227
%226 = OpLabel
%229 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%230 = OpLoad %v4float %229
%231 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%232 = OpLoad %v4float %231
%233 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%234 = OpLoad %v4float %233
%235 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%236 = OpLoad %v4float %235
%228 = OpSelect %v4float %44 %236 %234
%237 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%238 = OpLoad %v4float %237
%239 = OpCompositeExtract %float %238 0
%240 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%241 = OpLoad %v4float %240
%242 = OpCompositeExtract %float %241 2
%243 = OpCompositeConstruct %v4float %239 %float_1 %242 %float_1
%244 = OpFOrdEqual %v4bool %228 %243
%245 = OpAll %bool %244
OpBranch %227
%227 = OpLabel
%246 = OpPhi %bool %false %200 %245 %226
OpSelectionMerge %251 None
OpBranchConditional %246 %249 %250
%249 = OpLabel
%252 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%253 = OpLoad %v4float %252
OpStore %247 %253
OpBranch %251
%250 = OpLabel
%254 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%256 = OpLoad %v4float %254
OpStore %247 %256
OpBranch %251
%251 = OpLabel
%257 = OpLoad %v4float %247
OpReturnValue %257
OpFunctionEnd
