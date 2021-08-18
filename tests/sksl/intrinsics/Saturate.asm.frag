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
OpDecorate %expected RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_75 = OpConstant %float 0.75
%float_1 = OpConstant %float 1
%32 = OpConstantComposite %v4float %float_0 %float_0 %float_0_75 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%100 = OpConstantComposite %v3float %float_0 %float_0 %float_0_75
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
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
%expected = OpVariable %_ptr_Function_v4float Function
%112 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %32
%35 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%39 = OpLoad %v4float %35
%40 = OpCompositeExtract %float %39 0
%34 = OpExtInst %float %1 FClamp %40 %float_0 %float_1
%41 = OpLoad %v4float %expected
%42 = OpCompositeExtract %float %41 0
%43 = OpFOrdEqual %bool %34 %42
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%47 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%48 = OpLoad %v4float %47
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpCompositeConstruct %v2float %float_0 %float_0
%51 = OpCompositeConstruct %v2float %float_1 %float_1
%46 = OpExtInst %v2float %1 FClamp %49 %50 %51
%52 = OpLoad %v4float %expected
%53 = OpVectorShuffle %v2float %52 %52 0 1
%54 = OpFOrdEqual %v2bool %46 %53
%56 = OpAll %bool %54
OpBranch %45
%45 = OpLabel
%57 = OpPhi %bool %false %27 %56 %44
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%62 = OpLoad %v4float %61
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%65 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%66 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%60 = OpExtInst %v3float %1 FClamp %63 %65 %66
%67 = OpLoad %v4float %expected
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%69 = OpFOrdEqual %v3bool %60 %68
%71 = OpAll %bool %69
OpBranch %59
%59 = OpLabel
%72 = OpPhi %bool %false %45 %71 %58
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%77 = OpLoad %v4float %76
%78 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%79 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%75 = OpExtInst %v4float %1 FClamp %77 %78 %79
%80 = OpLoad %v4float %expected
%81 = OpFOrdEqual %v4bool %75 %80
%83 = OpAll %bool %81
OpBranch %74
%74 = OpLabel
%84 = OpPhi %bool %false %59 %83 %73
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %v4float %expected
%88 = OpCompositeExtract %float %87 0
%89 = OpFOrdEqual %bool %float_0 %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %74 %89 %85
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %v4float %expected
%94 = OpVectorShuffle %v2float %93 %93 0 1
%95 = OpFOrdEqual %v2bool %21 %94
%96 = OpAll %bool %95
OpBranch %92
%92 = OpLabel
%97 = OpPhi %bool %false %86 %96 %91
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpLoad %v4float %expected
%102 = OpVectorShuffle %v3float %101 %101 0 1 2
%103 = OpFOrdEqual %v3bool %100 %102
%104 = OpAll %bool %103
OpBranch %99
%99 = OpLabel
%105 = OpPhi %bool %false %92 %104 %98
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %v4float %expected
%109 = OpFOrdEqual %v4bool %32 %108
%110 = OpAll %bool %109
OpBranch %107
%107 = OpLabel
%111 = OpPhi %bool %false %99 %110 %106
OpSelectionMerge %115 None
OpBranchConditional %111 %113 %114
%113 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%118 = OpLoad %v4float %116
OpStore %112 %118
OpBranch %115
%114 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%121 = OpLoad %v4float %119
OpStore %112 %121
OpBranch %115
%115 = OpLabel
%122 = OpLoad %v4float %112
OpReturnValue %122
OpFunctionEnd
