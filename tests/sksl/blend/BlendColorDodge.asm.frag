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
               OpName %_0_dxScale "_0_dxScale"
               OpName %_1_delta "_1_delta"
               OpName %_2_dxScale "_2_dxScale"
               OpName %_3_delta "_3_delta"
               OpName %_4_dxScale "_4_dxScale"
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
               OpDecorate %_0_dxScale RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %_1_delta RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %_2_dxScale RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %_3_delta RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %_4_dxScale RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %_5_delta RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %241 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %256 RelaxedPrecision
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
 %_0_dxScale = OpVariable %_ptr_Function_float Function
   %_1_delta = OpVariable %_ptr_Function_float Function
         %57 = OpVariable %_ptr_Function_float Function
         %68 = OpVariable %_ptr_Function_float Function
         %76 = OpVariable %_ptr_Function_float Function
 %_2_dxScale = OpVariable %_ptr_Function_float Function
   %_3_delta = OpVariable %_ptr_Function_float Function
        %104 = OpVariable %_ptr_Function_float Function
        %115 = OpVariable %_ptr_Function_float Function
        %123 = OpVariable %_ptr_Function_float Function
 %_4_dxScale = OpVariable %_ptr_Function_float Function
   %_5_delta = OpVariable %_ptr_Function_float Function
        %151 = OpVariable %_ptr_Function_float Function
        %162 = OpVariable %_ptr_Function_float Function
        %170 = OpVariable %_ptr_Function_float Function
          %9 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %9
         %32 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %36 = OpLoad %v4float %32
         %37 = OpCompositeExtract %float %36 0
         %38 = OpFOrdEqual %bool %37 %float_0
         %39 = OpSelect %int %38 %int_0 %int_1
         %41 = OpConvertSToF %float %39
               OpStore %_0_dxScale %41
         %44 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %45 = OpLoad %v4float %44
         %46 = OpCompositeExtract %float %45 3
         %48 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %49 = OpLoad %v4float %48
         %50 = OpCompositeExtract %float %49 3
         %51 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %52 = OpLoad %v4float %51
         %53 = OpCompositeExtract %float %52 0
         %54 = OpFSub %float %50 %53
         %47 = OpExtInst %float %1 FAbs %54
         %56 = OpFOrdGreaterThanEqual %bool %47 %float_6_10351562en05
               OpSelectionMerge %60 None
               OpBranchConditional %56 %58 %59
         %58 = OpLabel
         %61 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %62 = OpLoad %v4float %61
         %63 = OpCompositeExtract %float %62 0
         %64 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %65 = OpLoad %v4float %64
         %66 = OpCompositeExtract %float %65 3
         %67 = OpFMul %float %63 %66
               OpStore %68 %67
         %69 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %70 = OpLoad %v4float %69
         %71 = OpCompositeExtract %float %70 3
         %72 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %73 = OpLoad %v4float %72
         %74 = OpCompositeExtract %float %73 0
         %75 = OpFSub %float %71 %74
               OpStore %76 %75
         %77 = OpFunctionCall %float %guarded_divide_Qhhh %68 %76
               OpStore %57 %77
               OpBranch %60
         %59 = OpLabel
         %78 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %79 = OpLoad %v4float %78
         %80 = OpCompositeExtract %float %79 3
               OpStore %57 %80
               OpBranch %60
         %60 = OpLabel
         %81 = OpLoad %float %57
         %43 = OpExtInst %float %1 FMin %46 %81
         %82 = OpFMul %float %41 %43
               OpStore %_1_delta %82
         %84 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %85 = OpLoad %v4float %84
         %86 = OpCompositeExtract %float %85 1
         %87 = OpFOrdEqual %bool %86 %float_0
         %88 = OpSelect %int %87 %int_0 %int_1
         %89 = OpConvertSToF %float %88
               OpStore %_2_dxScale %89
         %92 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %93 = OpLoad %v4float %92
         %94 = OpCompositeExtract %float %93 3
         %96 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %97 = OpLoad %v4float %96
         %98 = OpCompositeExtract %float %97 3
         %99 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %100 = OpLoad %v4float %99
        %101 = OpCompositeExtract %float %100 1
        %102 = OpFSub %float %98 %101
         %95 = OpExtInst %float %1 FAbs %102
        %103 = OpFOrdGreaterThanEqual %bool %95 %float_6_10351562en05
               OpSelectionMerge %107 None
               OpBranchConditional %103 %105 %106
        %105 = OpLabel
        %108 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %109 = OpLoad %v4float %108
        %110 = OpCompositeExtract %float %109 1
        %111 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %112 = OpLoad %v4float %111
        %113 = OpCompositeExtract %float %112 3
        %114 = OpFMul %float %110 %113
               OpStore %115 %114
        %116 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %117 = OpLoad %v4float %116
        %118 = OpCompositeExtract %float %117 3
        %119 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %120 = OpLoad %v4float %119
        %121 = OpCompositeExtract %float %120 1
        %122 = OpFSub %float %118 %121
               OpStore %123 %122
        %124 = OpFunctionCall %float %guarded_divide_Qhhh %115 %123
               OpStore %104 %124
               OpBranch %107
        %106 = OpLabel
        %125 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %126 = OpLoad %v4float %125
        %127 = OpCompositeExtract %float %126 3
               OpStore %104 %127
               OpBranch %107
        %107 = OpLabel
        %128 = OpLoad %float %104
         %91 = OpExtInst %float %1 FMin %94 %128
        %129 = OpFMul %float %89 %91
               OpStore %_3_delta %129
        %131 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %132 = OpLoad %v4float %131
        %133 = OpCompositeExtract %float %132 2
        %134 = OpFOrdEqual %bool %133 %float_0
        %135 = OpSelect %int %134 %int_0 %int_1
        %136 = OpConvertSToF %float %135
               OpStore %_4_dxScale %136
        %139 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %140 = OpLoad %v4float %139
        %141 = OpCompositeExtract %float %140 3
        %143 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %144 = OpLoad %v4float %143
        %145 = OpCompositeExtract %float %144 3
        %146 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %147 = OpLoad %v4float %146
        %148 = OpCompositeExtract %float %147 2
        %149 = OpFSub %float %145 %148
        %142 = OpExtInst %float %1 FAbs %149
        %150 = OpFOrdGreaterThanEqual %bool %142 %float_6_10351562en05
               OpSelectionMerge %154 None
               OpBranchConditional %150 %152 %153
        %152 = OpLabel
        %155 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %156 = OpLoad %v4float %155
        %157 = OpCompositeExtract %float %156 2
        %158 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %159 = OpLoad %v4float %158
        %160 = OpCompositeExtract %float %159 3
        %161 = OpFMul %float %157 %160
               OpStore %162 %161
        %163 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %164 = OpLoad %v4float %163
        %165 = OpCompositeExtract %float %164 3
        %166 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %167 = OpLoad %v4float %166
        %168 = OpCompositeExtract %float %167 2
        %169 = OpFSub %float %165 %168
               OpStore %170 %169
        %171 = OpFunctionCall %float %guarded_divide_Qhhh %162 %170
               OpStore %151 %171
               OpBranch %154
        %153 = OpLabel
        %172 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %173 = OpLoad %v4float %172
        %174 = OpCompositeExtract %float %173 3
               OpStore %151 %174
               OpBranch %154
        %154 = OpLabel
        %175 = OpLoad %float %151
        %138 = OpExtInst %float %1 FMin %141 %175
        %176 = OpFMul %float %136 %138
               OpStore %_5_delta %176
        %177 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %178 = OpLoad %v4float %177
        %179 = OpCompositeExtract %float %178 3
        %180 = OpFMul %float %82 %179
        %181 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %182 = OpLoad %v4float %181
        %183 = OpCompositeExtract %float %182 0
        %185 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %186 = OpLoad %v4float %185
        %187 = OpCompositeExtract %float %186 3
        %188 = OpFSub %float %float_1 %187
        %189 = OpFMul %float %183 %188
        %190 = OpFAdd %float %180 %189
        %191 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %192 = OpLoad %v4float %191
        %193 = OpCompositeExtract %float %192 0
        %194 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %195 = OpLoad %v4float %194
        %196 = OpCompositeExtract %float %195 3
        %197 = OpFSub %float %float_1 %196
        %198 = OpFMul %float %193 %197
        %199 = OpFAdd %float %190 %198
        %200 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %201 = OpLoad %v4float %200
        %202 = OpCompositeExtract %float %201 3
        %203 = OpFMul %float %129 %202
        %204 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %205 = OpLoad %v4float %204
        %206 = OpCompositeExtract %float %205 1
        %207 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %208 = OpLoad %v4float %207
        %209 = OpCompositeExtract %float %208 3
        %210 = OpFSub %float %float_1 %209
        %211 = OpFMul %float %206 %210
        %212 = OpFAdd %float %203 %211
        %213 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %214 = OpLoad %v4float %213
        %215 = OpCompositeExtract %float %214 1
        %216 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %217 = OpLoad %v4float %216
        %218 = OpCompositeExtract %float %217 3
        %219 = OpFSub %float %float_1 %218
        %220 = OpFMul %float %215 %219
        %221 = OpFAdd %float %212 %220
        %222 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %223 = OpLoad %v4float %222
        %224 = OpCompositeExtract %float %223 3
        %225 = OpFMul %float %176 %224
        %226 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %227 = OpLoad %v4float %226
        %228 = OpCompositeExtract %float %227 2
        %229 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %230 = OpLoad %v4float %229
        %231 = OpCompositeExtract %float %230 3
        %232 = OpFSub %float %float_1 %231
        %233 = OpFMul %float %228 %232
        %234 = OpFAdd %float %225 %233
        %235 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %236 = OpLoad %v4float %235
        %237 = OpCompositeExtract %float %236 2
        %238 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %239 = OpLoad %v4float %238
        %240 = OpCompositeExtract %float %239 3
        %241 = OpFSub %float %float_1 %240
        %242 = OpFMul %float %237 %241
        %243 = OpFAdd %float %234 %242
        %244 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %245 = OpLoad %v4float %244
        %246 = OpCompositeExtract %float %245 3
        %247 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %248 = OpLoad %v4float %247
        %249 = OpCompositeExtract %float %248 3
        %250 = OpFSub %float %float_1 %249
        %251 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %252 = OpLoad %v4float %251
        %253 = OpCompositeExtract %float %252 3
        %254 = OpFMul %float %250 %253
        %255 = OpFAdd %float %246 %254
        %256 = OpCompositeConstruct %v4float %199 %221 %243 %255
               OpStore %sk_FragColor %256
               OpReturn
               OpFunctionEnd
