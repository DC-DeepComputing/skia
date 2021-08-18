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
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_int_b "test_int_b"
OpName %one "one"
OpName %two "two"
OpName %result "result"
OpName %main "main"
OpName %_0_one "_0_one"
OpName %_1_two "_1_two"
OpName %_2_result "_2_result"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_0 = OpConstant %int 0
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%72 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v3float = OpTypeVector %float 3
%v4bool = OpTypeVector %bool 4
%float_n2 = OpConstant %float -2
%104 = OpConstantComposite %v2float %float_1 %float_n2
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%test_int_b = OpFunction %bool None %26
%27 = OpLabel
%one = OpVariable %_ptr_Function_int Function
%two = OpVariable %_ptr_Function_int Function
%result = OpVariable %_ptr_Function_v4int Function
OpStore %one %int_1
OpStore %two %int_2
%37 = OpAccessChain %_ptr_Function_int %result %int_0
OpStore %37 %int_1
%39 = OpAccessChain %_ptr_Function_int %result %int_1
OpStore %39 %int_1
%40 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %40 %int_1
%44 = OpLoad %int %one
%43 = OpSNegate %int %44
%45 = OpLoad %int %one
%46 = OpLoad %int %one
%47 = OpIAdd %int %45 %46
%48 = OpCompositeConstruct %v2int %43 %47
%41 = OpSNegate %v2int %48
%50 = OpLoad %int %one
%51 = OpLoad %int %two
%52 = OpISub %int %50 %51
%53 = OpCompositeConstruct %v2int %52 %int_2
%49 = OpSNegate %v2int %53
%54 = OpIEqual %v2bool %41 %49
%56 = OpAll %bool %54
%57 = OpSelect %int %56 %int_1 %int_0
%58 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %58 %57
%60 = OpLoad %v4int %result
%61 = OpCompositeExtract %int %60 0
%62 = OpLoad %v4int %result
%63 = OpCompositeExtract %int %62 1
%64 = OpIMul %int %61 %63
%65 = OpLoad %v4int %result
%66 = OpCompositeExtract %int %65 2
%67 = OpIMul %int %64 %66
%68 = OpLoad %v4int %result
%69 = OpCompositeExtract %int %68 3
%70 = OpIMul %int %67 %69
%71 = OpINotEqual %bool %70 %int_0
OpReturnValue %71
OpFunctionEnd
%main = OpFunction %v4float None %72
%73 = OpFunctionParameter %_ptr_Function_v2float
%74 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%133 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
%82 = OpAccessChain %_ptr_Function_float %_2_result %int_0
OpStore %82 %float_1
%83 = OpAccessChain %_ptr_Function_float %_2_result %int_1
OpStore %83 %float_1
%85 = OpLoad %float %_1_two
%86 = OpCompositeConstruct %v4float %85 %85 %85 %85
%84 = OpFNegate %v4float %86
%88 = OpLoad %float %_1_two
%87 = OpFNegate %float %88
%90 = OpLoad %float %_1_two
%89 = OpFNegate %float %90
%91 = OpCompositeConstruct %v3float %89 %89 %89
%93 = OpCompositeExtract %float %91 0
%94 = OpCompositeExtract %float %91 1
%95 = OpCompositeExtract %float %91 2
%96 = OpCompositeConstruct %v4float %87 %93 %94 %95
%97 = OpFOrdEqual %v4bool %84 %96
%99 = OpAll %bool %97
%100 = OpSelect %int %99 %int_1 %int_0
%101 = OpConvertSToF %float %100
%102 = OpAccessChain %_ptr_Function_float %_2_result %int_2
OpStore %102 %101
%106 = OpLoad %float %_0_one
%107 = OpLoad %float %_1_two
%108 = OpFSub %float %106 %107
%109 = OpLoad %float %_1_two
%110 = OpCompositeConstruct %v2float %108 %109
%105 = OpFNegate %v2float %110
%111 = OpFOrdEqual %v2bool %104 %105
%112 = OpAll %bool %111
%113 = OpSelect %int %112 %int_1 %int_0
%114 = OpConvertSToF %float %113
%115 = OpAccessChain %_ptr_Function_float %_2_result %int_3
OpStore %115 %114
%117 = OpLoad %v4float %_2_result
%118 = OpCompositeExtract %float %117 0
%119 = OpLoad %v4float %_2_result
%120 = OpCompositeExtract %float %119 1
%121 = OpFMul %float %118 %120
%122 = OpLoad %v4float %_2_result
%123 = OpCompositeExtract %float %122 2
%124 = OpFMul %float %121 %123
%125 = OpLoad %v4float %_2_result
%126 = OpCompositeExtract %float %125 3
%127 = OpFMul %float %124 %126
%128 = OpFUnordNotEqual %bool %127 %float_0
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%131 = OpFunctionCall %bool %test_int_b
OpBranch %130
%130 = OpLabel
%132 = OpPhi %bool %false %74 %131 %129
OpSelectionMerge %136 None
OpBranchConditional %132 %134 %135
%134 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%139 = OpLoad %v4float %137
OpStore %133 %139
OpBranch %136
%135 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%141 = OpLoad %v4float %140
OpStore %133 %141
OpBranch %136
%136 = OpLabel
%142 = OpLoad %v4float %133
OpReturnValue %142
OpFunctionEnd
