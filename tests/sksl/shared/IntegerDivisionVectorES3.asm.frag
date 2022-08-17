OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %zero "zero"
OpName %one "one"
OpName %x "x"
OpName %y "y"
OpName %_0_x "_0_x"
OpName %_1_result "_1_result"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_100 = OpConstant %int 100
%53 = OpConstantComposite %v4int %int_100 %int_100 %int_100 %int_100
%v4bool = OpTypeVector %bool 4
%67 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%int_1 = OpConstant %int 1
%80 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%float_1 = OpConstant %float 1
%float_0_00392156886 = OpConstant %float 0.00392156886
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
%zero = OpVariable %_ptr_Function_int Function
%one = OpVariable %_ptr_Function_int Function
%x = OpVariable %_ptr_Function_v4int Function
%y = OpVariable %_ptr_Function_int Function
%_0_x = OpVariable %_ptr_Function_v4int Function
%_1_result = OpVariable %_ptr_Function_v4int Function
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %29
%33 = OpCompositeExtract %float %32 0
%34 = OpConvertFToS %int %33
OpStore %zero %34
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %36
%38 = OpCompositeExtract %float %37 1
%39 = OpConvertFToS %int %38
OpStore %one %39
%43 = OpCompositeConstruct %v4int %34 %34 %34 %34
OpStore %x %43
OpBranch %44
%44 = OpLabel
OpLoopMerge %48 %47 None
OpBranch %45
%45 = OpLabel
%51 = OpLoad %v4int %x
%50 = OpSLessThan %v4bool %51 %53
%49 = OpAll %bool %50
OpBranchConditional %49 %46 %48
%46 = OpLabel
%56 = OpLoad %int %one
OpStore %y %56
OpBranch %57
%57 = OpLabel
OpLoopMerge %61 %60 None
OpBranch %58
%58 = OpLabel
%62 = OpLoad %int %y
%63 = OpSLessThan %bool %62 %int_100
OpBranchConditional %63 %59 %61
%59 = OpLabel
%65 = OpLoad %v4int %x
OpStore %_0_x %65
OpStore %_1_result %67
OpBranch %68
%68 = OpLabel
OpLoopMerge %72 %71 None
OpBranch %69
%69 = OpLabel
%75 = OpLoad %v4int %_0_x
%76 = OpLoad %int %y
%77 = OpCompositeConstruct %v4int %76 %76 %76 %76
%74 = OpSGreaterThanEqual %v4bool %75 %77
%73 = OpAll %bool %74
OpBranchConditional %73 %70 %72
%70 = OpLabel
%78 = OpLoad %v4int %_1_result
%81 = OpIAdd %v4int %78 %80
OpStore %_1_result %81
%82 = OpLoad %v4int %_0_x
%83 = OpLoad %int %y
%84 = OpCompositeConstruct %v4int %83 %83 %83 %83
%85 = OpISub %v4int %82 %84
OpStore %_0_x %85
OpBranch %71
%71 = OpLabel
OpBranch %68
%72 = OpLabel
%86 = OpLoad %v4int %x
%87 = OpLoad %int %y
%88 = OpCompositeConstruct %v4int %87 %87 %87 %87
%89 = OpSDiv %v4int %86 %88
%90 = OpLoad %v4int %_1_result
%91 = OpINotEqual %v4bool %89 %90
%92 = OpAny %bool %91
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpLoad %v4int %x
%97 = OpCompositeExtract %int %96 0
%98 = OpConvertSToF %float %97
%100 = OpFMul %float %98 %float_0_00392156886
%101 = OpLoad %int %y
%102 = OpConvertSToF %float %101
%103 = OpFMul %float %102 %float_0_00392156886
%104 = OpCompositeConstruct %v4float %float_1 %100 %103 %float_1
OpReturnValue %104
%94 = OpLabel
OpBranch %60
%60 = OpLabel
%105 = OpLoad %int %y
%106 = OpLoad %int %one
%107 = OpIAdd %int %105 %106
OpStore %y %107
OpBranch %57
%61 = OpLabel
OpBranch %47
%47 = OpLabel
%108 = OpLoad %v4int %x
%109 = OpLoad %int %one
%110 = OpCompositeConstruct %v4int %109 %109 %109 %109
%111 = OpIAdd %v4int %108 %110
OpStore %x %111
OpBranch %44
%48 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%113 = OpLoad %v4float %112
OpReturnValue %113
OpFunctionEnd
