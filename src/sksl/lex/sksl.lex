// *****************
// *** IMPORTANT ***
// *****************
//
// 1. This file is only used when gn arg skia_lex is set to true. It is used to regenerate the
//    SkSLLexer.h and SkSLLexer.cpp files.
// 2. Since token IDs are used to identify operators and baked into the .dehydrated.sksl files,
//    after modifying this file it is likely everything will break until you update the dehydrated
//    binaries. If things break after updating the lexer, set REHYDRATE in SkSLCompiler.cpp to 0,
//    rebuild, and then set it back to 1.

FLOAT_LITERAL  = [0-9]*\.[0-9]+([eE][+-]?[0-9]+)?|[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?|[0-9]+([eE][+-]?[0-9]+)
INT_LITERAL    = ([1-9][0-9]*|0[0-7]*|0[xX][0-9a-fA-F]+)[uU]?
BAD_OCTAL      = (0[0-9]+)[uU]?
TRUE_LITERAL   = "true"
FALSE_LITERAL  = "false"
IF             = "if"
ELSE           = "else"
FOR            = "for"
WHILE          = "while"
DO             = "do"
SWITCH         = "switch"
CASE           = "case"
DEFAULT        = "default"
BREAK          = "break"
CONTINUE       = "continue"
DISCARD        = "discard"
RETURN         = "return"
IN             = "in"
OUT            = "out"
INOUT          = "inout"
UNIFORM        = "uniform"
CONST          = "const"
FLAT           = "flat"
NOPERSPECTIVE  = "noperspective"
INLINE         = "inline"
NOINLINE       = "noinline"
PURE           = "$pure"
READONLY       = "readonly"
WRITEONLY      = "writeonly"
BUFFER         = "buffer"
STRUCT         = "struct"
LAYOUT         = "layout"
HIGHP          = "highp"
MEDIUMP        = "mediump"
LOWP           = "lowp"
ES3            = "$es3"
EXPORT         = "$export"
WORKGROUP      = "workgroup"

// Layout Tokens (treated as identifiers outside of a layout block)
ORIGIN_UPPER_LEFT           = "origin_upper_left"
PUSH_CONSTANT               = "push_constant"
BLEND_SUPPORT_ALL_EQUATIONS = "blend_support_all_equations"
COLOR                       = "color"
LOCATION                    = "location"
OFFSET                      = "offset"
BINDING                     = "binding"
TEXTURE                     = "texture"
SAMPLER                     = "sampler"
INDEX                       = "index"
SET                         = "set"
BUILTIN                     = "builtin"
INPUT_ATTACHMENT_INDEX      = "input_attachment_index"
SPIRV                       = "spirv"
METAL                       = "metal"
WGSL                        = "wgsl"
RGBA8                       = "rgba8"
RGBA32F                     = "rgba32f"
R32F                        = "r32f"
LOCAL_SIZE_X                = "local_size_x"
LOCAL_SIZE_Y                = "local_size_y"
LOCAL_SIZE_Z                = "local_size_z"

RESERVED       = atomic|attribute|varying|precision|invariant|asm|class|union|enum|typedef|template|this|packed|goto|volatile|public|static|extern|external|interface|long|double|fixed|unsigned|superp|input|output|hvec[234]|dvec[234]|fvec[234]|sampler[13]D|sampler[12]DShadow|sampler3DRect|sampler2DRectShadow|samplerCube|sizeof|cast|namespace|using|gl_[0-9a-zA-Z_]*
PRIVATE_IDENTIFIER = $[0-9a-zA-Z_]*
IDENTIFIER     = [a-zA-Z_][0-9a-zA-Z_]*
DIRECTIVE      = #[a-zA-Z_][0-9a-zA-Z_]*
LPAREN         = "("
RPAREN         = ")"
LBRACE         = "{"
RBRACE         = "}"
LBRACKET       = "["
RBRACKET       = "]"
DOT            = "."
COMMA          = ","
PLUSPLUS       = "++"
MINUSMINUS     = "--"
PLUS           = "+"
MINUS          = "-"
STAR           = "*"
SLASH          = "/"
PERCENT        = "%"
SHL            = "<<"
SHR            = ">>"
BITWISEOR      = "|"
BITWISEXOR     = "^"
BITWISEAND     = "&"
BITWISENOT     = "~"
LOGICALOR      = "||"
LOGICALXOR     = "^^"
LOGICALAND     = "&&"
LOGICALNOT     = "!"
QUESTION       = "?"
COLON          = ":"
EQ             = "="
EQEQ           = "=="
NEQ            = "!="
GT             = ">"
LT             = "<"
GTEQ           = ">="
LTEQ           = "<="
PLUSEQ         = "+="
MINUSEQ        = "-="
STAREQ         = "*="
SLASHEQ        = "/="
PERCENTEQ      = "%="
SHLEQ          = "<<="
SHREQ          = ">>="
BITWISEOREQ    = "|="
BITWISEXOREQ   = "^="
BITWISEANDEQ   = "&="
SEMICOLON      = ";"
WHITESPACE     = \s+
LINE_COMMENT   = //.*
BLOCK_COMMENT  = /\*([^*]|\*[^/])*\*/
INVALID        = .
