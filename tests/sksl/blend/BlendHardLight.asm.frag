               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %blend_overlay_h4h4h4 "blend_overlay_h4h4h4"
               OpName %result "result"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %result RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
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
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
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
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
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
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %12 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
       %void = OpTypeVoid
        %156 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%blend_overlay_h4h4h4 = OpFunction %v4float None %12
         %13 = OpFunctionParameter %_ptr_Function_v4float
         %14 = OpFunctionParameter %_ptr_Function_v4float
         %15 = OpLabel
     %result = OpVariable %_ptr_Function_v4float Function
         %25 = OpVariable %_ptr_Function_float Function
         %61 = OpVariable %_ptr_Function_float Function
         %96 = OpVariable %_ptr_Function_float Function
         %18 = OpLoad %v4float %14
         %19 = OpCompositeExtract %float %18 0
         %20 = OpFMul %float %float_2 %19
         %21 = OpLoad %v4float %14
         %22 = OpCompositeExtract %float %21 3
         %23 = OpFOrdLessThanEqual %bool %20 %22
               OpSelectionMerge %29 None
               OpBranchConditional %23 %27 %28
         %27 = OpLabel
         %30 = OpLoad %v4float %13
         %31 = OpCompositeExtract %float %30 0
         %32 = OpFMul %float %float_2 %31
         %33 = OpLoad %v4float %14
         %34 = OpCompositeExtract %float %33 0
         %35 = OpFMul %float %32 %34
               OpStore %25 %35
               OpBranch %29
         %28 = OpLabel
         %36 = OpLoad %v4float %13
         %37 = OpCompositeExtract %float %36 3
         %38 = OpLoad %v4float %14
         %39 = OpCompositeExtract %float %38 3
         %40 = OpFMul %float %37 %39
         %41 = OpLoad %v4float %14
         %42 = OpCompositeExtract %float %41 3
         %43 = OpLoad %v4float %14
         %44 = OpCompositeExtract %float %43 0
         %45 = OpFSub %float %42 %44
         %46 = OpFMul %float %float_2 %45
         %47 = OpLoad %v4float %13
         %48 = OpCompositeExtract %float %47 3
         %49 = OpLoad %v4float %13
         %50 = OpCompositeExtract %float %49 0
         %51 = OpFSub %float %48 %50
         %52 = OpFMul %float %46 %51
         %53 = OpFSub %float %40 %52
               OpStore %25 %53
               OpBranch %29
         %29 = OpLabel
         %54 = OpLoad %float %25
         %55 = OpLoad %v4float %14
         %56 = OpCompositeExtract %float %55 1
         %57 = OpFMul %float %float_2 %56
         %58 = OpLoad %v4float %14
         %59 = OpCompositeExtract %float %58 3
         %60 = OpFOrdLessThanEqual %bool %57 %59
               OpSelectionMerge %64 None
               OpBranchConditional %60 %62 %63
         %62 = OpLabel
         %65 = OpLoad %v4float %13
         %66 = OpCompositeExtract %float %65 1
         %67 = OpFMul %float %float_2 %66
         %68 = OpLoad %v4float %14
         %69 = OpCompositeExtract %float %68 1
         %70 = OpFMul %float %67 %69
               OpStore %61 %70
               OpBranch %64
         %63 = OpLabel
         %71 = OpLoad %v4float %13
         %72 = OpCompositeExtract %float %71 3
         %73 = OpLoad %v4float %14
         %74 = OpCompositeExtract %float %73 3
         %75 = OpFMul %float %72 %74
         %76 = OpLoad %v4float %14
         %77 = OpCompositeExtract %float %76 3
         %78 = OpLoad %v4float %14
         %79 = OpCompositeExtract %float %78 1
         %80 = OpFSub %float %77 %79
         %81 = OpFMul %float %float_2 %80
         %82 = OpLoad %v4float %13
         %83 = OpCompositeExtract %float %82 3
         %84 = OpLoad %v4float %13
         %85 = OpCompositeExtract %float %84 1
         %86 = OpFSub %float %83 %85
         %87 = OpFMul %float %81 %86
         %88 = OpFSub %float %75 %87
               OpStore %61 %88
               OpBranch %64
         %64 = OpLabel
         %89 = OpLoad %float %61
         %90 = OpLoad %v4float %14
         %91 = OpCompositeExtract %float %90 2
         %92 = OpFMul %float %float_2 %91
         %93 = OpLoad %v4float %14
         %94 = OpCompositeExtract %float %93 3
         %95 = OpFOrdLessThanEqual %bool %92 %94
               OpSelectionMerge %99 None
               OpBranchConditional %95 %97 %98
         %97 = OpLabel
        %100 = OpLoad %v4float %13
        %101 = OpCompositeExtract %float %100 2
        %102 = OpFMul %float %float_2 %101
        %103 = OpLoad %v4float %14
        %104 = OpCompositeExtract %float %103 2
        %105 = OpFMul %float %102 %104
               OpStore %96 %105
               OpBranch %99
         %98 = OpLabel
        %106 = OpLoad %v4float %13
        %107 = OpCompositeExtract %float %106 3
        %108 = OpLoad %v4float %14
        %109 = OpCompositeExtract %float %108 3
        %110 = OpFMul %float %107 %109
        %111 = OpLoad %v4float %14
        %112 = OpCompositeExtract %float %111 3
        %113 = OpLoad %v4float %14
        %114 = OpCompositeExtract %float %113 2
        %115 = OpFSub %float %112 %114
        %116 = OpFMul %float %float_2 %115
        %117 = OpLoad %v4float %13
        %118 = OpCompositeExtract %float %117 3
        %119 = OpLoad %v4float %13
        %120 = OpCompositeExtract %float %119 2
        %121 = OpFSub %float %118 %120
        %122 = OpFMul %float %116 %121
        %123 = OpFSub %float %110 %122
               OpStore %96 %123
               OpBranch %99
         %99 = OpLabel
        %124 = OpLoad %float %96
        %125 = OpLoad %v4float %13
        %126 = OpCompositeExtract %float %125 3
        %128 = OpLoad %v4float %13
        %129 = OpCompositeExtract %float %128 3
        %130 = OpFSub %float %float_1 %129
        %131 = OpLoad %v4float %14
        %132 = OpCompositeExtract %float %131 3
        %133 = OpFMul %float %130 %132
        %134 = OpFAdd %float %126 %133
        %135 = OpCompositeConstruct %v4float %54 %89 %124 %134
               OpStore %result %135
        %136 = OpLoad %v4float %result
        %137 = OpVectorShuffle %v3float %136 %136 0 1 2
        %139 = OpLoad %v4float %14
        %140 = OpVectorShuffle %v3float %139 %139 0 1 2
        %141 = OpLoad %v4float %13
        %142 = OpCompositeExtract %float %141 3
        %143 = OpFSub %float %float_1 %142
        %144 = OpVectorTimesScalar %v3float %140 %143
        %145 = OpLoad %v4float %13
        %146 = OpVectorShuffle %v3float %145 %145 0 1 2
        %147 = OpLoad %v4float %14
        %148 = OpCompositeExtract %float %147 3
        %149 = OpFSub %float %float_1 %148
        %150 = OpVectorTimesScalar %v3float %146 %149
        %151 = OpFAdd %v3float %144 %150
        %152 = OpFAdd %v3float %137 %151
        %153 = OpLoad %v4float %result
        %154 = OpVectorShuffle %v4float %153 %152 4 5 6 3
               OpStore %result %154
               OpReturnValue %154
               OpFunctionEnd
       %main = OpFunction %void None %156
        %157 = OpLabel
        %163 = OpVariable %_ptr_Function_v4float Function
        %167 = OpVariable %_ptr_Function_v4float Function
        %158 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %162 = OpLoad %v4float %158
               OpStore %163 %162
        %164 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %166 = OpLoad %v4float %164
               OpStore %167 %166
        %168 = OpFunctionCall %v4float %blend_overlay_h4h4h4 %163 %167
               OpStore %sk_FragColor %168
               OpReturn
               OpFunctionEnd
