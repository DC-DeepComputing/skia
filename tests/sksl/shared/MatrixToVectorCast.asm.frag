OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
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
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %33 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%50 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%v4bool = OpTypeVector %bool 4
%v4int = OpTypeVector %int 4
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%91 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%114 = OpConstantComposite %v4bool %true %true %true %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%119 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%33 = OpLoad %bool %ok
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%36 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_2
%40 = OpLoad %mat2v2float %36
%41 = OpCompositeExtract %float %40 0 0
%42 = OpCompositeExtract %float %40 0 1
%43 = OpCompositeExtract %float %40 1 0
%44 = OpCompositeExtract %float %40 1 1
%45 = OpCompositeConstruct %v4float %41 %42 %43 %44
%51 = OpFOrdEqual %v4bool %45 %50
%53 = OpAll %bool %51
OpBranch %35
%35 = OpLabel
%54 = OpPhi %bool %false %28 %53 %34
OpStore %ok %54
%55 = OpLoad %bool %ok
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_2
%59 = OpLoad %mat2v2float %58
%60 = OpCompositeExtract %float %59 0 0
%61 = OpCompositeExtract %float %59 0 1
%62 = OpCompositeExtract %float %59 1 0
%63 = OpCompositeExtract %float %59 1 1
%64 = OpCompositeConstruct %v4float %60 %61 %62 %63
%65 = OpFOrdEqual %v4bool %64 %50
%66 = OpAll %bool %65
OpBranch %57
%57 = OpLabel
%67 = OpPhi %bool %false %35 %66 %56
OpStore %ok %67
%68 = OpLoad %bool %ok
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_2
%72 = OpLoad %mat2v2float %71
%73 = OpCompositeExtract %float %72 0 0
%74 = OpCompositeExtract %float %72 0 1
%75 = OpCompositeExtract %float %72 1 0
%76 = OpCompositeExtract %float %72 1 1
%77 = OpCompositeConstruct %v4float %73 %74 %75 %76
%78 = OpCompositeExtract %float %77 0
%79 = OpConvertFToS %int %78
%80 = OpCompositeExtract %float %77 1
%81 = OpConvertFToS %int %80
%82 = OpCompositeExtract %float %77 2
%83 = OpConvertFToS %int %82
%84 = OpCompositeExtract %float %77 3
%85 = OpConvertFToS %int %84
%86 = OpCompositeConstruct %v4int %79 %81 %83 %85
%92 = OpIEqual %v4bool %86 %91
%93 = OpAll %bool %92
OpBranch %70
%70 = OpLabel
%94 = OpPhi %bool %false %57 %93 %69
OpStore %ok %94
%95 = OpLoad %bool %ok
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_2
%99 = OpLoad %mat2v2float %98
%100 = OpCompositeExtract %float %99 0 0
%101 = OpCompositeExtract %float %99 0 1
%102 = OpCompositeExtract %float %99 1 0
%103 = OpCompositeExtract %float %99 1 1
%104 = OpCompositeConstruct %v4float %100 %101 %102 %103
%105 = OpCompositeExtract %float %104 0
%106 = OpFUnordNotEqual %bool %105 %float_0
%107 = OpCompositeExtract %float %104 1
%108 = OpFUnordNotEqual %bool %107 %float_0
%109 = OpCompositeExtract %float %104 2
%110 = OpFUnordNotEqual %bool %109 %float_0
%111 = OpCompositeExtract %float %104 3
%112 = OpFUnordNotEqual %bool %111 %float_0
%113 = OpCompositeConstruct %v4bool %106 %108 %110 %112
%115 = OpLogicalEqual %v4bool %113 %114
%116 = OpAll %bool %115
OpBranch %97
%97 = OpLabel
%117 = OpPhi %bool %false %70 %116 %96
OpStore %ok %117
%118 = OpLoad %bool %ok
OpSelectionMerge %123 None
OpBranchConditional %118 %121 %122
%121 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%127 = OpLoad %v4float %124
OpStore %119 %127
OpBranch %123
%122 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%129 = OpLoad %v4float %128
OpStore %119 %129
OpBranch %123
%123 = OpLabel
%130 = OpLoad %v4float %119
OpReturnValue %130
OpFunctionEnd
