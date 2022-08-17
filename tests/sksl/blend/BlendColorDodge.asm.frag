OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
OpName %delta "delta"
OpName %blend_color_dodge_h4h4h4 "blend_color_dodge_h4h4h4"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
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
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
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
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
%17 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%102 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%138 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%guarded_divide_Qhhh = OpFunction %float None %17
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%21 = OpLoad %float %18
%22 = OpLoad %float %19
%23 = OpFDiv %float %21 %22
OpReturnValue %23
OpFunctionEnd
%color_dodge_component_Qhh2h2 = OpFunction %float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpFunctionParameter %_ptr_Function_v2float
%29 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%81 = OpVariable %_ptr_Function_float Function
%82 = OpVariable %_ptr_Function_float Function
%30 = OpLoad %v2float %28
%31 = OpCompositeExtract %float %30 0
%33 = OpFOrdEqual %bool %31 %float_0
OpSelectionMerge %36 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%37 = OpLoad %v2float %27
%38 = OpCompositeExtract %float %37 0
%40 = OpLoad %v2float %28
%41 = OpCompositeExtract %float %40 1
%42 = OpFSub %float %float_1 %41
%43 = OpFMul %float %38 %42
OpReturnValue %43
%35 = OpLabel
%45 = OpLoad %v2float %27
%46 = OpCompositeExtract %float %45 1
%47 = OpLoad %v2float %27
%48 = OpCompositeExtract %float %47 0
%49 = OpFSub %float %46 %48
OpStore %delta %49
%50 = OpFOrdEqual %bool %49 %float_0
OpSelectionMerge %53 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%54 = OpLoad %v2float %27
%55 = OpCompositeExtract %float %54 1
%56 = OpLoad %v2float %28
%57 = OpCompositeExtract %float %56 1
%58 = OpFMul %float %55 %57
%59 = OpLoad %v2float %27
%60 = OpCompositeExtract %float %59 0
%61 = OpLoad %v2float %28
%62 = OpCompositeExtract %float %61 1
%63 = OpFSub %float %float_1 %62
%64 = OpFMul %float %60 %63
%65 = OpFAdd %float %58 %64
%66 = OpLoad %v2float %28
%67 = OpCompositeExtract %float %66 0
%68 = OpLoad %v2float %27
%69 = OpCompositeExtract %float %68 1
%70 = OpFSub %float %float_1 %69
%71 = OpFMul %float %67 %70
%72 = OpFAdd %float %65 %71
OpReturnValue %72
%52 = OpLabel
%74 = OpLoad %v2float %28
%75 = OpCompositeExtract %float %74 1
%76 = OpLoad %v2float %28
%77 = OpCompositeExtract %float %76 0
%78 = OpLoad %v2float %27
%79 = OpCompositeExtract %float %78 1
%80 = OpFMul %float %77 %79
OpStore %81 %80
OpStore %82 %49
%83 = OpFunctionCall %float %guarded_divide_Qhhh %81 %82
%73 = OpExtInst %float %1 FMin %75 %83
OpStore %delta %73
%84 = OpLoad %v2float %27
%85 = OpCompositeExtract %float %84 1
%86 = OpFMul %float %73 %85
%87 = OpLoad %v2float %27
%88 = OpCompositeExtract %float %87 0
%89 = OpLoad %v2float %28
%90 = OpCompositeExtract %float %89 1
%91 = OpFSub %float %float_1 %90
%92 = OpFMul %float %88 %91
%93 = OpFAdd %float %86 %92
%94 = OpLoad %v2float %28
%95 = OpCompositeExtract %float %94 0
%96 = OpLoad %v2float %27
%97 = OpCompositeExtract %float %96 1
%98 = OpFSub %float %float_1 %97
%99 = OpFMul %float %95 %98
%100 = OpFAdd %float %93 %99
OpReturnValue %100
%53 = OpLabel
OpBranch %36
%36 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_color_dodge_h4h4h4 = OpFunction %v4float None %102
%103 = OpFunctionParameter %_ptr_Function_v4float
%104 = OpFunctionParameter %_ptr_Function_v4float
%105 = OpLabel
%108 = OpVariable %_ptr_Function_v2float Function
%111 = OpVariable %_ptr_Function_v2float Function
%115 = OpVariable %_ptr_Function_v2float Function
%118 = OpVariable %_ptr_Function_v2float Function
%122 = OpVariable %_ptr_Function_v2float Function
%125 = OpVariable %_ptr_Function_v2float Function
%106 = OpLoad %v4float %103
%107 = OpVectorShuffle %v2float %106 %106 0 3
OpStore %108 %107
%109 = OpLoad %v4float %104
%110 = OpVectorShuffle %v2float %109 %109 0 3
OpStore %111 %110
%112 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %108 %111
%113 = OpLoad %v4float %103
%114 = OpVectorShuffle %v2float %113 %113 1 3
OpStore %115 %114
%116 = OpLoad %v4float %104
%117 = OpVectorShuffle %v2float %116 %116 1 3
OpStore %118 %117
%119 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %115 %118
%120 = OpLoad %v4float %103
%121 = OpVectorShuffle %v2float %120 %120 2 3
OpStore %122 %121
%123 = OpLoad %v4float %104
%124 = OpVectorShuffle %v2float %123 %123 2 3
OpStore %125 %124
%126 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %122 %125
%127 = OpLoad %v4float %103
%128 = OpCompositeExtract %float %127 3
%129 = OpLoad %v4float %103
%130 = OpCompositeExtract %float %129 3
%131 = OpFSub %float %float_1 %130
%132 = OpLoad %v4float %104
%133 = OpCompositeExtract %float %132 3
%134 = OpFMul %float %131 %133
%135 = OpFAdd %float %128 %134
%136 = OpCompositeConstruct %v4float %112 %119 %126 %135
OpReturnValue %136
OpFunctionEnd
%main = OpFunction %void None %138
%139 = OpLabel
%145 = OpVariable %_ptr_Function_v4float Function
%149 = OpVariable %_ptr_Function_v4float Function
%140 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%144 = OpLoad %v4float %140
OpStore %145 %144
%146 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%148 = OpLoad %v4float %146
OpStore %149 %148
%150 = OpFunctionCall %v4float %blend_color_dodge_h4h4h4 %145 %149
OpStore %sk_FragColor %150
OpReturn
OpFunctionEnd
