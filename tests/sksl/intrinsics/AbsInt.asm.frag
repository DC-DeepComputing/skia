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
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%33 = OpConstantComposite %v4int %int_1 %int_0 %int_0 %int_2
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%108 = OpConstantComposite %v2int %int_1 %int_0
%116 = OpConstantComposite %v3int %int_1 %int_0 %int_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%expected = OpVariable %_ptr_Function_v4int Function
%128 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %36
%39 = OpCompositeExtract %float %38 0
%40 = OpConvertFToS %int %39
%35 = OpExtInst %int %1 SAbs %40
%41 = OpIEqual %bool %35 %int_1
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%46 = OpLoad %v4float %45
%47 = OpVectorShuffle %v2float %46 %46 0 1
%48 = OpCompositeExtract %float %47 0
%49 = OpConvertFToS %int %48
%50 = OpCompositeExtract %float %47 1
%51 = OpConvertFToS %int %50
%53 = OpCompositeConstruct %v2int %49 %51
%44 = OpExtInst %v2int %1 SAbs %53
%54 = OpLoad %v4int %expected
%55 = OpVectorShuffle %v2int %54 %54 0 1
%56 = OpIEqual %v2bool %44 %55
%58 = OpAll %bool %56
OpBranch %43
%43 = OpLabel
%59 = OpPhi %bool %false %25 %58 %42
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%64 = OpLoad %v4float %63
%65 = OpVectorShuffle %v3float %64 %64 0 1 2
%67 = OpCompositeExtract %float %65 0
%68 = OpConvertFToS %int %67
%69 = OpCompositeExtract %float %65 1
%70 = OpConvertFToS %int %69
%71 = OpCompositeExtract %float %65 2
%72 = OpConvertFToS %int %71
%74 = OpCompositeConstruct %v3int %68 %70 %72
%62 = OpExtInst %v3int %1 SAbs %74
%75 = OpLoad %v4int %expected
%76 = OpVectorShuffle %v3int %75 %75 0 1 2
%77 = OpIEqual %v3bool %62 %76
%79 = OpAll %bool %77
OpBranch %61
%61 = OpLabel
%80 = OpPhi %bool %false %43 %79 %60
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 0
%87 = OpConvertFToS %int %86
%88 = OpCompositeExtract %float %85 1
%89 = OpConvertFToS %int %88
%90 = OpCompositeExtract %float %85 2
%91 = OpConvertFToS %int %90
%92 = OpCompositeExtract %float %85 3
%93 = OpConvertFToS %int %92
%94 = OpCompositeConstruct %v4int %87 %89 %91 %93
%83 = OpExtInst %v4int %1 SAbs %94
%95 = OpLoad %v4int %expected
%96 = OpIEqual %v4bool %83 %95
%98 = OpAll %bool %96
OpBranch %82
%82 = OpLabel
%99 = OpPhi %bool %false %61 %98 %81
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpLoad %v4int %expected
%103 = OpCompositeExtract %int %102 0
%104 = OpIEqual %bool %int_1 %103
OpBranch %101
%101 = OpLabel
%105 = OpPhi %bool %false %82 %104 %100
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%109 = OpLoad %v4int %expected
%110 = OpVectorShuffle %v2int %109 %109 0 1
%111 = OpIEqual %v2bool %108 %110
%112 = OpAll %bool %111
OpBranch %107
%107 = OpLabel
%113 = OpPhi %bool %false %101 %112 %106
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpLoad %v4int %expected
%118 = OpVectorShuffle %v3int %117 %117 0 1 2
%119 = OpIEqual %v3bool %116 %118
%120 = OpAll %bool %119
OpBranch %115
%115 = OpLabel
%121 = OpPhi %bool %false %107 %120 %114
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpLoad %v4int %expected
%125 = OpIEqual %v4bool %33 %124
%126 = OpAll %bool %125
OpBranch %123
%123 = OpLabel
%127 = OpPhi %bool %false %115 %126 %122
OpSelectionMerge %132 None
OpBranchConditional %127 %130 %131
%130 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%134 = OpLoad %v4float %133
OpStore %128 %134
OpBranch %132
%131 = OpLabel
%135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%136 = OpLoad %v4float %135
OpStore %128 %136
OpBranch %132
%132 = OpLabel
%137 = OpLoad %v4float %128
OpReturnValue %137
OpFunctionEnd
