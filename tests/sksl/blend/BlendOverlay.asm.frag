               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %main "main"
               OpName %_0_result "_0_result"
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
               OpDecorate %_0_result RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_2 = OpConstant %float 2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
       %main = OpFunction %void None %11
         %12 = OpLabel
  %_0_result = OpVariable %_ptr_Function_v4float Function
         %28 = OpVariable %_ptr_Function_float Function
         %75 = OpVariable %_ptr_Function_float Function
        %120 = OpVariable %_ptr_Function_float Function
         %16 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %20 = OpLoad %v4float %16
         %21 = OpCompositeExtract %float %20 0
         %22 = OpFMul %float %float_2 %21
         %23 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %24 = OpLoad %v4float %23
         %25 = OpCompositeExtract %float %24 3
         %26 = OpFOrdLessThanEqual %bool %22 %25
               OpSelectionMerge %32 None
               OpBranchConditional %26 %30 %31
         %30 = OpLabel
         %33 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %35 = OpLoad %v4float %33
         %36 = OpCompositeExtract %float %35 0
         %37 = OpFMul %float %float_2 %36
         %38 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %39 = OpLoad %v4float %38
         %40 = OpCompositeExtract %float %39 0
         %41 = OpFMul %float %37 %40
               OpStore %28 %41
               OpBranch %32
         %31 = OpLabel
         %42 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %43 = OpLoad %v4float %42
         %44 = OpCompositeExtract %float %43 3
         %45 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %46 = OpLoad %v4float %45
         %47 = OpCompositeExtract %float %46 3
         %48 = OpFMul %float %44 %47
         %49 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %50 = OpLoad %v4float %49
         %51 = OpCompositeExtract %float %50 3
         %52 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %53 = OpLoad %v4float %52
         %54 = OpCompositeExtract %float %53 0
         %55 = OpFSub %float %51 %54
         %56 = OpFMul %float %float_2 %55
         %57 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %58 = OpLoad %v4float %57
         %59 = OpCompositeExtract %float %58 3
         %60 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %61 = OpLoad %v4float %60
         %62 = OpCompositeExtract %float %61 0
         %63 = OpFSub %float %59 %62
         %64 = OpFMul %float %56 %63
         %65 = OpFSub %float %48 %64
               OpStore %28 %65
               OpBranch %32
         %32 = OpLabel
         %66 = OpLoad %float %28
         %67 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %68 = OpLoad %v4float %67
         %69 = OpCompositeExtract %float %68 1
         %70 = OpFMul %float %float_2 %69
         %71 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %72 = OpLoad %v4float %71
         %73 = OpCompositeExtract %float %72 3
         %74 = OpFOrdLessThanEqual %bool %70 %73
               OpSelectionMerge %78 None
               OpBranchConditional %74 %76 %77
         %76 = OpLabel
         %79 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %80 = OpLoad %v4float %79
         %81 = OpCompositeExtract %float %80 1
         %82 = OpFMul %float %float_2 %81
         %83 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %84 = OpLoad %v4float %83
         %85 = OpCompositeExtract %float %84 1
         %86 = OpFMul %float %82 %85
               OpStore %75 %86
               OpBranch %78
         %77 = OpLabel
         %87 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %88 = OpLoad %v4float %87
         %89 = OpCompositeExtract %float %88 3
         %90 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %91 = OpLoad %v4float %90
         %92 = OpCompositeExtract %float %91 3
         %93 = OpFMul %float %89 %92
         %94 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %95 = OpLoad %v4float %94
         %96 = OpCompositeExtract %float %95 3
         %97 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %98 = OpLoad %v4float %97
         %99 = OpCompositeExtract %float %98 1
        %100 = OpFSub %float %96 %99
        %101 = OpFMul %float %float_2 %100
        %102 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %103 = OpLoad %v4float %102
        %104 = OpCompositeExtract %float %103 3
        %105 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %106 = OpLoad %v4float %105
        %107 = OpCompositeExtract %float %106 1
        %108 = OpFSub %float %104 %107
        %109 = OpFMul %float %101 %108
        %110 = OpFSub %float %93 %109
               OpStore %75 %110
               OpBranch %78
         %78 = OpLabel
        %111 = OpLoad %float %75
        %112 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %113 = OpLoad %v4float %112
        %114 = OpCompositeExtract %float %113 2
        %115 = OpFMul %float %float_2 %114
        %116 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %117 = OpLoad %v4float %116
        %118 = OpCompositeExtract %float %117 3
        %119 = OpFOrdLessThanEqual %bool %115 %118
               OpSelectionMerge %123 None
               OpBranchConditional %119 %121 %122
        %121 = OpLabel
        %124 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %125 = OpLoad %v4float %124
        %126 = OpCompositeExtract %float %125 2
        %127 = OpFMul %float %float_2 %126
        %128 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %129 = OpLoad %v4float %128
        %130 = OpCompositeExtract %float %129 2
        %131 = OpFMul %float %127 %130
               OpStore %120 %131
               OpBranch %123
        %122 = OpLabel
        %132 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %133 = OpLoad %v4float %132
        %134 = OpCompositeExtract %float %133 3
        %135 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %136 = OpLoad %v4float %135
        %137 = OpCompositeExtract %float %136 3
        %138 = OpFMul %float %134 %137
        %139 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %140 = OpLoad %v4float %139
        %141 = OpCompositeExtract %float %140 3
        %142 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %143 = OpLoad %v4float %142
        %144 = OpCompositeExtract %float %143 2
        %145 = OpFSub %float %141 %144
        %146 = OpFMul %float %float_2 %145
        %147 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %148 = OpLoad %v4float %147
        %149 = OpCompositeExtract %float %148 3
        %150 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %151 = OpLoad %v4float %150
        %152 = OpCompositeExtract %float %151 2
        %153 = OpFSub %float %149 %152
        %154 = OpFMul %float %146 %153
        %155 = OpFSub %float %138 %154
               OpStore %120 %155
               OpBranch %123
        %123 = OpLabel
        %156 = OpLoad %float %120
        %157 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %158 = OpLoad %v4float %157
        %159 = OpCompositeExtract %float %158 3
        %161 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %162 = OpLoad %v4float %161
        %163 = OpCompositeExtract %float %162 3
        %164 = OpFSub %float %float_1 %163
        %165 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %166 = OpLoad %v4float %165
        %167 = OpCompositeExtract %float %166 3
        %168 = OpFMul %float %164 %167
        %169 = OpFAdd %float %159 %168
        %170 = OpCompositeConstruct %v4float %66 %111 %156 %169
               OpStore %_0_result %170
        %171 = OpLoad %v4float %_0_result
        %172 = OpVectorShuffle %v3float %171 %171 0 1 2
        %174 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %175 = OpLoad %v4float %174
        %176 = OpVectorShuffle %v3float %175 %175 0 1 2
        %177 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %178 = OpLoad %v4float %177
        %179 = OpCompositeExtract %float %178 3
        %180 = OpFSub %float %float_1 %179
        %181 = OpVectorTimesScalar %v3float %176 %180
        %182 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %183 = OpLoad %v4float %182
        %184 = OpVectorShuffle %v3float %183 %183 0 1 2
        %185 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %186 = OpLoad %v4float %185
        %187 = OpCompositeExtract %float %186 3
        %188 = OpFSub %float %float_1 %187
        %189 = OpVectorTimesScalar %v3float %184 %188
        %190 = OpFAdd %v3float %181 %189
        %191 = OpFAdd %v3float %172 %190
        %192 = OpLoad %v4float %_0_result
        %193 = OpVectorShuffle %v4float %192 %191 4 5 6 3
               OpStore %_0_result %193
               OpStore %sk_FragColor %193
               OpReturn
               OpFunctionEnd
