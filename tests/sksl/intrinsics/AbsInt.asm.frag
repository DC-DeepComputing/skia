OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expected "expected"
OpDecorate %sk_FragCoord BuiltIn FragCoord
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%35 = OpConstantComposite %v4int %int_1 %int_0 %int_0 %int_2
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%112 = OpConstantComposite %v2int %int_1 %int_0
%120 = OpConstantComposite %v3int %int_1 %int_0 %int_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %25
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpLabel
%expected = OpVariable %_ptr_Function_v4int Function
%132 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %35
%38 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%40 = OpLoad %v4float %38
%41 = OpCompositeExtract %float %40 0
%42 = OpConvertFToS %int %41
%37 = OpExtInst %int %1 SAbs %42
%43 = OpLoad %v4int %expected
%44 = OpCompositeExtract %int %43 0
%45 = OpIEqual %bool %37 %44
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpCompositeExtract %float %51 0
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %51 1
%55 = OpConvertFToS %int %54
%56 = OpCompositeConstruct %v2int %53 %55
%48 = OpExtInst %v2int %1 SAbs %56
%58 = OpLoad %v4int %expected
%59 = OpVectorShuffle %v2int %58 %58 0 1
%60 = OpIEqual %v2bool %48 %59
%62 = OpAll %bool %60
OpBranch %47
%47 = OpLabel
%63 = OpPhi %bool %false %27 %62 %46
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%71 = OpCompositeExtract %float %69 0
%72 = OpConvertFToS %int %71
%73 = OpCompositeExtract %float %69 1
%74 = OpConvertFToS %int %73
%75 = OpCompositeExtract %float %69 2
%76 = OpConvertFToS %int %75
%77 = OpCompositeConstruct %v3int %72 %74 %76
%66 = OpExtInst %v3int %1 SAbs %77
%79 = OpLoad %v4int %expected
%80 = OpVectorShuffle %v3int %79 %79 0 1 2
%81 = OpIEqual %v3bool %66 %80
%83 = OpAll %bool %81
OpBranch %65
%65 = OpLabel
%84 = OpPhi %bool %false %47 %83 %64
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%89 = OpLoad %v4float %88
%90 = OpCompositeExtract %float %89 0
%91 = OpConvertFToS %int %90
%92 = OpCompositeExtract %float %89 1
%93 = OpConvertFToS %int %92
%94 = OpCompositeExtract %float %89 2
%95 = OpConvertFToS %int %94
%96 = OpCompositeExtract %float %89 3
%97 = OpConvertFToS %int %96
%98 = OpCompositeConstruct %v4int %91 %93 %95 %97
%87 = OpExtInst %v4int %1 SAbs %98
%99 = OpLoad %v4int %expected
%100 = OpIEqual %v4bool %87 %99
%102 = OpAll %bool %100
OpBranch %86
%86 = OpLabel
%103 = OpPhi %bool %false %65 %102 %85
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpLoad %v4int %expected
%107 = OpCompositeExtract %int %106 0
%108 = OpIEqual %bool %int_1 %107
OpBranch %105
%105 = OpLabel
%109 = OpPhi %bool %false %86 %108 %104
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpLoad %v4int %expected
%114 = OpVectorShuffle %v2int %113 %113 0 1
%115 = OpIEqual %v2bool %112 %114
%116 = OpAll %bool %115
OpBranch %111
%111 = OpLabel
%117 = OpPhi %bool %false %105 %116 %110
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpLoad %v4int %expected
%122 = OpVectorShuffle %v3int %121 %121 0 1 2
%123 = OpIEqual %v3bool %120 %122
%124 = OpAll %bool %123
OpBranch %119
%119 = OpLabel
%125 = OpPhi %bool %false %111 %124 %118
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpLoad %v4int %expected
%129 = OpIEqual %v4bool %35 %128
%130 = OpAll %bool %129
OpBranch %127
%127 = OpLabel
%131 = OpPhi %bool %false %119 %130 %126
OpSelectionMerge %136 None
OpBranchConditional %131 %134 %135
%134 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%138 = OpLoad %v4float %137
OpStore %132 %138
OpBranch %136
%135 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%140 = OpLoad %v4float %139
OpStore %132 %140
OpBranch %136
%136 = OpLabel
%141 = OpLoad %v4float %132
OpReturnValue %141
OpFunctionEnd
