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
OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
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
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
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
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
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
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%112 = OpTypeFunction %void
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
%color_dodge_component_Qhh2h2 = OpFunction %float None %36
%37 = OpFunctionParameter %_ptr_Function_v2float
%38 = OpFunctionParameter %_ptr_Function_v2float
%39 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%91 = OpVariable %_ptr_Function_float Function
%92 = OpVariable %_ptr_Function_float Function
%40 = OpLoad %v2float %38
%41 = OpCompositeExtract %float %40 0
%43 = OpFOrdEqual %bool %41 %float_0
OpSelectionMerge %46 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%47 = OpLoad %v2float %37
%48 = OpCompositeExtract %float %47 0
%50 = OpLoad %v2float %38
%51 = OpCompositeExtract %float %50 1
%52 = OpFSub %float %float_1 %51
%53 = OpFMul %float %48 %52
OpReturnValue %53
%45 = OpLabel
%55 = OpLoad %v2float %37
%56 = OpCompositeExtract %float %55 1
%57 = OpLoad %v2float %37
%58 = OpCompositeExtract %float %57 0
%59 = OpFSub %float %56 %58
OpStore %delta %59
%60 = OpFOrdEqual %bool %59 %float_0
OpSelectionMerge %63 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%64 = OpLoad %v2float %37
%65 = OpCompositeExtract %float %64 1
%66 = OpLoad %v2float %38
%67 = OpCompositeExtract %float %66 1
%68 = OpFMul %float %65 %67
%69 = OpLoad %v2float %37
%70 = OpCompositeExtract %float %69 0
%71 = OpLoad %v2float %38
%72 = OpCompositeExtract %float %71 1
%73 = OpFSub %float %float_1 %72
%74 = OpFMul %float %70 %73
%75 = OpFAdd %float %68 %74
%76 = OpLoad %v2float %38
%77 = OpCompositeExtract %float %76 0
%78 = OpLoad %v2float %37
%79 = OpCompositeExtract %float %78 1
%80 = OpFSub %float %float_1 %79
%81 = OpFMul %float %77 %80
%82 = OpFAdd %float %75 %81
OpReturnValue %82
%62 = OpLabel
%84 = OpLoad %v2float %38
%85 = OpCompositeExtract %float %84 1
%86 = OpLoad %v2float %38
%87 = OpCompositeExtract %float %86 0
%88 = OpLoad %v2float %37
%89 = OpCompositeExtract %float %88 1
%90 = OpFMul %float %87 %89
OpStore %91 %90
OpStore %92 %59
%93 = OpFunctionCall %float %guarded_divide_Qhhh %91 %92
%83 = OpExtInst %float %1 FMin %85 %93
OpStore %delta %83
%94 = OpLoad %v2float %37
%95 = OpCompositeExtract %float %94 1
%96 = OpFMul %float %83 %95
%97 = OpLoad %v2float %37
%98 = OpCompositeExtract %float %97 0
%99 = OpLoad %v2float %38
%100 = OpCompositeExtract %float %99 1
%101 = OpFSub %float %float_1 %100
%102 = OpFMul %float %98 %101
%103 = OpFAdd %float %96 %102
%104 = OpLoad %v2float %38
%105 = OpCompositeExtract %float %104 0
%106 = OpLoad %v2float %37
%107 = OpCompositeExtract %float %106 1
%108 = OpFSub %float %float_1 %107
%109 = OpFMul %float %105 %108
%110 = OpFAdd %float %103 %109
OpReturnValue %110
%63 = OpLabel
OpBranch %46
%46 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %112
%113 = OpLabel
%120 = OpVariable %_ptr_Function_v2float Function
%125 = OpVariable %_ptr_Function_v2float Function
%130 = OpVariable %_ptr_Function_v2float Function
%134 = OpVariable %_ptr_Function_v2float Function
%139 = OpVariable %_ptr_Function_v2float Function
%143 = OpVariable %_ptr_Function_v2float Function
%114 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%118 = OpLoad %v4float %114
%119 = OpVectorShuffle %v2float %118 %118 0 3
OpStore %120 %119
%121 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%123 = OpLoad %v4float %121
%124 = OpVectorShuffle %v2float %123 %123 0 3
OpStore %125 %124
%126 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %120 %125
%127 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%128 = OpLoad %v4float %127
%129 = OpVectorShuffle %v2float %128 %128 1 3
OpStore %130 %129
%131 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%132 = OpLoad %v4float %131
%133 = OpVectorShuffle %v2float %132 %132 1 3
OpStore %134 %133
%135 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %130 %134
%136 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%137 = OpLoad %v4float %136
%138 = OpVectorShuffle %v2float %137 %137 2 3
OpStore %139 %138
%140 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%141 = OpLoad %v4float %140
%142 = OpVectorShuffle %v2float %141 %141 2 3
OpStore %143 %142
%144 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %139 %143
%145 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%146 = OpLoad %v4float %145
%147 = OpCompositeExtract %float %146 3
%148 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%149 = OpLoad %v4float %148
%150 = OpCompositeExtract %float %149 3
%151 = OpFSub %float %float_1 %150
%152 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%153 = OpLoad %v4float %152
%154 = OpCompositeExtract %float %153 3
%155 = OpFMul %float %151 %154
%156 = OpFAdd %float %147 %155
%157 = OpCompositeConstruct %v4float %126 %135 %144 %156
OpStore %sk_FragColor %157
OpReturn
OpFunctionEnd
