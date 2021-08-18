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
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%float_1 = OpConstant %float 1
%float_3 = OpConstant %float 3
%33 = OpConstantComposite %v4float %float_n1 %float_0 %float_1 %float_3
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%88 = OpConstantComposite %v2float %float_n1 %float_0
%96 = OpConstantComposite %v3float %float_n1 %float_0 %float_1
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
%108 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %33
%36 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%40 = OpLoad %v4float %36
%41 = OpCompositeExtract %float %40 0
%35 = OpExtInst %float %1 Ceil %41
%42 = OpLoad %v4float %expected
%43 = OpCompositeExtract %float %42 0
%44 = OpFOrdEqual %bool %35 %43
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v2float %49 %49 0 1
%47 = OpExtInst %v2float %1 Ceil %50
%51 = OpLoad %v4float %expected
%52 = OpVectorShuffle %v2float %51 %51 0 1
%53 = OpFOrdEqual %v2bool %47 %52
%55 = OpAll %bool %53
OpBranch %46
%46 = OpLabel
%56 = OpPhi %bool %false %27 %55 %45
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%61 = OpLoad %v4float %60
%62 = OpVectorShuffle %v3float %61 %61 0 1 2
%59 = OpExtInst %v3float %1 Ceil %62
%64 = OpLoad %v4float %expected
%65 = OpVectorShuffle %v3float %64 %64 0 1 2
%66 = OpFOrdEqual %v3bool %59 %65
%68 = OpAll %bool %66
OpBranch %58
%58 = OpLabel
%69 = OpPhi %bool %false %46 %68 %57
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%74 = OpLoad %v4float %73
%72 = OpExtInst %v4float %1 Ceil %74
%75 = OpLoad %v4float %expected
%76 = OpFOrdEqual %v4bool %72 %75
%78 = OpAll %bool %76
OpBranch %71
%71 = OpLabel
%79 = OpPhi %bool %false %58 %78 %70
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %v4float %expected
%83 = OpCompositeExtract %float %82 0
%84 = OpFOrdEqual %bool %float_n1 %83
OpBranch %81
%81 = OpLabel
%85 = OpPhi %bool %false %71 %84 %80
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%89 = OpLoad %v4float %expected
%90 = OpVectorShuffle %v2float %89 %89 0 1
%91 = OpFOrdEqual %v2bool %88 %90
%92 = OpAll %bool %91
OpBranch %87
%87 = OpLabel
%93 = OpPhi %bool %false %81 %92 %86
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%97 = OpLoad %v4float %expected
%98 = OpVectorShuffle %v3float %97 %97 0 1 2
%99 = OpFOrdEqual %v3bool %96 %98
%100 = OpAll %bool %99
OpBranch %95
%95 = OpLabel
%101 = OpPhi %bool %false %87 %100 %94
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%104 = OpLoad %v4float %expected
%105 = OpFOrdEqual %v4bool %33 %104
%106 = OpAll %bool %105
OpBranch %103
%103 = OpLabel
%107 = OpPhi %bool %false %95 %106 %102
OpSelectionMerge %111 None
OpBranchConditional %107 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%114 = OpLoad %v4float %112
OpStore %108 %114
OpBranch %111
%110 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%117 = OpLoad %v4float %115
OpStore %108 %117
OpBranch %111
%111 = OpLabel
%118 = OpLoad %v4float %108
OpReturnValue %118
OpFunctionEnd
