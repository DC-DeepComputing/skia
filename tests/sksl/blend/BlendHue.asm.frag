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
               OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"
               OpName %main "main"
               OpName %_0_alpha "_0_alpha"
               OpName %_1_sda "_1_sda"
               OpName %_2_dsa "_2_dsa"
               OpName %_3_l "_3_l"
               OpName %_4_r "_4_r"
               OpName %_5_mn "_5_mn"
               OpName %_6_mx "_6_mx"
               OpName %_7_lum "_7_lum"
               OpName %_8_result "_8_result"
               OpName %_9_minComp "_9_minComp"
               OpName %_10_maxComp "_10_maxComp"
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
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %_0_alpha RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %_1_sda RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %_2_dsa RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %_3_l RelaxedPrecision
               OpDecorate %_4_r RelaxedPrecision
               OpDecorate %_5_mn RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %_6_mx RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %_7_lum RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %_8_result RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %_9_minComp RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %_10_maxComp RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
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
               OpDecorate %155 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
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
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %20 = OpTypeFunction %float %_ptr_Function_v3float
       %void = OpTypeVoid
         %41 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %97 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
        %104 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
%blend_color_saturation_Qhh3 = OpFunction %float None %20
         %21 = OpFunctionParameter %_ptr_Function_v3float
         %22 = OpLabel
         %25 = OpLoad %v3float %21
         %26 = OpCompositeExtract %float %25 0
         %27 = OpLoad %v3float %21
         %28 = OpCompositeExtract %float %27 1
         %24 = OpExtInst %float %1 FMax %26 %28
         %29 = OpLoad %v3float %21
         %30 = OpCompositeExtract %float %29 2
         %23 = OpExtInst %float %1 FMax %24 %30
         %33 = OpLoad %v3float %21
         %34 = OpCompositeExtract %float %33 0
         %35 = OpLoad %v3float %21
         %36 = OpCompositeExtract %float %35 1
         %32 = OpExtInst %float %1 FMin %34 %36
         %37 = OpLoad %v3float %21
         %38 = OpCompositeExtract %float %37 2
         %31 = OpExtInst %float %1 FMin %32 %38
         %39 = OpFSub %float %23 %31
               OpReturnValue %39
               OpFunctionEnd
       %main = OpFunction %void None %41
         %42 = OpLabel
   %_0_alpha = OpVariable %_ptr_Function_float Function
     %_1_sda = OpVariable %_ptr_Function_v3float Function
     %_2_dsa = OpVariable %_ptr_Function_v3float Function
       %_3_l = OpVariable %_ptr_Function_v3float Function
       %_4_r = OpVariable %_ptr_Function_v3float Function
      %_5_mn = OpVariable %_ptr_Function_float Function
      %_6_mx = OpVariable %_ptr_Function_float Function
         %84 = OpVariable %_ptr_Function_v3float Function
         %90 = OpVariable %_ptr_Function_v3float Function
     %_7_lum = OpVariable %_ptr_Function_float Function
  %_8_result = OpVariable %_ptr_Function_v3float Function
 %_9_minComp = OpVariable %_ptr_Function_float Function
%_10_maxComp = OpVariable %_ptr_Function_float Function
          %9 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %9
         %45 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %49 = OpLoad %v4float %45
         %50 = OpCompositeExtract %float %49 3
         %51 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %53 = OpLoad %v4float %51
         %54 = OpCompositeExtract %float %53 3
         %55 = OpFMul %float %50 %54
               OpStore %_0_alpha %55
         %57 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %58 = OpLoad %v4float %57
         %59 = OpVectorShuffle %v3float %58 %58 0 1 2
         %60 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %61 = OpLoad %v4float %60
         %62 = OpCompositeExtract %float %61 3
         %63 = OpVectorTimesScalar %v3float %59 %62
               OpStore %_1_sda %63
         %65 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
         %66 = OpLoad %v4float %65
         %67 = OpVectorShuffle %v3float %66 %66 0 1 2
         %68 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %69 = OpLoad %v4float %68
         %70 = OpCompositeExtract %float %69 3
         %71 = OpVectorTimesScalar %v3float %67 %70
               OpStore %_2_dsa %71
               OpStore %_3_l %63
               OpStore %_4_r %71
         %77 = OpCompositeExtract %float %63 0
         %78 = OpCompositeExtract %float %63 1
         %76 = OpExtInst %float %1 FMin %77 %78
         %79 = OpCompositeExtract %float %63 2
         %75 = OpExtInst %float %1 FMin %76 %79
               OpStore %_5_mn %75
         %82 = OpExtInst %float %1 FMax %77 %78
         %81 = OpExtInst %float %1 FMax %82 %79
               OpStore %_6_mx %81
         %83 = OpFOrdGreaterThan %bool %81 %75
               OpSelectionMerge %87 None
               OpBranchConditional %83 %85 %86
         %85 = OpLabel
         %88 = OpCompositeConstruct %v3float %75 %75 %75
         %89 = OpFSub %v3float %63 %88
               OpStore %90 %71
         %91 = OpFunctionCall %float %blend_color_saturation_Qhh3 %90
         %92 = OpVectorTimesScalar %v3float %89 %91
         %93 = OpFSub %float %81 %75
         %95 = OpFDiv %float %float_1 %93
         %96 = OpVectorTimesScalar %v3float %92 %95
               OpStore %84 %96
               OpBranch %87
         %86 = OpLabel
               OpStore %84 %97
               OpBranch %87
         %87 = OpLabel
         %98 = OpLoad %v3float %84
               OpStore %_3_l %98
               OpStore %_4_r %71
        %100 = OpDot %float %104 %71
               OpStore %_7_lum %100
        %106 = OpDot %float %104 %98
        %107 = OpFSub %float %100 %106
        %108 = OpCompositeConstruct %v3float %107 %107 %107
        %109 = OpFAdd %v3float %108 %98
               OpStore %_8_result %109
        %113 = OpCompositeExtract %float %109 0
        %114 = OpCompositeExtract %float %109 1
        %112 = OpExtInst %float %1 FMin %113 %114
        %115 = OpCompositeExtract %float %109 2
        %111 = OpExtInst %float %1 FMin %112 %115
               OpStore %_9_minComp %111
        %118 = OpExtInst %float %1 FMax %113 %114
        %117 = OpExtInst %float %1 FMax %118 %115
               OpStore %_10_maxComp %117
        %119 = OpFOrdLessThan %bool %111 %float_0
               OpSelectionMerge %121 None
               OpBranchConditional %119 %120 %121
        %120 = OpLabel
        %122 = OpFUnordNotEqual %bool %100 %111
               OpBranch %121
        %121 = OpLabel
        %123 = OpPhi %bool %false %87 %122 %120
               OpSelectionMerge %125 None
               OpBranchConditional %123 %124 %125
        %124 = OpLabel
        %126 = OpCompositeConstruct %v3float %100 %100 %100
        %127 = OpFSub %v3float %109 %126
        %128 = OpFSub %float %100 %111
        %130 = OpFAdd %float %128 %float_6_10351562en05
        %131 = OpLoad %float %_kGuardedDivideEpsilon
        %132 = OpFAdd %float %130 %131
        %133 = OpFDiv %float %100 %132
        %134 = OpVectorTimesScalar %v3float %127 %133
        %135 = OpFAdd %v3float %126 %134
               OpStore %_8_result %135
               OpBranch %125
        %125 = OpLabel
        %136 = OpFOrdGreaterThan %bool %117 %55
               OpSelectionMerge %138 None
               OpBranchConditional %136 %137 %138
        %137 = OpLabel
        %139 = OpFUnordNotEqual %bool %117 %100
               OpBranch %138
        %138 = OpLabel
        %140 = OpPhi %bool %false %125 %139 %137
               OpSelectionMerge %142 None
               OpBranchConditional %140 %141 %142
        %141 = OpLabel
        %143 = OpLoad %v3float %_8_result
        %144 = OpCompositeConstruct %v3float %100 %100 %100
        %145 = OpFSub %v3float %143 %144
        %146 = OpFSub %float %55 %100
        %147 = OpVectorTimesScalar %v3float %145 %146
        %148 = OpFSub %float %117 %100
        %149 = OpFAdd %float %148 %float_6_10351562en05
        %150 = OpLoad %float %_kGuardedDivideEpsilon
        %151 = OpFAdd %float %149 %150
        %152 = OpFDiv %float %float_1 %151
        %153 = OpVectorTimesScalar %v3float %147 %152
        %154 = OpFAdd %v3float %144 %153
               OpStore %_8_result %154
               OpBranch %142
        %142 = OpLabel
        %155 = OpLoad %v3float %_8_result
        %156 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %157 = OpLoad %v4float %156
        %158 = OpVectorShuffle %v3float %157 %157 0 1 2
        %159 = OpFAdd %v3float %155 %158
        %160 = OpFSub %v3float %159 %71
        %161 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %162 = OpLoad %v4float %161
        %163 = OpVectorShuffle %v3float %162 %162 0 1 2
        %164 = OpFAdd %v3float %160 %163
        %165 = OpFSub %v3float %164 %63
        %166 = OpCompositeExtract %float %165 0
        %167 = OpCompositeExtract %float %165 1
        %168 = OpCompositeExtract %float %165 2
        %169 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %170 = OpLoad %v4float %169
        %171 = OpCompositeExtract %float %170 3
        %172 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %173 = OpLoad %v4float %172
        %174 = OpCompositeExtract %float %173 3
        %175 = OpFAdd %float %171 %174
        %176 = OpFSub %float %175 %55
        %177 = OpCompositeConstruct %v4float %166 %167 %168 %176
               OpStore %sk_FragColor %177
               OpReturn
               OpFunctionEnd
