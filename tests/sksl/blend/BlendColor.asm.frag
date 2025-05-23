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
               OpName %_5_lum "_5_lum"
               OpName %_6_result "_6_result"
               OpName %_7_minComp "_7_minComp"
               OpName %_8_maxComp "_8_maxComp"
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
               OpDecorate %_5_lum RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %_6_result RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %_7_minComp RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %_8_maxComp RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
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
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
         %58 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
    %float_1 = OpConstant %float 1
       %main = OpFunction %void None %18
         %19 = OpLabel
   %_0_alpha = OpVariable %_ptr_Function_float Function
     %_1_sda = OpVariable %_ptr_Function_v3float Function
     %_2_dsa = OpVariable %_ptr_Function_v3float Function
       %_3_l = OpVariable %_ptr_Function_v3float Function
       %_4_r = OpVariable %_ptr_Function_v3float Function
     %_5_lum = OpVariable %_ptr_Function_float Function
  %_6_result = OpVariable %_ptr_Function_v3float Function
 %_7_minComp = OpVariable %_ptr_Function_float Function
 %_8_maxComp = OpVariable %_ptr_Function_float Function
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
               OpStore %_3_l %42
               OpStore %_4_r %50
         %54 = OpDot %float %58 %50
               OpStore %_5_lum %54
         %60 = OpDot %float %58 %42
         %61 = OpFSub %float %54 %60
         %62 = OpCompositeConstruct %v3float %61 %61 %61
         %63 = OpFAdd %v3float %62 %42
               OpStore %_6_result %63
         %67 = OpCompositeExtract %float %63 0
         %68 = OpCompositeExtract %float %63 1
         %66 = OpExtInst %float %1 FMin %67 %68
         %69 = OpCompositeExtract %float %63 2
         %65 = OpExtInst %float %1 FMin %66 %69
               OpStore %_7_minComp %65
         %72 = OpExtInst %float %1 FMax %67 %68
         %71 = OpExtInst %float %1 FMax %72 %69
               OpStore %_8_maxComp %71
         %73 = OpFOrdLessThan %bool %65 %float_0
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
         %76 = OpFUnordNotEqual %bool %54 %65
               OpBranch %75
         %75 = OpLabel
         %77 = OpPhi %bool %false %19 %76 %74
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
         %80 = OpCompositeConstruct %v3float %54 %54 %54
         %81 = OpFSub %v3float %63 %80
         %82 = OpFSub %float %54 %65
         %84 = OpFAdd %float %82 %float_6_10351562en05
         %85 = OpLoad %float %_kGuardedDivideEpsilon
         %86 = OpFAdd %float %84 %85
         %87 = OpFDiv %float %54 %86
         %88 = OpVectorTimesScalar %v3float %81 %87
         %89 = OpFAdd %v3float %80 %88
               OpStore %_6_result %89
               OpBranch %79
         %79 = OpLabel
         %90 = OpFOrdGreaterThan %bool %71 %32
               OpSelectionMerge %92 None
               OpBranchConditional %90 %91 %92
         %91 = OpLabel
         %93 = OpFUnordNotEqual %bool %71 %54
               OpBranch %92
         %92 = OpLabel
         %94 = OpPhi %bool %false %79 %93 %91
               OpSelectionMerge %96 None
               OpBranchConditional %94 %95 %96
         %95 = OpLabel
         %97 = OpLoad %v3float %_6_result
         %98 = OpCompositeConstruct %v3float %54 %54 %54
         %99 = OpFSub %v3float %97 %98
        %100 = OpFSub %float %32 %54
        %101 = OpVectorTimesScalar %v3float %99 %100
        %102 = OpFSub %float %71 %54
        %103 = OpFAdd %float %102 %float_6_10351562en05
        %104 = OpLoad %float %_kGuardedDivideEpsilon
        %105 = OpFAdd %float %103 %104
        %107 = OpFDiv %float %float_1 %105
        %108 = OpVectorTimesScalar %v3float %101 %107
        %109 = OpFAdd %v3float %98 %108
               OpStore %_6_result %109
               OpBranch %96
         %96 = OpLabel
        %110 = OpLoad %v3float %_6_result
        %111 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %112 = OpLoad %v4float %111
        %113 = OpVectorShuffle %v3float %112 %112 0 1 2
        %114 = OpFAdd %v3float %110 %113
        %115 = OpFSub %v3float %114 %50
        %116 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %117 = OpLoad %v4float %116
        %118 = OpVectorShuffle %v3float %117 %117 0 1 2
        %119 = OpFAdd %v3float %115 %118
        %120 = OpFSub %v3float %119 %42
        %121 = OpCompositeExtract %float %120 0
        %122 = OpCompositeExtract %float %120 1
        %123 = OpCompositeExtract %float %120 2
        %124 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %125 = OpLoad %v4float %124
        %126 = OpCompositeExtract %float %125 3
        %127 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %128 = OpLoad %v4float %127
        %129 = OpCompositeExtract %float %128 3
        %130 = OpFAdd %float %126 %129
        %131 = OpFSub %float %130 %32
        %132 = OpCompositeConstruct %v4float %121 %122 %123 %131
               OpStore %sk_FragColor %132
               OpReturn
               OpFunctionEnd
