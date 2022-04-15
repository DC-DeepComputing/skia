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
OpName %g "g"
OpName %h "h"
OpName %i "i"
OpName %j "j"
OpName %k "k"
OpName %l "l"
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
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%33 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%34 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%mat2v3float = OpTypeMatrix %v3float 2
%36 = OpConstantComposite %mat2v3float %33 %34
%37 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%38 = OpConstantComposite %mat3v3float %33 %34 %37
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
%48 = OpConstantComposite %v2float %float_1 %float_0
%49 = OpConstantComposite %v2float %float_0 %float_1
%mat3v2float = OpTypeMatrix %v2float 3
%51 = OpConstantComposite %mat3v2float %48 %49 %19
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%mat4v2float = OpTypeMatrix %v2float 4
%60 = OpConstantComposite %mat4v2float %48 %49 %19 %19
%61 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%mat4v3float = OpTypeMatrix %v3float 4
%63 = OpConstantComposite %mat4v3float %33 %34 %37 %61
%64 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
%65 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
%66 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
%67 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%68 = OpConstantComposite %mat4v4float %64 %65 %66 %67
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat2v4float = OpTypeMatrix %v4float 2
%76 = OpConstantComposite %mat2v4float %64 %65
%mat3v4float = OpTypeMatrix %v4float 3
%78 = OpConstantComposite %mat3v4float %64 %65 %66
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
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
%g = OpVariable %_ptr_Function_mat3v3float Function
%h = OpVariable %_ptr_Function_mat3v3float Function
%i = OpVariable %_ptr_Function_mat4v4float Function
%j = OpVariable %_ptr_Function_mat4v4float Function
%k = OpVariable %_ptr_Function_mat2v4float Function
%l = OpVariable %_ptr_Function_mat4v2float Function
%100 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
OpStore %g %38
%39 = OpLoad %float %result
%42 = OpAccessChain %_ptr_Function_v3float %g %int_0
%44 = OpLoad %v3float %42
%45 = OpCompositeExtract %float %44 0
%46 = OpFAdd %float %39 %45
OpStore %result %46
OpStore %h %38
%52 = OpAccessChain %_ptr_Function_v3float %h %int_0
%53 = OpLoad %v3float %52
%54 = OpCompositeExtract %float %53 0
%55 = OpFAdd %float %46 %54
OpStore %result %55
OpStore %i %68
%69 = OpAccessChain %_ptr_Function_v4float %i %int_0
%71 = OpLoad %v4float %69
%72 = OpCompositeExtract %float %71 0
%73 = OpFAdd %float %55 %72
OpStore %result %73
OpStore %j %68
%79 = OpAccessChain %_ptr_Function_v4float %j %int_0
%80 = OpLoad %v4float %79
%81 = OpCompositeExtract %float %80 0
%82 = OpFAdd %float %73 %81
OpStore %result %82
OpStore %k %76
%85 = OpAccessChain %_ptr_Function_v4float %k %int_0
%86 = OpLoad %v4float %85
%87 = OpCompositeExtract %float %86 0
%88 = OpFAdd %float %82 %87
OpStore %result %88
%91 = OpVectorShuffle %v2float %64 %64 0 1
%92 = OpVectorShuffle %v2float %65 %65 0 1
%93 = OpCompositeConstruct %mat4v2float %91 %92 %19 %19
OpStore %l %93
%94 = OpAccessChain %_ptr_Function_v2float %l %int_0
%95 = OpLoad %v2float %94
%96 = OpCompositeExtract %float %95 0
%97 = OpFAdd %float %88 %96
OpStore %result %97
%99 = OpFOrdEqual %bool %97 %float_6
OpSelectionMerge %103 None
OpBranchConditional %99 %101 %102
%101 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%106 = OpLoad %v4float %104
OpStore %100 %106
OpBranch %103
%102 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%109 = OpLoad %v4float %107
OpStore %100 %109
OpBranch %103
%103 = OpLabel
%110 = OpLoad %v4float %100
OpReturnValue %110
OpFunctionEnd
