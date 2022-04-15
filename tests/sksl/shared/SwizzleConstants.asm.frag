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
OpName %v "v"
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
OpDecorate %v RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%42 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
%v3float = OpTypeVector %float 3
%60 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
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
%v = OpVariable %_ptr_Function_v4float Function
%100 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
OpStore %v %32
%33 = OpLoad %v4float %v
%34 = OpCompositeExtract %float %33 0
%36 = OpCompositeConstruct %v4float %34 %float_1 %float_1 %float_1
OpStore %v %36
%37 = OpVectorShuffle %v2float %36 %36 0 1
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%40 = OpCompositeConstruct %v4float %38 %39 %float_1 %float_1
OpStore %v %40
%41 = OpCompositeConstruct %v4float %38 %float_1 %float_1 %float_1
OpStore %v %41
OpStore %v %42
%43 = OpVectorShuffle %v3float %42 %42 0 1 2
%45 = OpCompositeExtract %float %43 0
%46 = OpCompositeExtract %float %43 1
%47 = OpCompositeExtract %float %43 2
%48 = OpCompositeConstruct %v4float %45 %46 %47 %float_1
OpStore %v %48
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpCompositeExtract %float %49 0
%51 = OpCompositeExtract %float %49 1
%52 = OpCompositeConstruct %v4float %50 %51 %float_1 %float_1
OpStore %v %52
%53 = OpCompositeConstruct %v4float %50 %float_0 %float_1 %float_1
OpStore %v %53
%54 = OpCompositeConstruct %v4float %50 %float_1 %float_0 %float_1
OpStore %v %54
%55 = OpVectorShuffle %v2float %54 %54 1 2
%56 = OpCompositeExtract %float %55 0
%57 = OpCompositeExtract %float %55 1
%58 = OpCompositeConstruct %v4float %float_1 %56 %57 %float_1
OpStore %v %58
%59 = OpCompositeConstruct %v4float %float_0 %56 %float_1 %float_1
OpStore %v %59
OpStore %v %60
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%62 = OpCompositeExtract %float %61 0
%63 = OpCompositeExtract %float %61 1
%64 = OpCompositeExtract %float %61 2
%65 = OpCompositeConstruct %v4float %62 %63 %64 %float_1
OpStore %v %65
%66 = OpVectorShuffle %v2float %65 %65 0 1
%67 = OpCompositeExtract %float %66 0
%68 = OpCompositeExtract %float %66 1
%69 = OpCompositeConstruct %v4float %67 %68 %float_0 %float_1
OpStore %v %69
%70 = OpVectorShuffle %v2float %69 %69 0 1
%71 = OpCompositeExtract %float %70 0
%72 = OpCompositeExtract %float %70 1
%73 = OpCompositeConstruct %v4float %71 %72 %float_1 %float_0
OpStore %v %73
%74 = OpVectorShuffle %v2float %73 %73 2 3
%75 = OpCompositeExtract %float %74 0
%76 = OpCompositeExtract %float %74 1
%77 = OpCompositeConstruct %v4float %71 %float_1 %75 %76
OpStore %v %77
%78 = OpCompositeConstruct %v4float %71 %float_0 %75 %float_1
OpStore %v %78
%79 = OpCompositeConstruct %v4float %71 %float_1 %float_1 %float_1
OpStore %v %79
%80 = OpCompositeConstruct %v4float %71 %float_1 %float_0 %float_1
OpStore %v %80
%81 = OpVectorShuffle %v3float %80 %80 1 2 3
%82 = OpCompositeExtract %float %81 0
%83 = OpCompositeExtract %float %81 1
%84 = OpCompositeExtract %float %81 2
%85 = OpCompositeConstruct %v4float %float_1 %82 %83 %84
OpStore %v %85
%86 = OpVectorShuffle %v2float %85 %85 1 2
%87 = OpCompositeExtract %float %86 0
%88 = OpCompositeExtract %float %86 1
%89 = OpCompositeConstruct %v4float %float_0 %87 %88 %float_1
OpStore %v %89
%90 = OpCompositeConstruct %v4float %float_0 %87 %float_1 %float_1
OpStore %v %90
%91 = OpCompositeConstruct %v4float %float_1 %87 %float_1 %float_1
OpStore %v %91
%92 = OpVectorShuffle %v2float %91 %91 2 3
%93 = OpCompositeExtract %float %92 0
%94 = OpCompositeExtract %float %92 1
%95 = OpCompositeConstruct %v4float %float_0 %float_0 %93 %94
OpStore %v %95
%96 = OpCompositeConstruct %v4float %float_0 %float_0 %93 %float_1
OpStore %v %96
OpStore %v %42
%97 = OpFOrdEqual %v4bool %42 %42
%99 = OpAll %bool %97
OpSelectionMerge %103 None
OpBranchConditional %99 %101 %102
%101 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%106 = OpLoad %v4float %104
OpStore %100 %106
OpBranch %103
%102 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%109 = OpLoad %v4float %107
OpStore %100 %109
OpBranch %103
%103 = OpLabel
%110 = OpLoad %v4float %100
OpReturnValue %110
OpFunctionEnd
