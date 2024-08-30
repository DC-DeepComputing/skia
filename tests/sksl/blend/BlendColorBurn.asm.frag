               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
               OpName %color_burn_component_Qhh2h2 "color_burn_component_Qhh2h2"
               OpName %dyTerm "dyTerm"
               OpName %delta "delta"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %17 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %dyTerm RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
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
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
         %13 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
       %bool = OpTypeBool
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
       %void = OpTypeVoid
         %91 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%guarded_divide_Qhhh = OpFunction %float None %13
         %14 = OpFunctionParameter %_ptr_Function_float
         %15 = OpFunctionParameter %_ptr_Function_float
         %16 = OpLabel
         %17 = OpLoad %float %14
         %18 = OpLoad %float %15
         %20 = OpFAdd %float %18 %float_6_10351562en05
         %21 = OpFDiv %float %17 %20
               OpReturnValue %21
               OpFunctionEnd
%color_burn_component_Qhh2h2 = OpFunction %float None %24
         %25 = OpFunctionParameter %_ptr_Function_v2float
         %26 = OpFunctionParameter %_ptr_Function_v2float
         %27 = OpLabel
     %dyTerm = OpVariable %_ptr_Function_float Function
         %35 = OpVariable %_ptr_Function_float Function
      %delta = OpVariable %_ptr_Function_float Function
         %48 = OpVariable %_ptr_Function_float Function
         %65 = OpVariable %_ptr_Function_float Function
         %68 = OpVariable %_ptr_Function_float Function
         %29 = OpLoad %v2float %26
         %30 = OpCompositeExtract %float %29 1
         %31 = OpLoad %v2float %26
         %32 = OpCompositeExtract %float %31 0
         %33 = OpFOrdEqual %bool %30 %32
               OpSelectionMerge %38 None
               OpBranchConditional %33 %36 %37
         %36 = OpLabel
         %39 = OpLoad %v2float %26
         %40 = OpCompositeExtract %float %39 1
               OpStore %35 %40
               OpBranch %38
         %37 = OpLabel
               OpStore %35 %float_0
               OpBranch %38
         %38 = OpLabel
         %42 = OpLoad %float %35
               OpStore %dyTerm %42
         %45 = OpLoad %v2float %25
         %46 = OpCompositeExtract %float %45 0
         %44 = OpExtInst %float %1 FAbs %46
         %47 = OpFOrdGreaterThanEqual %bool %44 %float_6_10351562en05
               OpSelectionMerge %51 None
               OpBranchConditional %47 %49 %50
         %49 = OpLabel
         %52 = OpLoad %v2float %26
         %53 = OpCompositeExtract %float %52 1
         %55 = OpLoad %v2float %26
         %56 = OpCompositeExtract %float %55 1
         %57 = OpLoad %v2float %26
         %58 = OpCompositeExtract %float %57 1
         %59 = OpLoad %v2float %26
         %60 = OpCompositeExtract %float %59 0
         %61 = OpFSub %float %58 %60
         %62 = OpLoad %v2float %25
         %63 = OpCompositeExtract %float %62 1
         %64 = OpFMul %float %61 %63
               OpStore %65 %64
         %66 = OpLoad %v2float %25
         %67 = OpCompositeExtract %float %66 0
               OpStore %68 %67
         %69 = OpFunctionCall %float %guarded_divide_Qhhh %65 %68
         %54 = OpExtInst %float %1 FMin %56 %69
         %70 = OpFSub %float %53 %54
               OpStore %48 %70
               OpBranch %51
         %50 = OpLabel
               OpStore %48 %42
               OpBranch %51
         %51 = OpLabel
         %71 = OpLoad %float %48
               OpStore %delta %71
         %72 = OpLoad %v2float %25
         %73 = OpCompositeExtract %float %72 1
         %74 = OpFMul %float %71 %73
         %75 = OpLoad %v2float %25
         %76 = OpCompositeExtract %float %75 0
         %78 = OpLoad %v2float %26
         %79 = OpCompositeExtract %float %78 1
         %80 = OpFSub %float %float_1 %79
         %81 = OpFMul %float %76 %80
         %82 = OpFAdd %float %74 %81
         %83 = OpLoad %v2float %26
         %84 = OpCompositeExtract %float %83 0
         %85 = OpLoad %v2float %25
         %86 = OpCompositeExtract %float %85 1
         %87 = OpFSub %float %float_1 %86
         %88 = OpFMul %float %84 %87
         %89 = OpFAdd %float %82 %88
               OpReturnValue %89
               OpFunctionEnd
       %main = OpFunction %void None %91
         %92 = OpLabel
         %99 = OpVariable %_ptr_Function_v2float Function
        %104 = OpVariable %_ptr_Function_v2float Function
        %109 = OpVariable %_ptr_Function_v2float Function
        %113 = OpVariable %_ptr_Function_v2float Function
        %118 = OpVariable %_ptr_Function_v2float Function
        %122 = OpVariable %_ptr_Function_v2float Function
         %93 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %97 = OpLoad %v4float %93
         %98 = OpVectorShuffle %v2float %97 %97 0 3
               OpStore %99 %98
        %100 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %102 = OpLoad %v4float %100
        %103 = OpVectorShuffle %v2float %102 %102 0 3
               OpStore %104 %103
        %105 = OpFunctionCall %float %color_burn_component_Qhh2h2 %99 %104
        %106 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %107 = OpLoad %v4float %106
        %108 = OpVectorShuffle %v2float %107 %107 1 3
               OpStore %109 %108
        %110 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %111 = OpLoad %v4float %110
        %112 = OpVectorShuffle %v2float %111 %111 1 3
               OpStore %113 %112
        %114 = OpFunctionCall %float %color_burn_component_Qhh2h2 %109 %113
        %115 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %116 = OpLoad %v4float %115
        %117 = OpVectorShuffle %v2float %116 %116 2 3
               OpStore %118 %117
        %119 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %120 = OpLoad %v4float %119
        %121 = OpVectorShuffle %v2float %120 %120 2 3
               OpStore %122 %121
        %123 = OpFunctionCall %float %color_burn_component_Qhh2h2 %118 %122
        %124 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %125 = OpLoad %v4float %124
        %126 = OpCompositeExtract %float %125 3
        %127 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %128 = OpLoad %v4float %127
        %129 = OpCompositeExtract %float %128 3
        %130 = OpFSub %float %float_1 %129
        %131 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %132 = OpLoad %v4float %131
        %133 = OpCompositeExtract %float %132 3
        %134 = OpFMul %float %130 %133
        %135 = OpFAdd %float %126 %134
        %136 = OpCompositeConstruct %v4float %105 %114 %123 %135
               OpStore %sk_FragColor %136
               OpReturn
               OpFunctionEnd
