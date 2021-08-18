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
OpName %main "main"
OpName %v "v"
OpName %result "result"
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
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
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
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
%v = OpVariable %_ptr_Function_v4bool Function
%result = OpVariable %_ptr_Function_v4bool Function
%159 = OpVariable %_ptr_Function_v4float Function
%31 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%35 = OpLoad %v4float %31
%36 = OpCompositeExtract %float %35 1
%37 = OpFUnordNotEqual %bool %36 %float_0
%38 = OpCompositeConstruct %v4bool %37 %37 %37 %37
OpStore %v %38
%40 = OpLoad %v4bool %v
%41 = OpCompositeExtract %bool %40 0
%43 = OpCompositeConstruct %v4bool %41 %true %true %true
OpStore %result %43
%44 = OpLoad %v4bool %v
%45 = OpVectorShuffle %v2bool %44 %44 0 1
%47 = OpCompositeExtract %bool %45 0
%48 = OpCompositeExtract %bool %45 1
%50 = OpCompositeConstruct %v4bool %47 %48 %false %true
OpStore %result %50
%51 = OpLoad %v4bool %v
%52 = OpCompositeExtract %bool %51 0
%53 = OpCompositeConstruct %v4bool %52 %true %true %false
OpStore %result %53
%54 = OpLoad %v4bool %v
%55 = OpCompositeExtract %bool %54 1
%56 = OpCompositeConstruct %v4bool %false %55 %true %true
OpStore %result %56
%57 = OpLoad %v4bool %v
%58 = OpVectorShuffle %v3bool %57 %57 0 1 2
%60 = OpCompositeExtract %bool %58 0
%61 = OpCompositeExtract %bool %58 1
%62 = OpCompositeExtract %bool %58 2
%63 = OpCompositeConstruct %v4bool %60 %61 %62 %true
OpStore %result %63
%64 = OpLoad %v4bool %v
%65 = OpVectorShuffle %v2bool %64 %64 0 1
%66 = OpCompositeExtract %bool %65 0
%67 = OpCompositeExtract %bool %65 1
%68 = OpCompositeConstruct %v4bool %66 %67 %true %true
OpStore %result %68
%69 = OpLoad %v4bool %v
%70 = OpCompositeExtract %bool %69 0
%71 = OpLoad %v4bool %v
%72 = OpCompositeExtract %bool %71 2
%73 = OpCompositeConstruct %v4bool %70 %false %72 %true
OpStore %result %73
%74 = OpLoad %v4bool %v
%75 = OpCompositeExtract %bool %74 0
%76 = OpCompositeConstruct %v4bool %75 %true %false %false
OpStore %result %76
%77 = OpLoad %v4bool %v
%78 = OpVectorShuffle %v2bool %77 %77 1 2
%79 = OpCompositeExtract %bool %78 0
%80 = OpCompositeExtract %bool %78 1
%81 = OpCompositeConstruct %v4bool %true %79 %80 %false
OpStore %result %81
%82 = OpLoad %v4bool %v
%83 = OpCompositeExtract %bool %82 1
%84 = OpCompositeConstruct %v4bool %false %83 %true %false
OpStore %result %84
%85 = OpLoad %v4bool %v
%86 = OpCompositeExtract %bool %85 2
%87 = OpCompositeConstruct %v4bool %true %true %86 %false
OpStore %result %87
%88 = OpLoad %v4bool %v
OpStore %result %88
%89 = OpLoad %v4bool %v
%90 = OpVectorShuffle %v3bool %89 %89 0 1 2
%91 = OpCompositeExtract %bool %90 0
%92 = OpCompositeExtract %bool %90 1
%93 = OpCompositeExtract %bool %90 2
%94 = OpCompositeConstruct %v4bool %91 %92 %93 %true
OpStore %result %94
%95 = OpLoad %v4bool %v
%96 = OpVectorShuffle %v2bool %95 %95 0 1
%97 = OpCompositeExtract %bool %96 0
%98 = OpCompositeExtract %bool %96 1
%99 = OpLoad %v4bool %v
%100 = OpCompositeExtract %bool %99 3
%101 = OpCompositeConstruct %v4bool %97 %98 %false %100
OpStore %result %101
%102 = OpLoad %v4bool %v
%103 = OpVectorShuffle %v2bool %102 %102 0 1
%104 = OpCompositeExtract %bool %103 0
%105 = OpCompositeExtract %bool %103 1
%106 = OpCompositeConstruct %v4bool %104 %105 %true %false
OpStore %result %106
%107 = OpLoad %v4bool %v
%108 = OpCompositeExtract %bool %107 0
%109 = OpLoad %v4bool %v
%110 = OpVectorShuffle %v2bool %109 %109 2 3
%111 = OpCompositeExtract %bool %110 0
%112 = OpCompositeExtract %bool %110 1
%113 = OpCompositeConstruct %v4bool %108 %true %111 %112
OpStore %result %113
%114 = OpLoad %v4bool %v
%115 = OpCompositeExtract %bool %114 0
%116 = OpLoad %v4bool %v
%117 = OpCompositeExtract %bool %116 2
%118 = OpCompositeConstruct %v4bool %115 %false %117 %true
OpStore %result %118
%119 = OpLoad %v4bool %v
%120 = OpCompositeExtract %bool %119 0
%121 = OpLoad %v4bool %v
%122 = OpCompositeExtract %bool %121 3
%123 = OpCompositeConstruct %v4bool %120 %true %true %122
OpStore %result %123
%124 = OpLoad %v4bool %v
%125 = OpCompositeExtract %bool %124 0
%126 = OpCompositeConstruct %v4bool %125 %true %false %true
OpStore %result %126
%127 = OpLoad %v4bool %v
%128 = OpVectorShuffle %v3bool %127 %127 1 2 3
%129 = OpCompositeExtract %bool %128 0
%130 = OpCompositeExtract %bool %128 1
%131 = OpCompositeExtract %bool %128 2
%132 = OpCompositeConstruct %v4bool %true %129 %130 %131
OpStore %result %132
%133 = OpLoad %v4bool %v
%134 = OpVectorShuffle %v2bool %133 %133 1 2
%135 = OpCompositeExtract %bool %134 0
%136 = OpCompositeExtract %bool %134 1
%137 = OpCompositeConstruct %v4bool %false %135 %136 %true
OpStore %result %137
%138 = OpLoad %v4bool %v
%139 = OpCompositeExtract %bool %138 1
%140 = OpLoad %v4bool %v
%141 = OpCompositeExtract %bool %140 3
%142 = OpCompositeConstruct %v4bool %false %139 %true %141
OpStore %result %142
%143 = OpLoad %v4bool %v
%144 = OpCompositeExtract %bool %143 1
%145 = OpCompositeConstruct %v4bool %true %144 %true %true
OpStore %result %145
%146 = OpLoad %v4bool %v
%147 = OpVectorShuffle %v2bool %146 %146 2 3
%148 = OpCompositeExtract %bool %147 0
%149 = OpCompositeExtract %bool %147 1
%150 = OpCompositeConstruct %v4bool %false %false %148 %149
OpStore %result %150
%151 = OpLoad %v4bool %v
%152 = OpCompositeExtract %bool %151 2
%153 = OpCompositeConstruct %v4bool %false %false %152 %true
OpStore %result %153
%154 = OpLoad %v4bool %v
%155 = OpCompositeExtract %bool %154 3
%156 = OpCompositeConstruct %v4bool %false %true %true %155
OpStore %result %156
%158 = OpLoad %v4bool %result
%157 = OpAny %bool %158
OpSelectionMerge %163 None
OpBranchConditional %157 %161 %162
%161 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%165 = OpLoad %v4float %164
OpStore %159 %165
OpBranch %163
%162 = OpLabel
%166 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%168 = OpLoad %v4float %166
OpStore %159 %168
OpBranch %163
%163 = OpLabel
%169 = OpLoad %v4float %159
OpReturnValue %169
OpFunctionEnd
