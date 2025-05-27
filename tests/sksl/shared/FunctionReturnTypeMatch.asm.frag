               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %x1 "x1"
               OpName %x2 "x2"
               OpName %x3 "x3"
               OpName %x4 "x4"
               OpName %x5 "x5"
               OpName %x6 "x6"
               OpName %x7 "x7"
               OpName %x8 "x8"
               OpName %x9 "x9"
               OpName %x10 "x10"
               OpName %x11 "x11"
               OpName %x12 "x12"
               OpName %x13 "x13"
               OpName %x14 "x14"
               OpName %x15 "x15"
               OpName %x16 "x16"
               OpName %x17 "x17"
               OpName %x18 "x18"
               OpName %x19 "x19"
               OpName %x20 "x20"
               OpName %x21 "x21"
               OpName %x22 "x22"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %x8 RelaxedPrecision
               OpDecorate %x9 RelaxedPrecision
               OpDecorate %x10 RelaxedPrecision
               OpDecorate %x11 RelaxedPrecision
               OpDecorate %x12 RelaxedPrecision
               OpDecorate %x13 RelaxedPrecision
               OpDecorate %x14 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
         %28 = OpConstantComposite %v2float %float_2 %float_2
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_3 = OpConstant %float 3
         %33 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %37 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %41 = OpConstantComposite %v2float %float_2 %float_0
         %42 = OpConstantComposite %v2float %float_0 %float_2
         %43 = OpConstantComposite %mat2v2float %41 %42
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
         %47 = OpConstantComposite %v3float %float_3 %float_0 %float_0
         %48 = OpConstantComposite %v3float %float_0 %float_3 %float_0
         %49 = OpConstantComposite %v3float %float_0 %float_0 %float_3
         %50 = OpConstantComposite %mat3v3float %47 %48 %49
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
         %54 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
         %55 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
         %56 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
         %57 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
         %58 = OpConstantComposite %mat4v4float %54 %55 %56 %57
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
         %73 = OpConstantComposite %v2bool %true %true
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
         %77 = OpConstantComposite %v3bool %true %true %true
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
         %81 = OpConstantComposite %v4bool %true %true %true %true
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
      %int_2 = OpConstant %int 2
         %90 = OpConstantComposite %v2int %int_2 %int_2
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
      %int_3 = OpConstant %int 3
         %95 = OpConstantComposite %v3int %int_3 %int_3 %int_3
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %int_4 = OpConstant %int 4
        %100 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
         %x1 = OpVariable %_ptr_Function_float Function
         %x2 = OpVariable %_ptr_Function_v2float Function
         %x3 = OpVariable %_ptr_Function_v3float Function
         %x4 = OpVariable %_ptr_Function_v4float Function
         %x5 = OpVariable %_ptr_Function_mat2v2float Function
         %x6 = OpVariable %_ptr_Function_mat3v3float Function
         %x7 = OpVariable %_ptr_Function_mat4v4float Function
         %x8 = OpVariable %_ptr_Function_float Function
         %x9 = OpVariable %_ptr_Function_v2float Function
        %x10 = OpVariable %_ptr_Function_v3float Function
        %x11 = OpVariable %_ptr_Function_v4float Function
        %x12 = OpVariable %_ptr_Function_mat2v2float Function
        %x13 = OpVariable %_ptr_Function_mat3v3float Function
        %x14 = OpVariable %_ptr_Function_mat4v4float Function
        %x15 = OpVariable %_ptr_Function_bool Function
        %x16 = OpVariable %_ptr_Function_v2bool Function
        %x17 = OpVariable %_ptr_Function_v3bool Function
        %x18 = OpVariable %_ptr_Function_v4bool Function
        %x19 = OpVariable %_ptr_Function_int Function
        %x20 = OpVariable %_ptr_Function_v2int Function
        %x21 = OpVariable %_ptr_Function_v3int Function
        %x22 = OpVariable %_ptr_Function_v4int Function
        %213 = OpVariable %_ptr_Function_v4float Function
               OpStore %x1 %float_1
               OpStore %x2 %28
               OpStore %x3 %33
               OpStore %x4 %37
               OpStore %x5 %43
               OpStore %x6 %50
               OpStore %x7 %58
               OpStore %x8 %float_1
               OpStore %x9 %28
               OpStore %x10 %33
               OpStore %x11 %37
               OpStore %x12 %43
               OpStore %x13 %50
               OpStore %x14 %58
               OpStore %x15 %true
               OpStore %x16 %73
               OpStore %x17 %77
               OpStore %x18 %81
               OpStore %x19 %int_1
               OpStore %x20 %90
               OpStore %x21 %95
               OpStore %x22 %100
               OpSelectionMerge %103 None
               OpBranchConditional %true %102 %103
        %102 = OpLabel
               OpBranch %103
        %103 = OpLabel
        %104 = OpPhi %bool %false %22 %true %102
               OpSelectionMerge %106 None
               OpBranchConditional %104 %105 %106
        %105 = OpLabel
               OpBranch %106
        %106 = OpLabel
        %107 = OpPhi %bool %false %103 %true %105
               OpSelectionMerge %109 None
               OpBranchConditional %107 %108 %109
        %108 = OpLabel
               OpBranch %109
        %109 = OpLabel
        %110 = OpPhi %bool %false %106 %true %108
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %113 = OpFOrdEqual %v2bool %41 %41
        %114 = OpAll %bool %113
        %115 = OpFOrdEqual %v2bool %42 %42
        %116 = OpAll %bool %115
        %117 = OpLogicalAnd %bool %114 %116
               OpBranch %112
        %112 = OpLabel
        %118 = OpPhi %bool %false %109 %117 %111
               OpSelectionMerge %120 None
               OpBranchConditional %118 %119 %120
        %119 = OpLabel
        %121 = OpFOrdEqual %v3bool %47 %47
        %122 = OpAll %bool %121
        %123 = OpFOrdEqual %v3bool %48 %48
        %124 = OpAll %bool %123
        %125 = OpLogicalAnd %bool %122 %124
        %126 = OpFOrdEqual %v3bool %49 %49
        %127 = OpAll %bool %126
        %128 = OpLogicalAnd %bool %125 %127
               OpBranch %120
        %120 = OpLabel
        %129 = OpPhi %bool %false %112 %128 %119
               OpSelectionMerge %131 None
               OpBranchConditional %129 %130 %131
        %130 = OpLabel
        %132 = OpFOrdEqual %v4bool %54 %54
        %133 = OpAll %bool %132
        %134 = OpFOrdEqual %v4bool %55 %55
        %135 = OpAll %bool %134
        %136 = OpLogicalAnd %bool %133 %135
        %137 = OpFOrdEqual %v4bool %56 %56
        %138 = OpAll %bool %137
        %139 = OpLogicalAnd %bool %136 %138
        %140 = OpFOrdEqual %v4bool %57 %57
        %141 = OpAll %bool %140
        %142 = OpLogicalAnd %bool %139 %141
               OpBranch %131
        %131 = OpLabel
        %143 = OpPhi %bool %false %120 %142 %130
               OpSelectionMerge %145 None
               OpBranchConditional %143 %144 %145
        %144 = OpLabel
               OpBranch %145
        %145 = OpLabel
        %146 = OpPhi %bool %false %131 %true %144
               OpSelectionMerge %148 None
               OpBranchConditional %146 %147 %148
        %147 = OpLabel
               OpBranch %148
        %148 = OpLabel
        %149 = OpPhi %bool %false %145 %true %147
               OpSelectionMerge %151 None
               OpBranchConditional %149 %150 %151
        %150 = OpLabel
               OpBranch %151
        %151 = OpLabel
        %152 = OpPhi %bool %false %148 %true %150
               OpSelectionMerge %154 None
               OpBranchConditional %152 %153 %154
        %153 = OpLabel
               OpBranch %154
        %154 = OpLabel
        %155 = OpPhi %bool %false %151 %true %153
               OpSelectionMerge %157 None
               OpBranchConditional %155 %156 %157
        %156 = OpLabel
        %158 = OpFOrdEqual %v2bool %41 %41
        %159 = OpAll %bool %158
        %160 = OpFOrdEqual %v2bool %42 %42
        %161 = OpAll %bool %160
        %162 = OpLogicalAnd %bool %159 %161
               OpBranch %157
        %157 = OpLabel
        %163 = OpPhi %bool %false %154 %162 %156
               OpSelectionMerge %165 None
               OpBranchConditional %163 %164 %165
        %164 = OpLabel
        %166 = OpFOrdEqual %v3bool %47 %47
        %167 = OpAll %bool %166
        %168 = OpFOrdEqual %v3bool %48 %48
        %169 = OpAll %bool %168
        %170 = OpLogicalAnd %bool %167 %169
        %171 = OpFOrdEqual %v3bool %49 %49
        %172 = OpAll %bool %171
        %173 = OpLogicalAnd %bool %170 %172
               OpBranch %165
        %165 = OpLabel
        %174 = OpPhi %bool %false %157 %173 %164
               OpSelectionMerge %176 None
               OpBranchConditional %174 %175 %176
        %175 = OpLabel
        %177 = OpFOrdEqual %v4bool %54 %54
        %178 = OpAll %bool %177
        %179 = OpFOrdEqual %v4bool %55 %55
        %180 = OpAll %bool %179
        %181 = OpLogicalAnd %bool %178 %180
        %182 = OpFOrdEqual %v4bool %56 %56
        %183 = OpAll %bool %182
        %184 = OpLogicalAnd %bool %181 %183
        %185 = OpFOrdEqual %v4bool %57 %57
        %186 = OpAll %bool %185
        %187 = OpLogicalAnd %bool %184 %186
               OpBranch %176
        %176 = OpLabel
        %188 = OpPhi %bool %false %165 %187 %175
               OpSelectionMerge %190 None
               OpBranchConditional %188 %189 %190
        %189 = OpLabel
               OpBranch %190
        %190 = OpLabel
        %191 = OpPhi %bool %false %176 %true %189
               OpSelectionMerge %193 None
               OpBranchConditional %191 %192 %193
        %192 = OpLabel
               OpBranch %193
        %193 = OpLabel
        %194 = OpPhi %bool %false %190 %true %192
               OpSelectionMerge %196 None
               OpBranchConditional %194 %195 %196
        %195 = OpLabel
               OpBranch %196
        %196 = OpLabel
        %197 = OpPhi %bool %false %193 %true %195
               OpSelectionMerge %199 None
               OpBranchConditional %197 %198 %199
        %198 = OpLabel
               OpBranch %199
        %199 = OpLabel
        %200 = OpPhi %bool %false %196 %true %198
               OpSelectionMerge %202 None
               OpBranchConditional %200 %201 %202
        %201 = OpLabel
               OpBranch %202
        %202 = OpLabel
        %203 = OpPhi %bool %false %199 %true %201
               OpSelectionMerge %205 None
               OpBranchConditional %203 %204 %205
        %204 = OpLabel
               OpBranch %205
        %205 = OpLabel
        %206 = OpPhi %bool %false %202 %true %204
               OpSelectionMerge %208 None
               OpBranchConditional %206 %207 %208
        %207 = OpLabel
               OpBranch %208
        %208 = OpLabel
        %209 = OpPhi %bool %false %205 %true %207
               OpSelectionMerge %211 None
               OpBranchConditional %209 %210 %211
        %210 = OpLabel
               OpBranch %211
        %211 = OpLabel
        %212 = OpPhi %bool %false %208 %true %210
               OpSelectionMerge %216 None
               OpBranchConditional %212 %214 %215
        %214 = OpLabel
        %217 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %220 = OpLoad %v4float %217
               OpStore %213 %220
               OpBranch %216
        %215 = OpLabel
        %221 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %222 = OpLoad %v4float %221
               OpStore %213 %222
               OpBranch %216
        %216 = OpLabel
        %223 = OpLoad %v4float %213
               OpReturnValue %223
               OpFunctionEnd
