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
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
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
%float_3 = OpConstant %float 3
%float_5 = OpConstant %float 5
%float_13 = OpConstant %float 13
%33 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v3float = OpTypeVector %float 3
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
%99 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %33
%36 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%40 = OpLoad %v4float %36
%41 = OpCompositeExtract %float %40 0
%35 = OpExtInst %float %1 Length %41
%42 = OpLoad %v4float %expected
%43 = OpCompositeExtract %float %42 0
%44 = OpFOrdEqual %bool %35 %43
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v2float %49 %49 0 1
%47 = OpExtInst %float %1 Length %50
%51 = OpLoad %v4float %expected
%52 = OpCompositeExtract %float %51 1
%53 = OpFOrdEqual %bool %47 %52
OpBranch %46
%46 = OpLabel
%54 = OpPhi %bool %false %27 %53 %45
OpSelectionMerge %56 None
OpBranchConditional %54 %55 %56
%55 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%59 = OpLoad %v4float %58
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%57 = OpExtInst %float %1 Length %60
%62 = OpLoad %v4float %expected
%63 = OpCompositeExtract %float %62 2
%64 = OpFOrdEqual %bool %57 %63
OpBranch %56
%56 = OpLabel
%65 = OpPhi %bool %false %46 %64 %55
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%70 = OpLoad %v4float %69
%68 = OpExtInst %float %1 Length %70
%71 = OpLoad %v4float %expected
%72 = OpCompositeExtract %float %71 3
%73 = OpFOrdEqual %bool %68 %72
OpBranch %67
%67 = OpLabel
%74 = OpPhi %bool %false %56 %73 %66
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %v4float %expected
%78 = OpCompositeExtract %float %77 0
%79 = OpFOrdEqual %bool %float_3 %78
OpBranch %76
%76 = OpLabel
%80 = OpPhi %bool %false %67 %79 %75
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpLoad %v4float %expected
%84 = OpCompositeExtract %float %83 1
%85 = OpFOrdEqual %bool %float_3 %84
OpBranch %82
%82 = OpLabel
%86 = OpPhi %bool %false %76 %85 %81
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpLoad %v4float %expected
%90 = OpCompositeExtract %float %89 2
%91 = OpFOrdEqual %bool %float_5 %90
OpBranch %88
%88 = OpLabel
%92 = OpPhi %bool %false %82 %91 %87
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpLoad %v4float %expected
%96 = OpCompositeExtract %float %95 3
%97 = OpFOrdEqual %bool %float_13 %96
OpBranch %94
%94 = OpLabel
%98 = OpPhi %bool %false %88 %97 %93
OpSelectionMerge %102 None
OpBranchConditional %98 %100 %101
%100 = OpLabel
%103 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%105 = OpLoad %v4float %103
OpStore %99 %105
OpBranch %102
%101 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%108 = OpLoad %v4float %106
OpStore %99 %108
OpBranch %102
%102 = OpLabel
%109 = OpLoad %v4float %99
OpReturnValue %109
OpFunctionEnd
