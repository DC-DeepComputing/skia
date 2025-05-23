               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
               OpName %main "main"
               OpName %_0_dyTerm "_0_dyTerm"
               OpName %_1_delta "_1_delta"
               OpName %_2_dyTerm "_2_dyTerm"
               OpName %_3_delta "_3_delta"
               OpName %_4_dyTerm "_4_dyTerm"
               OpName %_5_delta "_5_delta"
               OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %_0_dyTerm RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %_1_delta RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %_2_dyTerm RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %_3_delta RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %_4_dyTerm RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %_5_delta RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %247 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %256 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision
               OpDecorate %263 RelaxedPrecision
               OpDecorate %264 RelaxedPrecision
               OpDecorate %265 RelaxedPrecision
               OpDecorate %267 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
      %float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
         %19 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
       %void = OpTypeVoid
         %29 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
    %float_1 = OpConstant %float 1
%guarded_divide_Qhhh = OpFunction %float None %19
         %20 = OpFunctionParameter %_ptr_Function_float
         %21 = OpFunctionParameter %_ptr_Function_float
         %22 = OpLabel
         %23 = OpLoad %float %20
         %24 = OpLoad %float %21
         %25 = OpLoad %float %_kGuardedDivideEpsilon
         %26 = OpFAdd %float %24 %25
         %27 = OpFDiv %float %23 %26
               OpReturnValue %27
               OpFunctionEnd
       %main = OpFunction %void None %29
         %30 = OpLabel
  %_0_dyTerm = OpVariable %_ptr_Function_float Function
         %42 = OpVariable %_ptr_Function_float Function
   %_1_delta = OpVariable %_ptr_Function_float Function
         %58 = OpVariable %_ptr_Function_float Function
         %80 = OpVariable %_ptr_Function_float Function
         %84 = OpVariable %_ptr_Function_float Function
  %_2_dyTerm = OpVariable %_ptr_Function_float Function
         %96 = OpVariable %_ptr_Function_float Function
   %_3_delta = OpVariable %_ptr_Function_float Function
        %110 = OpVariable %_ptr_Function_float Function
        %132 = OpVariable %_ptr_Function_float Function
        %136 = OpVariable %_ptr_Function_float Function
  %_4_dyTerm = OpVariable %_ptr_Function_float Function
        %148 = OpVariable %_ptr_Function_float Function
   %_5_delta = OpVariable %_ptr_Function_float Function
        %162 = OpVariable %_ptr_Function_float Function
        %184 = OpVariable %_ptr_Function_float Function
        %188 = OpVariable %_ptr_Function_float Function
          %9 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %9
         %32 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %36 = OpLoad %v4float %32
         %37 = OpCompositeExtract %float %36 3
         %38 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %39 = OpLoad %v4float %38
         %40 = OpCompositeExtract %float %39 0
         %41 = OpFOrdEqual %bool %37 %40
               OpSelectionMerge %45 None
               OpBranchConditional %41 %43 %44
         %43 = OpLabel
         %46 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %47 = OpLoad %v4float %46
         %48 = OpCompositeExtract %float %47 3
               OpStore %42 %48
               OpBranch %45
         %44 = OpLabel
               OpStore %42 %float_0
               OpBranch %45
         %45 = OpLabel
         %49 = OpLoad %float %42
               OpStore %_0_dyTerm %49
         %52 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %54 = OpLoad %v4float %52
         %55 = OpCompositeExtract %float %54 0
         %51 = OpExtInst %float %1 FAbs %55
         %57 = OpFOrdGreaterThanEqual %bool %51 %float_6_10351562en05
               OpSelectionMerge %61 None
               OpBranchConditional %57 %59 %60
         %59 = OpLabel
         %62 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %63 = OpLoad %v4float %62
         %64 = OpCompositeExtract %float %63 3
         %66 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %67 = OpLoad %v4float %66
         %68 = OpCompositeExtract %float %67 3
         %69 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %70 = OpLoad %v4float %69
         %71 = OpCompositeExtract %float %70 3
         %72 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %73 = OpLoad %v4float %72
         %74 = OpCompositeExtract %float %73 0
         %75 = OpFSub %float %71 %74
         %76 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %77 = OpLoad %v4float %76
         %78 = OpCompositeExtract %float %77 3
         %79 = OpFMul %float %75 %78
               OpStore %80 %79
         %81 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %82 = OpLoad %v4float %81
         %83 = OpCompositeExtract %float %82 0
               OpStore %84 %83
         %85 = OpFunctionCall %float %guarded_divide_Qhhh %80 %84
         %65 = OpExtInst %float %1 FMin %68 %85
         %86 = OpFSub %float %64 %65
               OpStore %58 %86
               OpBranch %61
         %60 = OpLabel
               OpStore %58 %49
               OpBranch %61
         %61 = OpLabel
         %87 = OpLoad %float %58
               OpStore %_1_delta %87
         %89 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %90 = OpLoad %v4float %89
         %91 = OpCompositeExtract %float %90 3
         %92 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %93 = OpLoad %v4float %92
         %94 = OpCompositeExtract %float %93 1
         %95 = OpFOrdEqual %bool %91 %94
               OpSelectionMerge %99 None
               OpBranchConditional %95 %97 %98
         %97 = OpLabel
        %100 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %101 = OpLoad %v4float %100
        %102 = OpCompositeExtract %float %101 3
               OpStore %96 %102
               OpBranch %99
         %98 = OpLabel
               OpStore %96 %float_0
               OpBranch %99
         %99 = OpLabel
        %103 = OpLoad %float %96
               OpStore %_2_dyTerm %103
        %106 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %107 = OpLoad %v4float %106
        %108 = OpCompositeExtract %float %107 1
        %105 = OpExtInst %float %1 FAbs %108
        %109 = OpFOrdGreaterThanEqual %bool %105 %float_6_10351562en05
               OpSelectionMerge %113 None
               OpBranchConditional %109 %111 %112
        %111 = OpLabel
        %114 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %115 = OpLoad %v4float %114
        %116 = OpCompositeExtract %float %115 3
        %118 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %119 = OpLoad %v4float %118
        %120 = OpCompositeExtract %float %119 3
        %121 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %122 = OpLoad %v4float %121
        %123 = OpCompositeExtract %float %122 3
        %124 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %125 = OpLoad %v4float %124
        %126 = OpCompositeExtract %float %125 1
        %127 = OpFSub %float %123 %126
        %128 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %129 = OpLoad %v4float %128
        %130 = OpCompositeExtract %float %129 3
        %131 = OpFMul %float %127 %130
               OpStore %132 %131
        %133 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %134 = OpLoad %v4float %133
        %135 = OpCompositeExtract %float %134 1
               OpStore %136 %135
        %137 = OpFunctionCall %float %guarded_divide_Qhhh %132 %136
        %117 = OpExtInst %float %1 FMin %120 %137
        %138 = OpFSub %float %116 %117
               OpStore %110 %138
               OpBranch %113
        %112 = OpLabel
               OpStore %110 %103
               OpBranch %113
        %113 = OpLabel
        %139 = OpLoad %float %110
               OpStore %_3_delta %139
        %141 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %142 = OpLoad %v4float %141
        %143 = OpCompositeExtract %float %142 3
        %144 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %145 = OpLoad %v4float %144
        %146 = OpCompositeExtract %float %145 2
        %147 = OpFOrdEqual %bool %143 %146
               OpSelectionMerge %151 None
               OpBranchConditional %147 %149 %150
        %149 = OpLabel
        %152 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %153 = OpLoad %v4float %152
        %154 = OpCompositeExtract %float %153 3
               OpStore %148 %154
               OpBranch %151
        %150 = OpLabel
               OpStore %148 %float_0
               OpBranch %151
        %151 = OpLabel
        %155 = OpLoad %float %148
               OpStore %_4_dyTerm %155
        %158 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %159 = OpLoad %v4float %158
        %160 = OpCompositeExtract %float %159 2
        %157 = OpExtInst %float %1 FAbs %160
        %161 = OpFOrdGreaterThanEqual %bool %157 %float_6_10351562en05
               OpSelectionMerge %165 None
               OpBranchConditional %161 %163 %164
        %163 = OpLabel
        %166 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %167 = OpLoad %v4float %166
        %168 = OpCompositeExtract %float %167 3
        %170 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %171 = OpLoad %v4float %170
        %172 = OpCompositeExtract %float %171 3
        %173 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %174 = OpLoad %v4float %173
        %175 = OpCompositeExtract %float %174 3
        %176 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %177 = OpLoad %v4float %176
        %178 = OpCompositeExtract %float %177 2
        %179 = OpFSub %float %175 %178
        %180 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %181 = OpLoad %v4float %180
        %182 = OpCompositeExtract %float %181 3
        %183 = OpFMul %float %179 %182
               OpStore %184 %183
        %185 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %186 = OpLoad %v4float %185
        %187 = OpCompositeExtract %float %186 2
               OpStore %188 %187
        %189 = OpFunctionCall %float %guarded_divide_Qhhh %184 %188
        %169 = OpExtInst %float %1 FMin %172 %189
        %190 = OpFSub %float %168 %169
               OpStore %162 %190
               OpBranch %165
        %164 = OpLabel
               OpStore %162 %155
               OpBranch %165
        %165 = OpLabel
        %191 = OpLoad %float %162
               OpStore %_5_delta %191
        %192 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %193 = OpLoad %v4float %192
        %194 = OpCompositeExtract %float %193 3
        %195 = OpFMul %float %87 %194
        %196 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %197 = OpLoad %v4float %196
        %198 = OpCompositeExtract %float %197 0
        %200 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %201 = OpLoad %v4float %200
        %202 = OpCompositeExtract %float %201 3
        %203 = OpFSub %float %float_1 %202
        %204 = OpFMul %float %198 %203
        %205 = OpFAdd %float %195 %204
        %206 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %207 = OpLoad %v4float %206
        %208 = OpCompositeExtract %float %207 0
        %209 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %210 = OpLoad %v4float %209
        %211 = OpCompositeExtract %float %210 3
        %212 = OpFSub %float %float_1 %211
        %213 = OpFMul %float %208 %212
        %214 = OpFAdd %float %205 %213
        %215 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %216 = OpLoad %v4float %215
        %217 = OpCompositeExtract %float %216 3
        %218 = OpFMul %float %139 %217
        %219 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %220 = OpLoad %v4float %219
        %221 = OpCompositeExtract %float %220 1
        %222 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %223 = OpLoad %v4float %222
        %224 = OpCompositeExtract %float %223 3
        %225 = OpFSub %float %float_1 %224
        %226 = OpFMul %float %221 %225
        %227 = OpFAdd %float %218 %226
        %228 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %229 = OpLoad %v4float %228
        %230 = OpCompositeExtract %float %229 1
        %231 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %232 = OpLoad %v4float %231
        %233 = OpCompositeExtract %float %232 3
        %234 = OpFSub %float %float_1 %233
        %235 = OpFMul %float %230 %234
        %236 = OpFAdd %float %227 %235
        %237 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %238 = OpLoad %v4float %237
        %239 = OpCompositeExtract %float %238 3
        %240 = OpFMul %float %191 %239
        %241 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %242 = OpLoad %v4float %241
        %243 = OpCompositeExtract %float %242 2
        %244 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %245 = OpLoad %v4float %244
        %246 = OpCompositeExtract %float %245 3
        %247 = OpFSub %float %float_1 %246
        %248 = OpFMul %float %243 %247
        %249 = OpFAdd %float %240 %248
        %250 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %251 = OpLoad %v4float %250
        %252 = OpCompositeExtract %float %251 2
        %253 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %254 = OpLoad %v4float %253
        %255 = OpCompositeExtract %float %254 3
        %256 = OpFSub %float %float_1 %255
        %257 = OpFMul %float %252 %256
        %258 = OpFAdd %float %249 %257
        %259 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %260 = OpLoad %v4float %259
        %261 = OpCompositeExtract %float %260 3
        %262 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %263 = OpLoad %v4float %262
        %264 = OpCompositeExtract %float %263 3
        %265 = OpFSub %float %float_1 %264
        %266 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %267 = OpLoad %v4float %266
        %268 = OpCompositeExtract %float %267 3
        %269 = OpFMul %float %265 %268
        %270 = OpFAdd %float %261 %269
        %271 = OpCompositeConstruct %v4float %214 %236 %258 %270
               OpStore %sk_FragColor %271
               OpReturn
               OpFunctionEnd
