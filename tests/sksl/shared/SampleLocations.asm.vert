OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3 %sk_InstanceID %sk_VertexID %vcoord_Stage0
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %sk_InstanceID "sk_InstanceID"
OpName %sk_VertexID "sk_VertexID"
OpName %vcoord_Stage0 "vcoord_Stage0"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %ileft "ileft"
OpName %iright "iright"
OpName %itop "itop"
OpName %ibot "ibot"
OpName %outset "outset"
OpName %l "l"
OpName %r "r"
OpName %t "t"
OpName %b "b"
OpName %vertexpos "vertexpos"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %sk_PerVertex Block
OpDecorate %sk_InstanceID BuiltIn InstanceIndex
OpDecorate %sk_VertexID BuiltIn VertexIndex
OpDecorate %vcoord_Stage0 Location 1
OpDecorate %vcoord_Stage0 NoPerspective
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%sk_InstanceID = OpVariable %_ptr_Input_int Input
%sk_VertexID = OpVariable %_ptr_Input_int Input
%v2float = OpTypeVector %float 2
%_ptr_Output_v2float = OpTypePointer Output %v2float
%vcoord_Stage0 = OpVariable %_ptr_Output_v2float Output
%void = OpTypeVoid
%16 = OpTypeFunction %void
%_ptr_Function_int = OpTypePointer Function %int
%int_200 = OpConstant %int 200
%int_929 = OpConstant %int 929
%int_17 = OpConstant %int 17
%int_1 = OpConstant %int 1
%int_1637 = OpConstant %int 1637
%int_313 = OpConstant %int 313
%int_1901 = OpConstant %int 1901
%_ptr_Function_float = OpTypePointer Function %float
%float_0_03125 = OpConstant %float 0.03125
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%bool = OpTypeBool
%float_0_0625 = OpConstant %float 0.0625
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int_n1 = OpConstant %int -1
%_ptr_Output_float = OpTypePointer Output %float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%main = OpFunction %void None %16
%17 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%y = OpVariable %_ptr_Function_int Function
%ileft = OpVariable %_ptr_Function_int Function
%iright = OpVariable %_ptr_Function_int Function
%itop = OpVariable %_ptr_Function_int Function
%ibot = OpVariable %_ptr_Function_int Function
%outset = OpVariable %_ptr_Function_float Function
%63 = OpVariable %_ptr_Function_float Function
%l = OpVariable %_ptr_Function_float Function
%r = OpVariable %_ptr_Function_float Function
%t = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%vertexpos = OpVariable %_ptr_Function_v2float Function
%99 = OpVariable %_ptr_Function_float Function
%113 = OpVariable %_ptr_Function_float Function
%20 = OpLoad %int %sk_InstanceID
%22 = OpSMod %int %20 %int_200
OpStore %x %22
%24 = OpLoad %int %sk_InstanceID
%25 = OpSDiv %int %24 %int_200
OpStore %y %25
%27 = OpLoad %int %sk_InstanceID
%29 = OpIMul %int %27 %int_929
%31 = OpSMod %int %29 %int_17
OpStore %ileft %31
%34 = OpIAdd %int %31 %int_1
%35 = OpLoad %int %sk_InstanceID
%37 = OpIMul %int %35 %int_1637
%38 = OpISub %int %int_17 %31
%39 = OpSMod %int %37 %38
%40 = OpIAdd %int %34 %39
OpStore %iright %40
%42 = OpLoad %int %sk_InstanceID
%44 = OpIMul %int %42 %int_313
%45 = OpSMod %int %44 %int_17
OpStore %itop %45
%47 = OpIAdd %int %45 %int_1
%48 = OpLoad %int %sk_InstanceID
%50 = OpIMul %int %48 %int_1901
%51 = OpISub %int %int_17 %45
%52 = OpSMod %int %50 %51
%53 = OpIAdd %int %47 %52
OpStore %ibot %53
OpStore %outset %float_0_03125
%58 = OpIAdd %int %22 %25
%60 = OpSMod %int %58 %int_2
%61 = OpIEqual %bool %int_0 %60
OpSelectionMerge %66 None
OpBranchConditional %61 %64 %65
%64 = OpLabel
%67 = OpLoad %float %outset
%68 = OpFNegate %float %67
OpStore %63 %68
OpBranch %66
%65 = OpLabel
%69 = OpLoad %float %outset
OpStore %63 %69
OpBranch %66
%66 = OpLabel
%70 = OpLoad %float %63
OpStore %outset %70
%72 = OpLoad %int %ileft
%73 = OpConvertSToF %float %72
%75 = OpFMul %float %73 %float_0_0625
%76 = OpFSub %float %75 %70
OpStore %l %76
%78 = OpLoad %int %iright
%79 = OpConvertSToF %float %78
%80 = OpFMul %float %79 %float_0_0625
%81 = OpFAdd %float %80 %70
OpStore %r %81
%83 = OpLoad %int %itop
%84 = OpConvertSToF %float %83
%85 = OpFMul %float %84 %float_0_0625
%86 = OpFSub %float %85 %70
OpStore %t %86
%88 = OpLoad %int %ibot
%89 = OpConvertSToF %float %88
%90 = OpFMul %float %89 %float_0_0625
%91 = OpFAdd %float %90 %70
OpStore %b %91
%94 = OpLoad %int %x
%95 = OpConvertSToF %float %94
%96 = OpLoad %int %sk_VertexID
%97 = OpSMod %int %96 %int_2
%98 = OpIEqual %bool %int_0 %97
OpSelectionMerge %102 None
OpBranchConditional %98 %100 %101
%100 = OpLabel
%103 = OpLoad %float %l
OpStore %99 %103
OpBranch %102
%101 = OpLabel
%104 = OpLoad %float %r
OpStore %99 %104
OpBranch %102
%102 = OpLabel
%105 = OpLoad %float %99
%106 = OpFAdd %float %95 %105
%107 = OpAccessChain %_ptr_Function_float %vertexpos %int_0
OpStore %107 %106
%108 = OpLoad %int %y
%109 = OpConvertSToF %float %108
%110 = OpLoad %int %sk_VertexID
%111 = OpSDiv %int %110 %int_2
%112 = OpIEqual %bool %int_0 %111
OpSelectionMerge %116 None
OpBranchConditional %112 %114 %115
%114 = OpLabel
%117 = OpLoad %float %t
OpStore %113 %117
OpBranch %116
%115 = OpLabel
%118 = OpLoad %float %b
OpStore %113 %118
OpBranch %116
%116 = OpLabel
%119 = OpLoad %float %113
%120 = OpFAdd %float %109 %119
%121 = OpAccessChain %_ptr_Function_float %vertexpos %int_1
OpStore %121 %120
%122 = OpLoad %int %sk_VertexID
%123 = OpSMod %int %122 %int_2
%124 = OpIEqual %bool %int_0 %123
%125 = OpSelect %int %124 %int_n1 %int_1
%127 = OpConvertSToF %float %125
%128 = OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_0
OpStore %128 %127
%130 = OpLoad %int %sk_VertexID
%131 = OpSDiv %int %130 %int_2
%132 = OpIEqual %bool %int_0 %131
%133 = OpSelect %int %132 %int_n1 %int_1
%134 = OpConvertSToF %float %133
%135 = OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_1
OpStore %135 %134
%136 = OpLoad %v2float %vertexpos
%137 = OpCompositeExtract %float %136 0
%138 = OpLoad %v2float %vertexpos
%139 = OpCompositeExtract %float %138 1
%142 = OpCompositeConstruct %v4float %137 %139 %float_0 %float_1
%143 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %143 %142
OpReturn
OpFunctionEnd
