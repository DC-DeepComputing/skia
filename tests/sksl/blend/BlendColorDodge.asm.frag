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
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
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
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
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
%92 = OpTypeFunction %void
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
%41 = OpFOrdEqual %bool %40 %float_0
OpSelectionMerge %44 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%45 = OpLoad %v2float %17
%46 = OpCompositeExtract %float %45 1
%47 = OpLoad %v2float %18
%48 = OpCompositeExtract %float %47 1
%49 = OpFMul %float %46 %48
%50 = OpLoad %v2float %17
%51 = OpCompositeExtract %float %50 0
%52 = OpLoad %v2float %18
%53 = OpCompositeExtract %float %52 1
%54 = OpFSub %float %float_1 %53
%55 = OpFMul %float %51 %54
%56 = OpFAdd %float %49 %55
%57 = OpLoad %v2float %18
%58 = OpCompositeExtract %float %57 0
%59 = OpLoad %v2float %17
%60 = OpCompositeExtract %float %59 1
%61 = OpFSub %float %float_1 %60
%62 = OpFMul %float %58 %61
%63 = OpFAdd %float %56 %62
OpReturnValue %63
%43 = OpLabel
%65 = OpLoad %v2float %18
%66 = OpCompositeExtract %float %65 1
%67 = OpLoad %v2float %18
%68 = OpCompositeExtract %float %67 0
%69 = OpLoad %v2float %17
%70 = OpCompositeExtract %float %69 1
%71 = OpFMul %float %68 %70
%72 = OpLoad %float %delta
%73 = OpFDiv %float %71 %72
%64 = OpExtInst %float %1 FMin %66 %73
OpStore %delta %64
%74 = OpLoad %v2float %17
%75 = OpCompositeExtract %float %74 1
%76 = OpFMul %float %64 %75
%77 = OpLoad %v2float %17
%78 = OpCompositeExtract %float %77 0
%79 = OpLoad %v2float %18
%80 = OpCompositeExtract %float %79 1
%81 = OpFSub %float %float_1 %80
%82 = OpFMul %float %78 %81
%83 = OpFAdd %float %76 %82
%84 = OpLoad %v2float %18
%85 = OpCompositeExtract %float %84 0
%86 = OpLoad %v2float %17
%87 = OpCompositeExtract %float %86 1
%88 = OpFSub %float %float_1 %87
%89 = OpFMul %float %85 %88
%90 = OpFAdd %float %83 %89
OpReturnValue %90
%44 = OpLabel
OpBranch %26
%26 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %92
%93 = OpLabel
%100 = OpVariable %_ptr_Function_v2float Function
%105 = OpVariable %_ptr_Function_v2float Function
%110 = OpVariable %_ptr_Function_v2float Function
%114 = OpVariable %_ptr_Function_v2float Function
%119 = OpVariable %_ptr_Function_v2float Function
%123 = OpVariable %_ptr_Function_v2float Function
%94 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%98 = OpLoad %v4float %94
%99 = OpVectorShuffle %v2float %98 %98 0 3
OpStore %100 %99
%101 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%103 = OpLoad %v4float %101
%104 = OpVectorShuffle %v2float %103 %103 0 3
OpStore %105 %104
%106 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %100 %105
%107 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%108 = OpLoad %v4float %107
%109 = OpVectorShuffle %v2float %108 %108 1 3
OpStore %110 %109
%111 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%112 = OpLoad %v4float %111
%113 = OpVectorShuffle %v2float %112 %112 1 3
OpStore %114 %113
%115 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %110 %114
%116 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%117 = OpLoad %v4float %116
%118 = OpVectorShuffle %v2float %117 %117 2 3
OpStore %119 %118
%120 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%121 = OpLoad %v4float %120
%122 = OpVectorShuffle %v2float %121 %121 2 3
OpStore %123 %122
%124 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %119 %123
%125 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%126 = OpLoad %v4float %125
%127 = OpCompositeExtract %float %126 3
%128 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%129 = OpLoad %v4float %128
%130 = OpCompositeExtract %float %129 3
%131 = OpFSub %float %float_1 %130
%132 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%133 = OpLoad %v4float %132
%134 = OpCompositeExtract %float %133 3
%135 = OpFMul %float %131 %134
%136 = OpFAdd %float %127 %135
%137 = OpCompositeConstruct %v4float %106 %115 %124 %136
OpStore %sk_FragColor %137
OpReturn
OpFunctionEnd
