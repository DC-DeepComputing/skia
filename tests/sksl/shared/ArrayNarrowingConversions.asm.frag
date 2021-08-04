### Compilation failed:

error: SPIR-V validation error: ID 4294967295[%4294967295] has not been defined
  %51 = OpCompositeExtract %int %4294967295 0

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
OpName %i2 "i2"
OpName %s2 "s2"
OpName %f2 "f2"
OpName %h2 "h2"
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
OpDecorate %_arr_int_int_2 ArrayStride 16
OpDecorate %s2 RelaxedPrecision
OpDecorate %_arr_int_int_2_0 ArrayStride 16
OpDecorate %36 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %h2 RelaxedPrecision
OpDecorate %_arr_float_int_2_0 ArrayStride 16
OpDecorate %46 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_int_int_2 = OpTypeArray %int %int_2
%_ptr_Function__arr_int_int_2 = OpTypePointer Function %_arr_int_int_2
%int_1 = OpConstant %int 1
%_arr_int_int_2_0 = OpTypeArray %int %int_2
%_ptr_Function__arr_int_int_2_0 = OpTypePointer Function %_arr_int_int_2_0
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_arr_float_int_2_0 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2_0 = OpTypePointer Function %_arr_float_int_2_0
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%i2 = OpVariable %_ptr_Function__arr_int_int_2 Function
%s2 = OpVariable %_ptr_Function__arr_int_int_2_0 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%h2 = OpVariable %_ptr_Function__arr_float_int_2_0 Function
%69 = OpVariable %_ptr_Function_v4float Function
%32 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %i2 %32
%36 = OpCompositeConstruct %_arr_int_int_2_0 %int_1 %int_2
OpStore %s2 %36
%42 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %f2 %42
%46 = OpCompositeConstruct %_arr_float_int_2_0 %float_1 %float_2
OpStore %h2 %46
%48 = OpLoad %_arr_int_int_2 %i2
%49 = OpLoad %_arr_int_int_2_0 %s2
%50 = OpCompositeExtract %int %48 0
%51 = OpCompositeExtract %int %4294967295 0
%52 = OpIEqual %bool %50 %51
%53 = OpCompositeExtract %int %48 1
%54 = OpCompositeExtract %int %4294967295 1
%55 = OpIEqual %bool %53 %54
%56 = OpLogicalAnd %bool %55 %52
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%59 = OpLoad %_arr_float_int_2 %f2
%60 = OpLoad %_arr_float_int_2_0 %h2
%61 = OpCompositeExtract %float %59 0
%62 = OpCompositeExtract %float %4294967295 0
%63 = OpFOrdEqual %bool %61 %62
%64 = OpCompositeExtract %float %59 1
%65 = OpCompositeExtract %float %4294967295 1
%66 = OpFOrdEqual %bool %64 %65
%67 = OpLogicalAnd %bool %66 %63
OpBranch %58
%58 = OpLabel
%68 = OpPhi %bool %false %25 %67 %57
OpSelectionMerge %73 None
OpBranchConditional %68 %71 %72
%71 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %74
OpStore %69 %77
OpBranch %73
%72 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%79 = OpLoad %v4float %78
OpStore %69 %79
OpBranch %73
%73 = OpLabel
%80 = OpLoad %v4float %69
OpReturnValue %80
OpFunctionEnd

1 error
