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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %v RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
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
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
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
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%150 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
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
%v = OpVariable %_ptr_Function_v4float Function
%154 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
OpStore %v %34
%35 = OpLoad %v4float %v
%36 = OpCompositeExtract %float %35 0
%38 = OpCompositeConstruct %v4float %36 %float_1 %float_1 %float_1
OpStore %v %38
%39 = OpLoad %v4float %v
%40 = OpVectorShuffle %v2float %39 %39 0 1
%41 = OpCompositeExtract %float %40 0
%42 = OpCompositeExtract %float %40 1
%43 = OpCompositeConstruct %v4float %41 %42 %float_1 %float_1
OpStore %v %43
%44 = OpLoad %v4float %v
%45 = OpCompositeExtract %float %44 0
%46 = OpCompositeConstruct %v4float %45 %float_1 %float_1 %float_1
OpStore %v %46
%47 = OpLoad %v4float %v
%48 = OpCompositeExtract %float %47 1
%49 = OpCompositeConstruct %v4float %float_0 %48 %float_1 %float_1
OpStore %v %49
%50 = OpLoad %v4float %v
%51 = OpVectorShuffle %v3float %50 %50 0 1 2
%53 = OpCompositeExtract %float %51 0
%54 = OpCompositeExtract %float %51 1
%55 = OpCompositeExtract %float %51 2
%56 = OpCompositeConstruct %v4float %53 %54 %55 %float_1
OpStore %v %56
%57 = OpLoad %v4float %v
%58 = OpVectorShuffle %v2float %57 %57 0 1
%59 = OpCompositeExtract %float %58 0
%60 = OpCompositeExtract %float %58 1
%61 = OpCompositeConstruct %v4float %59 %60 %float_1 %float_1
OpStore %v %61
%62 = OpLoad %v4float %v
%63 = OpCompositeExtract %float %62 0
%64 = OpLoad %v4float %v
%65 = OpCompositeExtract %float %64 2
%66 = OpCompositeConstruct %v4float %63 %float_0 %65 %float_1
OpStore %v %66
%67 = OpLoad %v4float %v
%68 = OpCompositeExtract %float %67 0
%69 = OpCompositeConstruct %v4float %68 %float_1 %float_0 %float_1
OpStore %v %69
%70 = OpLoad %v4float %v
%71 = OpVectorShuffle %v2float %70 %70 1 2
%72 = OpCompositeExtract %float %71 0
%73 = OpCompositeExtract %float %71 1
%74 = OpCompositeConstruct %v4float %float_1 %72 %73 %float_1
OpStore %v %74
%75 = OpLoad %v4float %v
%76 = OpCompositeExtract %float %75 1
%77 = OpCompositeConstruct %v4float %float_0 %76 %float_1 %float_1
OpStore %v %77
%78 = OpLoad %v4float %v
%79 = OpCompositeExtract %float %78 2
%80 = OpCompositeConstruct %v4float %float_1 %float_1 %79 %float_1
OpStore %v %80
%81 = OpLoad %v4float %v
%82 = OpVectorShuffle %v3float %81 %81 0 1 2
%83 = OpCompositeExtract %float %82 0
%84 = OpCompositeExtract %float %82 1
%85 = OpCompositeExtract %float %82 2
%86 = OpCompositeConstruct %v4float %83 %84 %85 %float_1
OpStore %v %86
%87 = OpLoad %v4float %v
%88 = OpVectorShuffle %v2float %87 %87 0 1
%89 = OpCompositeExtract %float %88 0
%90 = OpCompositeExtract %float %88 1
%91 = OpLoad %v4float %v
%92 = OpCompositeExtract %float %91 3
%93 = OpCompositeConstruct %v4float %89 %90 %float_0 %92
OpStore %v %93
%94 = OpLoad %v4float %v
%95 = OpVectorShuffle %v2float %94 %94 0 1
%96 = OpCompositeExtract %float %95 0
%97 = OpCompositeExtract %float %95 1
%98 = OpCompositeConstruct %v4float %96 %97 %float_1 %float_0
OpStore %v %98
%99 = OpLoad %v4float %v
%100 = OpCompositeExtract %float %99 0
%101 = OpLoad %v4float %v
%102 = OpVectorShuffle %v2float %101 %101 2 3
%103 = OpCompositeExtract %float %102 0
%104 = OpCompositeExtract %float %102 1
%105 = OpCompositeConstruct %v4float %100 %float_1 %103 %104
OpStore %v %105
%106 = OpLoad %v4float %v
%107 = OpCompositeExtract %float %106 0
%108 = OpLoad %v4float %v
%109 = OpCompositeExtract %float %108 2
%110 = OpCompositeConstruct %v4float %107 %float_0 %109 %float_1
OpStore %v %110
%111 = OpLoad %v4float %v
%112 = OpCompositeExtract %float %111 0
%113 = OpLoad %v4float %v
%114 = OpCompositeExtract %float %113 3
%115 = OpCompositeConstruct %v4float %112 %float_1 %float_1 %114
OpStore %v %115
%116 = OpLoad %v4float %v
%117 = OpCompositeExtract %float %116 0
%118 = OpCompositeConstruct %v4float %117 %float_1 %float_0 %float_1
OpStore %v %118
%119 = OpLoad %v4float %v
%120 = OpVectorShuffle %v3float %119 %119 1 2 3
%121 = OpCompositeExtract %float %120 0
%122 = OpCompositeExtract %float %120 1
%123 = OpCompositeExtract %float %120 2
%124 = OpCompositeConstruct %v4float %float_1 %121 %122 %123
OpStore %v %124
%125 = OpLoad %v4float %v
%126 = OpVectorShuffle %v2float %125 %125 1 2
%127 = OpCompositeExtract %float %126 0
%128 = OpCompositeExtract %float %126 1
%129 = OpCompositeConstruct %v4float %float_0 %127 %128 %float_1
OpStore %v %129
%130 = OpLoad %v4float %v
%131 = OpCompositeExtract %float %130 1
%132 = OpLoad %v4float %v
%133 = OpCompositeExtract %float %132 3
%134 = OpCompositeConstruct %v4float %float_0 %131 %float_1 %133
OpStore %v %134
%135 = OpLoad %v4float %v
%136 = OpCompositeExtract %float %135 1
%137 = OpCompositeConstruct %v4float %float_1 %136 %float_1 %float_1
OpStore %v %137
%138 = OpLoad %v4float %v
%139 = OpVectorShuffle %v2float %138 %138 2 3
%140 = OpCompositeExtract %float %139 0
%141 = OpCompositeExtract %float %139 1
%142 = OpCompositeConstruct %v4float %float_0 %float_0 %140 %141
OpStore %v %142
%143 = OpLoad %v4float %v
%144 = OpCompositeExtract %float %143 2
%145 = OpCompositeConstruct %v4float %float_0 %float_0 %144 %float_1
OpStore %v %145
%146 = OpLoad %v4float %v
%147 = OpCompositeExtract %float %146 3
%148 = OpCompositeConstruct %v4float %float_0 %float_1 %float_1 %147
OpStore %v %148
%149 = OpLoad %v4float %v
%151 = OpFOrdEqual %v4bool %149 %150
%153 = OpAll %bool %151
OpSelectionMerge %157 None
OpBranchConditional %153 %155 %156
%155 = OpLabel
%158 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%160 = OpLoad %v4float %158
OpStore %154 %160
OpBranch %157
%156 = OpLabel
%161 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%163 = OpLoad %v4float %161
OpStore %154 %163
OpBranch %157
%157 = OpLabel
%164 = OpLoad %v4float %154
OpReturnValue %164
OpFunctionEnd
