OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputH4"
OpMemberName %_UniformBuffer 1 "expectedH4"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %inputVal "inputVal"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%inputVal = OpVariable %_ptr_Function_v4bool Function
%expected = OpVariable %_ptr_Function_v4bool Function
%97 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %29
%34 = OpCompositeExtract %float %33 0
%35 = OpFUnordNotEqual %bool %34 %float_0
%36 = OpCompositeExtract %float %33 1
%37 = OpFUnordNotEqual %bool %36 %float_0
%38 = OpCompositeExtract %float %33 2
%39 = OpFUnordNotEqual %bool %38 %float_0
%40 = OpCompositeExtract %float %33 3
%41 = OpFUnordNotEqual %bool %40 %float_0
%42 = OpCompositeConstruct %v4bool %35 %37 %39 %41
OpStore %inputVal %42
%44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%46 = OpLoad %v4float %44
%47 = OpCompositeExtract %float %46 0
%48 = OpFUnordNotEqual %bool %47 %float_0
%49 = OpCompositeExtract %float %46 1
%50 = OpFUnordNotEqual %bool %49 %float_0
%51 = OpCompositeExtract %float %46 2
%52 = OpFUnordNotEqual %bool %51 %float_0
%53 = OpCompositeExtract %float %46 3
%54 = OpFUnordNotEqual %bool %53 %float_0
%55 = OpCompositeConstruct %v4bool %48 %50 %52 %54
OpStore %expected %55
%58 = OpVectorShuffle %v2bool %42 %42 0 1
%57 = OpAll %bool %58
%60 = OpCompositeExtract %bool %55 0
%61 = OpLogicalEqual %bool %57 %60
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpLoad %v4bool %inputVal
%66 = OpVectorShuffle %v3bool %65 %65 0 1 2
%64 = OpAll %bool %66
%68 = OpLoad %v4bool %expected
%69 = OpCompositeExtract %bool %68 1
%70 = OpLogicalEqual %bool %64 %69
OpBranch %63
%63 = OpLabel
%71 = OpPhi %bool %false %25 %70 %62
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpLoad %v4bool %inputVal
%74 = OpAll %bool %75
%76 = OpLoad %v4bool %expected
%77 = OpCompositeExtract %bool %76 2
%78 = OpLogicalEqual %bool %74 %77
OpBranch %73
%73 = OpLabel
%79 = OpPhi %bool %false %63 %78 %72
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %v4bool %expected
%83 = OpCompositeExtract %bool %82 0
OpBranch %81
%81 = OpLabel
%84 = OpPhi %bool %false %73 %83 %80
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %v4bool %expected
%88 = OpCompositeExtract %bool %87 1
%89 = OpLogicalEqual %bool %false %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %81 %89 %85
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %v4bool %expected
%94 = OpCompositeExtract %bool %93 2
%95 = OpLogicalEqual %bool %false %94
OpBranch %92
%92 = OpLabel
%96 = OpPhi %bool %false %86 %95 %91
OpSelectionMerge %101 None
OpBranchConditional %96 %99 %100
%99 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%104 = OpLoad %v4float %102
OpStore %97 %104
OpBranch %101
%100 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%107 = OpLoad %v4float %105
OpStore %97 %107
OpBranch %101
%101 = OpLabel
%108 = OpLoad %v4float %97
OpReturnValue %108
OpFunctionEnd
