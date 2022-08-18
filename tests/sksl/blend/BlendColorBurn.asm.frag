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
OpName %color_burn_component_Qhh2h2 "color_burn_component_Qhh2h2"
OpName %delta "delta"
OpName %blend_color_burn_h4h4h4 "blend_color_burn_h4h4h4"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
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
OpDecorate %delta RelaxedPrecision
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
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
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
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%107 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%143 = OpTypeFunction %void
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
%color_burn_component_Qhh2h2 = OpFunction %float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpFunctionParameter %_ptr_Function_v2float
%29 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%83 = OpVariable %_ptr_Function_float Function
%86 = OpVariable %_ptr_Function_float Function
%30 = OpLoad %v2float %28
%31 = OpCompositeExtract %float %30 1
%32 = OpLoad %v2float %28
%33 = OpCompositeExtract %float %32 0
%34 = OpFOrdEqual %bool %31 %33
OpSelectionMerge %37 None
OpBranchConditional %34 %35 %36
%35 = OpLabel
%38 = OpLoad %v2float %27
%39 = OpCompositeExtract %float %38 1
%40 = OpLoad %v2float %28
%41 = OpCompositeExtract %float %40 1
%42 = OpFMul %float %39 %41
%43 = OpLoad %v2float %27
%44 = OpCompositeExtract %float %43 0
%46 = OpLoad %v2float %28
%47 = OpCompositeExtract %float %46 1
%48 = OpFSub %float %float_1 %47
%49 = OpFMul %float %44 %48
%50 = OpFAdd %float %42 %49
%51 = OpLoad %v2float %28
%52 = OpCompositeExtract %float %51 0
%53 = OpLoad %v2float %27
%54 = OpCompositeExtract %float %53 1
%55 = OpFSub %float %float_1 %54
%56 = OpFMul %float %52 %55
%57 = OpFAdd %float %50 %56
OpReturnValue %57
%36 = OpLabel
%58 = OpLoad %v2float %27
%59 = OpCompositeExtract %float %58 0
%61 = OpFOrdEqual %bool %59 %float_0
OpSelectionMerge %64 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpLoad %v2float %28
%66 = OpCompositeExtract %float %65 0
%67 = OpLoad %v2float %27
%68 = OpCompositeExtract %float %67 1
%69 = OpFSub %float %float_1 %68
%70 = OpFMul %float %66 %69
OpReturnValue %70
%63 = OpLabel
%73 = OpLoad %v2float %28
%74 = OpCompositeExtract %float %73 1
%75 = OpLoad %v2float %28
%76 = OpCompositeExtract %float %75 1
%77 = OpLoad %v2float %28
%78 = OpCompositeExtract %float %77 0
%79 = OpFSub %float %76 %78
%80 = OpLoad %v2float %27
%81 = OpCompositeExtract %float %80 1
%82 = OpFMul %float %79 %81
OpStore %83 %82
%84 = OpLoad %v2float %27
%85 = OpCompositeExtract %float %84 0
OpStore %86 %85
%87 = OpFunctionCall %float %guarded_divide_Qhhh %83 %86
%88 = OpFSub %float %74 %87
%72 = OpExtInst %float %1 FMax %float_0 %88
OpStore %delta %72
%89 = OpLoad %v2float %27
%90 = OpCompositeExtract %float %89 1
%91 = OpFMul %float %72 %90
%92 = OpLoad %v2float %27
%93 = OpCompositeExtract %float %92 0
%94 = OpLoad %v2float %28
%95 = OpCompositeExtract %float %94 1
%96 = OpFSub %float %float_1 %95
%97 = OpFMul %float %93 %96
%98 = OpFAdd %float %91 %97
%99 = OpLoad %v2float %28
%100 = OpCompositeExtract %float %99 0
%101 = OpLoad %v2float %27
%102 = OpCompositeExtract %float %101 1
%103 = OpFSub %float %float_1 %102
%104 = OpFMul %float %100 %103
%105 = OpFAdd %float %98 %104
OpReturnValue %105
%64 = OpLabel
OpBranch %37
%37 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_color_burn_h4h4h4 = OpFunction %v4float None %107
%108 = OpFunctionParameter %_ptr_Function_v4float
%109 = OpFunctionParameter %_ptr_Function_v4float
%110 = OpLabel
%113 = OpVariable %_ptr_Function_v2float Function
%116 = OpVariable %_ptr_Function_v2float Function
%120 = OpVariable %_ptr_Function_v2float Function
%123 = OpVariable %_ptr_Function_v2float Function
%127 = OpVariable %_ptr_Function_v2float Function
%130 = OpVariable %_ptr_Function_v2float Function
%111 = OpLoad %v4float %108
%112 = OpVectorShuffle %v2float %111 %111 0 3
OpStore %113 %112
%114 = OpLoad %v4float %109
%115 = OpVectorShuffle %v2float %114 %114 0 3
OpStore %116 %115
%117 = OpFunctionCall %float %color_burn_component_Qhh2h2 %113 %116
%118 = OpLoad %v4float %108
%119 = OpVectorShuffle %v2float %118 %118 1 3
OpStore %120 %119
%121 = OpLoad %v4float %109
%122 = OpVectorShuffle %v2float %121 %121 1 3
OpStore %123 %122
%124 = OpFunctionCall %float %color_burn_component_Qhh2h2 %120 %123
%125 = OpLoad %v4float %108
%126 = OpVectorShuffle %v2float %125 %125 2 3
OpStore %127 %126
%128 = OpLoad %v4float %109
%129 = OpVectorShuffle %v2float %128 %128 2 3
OpStore %130 %129
%131 = OpFunctionCall %float %color_burn_component_Qhh2h2 %127 %130
%132 = OpLoad %v4float %108
%133 = OpCompositeExtract %float %132 3
%134 = OpLoad %v4float %108
%135 = OpCompositeExtract %float %134 3
%136 = OpFSub %float %float_1 %135
%137 = OpLoad %v4float %109
%138 = OpCompositeExtract %float %137 3
%139 = OpFMul %float %136 %138
%140 = OpFAdd %float %133 %139
%141 = OpCompositeConstruct %v4float %117 %124 %131 %140
OpReturnValue %141
OpFunctionEnd
%main = OpFunction %void None %143
%144 = OpLabel
%150 = OpVariable %_ptr_Function_v4float Function
%154 = OpVariable %_ptr_Function_v4float Function
%145 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%149 = OpLoad %v4float %145
OpStore %150 %149
%151 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%153 = OpLoad %v4float %151
OpStore %154 %153
%155 = OpFunctionCall %v4float %blend_color_burn_h4h4h4 %150 %154
OpStore %sk_FragColor %155
OpReturn
OpFunctionEnd
