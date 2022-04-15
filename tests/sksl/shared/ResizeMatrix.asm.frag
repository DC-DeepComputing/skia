OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %result "result"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%33 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%34 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%35 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%mat3v3float = OpTypeMatrix %v3float 3
%37 = OpConstantComposite %mat3v3float %33 %34 %35
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%49 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
%50 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
%51 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
%52 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%mat4v4float = OpTypeMatrix %v4float 4
%54 = OpConstantComposite %mat4v4float %49 %50 %51 %52
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%74 = OpConstantComposite %v2float %float_1 %float_0
%75 = OpConstantComposite %v2float %float_0 %float_1
%76 = OpConstantComposite %mat2v2float %74 %75
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_6 = OpConstant %float 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
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
%result = OpVariable %_ptr_Function_float Function
%a = OpVariable %_ptr_Function_mat2v2float Function
%b = OpVariable %_ptr_Function_mat2v2float Function
%c = OpVariable %_ptr_Function_mat3v3float Function
%d = OpVariable %_ptr_Function_mat3v3float Function
%e = OpVariable %_ptr_Function_mat4v4float Function
%f = OpVariable %_ptr_Function_mat2v2float Function
%102 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%38 = OpVectorShuffle %v2float %33 %33 0 1
%39 = OpVectorShuffle %v2float %34 %34 0 1
%40 = OpCompositeConstruct %mat2v2float %38 %39
OpStore %a %40
%41 = OpLoad %float %result
%44 = OpAccessChain %_ptr_Function_v2float %a %int_0
%45 = OpLoad %v2float %44
%46 = OpCompositeExtract %float %45 0
%47 = OpFAdd %float %41 %46
OpStore %result %47
%55 = OpVectorShuffle %v2float %49 %49 0 1
%56 = OpVectorShuffle %v2float %50 %50 0 1
%57 = OpCompositeConstruct %mat2v2float %55 %56
OpStore %b %57
%58 = OpAccessChain %_ptr_Function_v2float %b %int_0
%59 = OpLoad %v2float %58
%60 = OpCompositeExtract %float %59 0
%61 = OpFAdd %float %47 %60
OpStore %result %61
%64 = OpVectorShuffle %v3float %49 %49 0 1 2
%65 = OpVectorShuffle %v3float %50 %50 0 1 2
%66 = OpVectorShuffle %v3float %51 %51 0 1 2
%67 = OpCompositeConstruct %mat3v3float %64 %65 %66
OpStore %c %67
%68 = OpAccessChain %_ptr_Function_v3float %c %int_0
%70 = OpLoad %v3float %68
%71 = OpCompositeExtract %float %70 0
%72 = OpFAdd %float %61 %71
OpStore %result %72
OpStore %d %37
%77 = OpAccessChain %_ptr_Function_v3float %d %int_0
%78 = OpLoad %v3float %77
%79 = OpCompositeExtract %float %78 0
%80 = OpFAdd %float %72 %79
OpStore %result %80
OpStore %e %54
%83 = OpAccessChain %_ptr_Function_v4float %e %int_0
%85 = OpLoad %v4float %83
%86 = OpCompositeExtract %float %85 0
%87 = OpFAdd %float %80 %86
OpStore %result %87
%89 = OpVectorShuffle %v3float %49 %49 0 1 2
%90 = OpVectorShuffle %v3float %50 %50 0 1 2
%91 = OpVectorShuffle %v3float %51 %51 0 1 2
%92 = OpCompositeConstruct %mat3v3float %89 %90 %91
%93 = OpVectorShuffle %v2float %89 %89 0 1
%94 = OpVectorShuffle %v2float %90 %90 0 1
%95 = OpCompositeConstruct %mat2v2float %93 %94
OpStore %f %95
%96 = OpAccessChain %_ptr_Function_v2float %f %int_0
%97 = OpLoad %v2float %96
%98 = OpCompositeExtract %float %97 0
%99 = OpFAdd %float %87 %98
OpStore %result %99
%101 = OpFOrdEqual %bool %99 %float_6
OpSelectionMerge %105 None
OpBranchConditional %101 %103 %104
%103 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%108 = OpLoad %v4float %106
OpStore %102 %108
OpBranch %105
%104 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%111 = OpLoad %v4float %109
OpStore %102 %111
OpBranch %105
%105 = OpLabel
%112 = OpLoad %v4float %102
OpReturnValue %112
OpFunctionEnd
