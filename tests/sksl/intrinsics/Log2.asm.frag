OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "input"
OpMemberName %_UniformBuffer 1 "expected"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
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
OpDecorate %29 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
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
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1 = OpConstant %float 1
%89 = OpConstantComposite %v2float %float_0 %float_1
%float_2 = OpConstant %float 2
%99 = OpConstantComposite %v3float %float_0 %float_1 %float_2
%float_3 = OpConstant %float 3
%109 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
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
%115 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
%35 = OpCompositeExtract %float %34 0
%29 = OpExtInst %float %1 Log2 %35
%36 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%38 = OpLoad %v4float %36
%39 = OpCompositeExtract %float %38 0
%40 = OpFOrdEqual %bool %29 %39
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%45 = OpLoad %v4float %44
%46 = OpVectorShuffle %v2float %45 %45 0 1
%43 = OpExtInst %v2float %1 Log2 %46
%47 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%48 = OpLoad %v4float %47
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpFOrdEqual %v2bool %43 %49
%52 = OpAll %bool %50
OpBranch %42
%42 = OpLabel
%53 = OpPhi %bool %false %27 %52 %41
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%58 = OpLoad %v4float %57
%59 = OpVectorShuffle %v3float %58 %58 0 1 2
%56 = OpExtInst %v3float %1 Log2 %59
%61 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%62 = OpLoad %v4float %61
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%64 = OpFOrdEqual %v3bool %56 %63
%66 = OpAll %bool %64
OpBranch %55
%55 = OpLabel
%67 = OpPhi %bool %false %42 %66 %54
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%72 = OpLoad %v4float %71
%70 = OpExtInst %v4float %1 Log2 %72
%73 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%74 = OpLoad %v4float %73
%75 = OpFOrdEqual %v4bool %70 %74
%77 = OpAll %bool %75
OpBranch %69
%69 = OpLabel
%78 = OpPhi %bool %false %55 %77 %68
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%82 = OpLoad %v4float %81
%83 = OpCompositeExtract %float %82 0
%84 = OpFOrdEqual %bool %float_0 %83
OpBranch %80
%80 = OpLabel
%85 = OpPhi %bool %false %69 %84 %79
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%91 = OpLoad %v4float %90
%92 = OpVectorShuffle %v2float %91 %91 0 1
%93 = OpFOrdEqual %v2bool %89 %92
%94 = OpAll %bool %93
OpBranch %87
%87 = OpLabel
%95 = OpPhi %bool %false %80 %94 %86
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%101 = OpLoad %v4float %100
%102 = OpVectorShuffle %v3float %101 %101 0 1 2
%103 = OpFOrdEqual %v3bool %99 %102
%104 = OpAll %bool %103
OpBranch %97
%97 = OpLabel
%105 = OpPhi %bool %false %87 %104 %96
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%111 = OpLoad %v4float %110
%112 = OpFOrdEqual %v4bool %109 %111
%113 = OpAll %bool %112
OpBranch %107
%107 = OpLabel
%114 = OpPhi %bool %false %97 %113 %106
OpSelectionMerge %119 None
OpBranchConditional %114 %117 %118
%117 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%122 = OpLoad %v4float %120
OpStore %115 %122
OpBranch %119
%118 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%125 = OpLoad %v4float %123
OpStore %115 %125
OpBranch %119
%119 = OpLabel
%126 = OpLoad %v4float %115
OpReturnValue %126
OpFunctionEnd
