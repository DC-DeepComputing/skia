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
OpDecorate %expected RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%28 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1 = OpConstant %float 1
%80 = OpConstantComposite %v2float %float_1 %float_1
%91 = OpConstantComposite %v2float %float_1 %float_0
%111 = OpConstantComposite %v2float %float_0 %float_1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
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
%expected = OpVariable %_ptr_Function_v4float Function
%123 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %28
%31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%35 = OpLoad %v4float %31
%36 = OpCompositeExtract %float %35 0
%30 = OpDPdx %float %36
%37 = OpFOrdEqual %bool %30 %float_0
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%42 = OpLoad %v4float %41
%43 = OpVectorShuffle %v2float %42 %42 0 1
%40 = OpDPdx %v2float %43
%44 = OpLoad %v4float %expected
%45 = OpVectorShuffle %v2float %44 %44 0 1
%46 = OpFOrdEqual %v2bool %40 %45
%48 = OpAll %bool %46
OpBranch %39
%39 = OpLabel
%49 = OpPhi %bool %false %25 %48 %38
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%54 = OpLoad %v4float %53
%55 = OpVectorShuffle %v3float %54 %54 0 1 2
%52 = OpDPdx %v3float %55
%57 = OpLoad %v4float %expected
%58 = OpVectorShuffle %v3float %57 %57 0 1 2
%59 = OpFOrdEqual %v3bool %52 %58
%61 = OpAll %bool %59
OpBranch %51
%51 = OpLabel
%62 = OpPhi %bool %false %39 %61 %50
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%67 = OpLoad %v4float %66
%65 = OpDPdx %v4float %67
%68 = OpLoad %v4float %expected
%69 = OpFOrdEqual %v4bool %65 %68
%71 = OpAll %bool %69
OpBranch %64
%64 = OpLabel
%72 = OpPhi %bool %false %51 %71 %63
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%77 = OpLoad %v2float %24
%78 = OpVectorShuffle %v2float %77 %77 0 0
%76 = OpFwidth %v2float %78
%75 = OpExtInst %v2float %1 FSign %76
%81 = OpFOrdEqual %v2bool %75 %80
%82 = OpAll %bool %81
OpBranch %74
%74 = OpLabel
%83 = OpPhi %bool %false %64 %82 %73
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%88 = OpLoad %v2float %24
%89 = OpCompositeExtract %float %88 0
%90 = OpCompositeConstruct %v2float %89 %float_1
%87 = OpFwidth %v2float %90
%86 = OpExtInst %v2float %1 FSign %87
%92 = OpFOrdEqual %v2bool %86 %91
%93 = OpAll %bool %92
OpBranch %85
%85 = OpLabel
%94 = OpPhi %bool %false %74 %93 %84
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%99 = OpLoad %v2float %24
%100 = OpVectorShuffle %v2float %99 %99 1 1
%98 = OpFwidth %v2float %100
%97 = OpExtInst %v2float %1 FSign %98
%101 = OpFOrdEqual %v2bool %97 %80
%102 = OpAll %bool %101
OpBranch %96
%96 = OpLabel
%103 = OpPhi %bool %false %85 %102 %95
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%108 = OpLoad %v2float %24
%109 = OpCompositeExtract %float %108 1
%110 = OpCompositeConstruct %v2float %float_0 %109
%107 = OpFwidth %v2float %110
%106 = OpExtInst %v2float %1 FSign %107
%112 = OpFOrdEqual %v2bool %106 %111
%113 = OpAll %bool %112
OpBranch %105
%105 = OpLabel
%114 = OpPhi %bool %false %96 %113 %104
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%119 = OpLoad %v2float %24
%118 = OpFwidth %v2float %119
%117 = OpExtInst %v2float %1 FSign %118
%120 = OpFOrdEqual %v2bool %117 %80
%121 = OpAll %bool %120
OpBranch %116
%116 = OpLabel
%122 = OpPhi %bool %false %105 %121 %115
OpSelectionMerge %126 None
OpBranchConditional %122 %124 %125
%124 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%129 = OpLoad %v4float %127
OpStore %123 %129
OpBranch %126
%125 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%132 = OpLoad %v4float %130
OpStore %123 %132
OpBranch %126
%126 = OpLabel
%133 = OpLoad %v4float %123
OpReturnValue %133
OpFunctionEnd
