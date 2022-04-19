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
OpName %cf2 "cf2"
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
OpDecorate %34 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %h2 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
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
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_int_int_2 = OpTypeArray %int %int_2
%_ptr_Function__arr_int_int_2 = OpTypePointer Function %_arr_int_int_2
%int_1 = OpConstant %int 1
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
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
%s2 = OpVariable %_ptr_Function__arr_int_int_2 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%h2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%cf2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%64 = OpVariable %_ptr_Function_v4float Function
%32 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %i2 %32
%34 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %s2 %34
%40 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %f2 %40
%42 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %h2 %42
OpStore %i2 %34
OpStore %s2 %34
OpStore %f2 %42
OpStore %h2 %42
OpStore %cf2 %40
%45 = OpIEqual %bool %int_1 %int_1
%46 = OpIEqual %bool %int_2 %int_2
%47 = OpLogicalAnd %bool %46 %45
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%50 = OpFOrdEqual %bool %float_1 %float_1
%51 = OpFOrdEqual %bool %float_2 %float_2
%52 = OpLogicalAnd %bool %51 %50
OpBranch %49
%49 = OpLabel
%53 = OpPhi %bool %false %25 %52 %48
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%56 = OpLogicalAnd %bool %46 %45
OpBranch %55
%55 = OpLabel
%57 = OpPhi %bool %false %49 %56 %54
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%60 = OpFOrdEqual %bool %float_1 %float_1
%61 = OpFOrdEqual %bool %float_2 %float_2
%62 = OpLogicalAnd %bool %61 %60
OpBranch %59
%59 = OpLabel
%63 = OpPhi %bool %false %55 %62 %58
OpSelectionMerge %68 None
OpBranchConditional %63 %66 %67
%66 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %69
OpStore %64 %72
OpBranch %68
%67 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%74 = OpLoad %v4float %73
OpStore %64 %74
OpBranch %68
%68 = OpLabel
%75 = OpLoad %v4float %64
OpReturnValue %75
OpFunctionEnd
