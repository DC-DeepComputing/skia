OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpMemberName %_UniformBuffer 3 "u_skRTFlip"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expected "expected"
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
OpMemberDecorate %_UniformBuffer 3 Offset 16384
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_3 = OpConstant %int 3
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1 = OpConstant %float 1
%121 = OpConstantComposite %v2float %float_1 %float_1
%135 = OpConstantComposite %v2float %float_0 %float_1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%expected = OpVariable %_ptr_Function_v4float Function
%139 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %29
%32 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 0
%31 = OpDPdy %float %37
%39 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%41 = OpLoad %v2float %39
%42 = OpCompositeExtract %float %41 1
%43 = OpFMul %float %31 %42
%44 = OpFOrdEqual %bool %43 %float_0
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v2float %49 %49 0 1
%47 = OpDPdy %v2float %50
%51 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%52 = OpLoad %v2float %51
%53 = OpCompositeExtract %float %52 1
%54 = OpCompositeConstruct %v2float %53 %53
%55 = OpFMul %v2float %47 %54
%56 = OpLoad %v4float %expected
%57 = OpVectorShuffle %v2float %56 %56 0 1
%58 = OpFOrdEqual %v2bool %55 %57
%60 = OpAll %bool %58
OpBranch %46
%46 = OpLabel
%61 = OpPhi %bool %false %26 %60 %45
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%66 = OpLoad %v4float %65
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%64 = OpDPdy %v3float %67
%69 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%70 = OpLoad %v2float %69
%71 = OpCompositeExtract %float %70 1
%72 = OpCompositeConstruct %v3float %71 %71 %71
%73 = OpFMul %v3float %64 %72
%74 = OpLoad %v4float %expected
%75 = OpVectorShuffle %v3float %74 %74 0 1 2
%76 = OpFOrdEqual %v3bool %73 %75
%78 = OpAll %bool %76
OpBranch %63
%63 = OpLabel
%79 = OpPhi %bool %false %46 %78 %62
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%84 = OpLoad %v4float %83
%82 = OpDPdy %v4float %84
%85 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%86 = OpLoad %v2float %85
%87 = OpCompositeExtract %float %86 1
%88 = OpCompositeConstruct %v4float %87 %87 %87 %87
%89 = OpFMul %v4float %82 %88
%90 = OpLoad %v4float %expected
%91 = OpFOrdEqual %v4bool %89 %90
%93 = OpAll %bool %91
OpBranch %81
%81 = OpLabel
%94 = OpPhi %bool %false %63 %93 %80
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%99 = OpLoad %v2float %25
%100 = OpVectorShuffle %v2float %99 %99 0 0
%98 = OpDPdy %v2float %100
%101 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%102 = OpLoad %v2float %101
%103 = OpCompositeExtract %float %102 1
%104 = OpCompositeConstruct %v2float %103 %103
%105 = OpFMul %v2float %98 %104
%97 = OpExtInst %v2float %1 FSign %105
%106 = OpFOrdEqual %v2bool %97 %20
%107 = OpAll %bool %106
OpBranch %96
%96 = OpLabel
%108 = OpPhi %bool %false %81 %107 %95
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%113 = OpLoad %v2float %25
%114 = OpVectorShuffle %v2float %113 %113 1 1
%112 = OpDPdy %v2float %114
%115 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%116 = OpLoad %v2float %115
%117 = OpCompositeExtract %float %116 1
%118 = OpCompositeConstruct %v2float %117 %117
%119 = OpFMul %v2float %112 %118
%111 = OpExtInst %v2float %1 FSign %119
%122 = OpFOrdEqual %v2bool %111 %121
%123 = OpAll %bool %122
OpBranch %110
%110 = OpLabel
%124 = OpPhi %bool %false %96 %123 %109
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%129 = OpLoad %v2float %25
%128 = OpDPdy %v2float %129
%130 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%131 = OpLoad %v2float %130
%132 = OpCompositeExtract %float %131 1
%133 = OpCompositeConstruct %v2float %132 %132
%134 = OpFMul %v2float %128 %133
%127 = OpExtInst %v2float %1 FSign %134
%136 = OpFOrdEqual %v2bool %127 %135
%137 = OpAll %bool %136
OpBranch %126
%126 = OpLabel
%138 = OpPhi %bool %false %110 %137 %125
OpSelectionMerge %142 None
OpBranchConditional %138 %140 %141
%140 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%145 = OpLoad %v4float %143
OpStore %139 %145
OpBranch %142
%141 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%148 = OpLoad %v4float %146
OpStore %139 %148
OpBranch %142
%142 = OpLabel
%149 = OpLoad %v4float %139
OpReturnValue %149
OpFunctionEnd
