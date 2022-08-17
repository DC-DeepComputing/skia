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
OpName %blend_soft_light_h4h4h4 "blend_soft_light_h4h4h4"
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
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
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
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %DSqd RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %DCub RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %DaSqd RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %DaCub RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
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
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
%17 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%_ptr_Function_v4float = OpTypePointer Function %v4float
%188 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_0 = OpConstant %float 0
%void = OpTypeVoid
%234 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%guarded_divide_Qhhh = OpFunction %float None %17
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%21 = OpLoad %float %18
%22 = OpLoad %float %19
%23 = OpFDiv %float %21 %22
OpReturnValue %23
OpFunctionEnd
%soft_light_component_Qhh2h2 = OpFunction %float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpFunctionParameter %_ptr_Function_v2float
%29 = OpLabel
%52 = OpVariable %_ptr_Function_float Function
%55 = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%151 = OpVariable %_ptr_Function_float Function
%152 = OpVariable %_ptr_Function_float Function
%31 = OpLoad %v2float %27
%32 = OpCompositeExtract %float %31 0
%33 = OpFMul %float %float_2 %32
%34 = OpLoad %v2float %27
%35 = OpCompositeExtract %float %34 1
%36 = OpFOrdLessThanEqual %bool %33 %35
OpSelectionMerge %39 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
%40 = OpLoad %v2float %28
%41 = OpCompositeExtract %float %40 0
%42 = OpLoad %v2float %28
%43 = OpCompositeExtract %float %42 0
%44 = OpFMul %float %41 %43
%45 = OpLoad %v2float %27
%46 = OpCompositeExtract %float %45 1
%47 = OpLoad %v2float %27
%48 = OpCompositeExtract %float %47 0
%49 = OpFMul %float %float_2 %48
%50 = OpFSub %float %46 %49
%51 = OpFMul %float %44 %50
OpStore %52 %51
%53 = OpLoad %v2float %28
%54 = OpCompositeExtract %float %53 1
OpStore %55 %54
%56 = OpFunctionCall %float %guarded_divide_Qhhh %52 %55
%58 = OpLoad %v2float %28
%59 = OpCompositeExtract %float %58 1
%60 = OpFSub %float %float_1 %59
%61 = OpLoad %v2float %27
%62 = OpCompositeExtract %float %61 0
%63 = OpFMul %float %60 %62
%64 = OpFAdd %float %56 %63
%65 = OpLoad %v2float %28
%66 = OpCompositeExtract %float %65 0
%67 = OpLoad %v2float %27
%68 = OpCompositeExtract %float %67 1
%69 = OpFNegate %float %68
%70 = OpLoad %v2float %27
%71 = OpCompositeExtract %float %70 0
%72 = OpFMul %float %float_2 %71
%73 = OpFAdd %float %69 %72
%74 = OpFAdd %float %73 %float_1
%75 = OpFMul %float %66 %74
%76 = OpFAdd %float %64 %75
OpReturnValue %76
%38 = OpLabel
%78 = OpLoad %v2float %28
%79 = OpCompositeExtract %float %78 0
%80 = OpFMul %float %float_4 %79
%81 = OpLoad %v2float %28
%82 = OpCompositeExtract %float %81 1
%83 = OpFOrdLessThanEqual %bool %80 %82
OpSelectionMerge %86 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%88 = OpLoad %v2float %28
%89 = OpCompositeExtract %float %88 0
%90 = OpLoad %v2float %28
%91 = OpCompositeExtract %float %90 0
%92 = OpFMul %float %89 %91
OpStore %DSqd %92
%94 = OpLoad %v2float %28
%95 = OpCompositeExtract %float %94 0
%96 = OpFMul %float %92 %95
OpStore %DCub %96
%98 = OpLoad %v2float %28
%99 = OpCompositeExtract %float %98 1
%100 = OpLoad %v2float %28
%101 = OpCompositeExtract %float %100 1
%102 = OpFMul %float %99 %101
OpStore %DaSqd %102
%104 = OpLoad %v2float %28
%105 = OpCompositeExtract %float %104 1
%106 = OpFMul %float %102 %105
OpStore %DaCub %106
%107 = OpLoad %v2float %27
%108 = OpCompositeExtract %float %107 0
%109 = OpLoad %v2float %28
%110 = OpCompositeExtract %float %109 0
%112 = OpLoad %v2float %27
%113 = OpCompositeExtract %float %112 1
%114 = OpFMul %float %float_3 %113
%116 = OpLoad %v2float %27
%117 = OpCompositeExtract %float %116 0
%118 = OpFMul %float %float_6 %117
%119 = OpFSub %float %114 %118
%120 = OpFSub %float %119 %float_1
%121 = OpFMul %float %110 %120
%122 = OpFSub %float %108 %121
%123 = OpFMul %float %102 %122
%125 = OpLoad %v2float %28
%126 = OpCompositeExtract %float %125 1
%127 = OpFMul %float %float_12 %126
%128 = OpFMul %float %127 %92
%129 = OpLoad %v2float %27
%130 = OpCompositeExtract %float %129 1
%131 = OpLoad %v2float %27
%132 = OpCompositeExtract %float %131 0
%133 = OpFMul %float %float_2 %132
%134 = OpFSub %float %130 %133
%135 = OpFMul %float %128 %134
%136 = OpFAdd %float %123 %135
%138 = OpFMul %float %float_16 %96
%139 = OpLoad %v2float %27
%140 = OpCompositeExtract %float %139 1
%141 = OpLoad %v2float %27
%142 = OpCompositeExtract %float %141 0
%143 = OpFMul %float %float_2 %142
%144 = OpFSub %float %140 %143
%145 = OpFMul %float %138 %144
%146 = OpFSub %float %136 %145
%147 = OpLoad %v2float %27
%148 = OpCompositeExtract %float %147 0
%149 = OpFMul %float %106 %148
%150 = OpFSub %float %146 %149
OpStore %151 %150
OpStore %152 %102
%153 = OpFunctionCall %float %guarded_divide_Qhhh %151 %152
OpReturnValue %153
%85 = OpLabel
%154 = OpLoad %v2float %28
%155 = OpCompositeExtract %float %154 0
%156 = OpLoad %v2float %27
%157 = OpCompositeExtract %float %156 1
%158 = OpLoad %v2float %27
%159 = OpCompositeExtract %float %158 0
%160 = OpFMul %float %float_2 %159
%161 = OpFSub %float %157 %160
%162 = OpFAdd %float %161 %float_1
%163 = OpFMul %float %155 %162
%164 = OpLoad %v2float %27
%165 = OpCompositeExtract %float %164 0
%166 = OpFAdd %float %163 %165
%168 = OpLoad %v2float %28
%169 = OpCompositeExtract %float %168 1
%170 = OpLoad %v2float %28
%171 = OpCompositeExtract %float %170 0
%172 = OpFMul %float %169 %171
%167 = OpExtInst %float %1 Sqrt %172
%173 = OpLoad %v2float %27
%174 = OpCompositeExtract %float %173 1
%175 = OpLoad %v2float %27
%176 = OpCompositeExtract %float %175 0
%177 = OpFMul %float %float_2 %176
%178 = OpFSub %float %174 %177
%179 = OpFMul %float %167 %178
%180 = OpFSub %float %166 %179
%181 = OpLoad %v2float %28
%182 = OpCompositeExtract %float %181 1
%183 = OpLoad %v2float %27
%184 = OpCompositeExtract %float %183 0
%185 = OpFMul %float %182 %184
%186 = OpFSub %float %180 %185
OpReturnValue %186
%86 = OpLabel
OpBranch %39
%39 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_soft_light_h4h4h4 = OpFunction %v4float None %188
%189 = OpFunctionParameter %_ptr_Function_v4float
%190 = OpFunctionParameter %_ptr_Function_v4float
%191 = OpLabel
%196 = OpVariable %_ptr_Function_v4float Function
%203 = OpVariable %_ptr_Function_v2float Function
%206 = OpVariable %_ptr_Function_v2float Function
%210 = OpVariable %_ptr_Function_v2float Function
%213 = OpVariable %_ptr_Function_v2float Function
%217 = OpVariable %_ptr_Function_v2float Function
%220 = OpVariable %_ptr_Function_v2float Function
%192 = OpLoad %v4float %190
%193 = OpCompositeExtract %float %192 3
%195 = OpFOrdEqual %bool %193 %float_0
OpSelectionMerge %199 None
OpBranchConditional %195 %197 %198
%197 = OpLabel
%200 = OpLoad %v4float %189
OpStore %196 %200
OpBranch %199
%198 = OpLabel
%201 = OpLoad %v4float %189
%202 = OpVectorShuffle %v2float %201 %201 0 3
OpStore %203 %202
%204 = OpLoad %v4float %190
%205 = OpVectorShuffle %v2float %204 %204 0 3
OpStore %206 %205
%207 = OpFunctionCall %float %soft_light_component_Qhh2h2 %203 %206
%208 = OpLoad %v4float %189
%209 = OpVectorShuffle %v2float %208 %208 1 3
OpStore %210 %209
%211 = OpLoad %v4float %190
%212 = OpVectorShuffle %v2float %211 %211 1 3
OpStore %213 %212
%214 = OpFunctionCall %float %soft_light_component_Qhh2h2 %210 %213
%215 = OpLoad %v4float %189
%216 = OpVectorShuffle %v2float %215 %215 2 3
OpStore %217 %216
%218 = OpLoad %v4float %190
%219 = OpVectorShuffle %v2float %218 %218 2 3
OpStore %220 %219
%221 = OpFunctionCall %float %soft_light_component_Qhh2h2 %217 %220
%222 = OpLoad %v4float %189
%223 = OpCompositeExtract %float %222 3
%224 = OpLoad %v4float %189
%225 = OpCompositeExtract %float %224 3
%226 = OpFSub %float %float_1 %225
%227 = OpLoad %v4float %190
%228 = OpCompositeExtract %float %227 3
%229 = OpFMul %float %226 %228
%230 = OpFAdd %float %223 %229
%231 = OpCompositeConstruct %v4float %207 %214 %221 %230
OpStore %196 %231
OpBranch %199
%199 = OpLabel
%232 = OpLoad %v4float %196
OpReturnValue %232
OpFunctionEnd
%main = OpFunction %void None %234
%235 = OpLabel
%241 = OpVariable %_ptr_Function_v4float Function
%245 = OpVariable %_ptr_Function_v4float Function
%236 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%240 = OpLoad %v4float %236
OpStore %241 %240
%242 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%244 = OpLoad %v4float %242
OpStore %245 %244
%246 = OpFunctionCall %v4float %blend_soft_light_h4h4h4 %241 %245
OpStore %sk_FragColor %246
OpReturn
OpFunctionEnd
