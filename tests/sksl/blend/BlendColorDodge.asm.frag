OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
OpName %delta "delta"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %20 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
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
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
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
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%16 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%93 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%color_dodge_component_Qhh2h2 = OpFunction %float None %16
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%20 = OpLoad %v2float %18
%21 = OpCompositeExtract %float %20 0
%23 = OpFOrdEqual %bool %21 %float_0
OpSelectionMerge %26 None
OpBranchConditional %23 %24 %25
%24 = OpLabel
%27 = OpLoad %v2float %17
%28 = OpCompositeExtract %float %27 0
%30 = OpLoad %v2float %18
%31 = OpCompositeExtract %float %30 1
%32 = OpFSub %float %float_1 %31
%33 = OpFMul %float %28 %32
OpReturnValue %33
%25 = OpLabel
%36 = OpLoad %v2float %17
%37 = OpCompositeExtract %float %36 1
%38 = OpLoad %v2float %17
%39 = OpCompositeExtract %float %38 0
%40 = OpFSub %float %37 %39
OpStore %delta %40
%41 = OpLoad %float %delta
%42 = OpFOrdEqual %bool %41 %float_0
OpSelectionMerge %45 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpLoad %v2float %17
%47 = OpCompositeExtract %float %46 1
%48 = OpLoad %v2float %18
%49 = OpCompositeExtract %float %48 1
%50 = OpFMul %float %47 %49
%51 = OpLoad %v2float %17
%52 = OpCompositeExtract %float %51 0
%53 = OpLoad %v2float %18
%54 = OpCompositeExtract %float %53 1
%55 = OpFSub %float %float_1 %54
%56 = OpFMul %float %52 %55
%57 = OpFAdd %float %50 %56
%58 = OpLoad %v2float %18
%59 = OpCompositeExtract %float %58 0
%60 = OpLoad %v2float %17
%61 = OpCompositeExtract %float %60 1
%62 = OpFSub %float %float_1 %61
%63 = OpFMul %float %59 %62
%64 = OpFAdd %float %57 %63
OpReturnValue %64
%44 = OpLabel
%66 = OpLoad %v2float %18
%67 = OpCompositeExtract %float %66 1
%68 = OpLoad %v2float %18
%69 = OpCompositeExtract %float %68 0
%70 = OpLoad %v2float %17
%71 = OpCompositeExtract %float %70 1
%72 = OpFMul %float %69 %71
%73 = OpLoad %float %delta
%74 = OpFDiv %float %72 %73
%65 = OpExtInst %float %1 FMin %67 %74
OpStore %delta %65
%75 = OpLoad %v2float %17
%76 = OpCompositeExtract %float %75 1
%77 = OpFMul %float %65 %76
%78 = OpLoad %v2float %17
%79 = OpCompositeExtract %float %78 0
%80 = OpLoad %v2float %18
%81 = OpCompositeExtract %float %80 1
%82 = OpFSub %float %float_1 %81
%83 = OpFMul %float %79 %82
%84 = OpFAdd %float %77 %83
%85 = OpLoad %v2float %18
%86 = OpCompositeExtract %float %85 0
%87 = OpLoad %v2float %17
%88 = OpCompositeExtract %float %87 1
%89 = OpFSub %float %float_1 %88
%90 = OpFMul %float %86 %89
%91 = OpFAdd %float %84 %90
OpReturnValue %91
%45 = OpLabel
OpBranch %26
%26 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %93
%94 = OpLabel
%101 = OpVariable %_ptr_Function_v2float Function
%106 = OpVariable %_ptr_Function_v2float Function
%111 = OpVariable %_ptr_Function_v2float Function
%115 = OpVariable %_ptr_Function_v2float Function
%120 = OpVariable %_ptr_Function_v2float Function
%124 = OpVariable %_ptr_Function_v2float Function
%95 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%99 = OpLoad %v4float %95
%100 = OpVectorShuffle %v2float %99 %99 0 3
OpStore %101 %100
%102 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%104 = OpLoad %v4float %102
%105 = OpVectorShuffle %v2float %104 %104 0 3
OpStore %106 %105
%107 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %101 %106
%108 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%109 = OpLoad %v4float %108
%110 = OpVectorShuffle %v2float %109 %109 1 3
OpStore %111 %110
%112 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%113 = OpLoad %v4float %112
%114 = OpVectorShuffle %v2float %113 %113 1 3
OpStore %115 %114
%116 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %111 %115
%117 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%118 = OpLoad %v4float %117
%119 = OpVectorShuffle %v2float %118 %118 2 3
OpStore %120 %119
%121 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%122 = OpLoad %v4float %121
%123 = OpVectorShuffle %v2float %122 %122 2 3
OpStore %124 %123
%125 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %120 %124
%126 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%127 = OpLoad %v4float %126
%128 = OpCompositeExtract %float %127 3
%129 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%130 = OpLoad %v4float %129
%131 = OpCompositeExtract %float %130 3
%132 = OpFSub %float %float_1 %131
%133 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%134 = OpLoad %v4float %133
%135 = OpCompositeExtract %float %134 3
%136 = OpFMul %float %132 %135
%137 = OpFAdd %float %128 %136
%138 = OpCompositeConstruct %v4float %107 %116 %125 %137
OpStore %sk_FragColor %138
OpReturn
OpFunctionEnd
