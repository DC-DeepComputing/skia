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
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
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
%100 = OpVariable %_ptr_Function_v4float Function
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
%59 = OpAny %bool %61
%63 = OpLoad %v4bool %expected
%64 = OpCompositeExtract %bool %63 0
%65 = OpLogicalEqual %bool %59 %64
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%69 = OpLoad %v4bool %input
%70 = OpVectorShuffle %v3bool %69 %69 0 1 2
%68 = OpAny %bool %70
%72 = OpLoad %v4bool %expected
%73 = OpCompositeExtract %bool %72 1
%74 = OpLogicalEqual %bool %68 %73
OpBranch %67
%67 = OpLabel
%75 = OpPhi %bool %false %27 %74 %66
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%79 = OpLoad %v4bool %input
%78 = OpAny %bool %79
%80 = OpLoad %v4bool %expected
%81 = OpCompositeExtract %bool %80 2
%82 = OpLogicalEqual %bool %78 %81
OpBranch %77
%77 = OpLabel
%83 = OpPhi %bool %false %67 %82 %76
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %v4bool %expected
%87 = OpCompositeExtract %bool %86 0
%88 = OpLogicalEqual %bool %false %87
OpBranch %85
%85 = OpLabel
%89 = OpPhi %bool %false %77 %88 %84
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %v4bool %expected
%93 = OpCompositeExtract %bool %92 1
OpBranch %91
%91 = OpLabel
%94 = OpPhi %bool %false %85 %93 %90
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpLoad %v4bool %expected
%98 = OpCompositeExtract %bool %97 2
OpBranch %96
%96 = OpLabel
%99 = OpPhi %bool %false %91 %98 %95
OpSelectionMerge %104 None
OpBranchConditional %99 %102 %103
%102 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%107 = OpLoad %v4float %105
OpStore %100 %107
OpBranch %104
%103 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%110 = OpLoad %v4float %108
OpStore %100 %110
OpBranch %104
%104 = OpLabel
%111 = OpLoad %v4float %100
OpReturnValue %111
OpFunctionEnd
