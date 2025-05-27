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
               OpDecorate %14 Binding 0
               OpDecorate %14 DescriptorSet 0
               OpDecorate %_0_alpha RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %_1_sda RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %_2_dsa RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %_3_l RelaxedPrecision
               OpDecorate %_4_r RelaxedPrecision
               OpDecorate %_5_mn RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %_6_mx RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %_7_lum RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %_8_result RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %_9_minComp RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %_10_maxComp RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
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
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
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
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_1 = OpConstant %float 1
         %82 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
         %89 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
       %main = OpFunction %void None %18
         %19 = OpLabel
   %_0_alpha = OpVariable %_ptr_Function_float Function
     %_1_sda = OpVariable %_ptr_Function_v3float Function
     %_2_dsa = OpVariable %_ptr_Function_v3float Function
       %_3_l = OpVariable %_ptr_Function_v3float Function
       %_4_r = OpVariable %_ptr_Function_v3float Function
      %_5_mn = OpVariable %_ptr_Function_float Function
      %_6_mx = OpVariable %_ptr_Function_float Function
         %63 = OpVariable %_ptr_Function_v3float Function
     %_7_lum = OpVariable %_ptr_Function_float Function
  %_8_result = OpVariable %_ptr_Function_v3float Function
 %_9_minComp = OpVariable %_ptr_Function_float Function
%_10_maxComp = OpVariable %_ptr_Function_float Function
          %8 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %8
         %22 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %26 = OpLoad %v4float %22
         %27 = OpCompositeExtract %float %26 3
         %28 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %30 = OpLoad %v4float %28
         %31 = OpCompositeExtract %float %30 3
         %32 = OpFMul %float %27 %31
               OpStore %_0_alpha %32
         %36 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %37 = OpLoad %v4float %36
         %38 = OpVectorShuffle %v3float %37 %37 0 1 2
         %39 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %40 = OpLoad %v4float %39
         %41 = OpCompositeExtract %float %40 3
         %42 = OpVectorTimesScalar %v3float %38 %41
               OpStore %_1_sda %42
         %44 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %45 = OpLoad %v4float %44
         %46 = OpVectorShuffle %v3float %45 %45 0 1 2
         %47 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %48 = OpLoad %v4float %47
         %49 = OpCompositeExtract %float %48 3
         %50 = OpVectorTimesScalar %v3float %46 %49
               OpStore %_2_dsa %50
               OpStore %_3_l %50
               OpStore %_4_r %42
         %56 = OpCompositeExtract %float %50 0
         %57 = OpCompositeExtract %float %50 1
         %55 = OpExtInst %float %1 FMin %56 %57
         %58 = OpCompositeExtract %float %50 2
         %54 = OpExtInst %float %1 FMin %55 %58
               OpStore %_5_mn %54
         %61 = OpExtInst %float %1 FMax %56 %57
         %60 = OpExtInst %float %1 FMax %61 %58
               OpStore %_6_mx %60
         %62 = OpFOrdGreaterThan %bool %60 %54
               OpSelectionMerge %66 None
               OpBranchConditional %62 %64 %65
         %64 = OpLabel
         %67 = OpCompositeConstruct %v3float %54 %54 %54
         %68 = OpFSub %v3float %50 %67
         %71 = OpCompositeExtract %float %42 0
         %72 = OpCompositeExtract %float %42 1
         %70 = OpExtInst %float %1 FMax %71 %72
         %73 = OpCompositeExtract %float %42 2
         %69 = OpExtInst %float %1 FMax %70 %73
         %75 = OpExtInst %float %1 FMin %71 %72
         %74 = OpExtInst %float %1 FMin %75 %73
         %76 = OpFSub %float %69 %74
         %77 = OpVectorTimesScalar %v3float %68 %76
         %78 = OpFSub %float %60 %54
         %80 = OpFDiv %float %float_1 %78
         %81 = OpVectorTimesScalar %v3float %77 %80
               OpStore %63 %81
               OpBranch %66
         %65 = OpLabel
               OpStore %63 %82
               OpBranch %66
         %66 = OpLabel
         %83 = OpLoad %v3float %63
               OpStore %_3_l %83
               OpStore %_4_r %50
         %85 = OpDot %float %89 %50
               OpStore %_7_lum %85
         %91 = OpDot %float %89 %83
         %92 = OpFSub %float %85 %91
         %93 = OpCompositeConstruct %v3float %92 %92 %92
         %94 = OpFAdd %v3float %93 %83
               OpStore %_8_result %94
         %98 = OpCompositeExtract %float %94 0
         %99 = OpCompositeExtract %float %94 1
         %97 = OpExtInst %float %1 FMin %98 %99
        %100 = OpCompositeExtract %float %94 2
         %96 = OpExtInst %float %1 FMin %97 %100
               OpStore %_9_minComp %96
        %103 = OpExtInst %float %1 FMax %98 %99
        %102 = OpExtInst %float %1 FMax %103 %100
               OpStore %_10_maxComp %102
        %104 = OpFOrdLessThan %bool %96 %float_0
               OpSelectionMerge %106 None
               OpBranchConditional %104 %105 %106
        %105 = OpLabel
        %107 = OpFUnordNotEqual %bool %85 %96
               OpBranch %106
        %106 = OpLabel
        %108 = OpPhi %bool %false %66 %107 %105
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
        %111 = OpCompositeConstruct %v3float %85 %85 %85
        %112 = OpFSub %v3float %94 %111
        %113 = OpFSub %float %85 %96
        %115 = OpFAdd %float %113 %float_6_10351562en05
        %116 = OpLoad %float %_kGuardedDivideEpsilon
        %117 = OpFAdd %float %115 %116
        %118 = OpFDiv %float %85 %117
        %119 = OpVectorTimesScalar %v3float %112 %118
        %120 = OpFAdd %v3float %111 %119
               OpStore %_8_result %120
               OpBranch %110
        %110 = OpLabel
        %121 = OpFOrdGreaterThan %bool %102 %32
               OpSelectionMerge %123 None
               OpBranchConditional %121 %122 %123
        %122 = OpLabel
        %124 = OpFUnordNotEqual %bool %102 %85
               OpBranch %123
        %123 = OpLabel
        %125 = OpPhi %bool %false %110 %124 %122
               OpSelectionMerge %127 None
               OpBranchConditional %125 %126 %127
        %126 = OpLabel
        %128 = OpLoad %v3float %_8_result
        %129 = OpCompositeConstruct %v3float %85 %85 %85
        %130 = OpFSub %v3float %128 %129
        %131 = OpFSub %float %32 %85
        %132 = OpVectorTimesScalar %v3float %130 %131
        %133 = OpFSub %float %102 %85
        %134 = OpFAdd %float %133 %float_6_10351562en05
        %135 = OpLoad %float %_kGuardedDivideEpsilon
        %136 = OpFAdd %float %134 %135
        %137 = OpFDiv %float %float_1 %136
        %138 = OpVectorTimesScalar %v3float %132 %137
        %139 = OpFAdd %v3float %129 %138
               OpStore %_8_result %139
               OpBranch %127
        %127 = OpLabel
        %140 = OpLoad %v3float %_8_result
        %141 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %142 = OpLoad %v4float %141
        %143 = OpVectorShuffle %v3float %142 %142 0 1 2
        %144 = OpFAdd %v3float %140 %143
        %145 = OpFSub %v3float %144 %50
        %146 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %147 = OpLoad %v4float %146
        %148 = OpVectorShuffle %v3float %147 %147 0 1 2
        %149 = OpFAdd %v3float %145 %148
        %150 = OpFSub %v3float %149 %42
        %151 = OpCompositeExtract %float %150 0
        %152 = OpCompositeExtract %float %150 1
        %153 = OpCompositeExtract %float %150 2
        %154 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %155 = OpLoad %v4float %154
        %156 = OpCompositeExtract %float %155 3
        %157 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %158 = OpLoad %v4float %157
        %159 = OpCompositeExtract %float %158 3
        %160 = OpFAdd %float %156 %159
        %161 = OpFSub %float %160 %32
        %162 = OpCompositeConstruct %v4float %151 %152 %153 %161
               OpStore %sk_FragColor %162
               OpReturn
               OpFunctionEnd
