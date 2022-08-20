OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
OpName %color_burn_component_Qhh2h2 "color_burn_component_Qhh2h2"
OpName %delta "delta"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
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
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
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
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
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
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
%16 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%36 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%void = OpTypeVoid
%117 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%guarded_divide_Qhhh = OpFunction %float None %16
%17 = OpFunctionParameter %_ptr_Function_float
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpLabel
%21 = OpVariable %_ptr_Function_float Function
OpSelectionMerge %24 None
OpBranchConditional %false %22 %23
%22 = OpLabel
%25 = OpLoad %float %17
%26 = OpLoad %float %18
%28 = OpFAdd %float %26 %float_9_99999994en09
%29 = OpFDiv %float %25 %28
OpStore %21 %29
OpBranch %24
%23 = OpLabel
%30 = OpLoad %float %17
%31 = OpLoad %float %18
%32 = OpFDiv %float %30 %31
OpStore %21 %32
OpBranch %24
%24 = OpLabel
%33 = OpLoad %float %21
OpReturnValue %33
OpFunctionEnd
%color_burn_component_Qhh2h2 = OpFunction %float None %36
%37 = OpFunctionParameter %_ptr_Function_v2float
%38 = OpFunctionParameter %_ptr_Function_v2float
%39 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%93 = OpVariable %_ptr_Function_float Function
%96 = OpVariable %_ptr_Function_float Function
%40 = OpLoad %v2float %38
%41 = OpCompositeExtract %float %40 1
%42 = OpLoad %v2float %38
%43 = OpCompositeExtract %float %42 0
%44 = OpFOrdEqual %bool %41 %43
OpSelectionMerge %47 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpLoad %v2float %37
%49 = OpCompositeExtract %float %48 1
%50 = OpLoad %v2float %38
%51 = OpCompositeExtract %float %50 1
%52 = OpFMul %float %49 %51
%53 = OpLoad %v2float %37
%54 = OpCompositeExtract %float %53 0
%56 = OpLoad %v2float %38
%57 = OpCompositeExtract %float %56 1
%58 = OpFSub %float %float_1 %57
%59 = OpFMul %float %54 %58
%60 = OpFAdd %float %52 %59
%61 = OpLoad %v2float %38
%62 = OpCompositeExtract %float %61 0
%63 = OpLoad %v2float %37
%64 = OpCompositeExtract %float %63 1
%65 = OpFSub %float %float_1 %64
%66 = OpFMul %float %62 %65
%67 = OpFAdd %float %60 %66
OpReturnValue %67
%46 = OpLabel
%68 = OpLoad %v2float %37
%69 = OpCompositeExtract %float %68 0
%71 = OpFOrdEqual %bool %69 %float_0
OpSelectionMerge %74 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpLoad %v2float %38
%76 = OpCompositeExtract %float %75 0
%77 = OpLoad %v2float %37
%78 = OpCompositeExtract %float %77 1
%79 = OpFSub %float %float_1 %78
%80 = OpFMul %float %76 %79
OpReturnValue %80
%73 = OpLabel
%83 = OpLoad %v2float %38
%84 = OpCompositeExtract %float %83 1
%85 = OpLoad %v2float %38
%86 = OpCompositeExtract %float %85 1
%87 = OpLoad %v2float %38
%88 = OpCompositeExtract %float %87 0
%89 = OpFSub %float %86 %88
%90 = OpLoad %v2float %37
%91 = OpCompositeExtract %float %90 1
%92 = OpFMul %float %89 %91
OpStore %93 %92
%94 = OpLoad %v2float %37
%95 = OpCompositeExtract %float %94 0
OpStore %96 %95
%97 = OpFunctionCall %float %guarded_divide_Qhhh %93 %96
%98 = OpFSub %float %84 %97
%82 = OpExtInst %float %1 FMax %float_0 %98
OpStore %delta %82
%99 = OpLoad %v2float %37
%100 = OpCompositeExtract %float %99 1
%101 = OpFMul %float %82 %100
%102 = OpLoad %v2float %37
%103 = OpCompositeExtract %float %102 0
%104 = OpLoad %v2float %38
%105 = OpCompositeExtract %float %104 1
%106 = OpFSub %float %float_1 %105
%107 = OpFMul %float %103 %106
%108 = OpFAdd %float %101 %107
%109 = OpLoad %v2float %38
%110 = OpCompositeExtract %float %109 0
%111 = OpLoad %v2float %37
%112 = OpCompositeExtract %float %111 1
%113 = OpFSub %float %float_1 %112
%114 = OpFMul %float %110 %113
%115 = OpFAdd %float %108 %114
OpReturnValue %115
%74 = OpLabel
OpBranch %47
%47 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %117
%118 = OpLabel
%125 = OpVariable %_ptr_Function_v2float Function
%130 = OpVariable %_ptr_Function_v2float Function
%135 = OpVariable %_ptr_Function_v2float Function
%139 = OpVariable %_ptr_Function_v2float Function
%144 = OpVariable %_ptr_Function_v2float Function
%148 = OpVariable %_ptr_Function_v2float Function
%119 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%123 = OpLoad %v4float %119
%124 = OpVectorShuffle %v2float %123 %123 0 3
OpStore %125 %124
%126 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%128 = OpLoad %v4float %126
%129 = OpVectorShuffle %v2float %128 %128 0 3
OpStore %130 %129
%131 = OpFunctionCall %float %color_burn_component_Qhh2h2 %125 %130
%132 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%133 = OpLoad %v4float %132
%134 = OpVectorShuffle %v2float %133 %133 1 3
OpStore %135 %134
%136 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%137 = OpLoad %v4float %136
%138 = OpVectorShuffle %v2float %137 %137 1 3
OpStore %139 %138
%140 = OpFunctionCall %float %color_burn_component_Qhh2h2 %135 %139
%141 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%142 = OpLoad %v4float %141
%143 = OpVectorShuffle %v2float %142 %142 2 3
OpStore %144 %143
%145 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%146 = OpLoad %v4float %145
%147 = OpVectorShuffle %v2float %146 %146 2 3
OpStore %148 %147
%149 = OpFunctionCall %float %color_burn_component_Qhh2h2 %144 %148
%150 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%151 = OpLoad %v4float %150
%152 = OpCompositeExtract %float %151 3
%153 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%154 = OpLoad %v4float %153
%155 = OpCompositeExtract %float %154 3
%156 = OpFSub %float %float_1 %155
%157 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%158 = OpLoad %v4float %157
%159 = OpCompositeExtract %float %158 3
%160 = OpFMul %float %156 %159
%161 = OpFAdd %float %152 %160
%162 = OpCompositeConstruct %v4float %131 %140 %149 %161
OpStore %sk_FragColor %162
OpReturn
OpFunctionEnd
