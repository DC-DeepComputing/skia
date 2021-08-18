OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %fn_hh4 "fn_hh4"
OpName %x "x"
OpName %main "main"
OpName %v "v"
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
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %v RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
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
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
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
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%26 = OpTypeFunction %float %_ptr_Function_v4float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%46 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%115 = OpConstantComposite %v4float %float_1 %float_1 %float_2 %float_3
%int_0 = OpConstant %int 0
%148 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%fn_hh4 = OpFunction %float None %26
%28 = OpFunctionParameter %_ptr_Function_v4float
%29 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_1
OpBranch %34
%34 = OpLabel
OpLoopMerge %38 %37 None
OpBranch %35
%35 = OpLabel
%39 = OpLoad %int %x
%41 = OpSLessThanEqual %bool %39 %int_2
OpBranchConditional %41 %36 %38
%36 = OpLabel
%42 = OpLoad %v4float %28
%43 = OpCompositeExtract %float %42 0
OpReturnValue %43
%37 = OpLabel
%44 = OpLoad %int %x
%45 = OpIAdd %int %44 %int_1
OpStore %x %45
OpBranch %34
%38 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %46
%47 = OpFunctionParameter %_ptr_Function_v2float
%48 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%84 = OpVariable %_ptr_Function_v4float Function
%91 = OpVariable %_ptr_Function_v4float Function
%96 = OpVariable %_ptr_Function_v4float Function
%100 = OpVariable %_ptr_Function_v4float Function
%104 = OpVariable %_ptr_Function_v4float Function
%109 = OpVariable %_ptr_Function_v4float Function
%152 = OpVariable %_ptr_Function_v4float Function
%50 = OpAccessChain %_ptr_Uniform_v4float %13 %int_2
%52 = OpLoad %v4float %50
OpStore %v %52
%53 = OpLoad %v4float %v
%54 = OpVectorShuffle %v3float %53 %53 2 1 0
%56 = OpCompositeExtract %float %54 0
%57 = OpCompositeExtract %float %54 1
%58 = OpCompositeExtract %float %54 2
%59 = OpCompositeConstruct %v4float %float_0 %56 %57 %58
OpStore %v %59
%60 = OpLoad %v4float %v
%61 = OpVectorShuffle %v2float %60 %60 0 3
%62 = OpCompositeExtract %float %61 0
%63 = OpCompositeExtract %float %61 1
%64 = OpCompositeConstruct %v4float %float_0 %float_0 %62 %63
OpStore %v %64
%66 = OpLoad %v4float %v
%67 = OpVectorShuffle %v2float %66 %66 3 0
%68 = OpCompositeExtract %float %67 0
%69 = OpCompositeExtract %float %67 1
%70 = OpCompositeConstruct %v4float %float_1 %float_1 %68 %69
OpStore %v %70
%71 = OpLoad %v4float %v
%72 = OpVectorShuffle %v2float %71 %71 2 1
%73 = OpCompositeExtract %float %72 0
%74 = OpCompositeExtract %float %72 1
%75 = OpCompositeConstruct %v4float %73 %74 %float_1 %float_1
OpStore %v %75
%76 = OpLoad %v4float %v
%77 = OpVectorShuffle %v2float %76 %76 0 0
%78 = OpCompositeExtract %float %77 0
%79 = OpCompositeExtract %float %77 1
%80 = OpCompositeConstruct %v4float %78 %79 %float_1 %float_1
OpStore %v %80
%81 = OpLoad %v4float %v
%82 = OpVectorShuffle %v4float %81 %81 3 2 3 2
OpStore %v %82
%83 = OpLoad %v4float %v
OpStore %84 %83
%85 = OpFunctionCall %float %fn_hh4 %84
%88 = OpCompositeConstruct %v3float %85 %float_123 %float_456
%89 = OpVectorShuffle %v4float %88 %88 1 1 2 2
OpStore %v %89
%90 = OpLoad %v4float %v
OpStore %91 %90
%92 = OpFunctionCall %float %fn_hh4 %91
%93 = OpCompositeConstruct %v3float %92 %float_123 %float_456
%94 = OpVectorShuffle %v4float %93 %93 1 1 2 2
OpStore %v %94
%95 = OpLoad %v4float %v
OpStore %96 %95
%97 = OpFunctionCall %float %fn_hh4 %96
%98 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %97
OpStore %v %98
%99 = OpLoad %v4float %v
OpStore %100 %99
%101 = OpFunctionCall %float %fn_hh4 %100
%102 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %101
OpStore %v %102
%103 = OpLoad %v4float %v
OpStore %104 %103
%105 = OpFunctionCall %float %fn_hh4 %104
%106 = OpCompositeConstruct %v3float %105 %float_123 %float_456
%107 = OpVectorShuffle %v4float %106 %106 1 0 0 2
OpStore %v %107
%108 = OpLoad %v4float %v
OpStore %109 %108
%110 = OpFunctionCall %float %fn_hh4 %109
%111 = OpCompositeConstruct %v3float %110 %float_123 %float_456
%112 = OpVectorShuffle %v4float %111 %111 1 0 0 2
OpStore %v %112
OpStore %v %115
%116 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%118 = OpLoad %v4float %116
%119 = OpVectorShuffle %v3float %118 %118 0 1 2
%120 = OpCompositeExtract %float %119 0
%121 = OpCompositeExtract %float %119 1
%122 = OpCompositeExtract %float %119 2
%123 = OpCompositeConstruct %v4float %120 %121 %122 %float_1
OpStore %v %123
%124 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%125 = OpLoad %v4float %124
%126 = OpCompositeExtract %float %125 0
%127 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%128 = OpLoad %v4float %127
%129 = OpVectorShuffle %v2float %128 %128 1 2
%130 = OpCompositeExtract %float %129 0
%131 = OpCompositeExtract %float %129 1
%132 = OpCompositeConstruct %v4float %126 %float_1 %130 %131
OpStore %v %132
%133 = OpLoad %v4float %v
%134 = OpLoad %v4float %v
%135 = OpVectorShuffle %v4float %134 %133 7 6 5 4
OpStore %v %135
%136 = OpLoad %v4float %v
%137 = OpVectorShuffle %v2float %136 %136 1 2
%138 = OpLoad %v4float %v
%139 = OpVectorShuffle %v4float %138 %137 4 1 2 5
OpStore %v %139
%140 = OpLoad %v4float %v
%141 = OpVectorShuffle %v2float %140 %140 3 3
%142 = OpCompositeExtract %float %141 0
%143 = OpCompositeExtract %float %141 1
%144 = OpCompositeConstruct %v3float %142 %143 %float_1
%145 = OpLoad %v4float %v
%146 = OpVectorShuffle %v4float %145 %144 6 5 4 3
OpStore %v %146
%147 = OpLoad %v4float %v
%149 = OpFOrdEqual %v4bool %147 %148
%151 = OpAll %bool %149
OpSelectionMerge %155 None
OpBranchConditional %151 %153 %154
%153 = OpLabel
%156 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%157 = OpLoad %v4float %156
OpStore %152 %157
OpBranch %155
%154 = OpLabel
%158 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%159 = OpLoad %v4float %158
OpStore %152 %159
OpBranch %155
%155 = OpLabel
%160 = OpLoad %v4float %152
OpReturnValue %160
OpFunctionEnd
