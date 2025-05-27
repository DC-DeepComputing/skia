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
               OpDecorate %14 Binding 0
               OpDecorate %14 DescriptorSet 0
               OpDecorate %_0_dxScale RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %_1_delta RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %_2_dxScale RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %_3_delta RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %_4_dxScale RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %_5_delta RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %244 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
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
 %_0_dxScale = OpVariable %_ptr_Function_float Function
   %_1_delta = OpVariable %_ptr_Function_float Function
         %47 = OpVariable %_ptr_Function_float Function
 %_2_dxScale = OpVariable %_ptr_Function_float Function
   %_3_delta = OpVariable %_ptr_Function_float Function
         %94 = OpVariable %_ptr_Function_float Function
 %_4_dxScale = OpVariable %_ptr_Function_float Function
   %_5_delta = OpVariable %_ptr_Function_float Function
        %141 = OpVariable %_ptr_Function_float Function
          %8 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %8
         %22 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %26 = OpLoad %v4float %22
         %27 = OpCompositeExtract %float %26 0
         %28 = OpFOrdEqual %bool %27 %float_0
         %29 = OpSelect %int %28 %int_0 %int_1
         %31 = OpConvertSToF %float %29
               OpStore %_0_dxScale %31
         %34 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %35 = OpLoad %v4float %34
         %36 = OpCompositeExtract %float %35 3
         %38 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %39 = OpLoad %v4float %38
         %40 = OpCompositeExtract %float %39 3
         %41 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %42 = OpLoad %v4float %41
         %43 = OpCompositeExtract %float %42 0
         %44 = OpFSub %float %40 %43
         %37 = OpExtInst %float %1 FAbs %44
         %46 = OpFOrdGreaterThanEqual %bool %37 %float_6_10351562en05
               OpSelectionMerge %50 None
               OpBranchConditional %46 %48 %49
         %48 = OpLabel
         %51 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %52 = OpLoad %v4float %51
         %53 = OpCompositeExtract %float %52 0
         %54 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %55 = OpLoad %v4float %54
         %56 = OpCompositeExtract %float %55 3
         %57 = OpFMul %float %53 %56
         %58 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %59 = OpLoad %v4float %58
         %60 = OpCompositeExtract %float %59 3
         %61 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %62 = OpLoad %v4float %61
         %63 = OpCompositeExtract %float %62 0
         %64 = OpFSub %float %60 %63
         %65 = OpLoad %float %_kGuardedDivideEpsilon
         %66 = OpFAdd %float %64 %65
         %67 = OpFDiv %float %57 %66
               OpStore %47 %67
               OpBranch %50
         %49 = OpLabel
         %68 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %69 = OpLoad %v4float %68
         %70 = OpCompositeExtract %float %69 3
               OpStore %47 %70
               OpBranch %50
         %50 = OpLabel
         %71 = OpLoad %float %47
         %33 = OpExtInst %float %1 FMin %36 %71
         %72 = OpFMul %float %31 %33
               OpStore %_1_delta %72
         %74 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %75 = OpLoad %v4float %74
         %76 = OpCompositeExtract %float %75 1
         %77 = OpFOrdEqual %bool %76 %float_0
         %78 = OpSelect %int %77 %int_0 %int_1
         %79 = OpConvertSToF %float %78
               OpStore %_2_dxScale %79
         %82 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %83 = OpLoad %v4float %82
         %84 = OpCompositeExtract %float %83 3
         %86 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %87 = OpLoad %v4float %86
         %88 = OpCompositeExtract %float %87 3
         %89 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %90 = OpLoad %v4float %89
         %91 = OpCompositeExtract %float %90 1
         %92 = OpFSub %float %88 %91
         %85 = OpExtInst %float %1 FAbs %92
         %93 = OpFOrdGreaterThanEqual %bool %85 %float_6_10351562en05
               OpSelectionMerge %97 None
               OpBranchConditional %93 %95 %96
         %95 = OpLabel
         %98 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %99 = OpLoad %v4float %98
        %100 = OpCompositeExtract %float %99 1
        %101 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %102 = OpLoad %v4float %101
        %103 = OpCompositeExtract %float %102 3
        %104 = OpFMul %float %100 %103
        %105 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %106 = OpLoad %v4float %105
        %107 = OpCompositeExtract %float %106 3
        %108 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %109 = OpLoad %v4float %108
        %110 = OpCompositeExtract %float %109 1
        %111 = OpFSub %float %107 %110
        %112 = OpLoad %float %_kGuardedDivideEpsilon
        %113 = OpFAdd %float %111 %112
        %114 = OpFDiv %float %104 %113
               OpStore %94 %114
               OpBranch %97
         %96 = OpLabel
        %115 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %116 = OpLoad %v4float %115
        %117 = OpCompositeExtract %float %116 3
               OpStore %94 %117
               OpBranch %97
         %97 = OpLabel
        %118 = OpLoad %float %94
         %81 = OpExtInst %float %1 FMin %84 %118
        %119 = OpFMul %float %79 %81
               OpStore %_3_delta %119
        %121 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %122 = OpLoad %v4float %121
        %123 = OpCompositeExtract %float %122 2
        %124 = OpFOrdEqual %bool %123 %float_0
        %125 = OpSelect %int %124 %int_0 %int_1
        %126 = OpConvertSToF %float %125
               OpStore %_4_dxScale %126
        %129 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %130 = OpLoad %v4float %129
        %131 = OpCompositeExtract %float %130 3
        %133 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %134 = OpLoad %v4float %133
        %135 = OpCompositeExtract %float %134 3
        %136 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %137 = OpLoad %v4float %136
        %138 = OpCompositeExtract %float %137 2
        %139 = OpFSub %float %135 %138
        %132 = OpExtInst %float %1 FAbs %139
        %140 = OpFOrdGreaterThanEqual %bool %132 %float_6_10351562en05
               OpSelectionMerge %144 None
               OpBranchConditional %140 %142 %143
        %142 = OpLabel
        %145 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %146 = OpLoad %v4float %145
        %147 = OpCompositeExtract %float %146 2
        %148 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %149 = OpLoad %v4float %148
        %150 = OpCompositeExtract %float %149 3
        %151 = OpFMul %float %147 %150
        %152 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %153 = OpLoad %v4float %152
        %154 = OpCompositeExtract %float %153 3
        %155 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %156 = OpLoad %v4float %155
        %157 = OpCompositeExtract %float %156 2
        %158 = OpFSub %float %154 %157
        %159 = OpLoad %float %_kGuardedDivideEpsilon
        %160 = OpFAdd %float %158 %159
        %161 = OpFDiv %float %151 %160
               OpStore %141 %161
               OpBranch %144
        %143 = OpLabel
        %162 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %163 = OpLoad %v4float %162
        %164 = OpCompositeExtract %float %163 3
               OpStore %141 %164
               OpBranch %144
        %144 = OpLabel
        %165 = OpLoad %float %141
        %128 = OpExtInst %float %1 FMin %131 %165
        %166 = OpFMul %float %126 %128
               OpStore %_5_delta %166
        %167 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %168 = OpLoad %v4float %167
        %169 = OpCompositeExtract %float %168 3
        %170 = OpFMul %float %72 %169
        %171 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %172 = OpLoad %v4float %171
        %173 = OpCompositeExtract %float %172 0
        %175 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %176 = OpLoad %v4float %175
        %177 = OpCompositeExtract %float %176 3
        %178 = OpFSub %float %float_1 %177
        %179 = OpFMul %float %173 %178
        %180 = OpFAdd %float %170 %179
        %181 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %182 = OpLoad %v4float %181
        %183 = OpCompositeExtract %float %182 0
        %184 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %185 = OpLoad %v4float %184
        %186 = OpCompositeExtract %float %185 3
        %187 = OpFSub %float %float_1 %186
        %188 = OpFMul %float %183 %187
        %189 = OpFAdd %float %180 %188
        %190 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %191 = OpLoad %v4float %190
        %192 = OpCompositeExtract %float %191 3
        %193 = OpFMul %float %119 %192
        %194 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %195 = OpLoad %v4float %194
        %196 = OpCompositeExtract %float %195 1
        %197 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %198 = OpLoad %v4float %197
        %199 = OpCompositeExtract %float %198 3
        %200 = OpFSub %float %float_1 %199
        %201 = OpFMul %float %196 %200
        %202 = OpFAdd %float %193 %201
        %203 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %204 = OpLoad %v4float %203
        %205 = OpCompositeExtract %float %204 1
        %206 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %207 = OpLoad %v4float %206
        %208 = OpCompositeExtract %float %207 3
        %209 = OpFSub %float %float_1 %208
        %210 = OpFMul %float %205 %209
        %211 = OpFAdd %float %202 %210
        %212 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %213 = OpLoad %v4float %212
        %214 = OpCompositeExtract %float %213 3
        %215 = OpFMul %float %166 %214
        %216 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %217 = OpLoad %v4float %216
        %218 = OpCompositeExtract %float %217 2
        %219 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %220 = OpLoad %v4float %219
        %221 = OpCompositeExtract %float %220 3
        %222 = OpFSub %float %float_1 %221
        %223 = OpFMul %float %218 %222
        %224 = OpFAdd %float %215 %223
        %225 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %226 = OpLoad %v4float %225
        %227 = OpCompositeExtract %float %226 2
        %228 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %229 = OpLoad %v4float %228
        %230 = OpCompositeExtract %float %229 3
        %231 = OpFSub %float %float_1 %230
        %232 = OpFMul %float %227 %231
        %233 = OpFAdd %float %224 %232
        %234 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %235 = OpLoad %v4float %234
        %236 = OpCompositeExtract %float %235 3
        %237 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %238 = OpLoad %v4float %237
        %239 = OpCompositeExtract %float %238 3
        %240 = OpFSub %float %float_1 %239
        %241 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %242 = OpLoad %v4float %241
        %243 = OpCompositeExtract %float %242 3
        %244 = OpFMul %float %240 %243
        %245 = OpFAdd %float %236 %244
        %246 = OpCompositeConstruct %v4float %189 %211 %233 %245
               OpStore %sk_FragColor %246
               OpReturn
               OpFunctionEnd
