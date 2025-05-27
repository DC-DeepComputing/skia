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
               OpDecorate %14 Binding 0
               OpDecorate %14 DescriptorSet 0
               OpDecorate %_0_dyTerm RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %_1_delta RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %_2_dyTerm RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %_3_delta RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
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
               OpDecorate %_4_dyTerm RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %_5_delta RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %241 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %244 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %247 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision
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
         %14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
    %float_1 = OpConstant %float 1
       %main = OpFunction %void None %18
         %19 = OpLabel
  %_0_dyTerm = OpVariable %_ptr_Function_float Function
         %32 = OpVariable %_ptr_Function_float Function
   %_1_delta = OpVariable %_ptr_Function_float Function
         %48 = OpVariable %_ptr_Function_float Function
  %_2_dyTerm = OpVariable %_ptr_Function_float Function
         %86 = OpVariable %_ptr_Function_float Function
   %_3_delta = OpVariable %_ptr_Function_float Function
        %100 = OpVariable %_ptr_Function_float Function
  %_4_dyTerm = OpVariable %_ptr_Function_float Function
        %138 = OpVariable %_ptr_Function_float Function
   %_5_delta = OpVariable %_ptr_Function_float Function
        %152 = OpVariable %_ptr_Function_float Function
          %8 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %8
         %22 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %26 = OpLoad %v4float %22
         %27 = OpCompositeExtract %float %26 3
         %28 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %29 = OpLoad %v4float %28
         %30 = OpCompositeExtract %float %29 0
         %31 = OpFOrdEqual %bool %27 %30
               OpSelectionMerge %35 None
               OpBranchConditional %31 %33 %34
         %33 = OpLabel
         %36 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %37 = OpLoad %v4float %36
         %38 = OpCompositeExtract %float %37 3
               OpStore %32 %38
               OpBranch %35
         %34 = OpLabel
               OpStore %32 %float_0
               OpBranch %35
         %35 = OpLabel
         %39 = OpLoad %float %32
               OpStore %_0_dyTerm %39
         %42 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %44 = OpLoad %v4float %42
         %45 = OpCompositeExtract %float %44 0
         %41 = OpExtInst %float %1 FAbs %45
         %47 = OpFOrdGreaterThanEqual %bool %41 %float_6_10351562en05
               OpSelectionMerge %51 None
               OpBranchConditional %47 %49 %50
         %49 = OpLabel
         %52 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %53 = OpLoad %v4float %52
         %54 = OpCompositeExtract %float %53 3
         %56 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %57 = OpLoad %v4float %56
         %58 = OpCompositeExtract %float %57 3
         %59 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %60 = OpLoad %v4float %59
         %61 = OpCompositeExtract %float %60 3
         %62 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %63 = OpLoad %v4float %62
         %64 = OpCompositeExtract %float %63 0
         %65 = OpFSub %float %61 %64
         %66 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %67 = OpLoad %v4float %66
         %68 = OpCompositeExtract %float %67 3
         %69 = OpFMul %float %65 %68
         %70 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %71 = OpLoad %v4float %70
         %72 = OpCompositeExtract %float %71 0
         %73 = OpLoad %float %_kGuardedDivideEpsilon
         %74 = OpFAdd %float %72 %73
         %75 = OpFDiv %float %69 %74
         %55 = OpExtInst %float %1 FMin %58 %75
         %76 = OpFSub %float %54 %55
               OpStore %48 %76
               OpBranch %51
         %50 = OpLabel
               OpStore %48 %39
               OpBranch %51
         %51 = OpLabel
         %77 = OpLoad %float %48
               OpStore %_1_delta %77
         %79 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %80 = OpLoad %v4float %79
         %81 = OpCompositeExtract %float %80 3
         %82 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %83 = OpLoad %v4float %82
         %84 = OpCompositeExtract %float %83 1
         %85 = OpFOrdEqual %bool %81 %84
               OpSelectionMerge %89 None
               OpBranchConditional %85 %87 %88
         %87 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %91 = OpLoad %v4float %90
         %92 = OpCompositeExtract %float %91 3
               OpStore %86 %92
               OpBranch %89
         %88 = OpLabel
               OpStore %86 %float_0
               OpBranch %89
         %89 = OpLabel
         %93 = OpLoad %float %86
               OpStore %_2_dyTerm %93
         %96 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %97 = OpLoad %v4float %96
         %98 = OpCompositeExtract %float %97 1
         %95 = OpExtInst %float %1 FAbs %98
         %99 = OpFOrdGreaterThanEqual %bool %95 %float_6_10351562en05
               OpSelectionMerge %103 None
               OpBranchConditional %99 %101 %102
        %101 = OpLabel
        %104 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %105 = OpLoad %v4float %104
        %106 = OpCompositeExtract %float %105 3
        %108 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %109 = OpLoad %v4float %108
        %110 = OpCompositeExtract %float %109 3
        %111 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %112 = OpLoad %v4float %111
        %113 = OpCompositeExtract %float %112 3
        %114 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %115 = OpLoad %v4float %114
        %116 = OpCompositeExtract %float %115 1
        %117 = OpFSub %float %113 %116
        %118 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %119 = OpLoad %v4float %118
        %120 = OpCompositeExtract %float %119 3
        %121 = OpFMul %float %117 %120
        %122 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %123 = OpLoad %v4float %122
        %124 = OpCompositeExtract %float %123 1
        %125 = OpLoad %float %_kGuardedDivideEpsilon
        %126 = OpFAdd %float %124 %125
        %127 = OpFDiv %float %121 %126
        %107 = OpExtInst %float %1 FMin %110 %127
        %128 = OpFSub %float %106 %107
               OpStore %100 %128
               OpBranch %103
        %102 = OpLabel
               OpStore %100 %93
               OpBranch %103
        %103 = OpLabel
        %129 = OpLoad %float %100
               OpStore %_3_delta %129
        %131 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %132 = OpLoad %v4float %131
        %133 = OpCompositeExtract %float %132 3
        %134 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %135 = OpLoad %v4float %134
        %136 = OpCompositeExtract %float %135 2
        %137 = OpFOrdEqual %bool %133 %136
               OpSelectionMerge %141 None
               OpBranchConditional %137 %139 %140
        %139 = OpLabel
        %142 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %143 = OpLoad %v4float %142
        %144 = OpCompositeExtract %float %143 3
               OpStore %138 %144
               OpBranch %141
        %140 = OpLabel
               OpStore %138 %float_0
               OpBranch %141
        %141 = OpLabel
        %145 = OpLoad %float %138
               OpStore %_4_dyTerm %145
        %148 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %149 = OpLoad %v4float %148
        %150 = OpCompositeExtract %float %149 2
        %147 = OpExtInst %float %1 FAbs %150
        %151 = OpFOrdGreaterThanEqual %bool %147 %float_6_10351562en05
               OpSelectionMerge %155 None
               OpBranchConditional %151 %153 %154
        %153 = OpLabel
        %156 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %157 = OpLoad %v4float %156
        %158 = OpCompositeExtract %float %157 3
        %160 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %161 = OpLoad %v4float %160
        %162 = OpCompositeExtract %float %161 3
        %163 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %164 = OpLoad %v4float %163
        %165 = OpCompositeExtract %float %164 3
        %166 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %167 = OpLoad %v4float %166
        %168 = OpCompositeExtract %float %167 2
        %169 = OpFSub %float %165 %168
        %170 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %171 = OpLoad %v4float %170
        %172 = OpCompositeExtract %float %171 3
        %173 = OpFMul %float %169 %172
        %174 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %175 = OpLoad %v4float %174
        %176 = OpCompositeExtract %float %175 2
        %177 = OpLoad %float %_kGuardedDivideEpsilon
        %178 = OpFAdd %float %176 %177
        %179 = OpFDiv %float %173 %178
        %159 = OpExtInst %float %1 FMin %162 %179
        %180 = OpFSub %float %158 %159
               OpStore %152 %180
               OpBranch %155
        %154 = OpLabel
               OpStore %152 %145
               OpBranch %155
        %155 = OpLabel
        %181 = OpLoad %float %152
               OpStore %_5_delta %181
        %182 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %183 = OpLoad %v4float %182
        %184 = OpCompositeExtract %float %183 3
        %185 = OpFMul %float %77 %184
        %186 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %187 = OpLoad %v4float %186
        %188 = OpCompositeExtract %float %187 0
        %190 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %191 = OpLoad %v4float %190
        %192 = OpCompositeExtract %float %191 3
        %193 = OpFSub %float %float_1 %192
        %194 = OpFMul %float %188 %193
        %195 = OpFAdd %float %185 %194
        %196 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %197 = OpLoad %v4float %196
        %198 = OpCompositeExtract %float %197 0
        %199 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %200 = OpLoad %v4float %199
        %201 = OpCompositeExtract %float %200 3
        %202 = OpFSub %float %float_1 %201
        %203 = OpFMul %float %198 %202
        %204 = OpFAdd %float %195 %203
        %205 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %206 = OpLoad %v4float %205
        %207 = OpCompositeExtract %float %206 3
        %208 = OpFMul %float %129 %207
        %209 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %210 = OpLoad %v4float %209
        %211 = OpCompositeExtract %float %210 1
        %212 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %213 = OpLoad %v4float %212
        %214 = OpCompositeExtract %float %213 3
        %215 = OpFSub %float %float_1 %214
        %216 = OpFMul %float %211 %215
        %217 = OpFAdd %float %208 %216
        %218 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %219 = OpLoad %v4float %218
        %220 = OpCompositeExtract %float %219 1
        %221 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %222 = OpLoad %v4float %221
        %223 = OpCompositeExtract %float %222 3
        %224 = OpFSub %float %float_1 %223
        %225 = OpFMul %float %220 %224
        %226 = OpFAdd %float %217 %225
        %227 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %228 = OpLoad %v4float %227
        %229 = OpCompositeExtract %float %228 3
        %230 = OpFMul %float %181 %229
        %231 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %232 = OpLoad %v4float %231
        %233 = OpCompositeExtract %float %232 2
        %234 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %235 = OpLoad %v4float %234
        %236 = OpCompositeExtract %float %235 3
        %237 = OpFSub %float %float_1 %236
        %238 = OpFMul %float %233 %237
        %239 = OpFAdd %float %230 %238
        %240 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %241 = OpLoad %v4float %240
        %242 = OpCompositeExtract %float %241 2
        %243 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %244 = OpLoad %v4float %243
        %245 = OpCompositeExtract %float %244 3
        %246 = OpFSub %float %float_1 %245
        %247 = OpFMul %float %242 %246
        %248 = OpFAdd %float %239 %247
        %249 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %250 = OpLoad %v4float %249
        %251 = OpCompositeExtract %float %250 3
        %252 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %253 = OpLoad %v4float %252
        %254 = OpCompositeExtract %float %253 3
        %255 = OpFSub %float %float_1 %254
        %256 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %257 = OpLoad %v4float %256
        %258 = OpCompositeExtract %float %257 3
        %259 = OpFMul %float %255 %258
        %260 = OpFAdd %float %251 %259
        %261 = OpCompositeConstruct %v4float %204 %226 %248 %260
               OpStore %sk_FragColor %261
               OpReturn
               OpFunctionEnd
