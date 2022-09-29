OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"
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
OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %18 Binding 0
OpDecorate %18 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
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
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %DSqd RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %DCub RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %DaSqd RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %DaCub RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
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
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
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
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
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
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
%float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
%bool = OpTypeBool
%false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
%float_0 = OpConstant %float 0
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
%22 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%33 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%void = OpTypeVoid
%195 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_0 = OpConstant %int 0
%guarded_divide_Qhhh = OpFunction %float None %22
%23 = OpFunctionParameter %_ptr_Function_float
%24 = OpFunctionParameter %_ptr_Function_float
%25 = OpLabel
%26 = OpLoad %float %23
%27 = OpLoad %float %24
%28 = OpLoad %float %_kGuardedDivideEpsilon
%29 = OpFAdd %float %27 %28
%30 = OpFDiv %float %26 %29
OpReturnValue %30
OpFunctionEnd
%soft_light_component_Qhh2h2 = OpFunction %float None %33
%34 = OpFunctionParameter %_ptr_Function_v2float
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpLabel
%59 = OpVariable %_ptr_Function_float Function
%62 = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%158 = OpVariable %_ptr_Function_float Function
%159 = OpVariable %_ptr_Function_float Function
%38 = OpLoad %v2float %34
%39 = OpCompositeExtract %float %38 0
%40 = OpFMul %float %float_2 %39
%41 = OpLoad %v2float %34
%42 = OpCompositeExtract %float %41 1
%43 = OpFOrdLessThanEqual %bool %40 %42
OpSelectionMerge %46 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%47 = OpLoad %v2float %35
%48 = OpCompositeExtract %float %47 0
%49 = OpLoad %v2float %35
%50 = OpCompositeExtract %float %49 0
%51 = OpFMul %float %48 %50
%52 = OpLoad %v2float %34
%53 = OpCompositeExtract %float %52 1
%54 = OpLoad %v2float %34
%55 = OpCompositeExtract %float %54 0
%56 = OpFMul %float %float_2 %55
%57 = OpFSub %float %53 %56
%58 = OpFMul %float %51 %57
OpStore %59 %58
%60 = OpLoad %v2float %35
%61 = OpCompositeExtract %float %60 1
OpStore %62 %61
%63 = OpFunctionCall %float %guarded_divide_Qhhh %59 %62
%65 = OpLoad %v2float %35
%66 = OpCompositeExtract %float %65 1
%67 = OpFSub %float %float_1 %66
%68 = OpLoad %v2float %34
%69 = OpCompositeExtract %float %68 0
%70 = OpFMul %float %67 %69
%71 = OpFAdd %float %63 %70
%72 = OpLoad %v2float %35
%73 = OpCompositeExtract %float %72 0
%74 = OpLoad %v2float %34
%75 = OpCompositeExtract %float %74 1
%76 = OpFNegate %float %75
%77 = OpLoad %v2float %34
%78 = OpCompositeExtract %float %77 0
%79 = OpFMul %float %float_2 %78
%80 = OpFAdd %float %76 %79
%81 = OpFAdd %float %80 %float_1
%82 = OpFMul %float %73 %81
%83 = OpFAdd %float %71 %82
OpReturnValue %83
%45 = OpLabel
%85 = OpLoad %v2float %35
%86 = OpCompositeExtract %float %85 0
%87 = OpFMul %float %float_4 %86
%88 = OpLoad %v2float %35
%89 = OpCompositeExtract %float %88 1
%90 = OpFOrdLessThanEqual %bool %87 %89
OpSelectionMerge %93 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%95 = OpLoad %v2float %35
%96 = OpCompositeExtract %float %95 0
%97 = OpLoad %v2float %35
%98 = OpCompositeExtract %float %97 0
%99 = OpFMul %float %96 %98
OpStore %DSqd %99
%101 = OpLoad %v2float %35
%102 = OpCompositeExtract %float %101 0
%103 = OpFMul %float %99 %102
OpStore %DCub %103
%105 = OpLoad %v2float %35
%106 = OpCompositeExtract %float %105 1
%107 = OpLoad %v2float %35
%108 = OpCompositeExtract %float %107 1
%109 = OpFMul %float %106 %108
OpStore %DaSqd %109
%111 = OpLoad %v2float %35
%112 = OpCompositeExtract %float %111 1
%113 = OpFMul %float %109 %112
OpStore %DaCub %113
%114 = OpLoad %v2float %34
%115 = OpCompositeExtract %float %114 0
%116 = OpLoad %v2float %35
%117 = OpCompositeExtract %float %116 0
%119 = OpLoad %v2float %34
%120 = OpCompositeExtract %float %119 1
%121 = OpFMul %float %float_3 %120
%123 = OpLoad %v2float %34
%124 = OpCompositeExtract %float %123 0
%125 = OpFMul %float %float_6 %124
%126 = OpFSub %float %121 %125
%127 = OpFSub %float %126 %float_1
%128 = OpFMul %float %117 %127
%129 = OpFSub %float %115 %128
%130 = OpFMul %float %109 %129
%132 = OpLoad %v2float %35
%133 = OpCompositeExtract %float %132 1
%134 = OpFMul %float %float_12 %133
%135 = OpFMul %float %134 %99
%136 = OpLoad %v2float %34
%137 = OpCompositeExtract %float %136 1
%138 = OpLoad %v2float %34
%139 = OpCompositeExtract %float %138 0
%140 = OpFMul %float %float_2 %139
%141 = OpFSub %float %137 %140
%142 = OpFMul %float %135 %141
%143 = OpFAdd %float %130 %142
%145 = OpFMul %float %float_16 %103
%146 = OpLoad %v2float %34
%147 = OpCompositeExtract %float %146 1
%148 = OpLoad %v2float %34
%149 = OpCompositeExtract %float %148 0
%150 = OpFMul %float %float_2 %149
%151 = OpFSub %float %147 %150
%152 = OpFMul %float %145 %151
%153 = OpFSub %float %143 %152
%154 = OpLoad %v2float %34
%155 = OpCompositeExtract %float %154 0
%156 = OpFMul %float %113 %155
%157 = OpFSub %float %153 %156
OpStore %158 %157
OpStore %159 %109
%160 = OpFunctionCall %float %guarded_divide_Qhhh %158 %159
OpReturnValue %160
%92 = OpLabel
%161 = OpLoad %v2float %35
%162 = OpCompositeExtract %float %161 0
%163 = OpLoad %v2float %34
%164 = OpCompositeExtract %float %163 1
%165 = OpLoad %v2float %34
%166 = OpCompositeExtract %float %165 0
%167 = OpFMul %float %float_2 %166
%168 = OpFSub %float %164 %167
%169 = OpFAdd %float %168 %float_1
%170 = OpFMul %float %162 %169
%171 = OpLoad %v2float %34
%172 = OpCompositeExtract %float %171 0
%173 = OpFAdd %float %170 %172
%175 = OpLoad %v2float %35
%176 = OpCompositeExtract %float %175 1
%177 = OpLoad %v2float %35
%178 = OpCompositeExtract %float %177 0
%179 = OpFMul %float %176 %178
%174 = OpExtInst %float %1 Sqrt %179
%180 = OpLoad %v2float %34
%181 = OpCompositeExtract %float %180 1
%182 = OpLoad %v2float %34
%183 = OpCompositeExtract %float %182 0
%184 = OpFMul %float %float_2 %183
%185 = OpFSub %float %181 %184
%186 = OpFMul %float %174 %185
%187 = OpFSub %float %173 %186
%188 = OpLoad %v2float %35
%189 = OpCompositeExtract %float %188 1
%190 = OpLoad %v2float %34
%191 = OpCompositeExtract %float %190 0
%192 = OpFMul %float %189 %191
%193 = OpFSub %float %187 %192
OpReturnValue %193
%93 = OpLabel
OpBranch %46
%46 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %195
%196 = OpLabel
%204 = OpVariable %_ptr_Function_v4float Function
%215 = OpVariable %_ptr_Function_v2float Function
%219 = OpVariable %_ptr_Function_v2float Function
%224 = OpVariable %_ptr_Function_v2float Function
%228 = OpVariable %_ptr_Function_v2float Function
%233 = OpVariable %_ptr_Function_v2float Function
%237 = OpVariable %_ptr_Function_v2float Function
%10 = OpSelect %float %false %float_9_99999994en09 %float_0
OpStore %_kGuardedDivideEpsilon %10
%197 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%201 = OpLoad %v4float %197
%202 = OpCompositeExtract %float %201 3
%203 = OpFOrdEqual %bool %202 %float_0
OpSelectionMerge %208 None
OpBranchConditional %203 %206 %207
%206 = OpLabel
%209 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%211 = OpLoad %v4float %209
OpStore %204 %211
OpBranch %208
%207 = OpLabel
%212 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%213 = OpLoad %v4float %212
%214 = OpVectorShuffle %v2float %213 %213 0 3
OpStore %215 %214
%216 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%217 = OpLoad %v4float %216
%218 = OpVectorShuffle %v2float %217 %217 0 3
OpStore %219 %218
%220 = OpFunctionCall %float %soft_light_component_Qhh2h2 %215 %219
%221 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%222 = OpLoad %v4float %221
%223 = OpVectorShuffle %v2float %222 %222 1 3
OpStore %224 %223
%225 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%226 = OpLoad %v4float %225
%227 = OpVectorShuffle %v2float %226 %226 1 3
OpStore %228 %227
%229 = OpFunctionCall %float %soft_light_component_Qhh2h2 %224 %228
%230 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%231 = OpLoad %v4float %230
%232 = OpVectorShuffle %v2float %231 %231 2 3
OpStore %233 %232
%234 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%235 = OpLoad %v4float %234
%236 = OpVectorShuffle %v2float %235 %235 2 3
OpStore %237 %236
%238 = OpFunctionCall %float %soft_light_component_Qhh2h2 %233 %237
%239 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%240 = OpLoad %v4float %239
%241 = OpCompositeExtract %float %240 3
%242 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%243 = OpLoad %v4float %242
%244 = OpCompositeExtract %float %243 3
%245 = OpFSub %float %float_1 %244
%246 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%247 = OpLoad %v4float %246
%248 = OpCompositeExtract %float %247 3
%249 = OpFMul %float %245 %248
%250 = OpFAdd %float %241 %249
%251 = OpCompositeConstruct %v4float %220 %229 %238 %250
OpStore %204 %251
OpBranch %208
%208 = OpLabel
%252 = OpLoad %v4float %204
OpStore %sk_FragColor %252
OpReturn
OpFunctionEnd
