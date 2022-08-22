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
OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
OpName %soft_light_component_Qhh2h2 "soft_light_component_Qhh2h2"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %DSqd RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %DCub RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %DaSqd RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %DaCub RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
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
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
%16 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%36 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%void = OpTypeVoid
%198 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_0 = OpConstant %int 0
%guarded_divide_Qhhh = OpFunction %float None %16
%17 = OpFunctionParameter %_ptr_Function_float
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpLabel
%21 = OpVariable %_ptr_Function_float Function
OpSelectionMerge %24 None
OpBranchConditional %false %22 %23
%22 = OpLabel
%25 = OpLoad %float %17
%26 = OpLoad %float %18
%28 = OpFAdd %float %26 %float_9_99999994en09
%29 = OpFDiv %float %25 %28
OpStore %21 %29
OpBranch %24
%23 = OpLabel
%30 = OpLoad %float %17
%31 = OpLoad %float %18
%32 = OpFDiv %float %30 %31
OpStore %21 %32
OpBranch %24
%24 = OpLabel
%33 = OpLoad %float %21
OpReturnValue %33
OpFunctionEnd
%soft_light_component_Qhh2h2 = OpFunction %float None %36
%37 = OpFunctionParameter %_ptr_Function_v2float
%38 = OpFunctionParameter %_ptr_Function_v2float
%39 = OpLabel
%62 = OpVariable %_ptr_Function_float Function
%65 = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%161 = OpVariable %_ptr_Function_float Function
%162 = OpVariable %_ptr_Function_float Function
%41 = OpLoad %v2float %37
%42 = OpCompositeExtract %float %41 0
%43 = OpFMul %float %float_2 %42
%44 = OpLoad %v2float %37
%45 = OpCompositeExtract %float %44 1
%46 = OpFOrdLessThanEqual %bool %43 %45
OpSelectionMerge %49 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%50 = OpLoad %v2float %38
%51 = OpCompositeExtract %float %50 0
%52 = OpLoad %v2float %38
%53 = OpCompositeExtract %float %52 0
%54 = OpFMul %float %51 %53
%55 = OpLoad %v2float %37
%56 = OpCompositeExtract %float %55 1
%57 = OpLoad %v2float %37
%58 = OpCompositeExtract %float %57 0
%59 = OpFMul %float %float_2 %58
%60 = OpFSub %float %56 %59
%61 = OpFMul %float %54 %60
OpStore %62 %61
%63 = OpLoad %v2float %38
%64 = OpCompositeExtract %float %63 1
OpStore %65 %64
%66 = OpFunctionCall %float %guarded_divide_Qhhh %62 %65
%68 = OpLoad %v2float %38
%69 = OpCompositeExtract %float %68 1
%70 = OpFSub %float %float_1 %69
%71 = OpLoad %v2float %37
%72 = OpCompositeExtract %float %71 0
%73 = OpFMul %float %70 %72
%74 = OpFAdd %float %66 %73
%75 = OpLoad %v2float %38
%76 = OpCompositeExtract %float %75 0
%77 = OpLoad %v2float %37
%78 = OpCompositeExtract %float %77 1
%79 = OpFNegate %float %78
%80 = OpLoad %v2float %37
%81 = OpCompositeExtract %float %80 0
%82 = OpFMul %float %float_2 %81
%83 = OpFAdd %float %79 %82
%84 = OpFAdd %float %83 %float_1
%85 = OpFMul %float %76 %84
%86 = OpFAdd %float %74 %85
OpReturnValue %86
%48 = OpLabel
%88 = OpLoad %v2float %38
%89 = OpCompositeExtract %float %88 0
%90 = OpFMul %float %float_4 %89
%91 = OpLoad %v2float %38
%92 = OpCompositeExtract %float %91 1
%93 = OpFOrdLessThanEqual %bool %90 %92
OpSelectionMerge %96 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%98 = OpLoad %v2float %38
%99 = OpCompositeExtract %float %98 0
%100 = OpLoad %v2float %38
%101 = OpCompositeExtract %float %100 0
%102 = OpFMul %float %99 %101
OpStore %DSqd %102
%104 = OpLoad %v2float %38
%105 = OpCompositeExtract %float %104 0
%106 = OpFMul %float %102 %105
OpStore %DCub %106
%108 = OpLoad %v2float %38
%109 = OpCompositeExtract %float %108 1
%110 = OpLoad %v2float %38
%111 = OpCompositeExtract %float %110 1
%112 = OpFMul %float %109 %111
OpStore %DaSqd %112
%114 = OpLoad %v2float %38
%115 = OpCompositeExtract %float %114 1
%116 = OpFMul %float %112 %115
OpStore %DaCub %116
%117 = OpLoad %v2float %37
%118 = OpCompositeExtract %float %117 0
%119 = OpLoad %v2float %38
%120 = OpCompositeExtract %float %119 0
%122 = OpLoad %v2float %37
%123 = OpCompositeExtract %float %122 1
%124 = OpFMul %float %float_3 %123
%126 = OpLoad %v2float %37
%127 = OpCompositeExtract %float %126 0
%128 = OpFMul %float %float_6 %127
%129 = OpFSub %float %124 %128
%130 = OpFSub %float %129 %float_1
%131 = OpFMul %float %120 %130
%132 = OpFSub %float %118 %131
%133 = OpFMul %float %112 %132
%135 = OpLoad %v2float %38
%136 = OpCompositeExtract %float %135 1
%137 = OpFMul %float %float_12 %136
%138 = OpFMul %float %137 %102
%139 = OpLoad %v2float %37
%140 = OpCompositeExtract %float %139 1
%141 = OpLoad %v2float %37
%142 = OpCompositeExtract %float %141 0
%143 = OpFMul %float %float_2 %142
%144 = OpFSub %float %140 %143
%145 = OpFMul %float %138 %144
%146 = OpFAdd %float %133 %145
%148 = OpFMul %float %float_16 %106
%149 = OpLoad %v2float %37
%150 = OpCompositeExtract %float %149 1
%151 = OpLoad %v2float %37
%152 = OpCompositeExtract %float %151 0
%153 = OpFMul %float %float_2 %152
%154 = OpFSub %float %150 %153
%155 = OpFMul %float %148 %154
%156 = OpFSub %float %146 %155
%157 = OpLoad %v2float %37
%158 = OpCompositeExtract %float %157 0
%159 = OpFMul %float %116 %158
%160 = OpFSub %float %156 %159
OpStore %161 %160
OpStore %162 %112
%163 = OpFunctionCall %float %guarded_divide_Qhhh %161 %162
OpReturnValue %163
%95 = OpLabel
%164 = OpLoad %v2float %38
%165 = OpCompositeExtract %float %164 0
%166 = OpLoad %v2float %37
%167 = OpCompositeExtract %float %166 1
%168 = OpLoad %v2float %37
%169 = OpCompositeExtract %float %168 0
%170 = OpFMul %float %float_2 %169
%171 = OpFSub %float %167 %170
%172 = OpFAdd %float %171 %float_1
%173 = OpFMul %float %165 %172
%174 = OpLoad %v2float %37
%175 = OpCompositeExtract %float %174 0
%176 = OpFAdd %float %173 %175
%178 = OpLoad %v2float %38
%179 = OpCompositeExtract %float %178 1
%180 = OpLoad %v2float %38
%181 = OpCompositeExtract %float %180 0
%182 = OpFMul %float %179 %181
%177 = OpExtInst %float %1 Sqrt %182
%183 = OpLoad %v2float %37
%184 = OpCompositeExtract %float %183 1
%185 = OpLoad %v2float %37
%186 = OpCompositeExtract %float %185 0
%187 = OpFMul %float %float_2 %186
%188 = OpFSub %float %184 %187
%189 = OpFMul %float %177 %188
%190 = OpFSub %float %176 %189
%191 = OpLoad %v2float %38
%192 = OpCompositeExtract %float %191 1
%193 = OpLoad %v2float %37
%194 = OpCompositeExtract %float %193 0
%195 = OpFMul %float %192 %194
%196 = OpFSub %float %190 %195
OpReturnValue %196
%96 = OpLabel
OpBranch %49
%49 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %198
%199 = OpLabel
%208 = OpVariable %_ptr_Function_v4float Function
%219 = OpVariable %_ptr_Function_v2float Function
%223 = OpVariable %_ptr_Function_v2float Function
%228 = OpVariable %_ptr_Function_v2float Function
%232 = OpVariable %_ptr_Function_v2float Function
%237 = OpVariable %_ptr_Function_v2float Function
%241 = OpVariable %_ptr_Function_v2float Function
%200 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%204 = OpLoad %v4float %200
%205 = OpCompositeExtract %float %204 3
%207 = OpFOrdEqual %bool %205 %float_0
OpSelectionMerge %212 None
OpBranchConditional %207 %210 %211
%210 = OpLabel
%213 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%215 = OpLoad %v4float %213
OpStore %208 %215
OpBranch %212
%211 = OpLabel
%216 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%217 = OpLoad %v4float %216
%218 = OpVectorShuffle %v2float %217 %217 0 3
OpStore %219 %218
%220 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%221 = OpLoad %v4float %220
%222 = OpVectorShuffle %v2float %221 %221 0 3
OpStore %223 %222
%224 = OpFunctionCall %float %soft_light_component_Qhh2h2 %219 %223
%225 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%226 = OpLoad %v4float %225
%227 = OpVectorShuffle %v2float %226 %226 1 3
OpStore %228 %227
%229 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%230 = OpLoad %v4float %229
%231 = OpVectorShuffle %v2float %230 %230 1 3
OpStore %232 %231
%233 = OpFunctionCall %float %soft_light_component_Qhh2h2 %228 %232
%234 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%235 = OpLoad %v4float %234
%236 = OpVectorShuffle %v2float %235 %235 2 3
OpStore %237 %236
%238 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%239 = OpLoad %v4float %238
%240 = OpVectorShuffle %v2float %239 %239 2 3
OpStore %241 %240
%242 = OpFunctionCall %float %soft_light_component_Qhh2h2 %237 %241
%243 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%244 = OpLoad %v4float %243
%245 = OpCompositeExtract %float %244 3
%246 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%247 = OpLoad %v4float %246
%248 = OpCompositeExtract %float %247 3
%249 = OpFSub %float %float_1 %248
%250 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%251 = OpLoad %v4float %250
%252 = OpCompositeExtract %float %251 3
%253 = OpFMul %float %249 %252
%254 = OpFAdd %float %245 %253
%255 = OpCompositeConstruct %v4float %224 %233 %242 %254
OpStore %208 %255
OpBranch %212
%212 = OpLabel
%256 = OpLoad %v4float %208
OpStore %sk_FragColor %256
OpReturn
OpFunctionEnd
