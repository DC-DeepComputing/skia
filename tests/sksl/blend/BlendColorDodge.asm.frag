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
               OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
               OpName %dxScale "dxScale"
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
               OpDecorate %dxScale RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
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
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
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
    %float_0 = OpConstant %float 0
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
       %void = OpTypeVoid
         %90 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%color_dodge_component_Qhh2h2 = OpFunction %float None %24
         %25 = OpFunctionParameter %_ptr_Function_v2float
         %26 = OpFunctionParameter %_ptr_Function_v2float
         %27 = OpLabel
    %dxScale = OpVariable %_ptr_Function_float Function
      %delta = OpVariable %_ptr_Function_float Function
         %50 = OpVariable %_ptr_Function_float Function
         %59 = OpVariable %_ptr_Function_float Function
         %65 = OpVariable %_ptr_Function_float Function
         %29 = OpLoad %v2float %26
         %30 = OpCompositeExtract %float %29 0
         %32 = OpFOrdEqual %bool %30 %float_0
         %34 = OpSelect %int %32 %int_0 %int_1
         %38 = OpConvertSToF %float %34
               OpStore %dxScale %38
         %41 = OpLoad %v2float %26
         %42 = OpCompositeExtract %float %41 1
         %44 = OpLoad %v2float %25
         %45 = OpCompositeExtract %float %44 1
         %46 = OpLoad %v2float %25
         %47 = OpCompositeExtract %float %46 0
         %48 = OpFSub %float %45 %47
         %43 = OpExtInst %float %1 FAbs %48
         %49 = OpFOrdGreaterThanEqual %bool %43 %float_6_10351562en05
               OpSelectionMerge %53 None
               OpBranchConditional %49 %51 %52
         %51 = OpLabel
         %54 = OpLoad %v2float %26
         %55 = OpCompositeExtract %float %54 0
         %56 = OpLoad %v2float %25
         %57 = OpCompositeExtract %float %56 1
         %58 = OpFMul %float %55 %57
               OpStore %59 %58
         %60 = OpLoad %v2float %25
         %61 = OpCompositeExtract %float %60 1
         %62 = OpLoad %v2float %25
         %63 = OpCompositeExtract %float %62 0
         %64 = OpFSub %float %61 %63
               OpStore %65 %64
         %66 = OpFunctionCall %float %guarded_divide_Qhhh %59 %65
               OpStore %50 %66
               OpBranch %53
         %52 = OpLabel
         %67 = OpLoad %v2float %26
         %68 = OpCompositeExtract %float %67 1
               OpStore %50 %68
               OpBranch %53
         %53 = OpLabel
         %69 = OpLoad %float %50
         %40 = OpExtInst %float %1 FMin %42 %69
         %70 = OpFMul %float %38 %40
               OpStore %delta %70
         %71 = OpLoad %v2float %25
         %72 = OpCompositeExtract %float %71 1
         %73 = OpFMul %float %70 %72
         %74 = OpLoad %v2float %25
         %75 = OpCompositeExtract %float %74 0
         %77 = OpLoad %v2float %26
         %78 = OpCompositeExtract %float %77 1
         %79 = OpFSub %float %float_1 %78
         %80 = OpFMul %float %75 %79
         %81 = OpFAdd %float %73 %80
         %82 = OpLoad %v2float %26
         %83 = OpCompositeExtract %float %82 0
         %84 = OpLoad %v2float %25
         %85 = OpCompositeExtract %float %84 1
         %86 = OpFSub %float %float_1 %85
         %87 = OpFMul %float %83 %86
         %88 = OpFAdd %float %81 %87
               OpReturnValue %88
               OpFunctionEnd
       %main = OpFunction %void None %90
         %91 = OpLabel
         %96 = OpVariable %_ptr_Function_v2float Function
        %100 = OpVariable %_ptr_Function_v2float Function
        %105 = OpVariable %_ptr_Function_v2float Function
        %109 = OpVariable %_ptr_Function_v2float Function
        %114 = OpVariable %_ptr_Function_v2float Function
        %118 = OpVariable %_ptr_Function_v2float Function
         %92 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %94 = OpLoad %v4float %92
         %95 = OpVectorShuffle %v2float %94 %94 0 3
               OpStore %96 %95
         %97 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
         %98 = OpLoad %v4float %97
         %99 = OpVectorShuffle %v2float %98 %98 0 3
               OpStore %100 %99
        %101 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %96 %100
        %102 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %103 = OpLoad %v4float %102
        %104 = OpVectorShuffle %v2float %103 %103 1 3
               OpStore %105 %104
        %106 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %107 = OpLoad %v4float %106
        %108 = OpVectorShuffle %v2float %107 %107 1 3
               OpStore %109 %108
        %110 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %105 %109
        %111 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %112 = OpLoad %v4float %111
        %113 = OpVectorShuffle %v2float %112 %112 2 3
               OpStore %114 %113
        %115 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %116 = OpLoad %v4float %115
        %117 = OpVectorShuffle %v2float %116 %116 2 3
               OpStore %118 %117
        %119 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %114 %118
        %120 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %121 = OpLoad %v4float %120
        %122 = OpCompositeExtract %float %121 3
        %123 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %124 = OpLoad %v4float %123
        %125 = OpCompositeExtract %float %124 3
        %126 = OpFSub %float %float_1 %125
        %127 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %128 = OpLoad %v4float %127
        %129 = OpCompositeExtract %float %128 3
        %130 = OpFMul %float %126 %129
        %131 = OpFAdd %float %122 %130
        %132 = OpCompositeConstruct %v4float %101 %110 %119 %131
               OpStore %sk_FragColor %132
               OpReturn
               OpFunctionEnd
