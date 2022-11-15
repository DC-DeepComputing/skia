### Compilation failed:

error: SPIR-V validation error: Structure id 12 decorated as Block for variable in Uniform storage class must follow standard uniform buffer layout rules: member 0 contains an array with stride 4 not satisfying alignment to 16
  %testUniforms = OpTypeStruct %_arr_float_int_2 %S

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
OpDecorate %_arr_float_int_2 ArrayStride 16
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
OpDecorate %39 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%S = OpTypeStruct %_arr_float_int_2
%testPushConstants = OpTypeStruct %_arr_float_int_2 %S
%_ptr_PushConstant_testPushConstants = OpTypePointer PushConstant %testPushConstants
%3 = OpVariable %_ptr_PushConstant_testPushConstants PushConstant
%testUniforms = OpTypeStruct %_arr_float_int_2 %S
%_ptr_Uniform_testUniforms = OpTypePointer Uniform %testUniforms
%11 = OpVariable %_ptr_Uniform_testUniforms Uniform
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%void = OpTypeVoid
%21 = OpTypeFunction %void
%_ptr_Function_S = OpTypePointer Function %S
%int_0 = OpConstant %int 0
%_ptr_PushConstant__arr_float_int_2 = OpTypePointer PushConstant %_arr_float_int_2
%_ptr_Uniform__arr_float_int_2 = OpTypePointer Uniform %_arr_float_int_2
%int_1 = OpConstant %int 1
%_ptr_PushConstant_S = OpTypePointer PushConstant %S
%_ptr_Uniform_S = OpTypePointer Uniform %S
%true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%70 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%72 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%main = OpFunction %void None %21
%22 = OpLabel
%s1 = OpVariable %_ptr_Function_S Function
%s2 = OpVariable %_ptr_Function_S Function
%s3 = OpVariable %_ptr_Function_S Function
%s4 = OpVariable %_ptr_Function_S Function
%64 = OpVariable %_ptr_Function_v4float Function
%26 = OpAccessChain %_ptr_PushConstant__arr_float_int_2 %3 %int_0
%28 = OpLoad %_arr_float_int_2 %26
%29 = OpCompositeConstruct %S %28
OpStore %s1 %29
%31 = OpAccessChain %_ptr_Uniform__arr_float_int_2 %11 %int_0
%33 = OpLoad %_arr_float_int_2 %31
%34 = OpCompositeConstruct %S %33
OpStore %s2 %34
%37 = OpAccessChain %_ptr_PushConstant_S %3 %int_1
%39 = OpLoad %S %37
OpStore %s3 %39
%41 = OpAccessChain %_ptr_Uniform_S %11 %int_1
%43 = OpLoad %S %41
OpStore %s4 %43
%45 = OpCompositeExtract %float %28 0
%46 = OpCompositeExtract %float %33 0
%47 = OpFOrdEqual %bool %45 %46
%48 = OpCompositeExtract %float %28 1
%49 = OpCompositeExtract %float %33 1
%50 = OpFOrdEqual %bool %48 %49
%51 = OpLogicalAnd %bool %50 %47
OpSelectionMerge %53 None
OpBranchConditional %51 %53 %52
%52 = OpLabel
%54 = OpCompositeExtract %_arr_float_int_2 %39 0
%55 = OpCompositeExtract %_arr_float_int_2 %43 0
%56 = OpCompositeExtract %float %54 0
%57 = OpCompositeExtract %float %55 0
%58 = OpFOrdEqual %bool %56 %57
%59 = OpCompositeExtract %float %54 1
%60 = OpCompositeExtract %float %55 1
%61 = OpFOrdEqual %bool %59 %60
%62 = OpLogicalAnd %bool %61 %58
OpBranch %53
%53 = OpLabel
%63 = OpPhi %bool %true %22 %62 %52
OpSelectionMerge %68 None
OpBranchConditional %63 %66 %67
%66 = OpLabel
OpStore %64 %70
OpBranch %68
%67 = OpLabel
OpStore %64 %72
OpBranch %68
%68 = OpLabel
%73 = OpLoad %v4float %64
OpStore %sk_FragColor %73
OpReturn
OpFunctionEnd

1 error
