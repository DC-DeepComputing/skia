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

DEC_FLOAT_LITERAL  = (0[fh]|[1-9][0-9]*[fh]|[0-9]*\.[0-9]+([eE][+-]?[0-9]+)?[fh]?|[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?[fh]?|[0-9]+[eE][+-]?[0-9]+[fh]?)
HEX_FLOAT_LITERAL  = (0[xX][0-9a-fA-F]*\.[0-9a-fA-F]+([pP][+-]?[0-9]+[fh]?)?|0[xX][0-9a-fA-F]+\.[0-9a-fA-F]*([pP][+-]?[0-9]+[fh]?)?|/0[xX][0-9a-fA-F]+[pP][+-]?[0-9]+[fh]?)
DEC_INT_LITERAL    = (0[iu]?|[1-9][0-9]*[iu]?)
HEX_INT_LITERAL    = 0[xX][0-9a-fA-F]+[iu]?
ALIAS             = "alias"
BITCAST           = "bitcast"
BREAK             = "break"
CASE              = "case"
CONST             = "const"
CONST_ASSERT      = "const_assert"
CONTINUE          = "continue"
CONTINUING        = "continuing"
DIAGNOSTIC        = "diagnostic"
DISCARD           = "discard"
DEFAULT           = "default"
ELSE              = "else"
ENABLE            = "enable"
FALLTHROUGH       = "fallthrough"
FALSE             = "false"
FN                = "fn"
FOR               = "for"
IF                = "if"
LET               = "let"
LOOP              = "loop"
OVERRIDE          = "override"
RETURN            = "return"
REQUIRES          = "requires"
STRUCT            = "struct"
SWITCH            = "switch"
TRUE              = "true"
VAR               = "var"
WHILE             = "while"
ATTR               = "@"
LPAREN             = "("
RPAREN             = ")"
LBRACE             = "{"
RBRACE             = "}"
LBRACKET           = "["
RBRACKET           = "]"
DOT                = "."
COMMA              = ","
ARROW              = "->"
PLUSPLUS           = "++"
MINUSMINUS         = "--"
PLUS               = "+"
MINUS              = "-"
STAR               = "*"
SLASH              = "/"
PERCENT            = "%"
SHL                = "<<"
SHR                = ">>"
BITWISEOR          = "|"
BITWISEXOR         = "^"
BITWISEAND         = "&"
BITWISENOT         = "~"
LOGICALOR          = "||"
LOGICALAND         = "&&"
LOGICALNOT         = "!"
QUESTION           = "?"
COLON              = ":"
EQ                 = "="
EQEQ               = "=="
NEQ                = "!="
GT                 = ">"
LT                 = "<"
GTEQ               = ">="
LTEQ               = "<="
PLUSEQ             = "+="
MINUSEQ            = "-="
STAREQ             = "*="
SLASHEQ            = "/="
PERCENTEQ          = "%="
SHLEQ              = "<<="
SHREQ              = ">>="
BITWISEOREQ        = "|="
BITWISEXOREQ       = "^="
BITWISEANDEQ       = "&="
UNDERSCORE         = "_"
SEMICOLON          = ";"
TRUE_LITERAL       = "true"
FALSE_LITERAL      = "false"
// Caller must enforce valid UTF8 and check XID_Start/XID_Continue
IDENTIFIER         = [a-zA-Z_\x80-\xF7][0-9a-zA-Z_\x80-\xF7]*
WHITESPACE         = \s+
LINE_COMMENT       = //.*
BLOCK_COMMENT      = /\*([^*]|\*[^/])*\*/
INVALID            = .
