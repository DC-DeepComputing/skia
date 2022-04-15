OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %fn_hh4 "fn_hh4"
OpName %x "x"
OpName %main "main"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %v RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
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
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%25 = OpTypeFunction %float %_ptr_Function_v4float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%44 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%102 = OpConstantComposite %v4float %float_1 %float_1 %float_2 %float_3
%int_0 = OpConstant %int 0
%131 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%fn_hh4 = OpFunction %float None %25
%26 = OpFunctionParameter %_ptr_Function_v4float
%27 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_1
OpBranch %32
%32 = OpLabel
OpLoopMerge %36 %35 None
OpBranch %33
%33 = OpLabel
%37 = OpLoad %int %x
%39 = OpSLessThanEqual %bool %37 %int_2
OpBranchConditional %39 %34 %36
%34 = OpLabel
%40 = OpLoad %v4float %26
%41 = OpCompositeExtract %float %40 0
OpReturnValue %41
%35 = OpLabel
%42 = OpLoad %int %x
%43 = OpIAdd %int %42 %int_1
OpStore %x %43
OpBranch %32
%36 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %44
%45 = OpFunctionParameter %_ptr_Function_v2float
%46 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%76 = OpVariable %_ptr_Function_v4float Function
%82 = OpVariable %_ptr_Function_v4float Function
%86 = OpVariable %_ptr_Function_v4float Function
%89 = OpVariable %_ptr_Function_v4float Function
%92 = OpVariable %_ptr_Function_v4float Function
%96 = OpVariable %_ptr_Function_v4float Function
%135 = OpVariable %_ptr_Function_v4float Function
%48 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%50 = OpLoad %v4float %48
OpStore %v %50
%51 = OpLoad %v4float %v
%52 = OpVectorShuffle %v3float %51 %51 2 1 0
%54 = OpCompositeExtract %float %52 0
%55 = OpCompositeExtract %float %52 1
%56 = OpCompositeExtract %float %52 2
%57 = OpCompositeConstruct %v4float %float_0 %54 %55 %56
OpStore %v %57
%58 = OpVectorShuffle %v2float %57 %57 0 3
%59 = OpCompositeExtract %float %58 0
%60 = OpCompositeExtract %float %58 1
%61 = OpCompositeConstruct %v4float %float_0 %float_0 %59 %60
OpStore %v %61
%63 = OpVectorShuffle %v2float %61 %61 3 0
%64 = OpCompositeExtract %float %63 0
%65 = OpCompositeExtract %float %63 1
%66 = OpCompositeConstruct %v4float %float_1 %float_1 %64 %65
OpStore %v %66
%67 = OpVectorShuffle %v2float %66 %66 2 1
%68 = OpCompositeExtract %float %67 0
%69 = OpCompositeExtract %float %67 1
%70 = OpCompositeConstruct %v4float %68 %69 %float_1 %float_1
OpStore %v %70
%71 = OpVectorShuffle %v2float %70 %70 0 0
%72 = OpCompositeExtract %float %71 0
%73 = OpCompositeExtract %float %71 1
%74 = OpCompositeConstruct %v4float %72 %73 %float_1 %float_1
OpStore %v %74
%75 = OpVectorShuffle %v4float %74 %74 3 2 3 2
OpStore %v %75
OpStore %76 %75
%77 = OpFunctionCall %float %fn_hh4 %76
%80 = OpCompositeConstruct %v3float %77 %float_123 %float_456
%81 = OpVectorShuffle %v4float %80 %80 1 1 2 2
OpStore %v %81
OpStore %82 %81
%83 = OpFunctionCall %float %fn_hh4 %82
%84 = OpCompositeConstruct %v3float %83 %float_123 %float_456
%85 = OpVectorShuffle %v4float %84 %84 1 1 2 2
OpStore %v %85
OpStore %86 %85
%87 = OpFunctionCall %float %fn_hh4 %86
%88 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %87
OpStore %v %88
OpStore %89 %88
%90 = OpFunctionCall %float %fn_hh4 %89
%91 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %90
OpStore %v %91
OpStore %92 %91
%93 = OpFunctionCall %float %fn_hh4 %92
%94 = OpCompositeConstruct %v3float %93 %float_123 %float_456
%95 = OpVectorShuffle %v4float %94 %94 1 0 0 2
OpStore %v %95
OpStore %96 %95
%97 = OpFunctionCall %float %fn_hh4 %96
%98 = OpCompositeConstruct %v3float %97 %float_123 %float_456
%99 = OpVectorShuffle %v4float %98 %98 1 0 0 2
OpStore %v %99
OpStore %v %102
%103 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%105 = OpLoad %v4float %103
%106 = OpVectorShuffle %v3float %105 %105 0 1 2
%107 = OpCompositeExtract %float %106 0
%108 = OpCompositeExtract %float %106 1
%109 = OpCompositeExtract %float %106 2
%110 = OpCompositeConstruct %v4float %107 %108 %109 %float_1
OpStore %v %110
%111 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%112 = OpLoad %v4float %111
%113 = OpCompositeExtract %float %112 0
%114 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%115 = OpLoad %v4float %114
%116 = OpVectorShuffle %v2float %115 %115 1 2
%117 = OpCompositeExtract %float %116 0
%118 = OpCompositeExtract %float %116 1
%119 = OpCompositeConstruct %v4float %113 %float_1 %117 %118
OpStore %v %119
%120 = OpLoad %v4float %v
%121 = OpVectorShuffle %v4float %120 %119 7 6 5 4
OpStore %v %121
%122 = OpVectorShuffle %v2float %119 %119 1 2
%123 = OpLoad %v4float %v
%124 = OpVectorShuffle %v4float %123 %122 4 1 2 5
OpStore %v %124
%125 = OpVectorShuffle %v2float %119 %119 3 3
%126 = OpCompositeExtract %float %125 0
%127 = OpCompositeExtract %float %125 1
%128 = OpCompositeConstruct %v3float %126 %127 %float_1
%129 = OpLoad %v4float %v
%130 = OpVectorShuffle %v4float %129 %128 6 5 4 3
OpStore %v %130
%132 = OpFOrdEqual %v4bool %119 %131
%134 = OpAll %bool %132
OpSelectionMerge %138 None
OpBranchConditional %134 %136 %137
%136 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%140 = OpLoad %v4float %139
OpStore %135 %140
OpBranch %138
%137 = OpLabel
%141 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%142 = OpLoad %v4float %141
OpStore %135 %142
OpBranch %138
%138 = OpLabel
%143 = OpLoad %v4float %135
OpReturnValue %143
OpFunctionEnd
