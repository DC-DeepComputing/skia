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
%69 = OpVariable %_ptr_Function_float Function
%l = OpVariable %_ptr_Function_float Function
%r = OpVariable %_ptr_Function_float Function
%t = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%vertexpos = OpVariable %_ptr_Function_v2float Function
%105 = OpVariable %_ptr_Function_float Function
%119 = OpVariable %_ptr_Function_float Function
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
%33 = OpLoad %int %ileft
%35 = OpIAdd %int %33 %int_1
%36 = OpLoad %int %sk_InstanceID
%38 = OpIMul %int %36 %int_1637
%39 = OpLoad %int %ileft
%40 = OpISub %int %int_17 %39
%41 = OpSMod %int %38 %40
%42 = OpIAdd %int %35 %41
OpStore %iright %42
%44 = OpLoad %int %sk_InstanceID
%46 = OpIMul %int %44 %int_313
%47 = OpSMod %int %46 %int_17
OpStore %itop %47
%49 = OpLoad %int %itop
%50 = OpIAdd %int %49 %int_1
%51 = OpLoad %int %sk_InstanceID
%53 = OpIMul %int %51 %int_1901
%54 = OpLoad %int %itop
%55 = OpISub %int %int_17 %54
%56 = OpSMod %int %53 %55
%57 = OpIAdd %int %50 %56
OpStore %ibot %57
OpStore %outset %float_0_03125
%62 = OpLoad %int %x
%63 = OpLoad %int %y
%64 = OpIAdd %int %62 %63
%66 = OpSMod %int %64 %int_2
%67 = OpIEqual %bool %int_0 %66
OpSelectionMerge %72 None
OpBranchConditional %67 %70 %71
%70 = OpLabel
%73 = OpLoad %float %outset
%74 = OpFNegate %float %73
OpStore %69 %74
OpBranch %72
%71 = OpLabel
%75 = OpLoad %float %outset
OpStore %69 %75
OpBranch %72
%72 = OpLabel
%76 = OpLoad %float %69
OpStore %outset %76
%78 = OpLoad %int %ileft
%79 = OpConvertSToF %float %78
%81 = OpFMul %float %79 %float_0_0625
%82 = OpFSub %float %81 %76
OpStore %l %82
%84 = OpLoad %int %iright
%85 = OpConvertSToF %float %84
%86 = OpFMul %float %85 %float_0_0625
%87 = OpFAdd %float %86 %76
OpStore %r %87
%89 = OpLoad %int %itop
%90 = OpConvertSToF %float %89
%91 = OpFMul %float %90 %float_0_0625
%92 = OpFSub %float %91 %76
OpStore %t %92
%94 = OpLoad %int %ibot
%95 = OpConvertSToF %float %94
%96 = OpFMul %float %95 %float_0_0625
%97 = OpFAdd %float %96 %76
OpStore %b %97
%100 = OpLoad %int %x
%101 = OpConvertSToF %float %100
%102 = OpLoad %int %sk_VertexID
%103 = OpSMod %int %102 %int_2
%104 = OpIEqual %bool %int_0 %103
OpSelectionMerge %108 None
OpBranchConditional %104 %106 %107
%106 = OpLabel
%109 = OpLoad %float %l
OpStore %105 %109
OpBranch %108
%107 = OpLabel
%110 = OpLoad %float %r
OpStore %105 %110
OpBranch %108
%108 = OpLabel
%111 = OpLoad %float %105
%112 = OpFAdd %float %101 %111
%113 = OpAccessChain %_ptr_Function_float %vertexpos %int_0
OpStore %113 %112
%114 = OpLoad %int %y
%115 = OpConvertSToF %float %114
%116 = OpLoad %int %sk_VertexID
%117 = OpSDiv %int %116 %int_2
%118 = OpIEqual %bool %int_0 %117
OpSelectionMerge %122 None
OpBranchConditional %118 %120 %121
%120 = OpLabel
%123 = OpLoad %float %t
OpStore %119 %123
OpBranch %122
%121 = OpLabel
%124 = OpLoad %float %b
OpStore %119 %124
OpBranch %122
%122 = OpLabel
%125 = OpLoad %float %119
%126 = OpFAdd %float %115 %125
%127 = OpAccessChain %_ptr_Function_float %vertexpos %int_1
OpStore %127 %126
%128 = OpLoad %int %sk_VertexID
%129 = OpSMod %int %128 %int_2
%130 = OpIEqual %bool %int_0 %129
%131 = OpSelect %int %130 %int_n1 %int_1
%133 = OpConvertSToF %float %131
%134 = OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_0
OpStore %134 %133
%136 = OpLoad %int %sk_VertexID
%137 = OpSDiv %int %136 %int_2
%138 = OpIEqual %bool %int_0 %137
%139 = OpSelect %int %138 %int_n1 %int_1
%140 = OpConvertSToF %float %139
%141 = OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_1
OpStore %141 %140
%142 = OpLoad %v2float %vertexpos
%143 = OpCompositeExtract %float %142 0
%144 = OpLoad %v2float %vertexpos
%145 = OpCompositeExtract %float %144 1
%148 = OpCompositeConstruct %v4float %143 %145 %float_0 %float_1
%149 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %149 %148
OpReturn
OpFunctionEnd
