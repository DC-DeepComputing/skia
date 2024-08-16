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
               OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
               OpName %dxScale "dxScale"
               OpName %delta "delta"
               OpName %main "main"
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
               OpDecorate %dxScale RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
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
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
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
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
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
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
       %void = OpTypeVoid
        %100 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%color_dodge_component_Qhh2h2 = OpFunction %float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpFunctionParameter %_ptr_Function_v2float
         %23 = OpLabel
    %dxScale = OpVariable %_ptr_Function_float Function
      %delta = OpVariable %_ptr_Function_float Function
         %26 = OpLoad %v2float %22
         %27 = OpCompositeExtract %float %26 0
         %28 = OpFUnordNotEqual %bool %27 %float_0
         %29 = OpSelect %float %28 %float_1 %float_0
               OpStore %dxScale %29
         %33 = OpLoad %v2float %22
         %34 = OpCompositeExtract %float %33 1
         %36 = OpLoad %v2float %22
         %37 = OpCompositeExtract %float %36 1
         %38 = OpLoad %v2float %22
         %39 = OpCompositeExtract %float %38 0
         %40 = OpLoad %v2float %21
         %41 = OpCompositeExtract %float %40 1
         %42 = OpFMul %float %39 %41
         %43 = OpLoad %v2float %21
         %44 = OpCompositeExtract %float %43 1
         %45 = OpLoad %v2float %21
         %46 = OpCompositeExtract %float %45 0
         %47 = OpFSub %float %44 %46
         %48 = OpLoad %float %_kGuardedDivideEpsilon
         %49 = OpFAdd %float %47 %48
         %50 = OpFDiv %float %42 %49
         %52 = OpLoad %v2float %21
         %53 = OpCompositeExtract %float %52 1
         %54 = OpLoad %v2float %21
         %55 = OpCompositeExtract %float %54 0
         %56 = OpFSub %float %53 %55
         %51 = OpExtInst %float %1 FAbs %56
         %58 = OpFOrdGreaterThanEqual %bool %51 %float_6_10351562en05
         %59 = OpLoad %v2float %22
         %60 = OpCompositeExtract %float %59 1
         %61 = OpLoad %v2float %22
         %62 = OpCompositeExtract %float %61 0
         %63 = OpLoad %v2float %21
         %64 = OpCompositeExtract %float %63 1
         %65 = OpFMul %float %62 %64
         %66 = OpLoad %v2float %21
         %67 = OpCompositeExtract %float %66 1
         %68 = OpLoad %v2float %21
         %69 = OpCompositeExtract %float %68 0
         %70 = OpFSub %float %67 %69
         %71 = OpLoad %float %_kGuardedDivideEpsilon
         %72 = OpFAdd %float %70 %71
         %73 = OpFDiv %float %65 %72
         %75 = OpLoad %v2float %21
         %76 = OpCompositeExtract %float %75 1
         %77 = OpLoad %v2float %21
         %78 = OpCompositeExtract %float %77 0
         %79 = OpFSub %float %76 %78
         %74 = OpExtInst %float %1 FAbs %79
         %80 = OpFOrdGreaterThanEqual %bool %74 %float_6_10351562en05
         %35 = OpSelect %float %80 %73 %60
         %32 = OpExtInst %float %1 FMin %34 %35
         %81 = OpFMul %float %29 %32
               OpStore %delta %81
         %82 = OpLoad %v2float %21
         %83 = OpCompositeExtract %float %82 1
         %84 = OpFMul %float %81 %83
         %85 = OpLoad %v2float %21
         %86 = OpCompositeExtract %float %85 0
         %87 = OpLoad %v2float %22
         %88 = OpCompositeExtract %float %87 1
         %89 = OpFSub %float %float_1 %88
         %90 = OpFMul %float %86 %89
         %91 = OpFAdd %float %84 %90
         %92 = OpLoad %v2float %22
         %93 = OpCompositeExtract %float %92 0
         %94 = OpLoad %v2float %21
         %95 = OpCompositeExtract %float %94 1
         %96 = OpFSub %float %float_1 %95
         %97 = OpFMul %float %93 %96
         %98 = OpFAdd %float %91 %97
               OpReturnValue %98
               OpFunctionEnd
       %main = OpFunction %void None %100
        %101 = OpLabel
        %108 = OpVariable %_ptr_Function_v2float Function
        %113 = OpVariable %_ptr_Function_v2float Function
        %118 = OpVariable %_ptr_Function_v2float Function
        %122 = OpVariable %_ptr_Function_v2float Function
        %127 = OpVariable %_ptr_Function_v2float Function
        %131 = OpVariable %_ptr_Function_v2float Function
          %9 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %9
        %102 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %106 = OpLoad %v4float %102
        %107 = OpVectorShuffle %v2float %106 %106 0 3
               OpStore %108 %107
        %109 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %111 = OpLoad %v4float %109
        %112 = OpVectorShuffle %v2float %111 %111 0 3
               OpStore %113 %112
        %114 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %108 %113
        %115 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %116 = OpLoad %v4float %115
        %117 = OpVectorShuffle %v2float %116 %116 1 3
               OpStore %118 %117
        %119 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %120 = OpLoad %v4float %119
        %121 = OpVectorShuffle %v2float %120 %120 1 3
               OpStore %122 %121
        %123 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %118 %122
        %124 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %125 = OpLoad %v4float %124
        %126 = OpVectorShuffle %v2float %125 %125 2 3
               OpStore %127 %126
        %128 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %129 = OpLoad %v4float %128
        %130 = OpVectorShuffle %v2float %129 %129 2 3
               OpStore %131 %130
        %132 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %127 %131
        %133 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %134 = OpLoad %v4float %133
        %135 = OpCompositeExtract %float %134 3
        %136 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %137 = OpLoad %v4float %136
        %138 = OpCompositeExtract %float %137 3
        %139 = OpFSub %float %float_1 %138
        %140 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %141 = OpLoad %v4float %140
        %142 = OpCompositeExtract %float %141 3
        %143 = OpFMul %float %139 %142
        %144 = OpFAdd %float %135 %143
        %145 = OpCompositeConstruct %v4float %114 %123 %132 %144
               OpStore %sk_FragColor %145
               OpReturn
               OpFunctionEnd
