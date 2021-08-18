OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputH4"
OpMemberName %_UniformBuffer 1 "expectedH4"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %input "input"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%true = OpConstantTrue %bool
%89 = OpConstantComposite %v2bool %false %true
%97 = OpConstantComposite %v3bool %false %true %false
%105 = OpConstantComposite %v4bool %false %true %false %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%input = OpVariable %_ptr_Function_v4bool Function
%expected = OpVariable %_ptr_Function_v4bool Function
%110 = OpVariable %_ptr_Function_v4float Function
%31 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%35 = OpLoad %v4float %31
%36 = OpCompositeExtract %float %35 0
%37 = OpFUnordNotEqual %bool %36 %float_0
%38 = OpCompositeExtract %float %35 1
%39 = OpFUnordNotEqual %bool %38 %float_0
%40 = OpCompositeExtract %float %35 2
%41 = OpFUnordNotEqual %bool %40 %float_0
%42 = OpCompositeExtract %float %35 3
%43 = OpFUnordNotEqual %bool %42 %float_0
%44 = OpCompositeConstruct %v4bool %37 %39 %41 %43
OpStore %input %44
%46 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%48 = OpLoad %v4float %46
%49 = OpCompositeExtract %float %48 0
%50 = OpFUnordNotEqual %bool %49 %float_0
%51 = OpCompositeExtract %float %48 1
%52 = OpFUnordNotEqual %bool %51 %float_0
%53 = OpCompositeExtract %float %48 2
%54 = OpFUnordNotEqual %bool %53 %float_0
%55 = OpCompositeExtract %float %48 3
%56 = OpFUnordNotEqual %bool %55 %float_0
%57 = OpCompositeConstruct %v4bool %50 %52 %54 %56
OpStore %expected %57
%60 = OpLoad %v4bool %input
%61 = OpVectorShuffle %v2bool %60 %60 0 1
%59 = OpLogicalNot %v2bool %61
%63 = OpLoad %v4bool %expected
%64 = OpVectorShuffle %v2bool %63 %63 0 1
%65 = OpLogicalEqual %v2bool %59 %64
%66 = OpAll %bool %65
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%70 = OpLoad %v4bool %input
%71 = OpVectorShuffle %v3bool %70 %70 0 1 2
%69 = OpLogicalNot %v3bool %71
%73 = OpLoad %v4bool %expected
%74 = OpVectorShuffle %v3bool %73 %73 0 1 2
%75 = OpLogicalEqual %v3bool %69 %74
%76 = OpAll %bool %75
OpBranch %68
%68 = OpLabel
%77 = OpPhi %bool %false %27 %76 %67
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%81 = OpLoad %v4bool %input
%80 = OpLogicalNot %v4bool %81
%82 = OpLoad %v4bool %expected
%83 = OpLogicalEqual %v4bool %80 %82
%84 = OpAll %bool %83
OpBranch %79
%79 = OpLabel
%85 = OpPhi %bool %false %68 %84 %78
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%90 = OpLoad %v4bool %expected
%91 = OpVectorShuffle %v2bool %90 %90 0 1
%92 = OpLogicalEqual %v2bool %89 %91
%93 = OpAll %bool %92
OpBranch %87
%87 = OpLabel
%94 = OpPhi %bool %false %79 %93 %86
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpLoad %v4bool %expected
%99 = OpVectorShuffle %v3bool %98 %98 0 1 2
%100 = OpLogicalEqual %v3bool %97 %99
%101 = OpAll %bool %100
OpBranch %96
%96 = OpLabel
%102 = OpPhi %bool %false %87 %101 %95
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpLoad %v4bool %expected
%107 = OpLogicalEqual %v4bool %105 %106
%108 = OpAll %bool %107
OpBranch %104
%104 = OpLabel
%109 = OpPhi %bool %false %96 %108 %103
OpSelectionMerge %114 None
OpBranchConditional %109 %112 %113
%112 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%117 = OpLoad %v4float %115
OpStore %110 %117
OpBranch %114
%113 = OpLabel
%118 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%120 = OpLoad %v4float %118
OpStore %110 %120
OpBranch %114
%114 = OpLabel
%121 = OpLoad %v4float %110
OpReturnValue %121
OpFunctionEnd
