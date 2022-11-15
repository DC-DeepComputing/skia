### Compilation failed:

error: SPIR-V validation error: Expected Constituent type to be equal to the corresponding member type of Result Type struct
  %35 = OpCompositeConstruct %S %34

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %S "S"
OpMemberName %S 0 "a"
OpName %testPushConstants "testPushConstants"
OpMemberName %testPushConstants 0 "pushConstantArray"
OpMemberName %testPushConstants 1 "pushConstantStruct"
OpName %testUniforms "testUniforms"
OpMemberName %testUniforms 0 "uboArray"
OpMemberName %testUniforms 1 "uboStruct"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %main "main"
OpName %s1 "s1"
OpName %s2 "s2"
OpName %s3 "s3"
OpName %s4 "s4"
OpDecorate %_arr_float_int_2 ArrayStride 4
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %testPushConstants 0 Offset 0
OpMemberDecorate %testPushConstants 1 Offset 8
OpMemberDecorate %testPushConstants 1 RelaxedPrecision
OpDecorate %testPushConstants Block
OpDecorate %_arr_float_int_2_0 ArrayStride 16
OpMemberDecorate %testUniforms 0 Offset 0
OpMemberDecorate %testUniforms 1 Offset 32
OpMemberDecorate %testUniforms 1 RelaxedPrecision
OpDecorate %testUniforms Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %40 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%S = OpTypeStruct %_arr_float_int_2
%testPushConstants = OpTypeStruct %_arr_float_int_2 %S
%_ptr_PushConstant_testPushConstants = OpTypePointer PushConstant %testPushConstants
%3 = OpVariable %_ptr_PushConstant_testPushConstants PushConstant
%_arr_float_int_2_0 = OpTypeArray %float %int_2
%testUniforms = OpTypeStruct %_arr_float_int_2_0 %S
%_ptr_Uniform_testUniforms = OpTypePointer Uniform %testUniforms
%11 = OpVariable %_ptr_Uniform_testUniforms Uniform
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%void = OpTypeVoid
%22 = OpTypeFunction %void
%_ptr_Function_S = OpTypePointer Function %S
%int_0 = OpConstant %int 0
%_ptr_PushConstant__arr_float_int_2 = OpTypePointer PushConstant %_arr_float_int_2
%_ptr_Uniform__arr_float_int_2_0 = OpTypePointer Uniform %_arr_float_int_2_0
%int_1 = OpConstant %int 1
%_ptr_PushConstant_S = OpTypePointer PushConstant %S
%_ptr_Uniform_S = OpTypePointer Uniform %S
%true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%71 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%73 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%main = OpFunction %void None %22
%23 = OpLabel
%s1 = OpVariable %_ptr_Function_S Function
%s2 = OpVariable %_ptr_Function_S Function
%s3 = OpVariable %_ptr_Function_S Function
%s4 = OpVariable %_ptr_Function_S Function
%65 = OpVariable %_ptr_Function_v4float Function
%27 = OpAccessChain %_ptr_PushConstant__arr_float_int_2 %3 %int_0
%29 = OpLoad %_arr_float_int_2 %27
%30 = OpCompositeConstruct %S %29
OpStore %s1 %30
%32 = OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %11 %int_0
%34 = OpLoad %_arr_float_int_2_0 %32
%35 = OpCompositeConstruct %S %34
OpStore %s2 %35
%38 = OpAccessChain %_ptr_PushConstant_S %3 %int_1
%40 = OpLoad %S %38
OpStore %s3 %40
%42 = OpAccessChain %_ptr_Uniform_S %11 %int_1
%44 = OpLoad %S %42
OpStore %s4 %44
%46 = OpCompositeExtract %float %29 0
%47 = OpCompositeExtract %float %34 0
%48 = OpFOrdEqual %bool %46 %47
%49 = OpCompositeExtract %float %29 1
%50 = OpCompositeExtract %float %34 1
%51 = OpFOrdEqual %bool %49 %50
%52 = OpLogicalAnd %bool %51 %48
OpSelectionMerge %54 None
OpBranchConditional %52 %54 %53
%53 = OpLabel
%55 = OpCompositeExtract %_arr_float_int_2_0 %40 0
%56 = OpCompositeExtract %_arr_float_int_2_0 %44 0
%57 = OpCompositeExtract %float %55 0
%58 = OpCompositeExtract %float %56 0
%59 = OpFOrdEqual %bool %57 %58
%60 = OpCompositeExtract %float %55 1
%61 = OpCompositeExtract %float %56 1
%62 = OpFOrdEqual %bool %60 %61
%63 = OpLogicalAnd %bool %62 %59
OpBranch %54
%54 = OpLabel
%64 = OpPhi %bool %true %23 %63 %53
OpSelectionMerge %69 None
OpBranchConditional %64 %67 %68
%67 = OpLabel
OpStore %65 %71
OpBranch %69
%68 = OpLabel
OpStore %65 %73
OpBranch %69
%69 = OpLabel
%74 = OpLoad %v4float %65
OpStore %sk_FragColor %74
OpReturn
OpFunctionEnd

1 error
