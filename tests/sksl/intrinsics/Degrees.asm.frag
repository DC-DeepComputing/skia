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
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
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
%float_90 = OpConstant %float 90
%float_180 = OpConstant %float 180
%90 = OpConstantComposite %v2float %float_90 %float_180
%float_270 = OpConstant %float 270
%100 = OpConstantComposite %v3float %float_90 %float_180 %float_270
%float_360 = OpConstant %float 360
%110 = OpConstantComposite %v4float %float_90 %float_180 %float_270 %float_360
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
%116 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
%35 = OpCompositeExtract %float %34 0
%29 = OpExtInst %float %1 Degrees %35
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
%43 = OpExtInst %v2float %1 Degrees %46
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
%56 = OpExtInst %v3float %1 Degrees %59
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
%70 = OpExtInst %v4float %1 Degrees %72
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
%82 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%83 = OpLoad %v4float %82
%84 = OpCompositeExtract %float %83 0
%85 = OpFOrdEqual %bool %float_90 %84
OpBranch %80
%80 = OpLabel
%86 = OpPhi %bool %false %69 %85 %79
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%92 = OpLoad %v4float %91
%93 = OpVectorShuffle %v2float %92 %92 0 1
%94 = OpFOrdEqual %v2bool %90 %93
%95 = OpAll %bool %94
OpBranch %88
%88 = OpLabel
%96 = OpPhi %bool %false %80 %95 %87
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%102 = OpLoad %v4float %101
%103 = OpVectorShuffle %v3float %102 %102 0 1 2
%104 = OpFOrdEqual %v3bool %100 %103
%105 = OpAll %bool %104
OpBranch %98
%98 = OpLabel
%106 = OpPhi %bool %false %88 %105 %97
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%111 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%112 = OpLoad %v4float %111
%113 = OpFOrdEqual %v4bool %110 %112
%114 = OpAll %bool %113
OpBranch %108
%108 = OpLabel
%115 = OpPhi %bool %false %98 %114 %107
OpSelectionMerge %120 None
OpBranchConditional %115 %118 %119
%118 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%123 = OpLoad %v4float %121
OpStore %116 %123
OpBranch %120
%119 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%126 = OpLoad %v4float %124
OpStore %116 %126
OpBranch %120
%120 = OpLabel
%127 = OpLoad %v4float %116
OpReturnValue %127
OpFunctionEnd
