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
OpDecorate %34 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
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
%float_n2 = OpConstant %float -2
%float_2 = OpConstant %float 2
%32 = OpConstantComposite %v4float %float_n2 %float_0 %float_0 %float_2
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%87 = OpConstantComposite %v2float %float_n2 %float_0
%95 = OpConstantComposite %v3float %float_n2 %float_0 %float_0
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
%107 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %32
%35 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%39 = OpLoad %v4float %35
%40 = OpCompositeExtract %float %39 0
%34 = OpExtInst %float %1 Floor %40
%41 = OpLoad %v4float %expected
%42 = OpCompositeExtract %float %41 0
%43 = OpFOrdEqual %bool %34 %42
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%47 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%48 = OpLoad %v4float %47
%49 = OpVectorShuffle %v2float %48 %48 0 1
%46 = OpExtInst %v2float %1 Floor %49
%50 = OpLoad %v4float %expected
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpFOrdEqual %v2bool %46 %51
%54 = OpAll %bool %52
OpBranch %45
%45 = OpLabel
%55 = OpPhi %bool %false %27 %54 %44
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%60 = OpLoad %v4float %59
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%58 = OpExtInst %v3float %1 Floor %61
%63 = OpLoad %v4float %expected
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%65 = OpFOrdEqual %v3bool %58 %64
%67 = OpAll %bool %65
OpBranch %57
%57 = OpLabel
%68 = OpPhi %bool %false %45 %67 %56
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%73 = OpLoad %v4float %72
%71 = OpExtInst %v4float %1 Floor %73
%74 = OpLoad %v4float %expected
%75 = OpFOrdEqual %v4bool %71 %74
%77 = OpAll %bool %75
OpBranch %70
%70 = OpLabel
%78 = OpPhi %bool %false %57 %77 %69
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpLoad %v4float %expected
%82 = OpCompositeExtract %float %81 0
%83 = OpFOrdEqual %bool %float_n2 %82
OpBranch %80
%80 = OpLabel
%84 = OpPhi %bool %false %70 %83 %79
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%88 = OpLoad %v4float %expected
%89 = OpVectorShuffle %v2float %88 %88 0 1
%90 = OpFOrdEqual %v2bool %87 %89
%91 = OpAll %bool %90
OpBranch %86
%86 = OpLabel
%92 = OpPhi %bool %false %80 %91 %85
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpLoad %v4float %expected
%97 = OpVectorShuffle %v3float %96 %96 0 1 2
%98 = OpFOrdEqual %v3bool %95 %97
%99 = OpAll %bool %98
OpBranch %94
%94 = OpLabel
%100 = OpPhi %bool %false %86 %99 %93
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpLoad %v4float %expected
%104 = OpFOrdEqual %v4bool %32 %103
%105 = OpAll %bool %104
OpBranch %102
%102 = OpLabel
%106 = OpPhi %bool %false %94 %105 %101
OpSelectionMerge %110 None
OpBranchConditional %106 %108 %109
%108 = OpLabel
%111 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%113 = OpLoad %v4float %111
OpStore %107 %113
OpBranch %110
%109 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%116 = OpLoad %v4float %114
OpStore %107 %116
OpBranch %110
%110 = OpLabel
%117 = OpLoad %v4float %107
OpReturnValue %117
OpFunctionEnd
