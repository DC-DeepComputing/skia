#!/usr/bin/env python3
#
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# GLSL ES2 conformance test files can be found at
# https://github.com/KhronosGroup/VK-GL-CTS/tree/master/data/gles2/shaders

import os
import pprint
import pyparsing as pp
import sys

pipeList = pp.delimited_list(pp.SkipTo(pp.Literal("|") | pp.Literal("]")), delim="|")
bracketedPipeList = (pp.Literal("[").suppress() +
                     pp.Group(pipeList) +
                     pp.Literal("]").suppress())
unbracketedValue = pp.Group(pp.SkipTo(";"))

valueList = (pp.Word(pp.alphanums) +  # type
             pp.Word(pp.alphanums) +  # varname
             pp.Literal("=").suppress() +
             (bracketedPipeList | unbracketedValue) +
             pp.Literal(";").suppress())
value = pp.Group((pp.Keyword("input") | pp.Keyword("output") | pp.Keyword("uniform")) +
                 valueList)
values = pp.Group(pp.Keyword("values") +
                  pp.Literal("{").suppress() +
                  pp.ZeroOrMore(value) +
                  pp.Literal("}").suppress())

expectation = pp.Group(pp.Keyword("expect").suppress() +
                       (pp.Keyword("compile_fail") | pp.Keyword("pass")))

code = ((pp.Group(pp.Keyword("both") + pp.QuotedString('""', multiline=True))) |
        (pp.Group(pp.Keyword("vertex") + pp.QuotedString('""', multiline=True)) +
                  pp.Keyword("fragment") + pp.QuotedString('""', multiline=True)))

desc = pp.Keyword("desc") + pp.QuotedString('"')

version100es = pp.Keyword("version") + pp.Keyword("100") + pp.Keyword("es")

reqGlsl100 = pp.Keyword("require") + pp.Keyword("full_glsl_es_100_support")

ignoredCaseItem = (desc | version100es | reqGlsl100).suppress()

caseItem = values | expectation | code | ignoredCaseItem

caseBody = pp.ZeroOrMore(caseItem)

caseHeader = pp.Keyword("case") + pp.Word(pp.alphanums + '_')
case = pp.Group(caseHeader + caseBody + pp.Keyword("end").suppress())

groupHeader = pp.Keyword("group") + pp.Word(pp.alphanums + '_') + pp.QuotedString('"')

group = pp.Forward()
group <<= pp.OneOrMore((pp.Group(pp.Keyword("group") +
                                 pp.Word(pp.alphanums + '_') +
                                 pp.QuotedString('"')) +
                        group +
                        pp.Keyword("end").suppress()) |
                       case)

grammar = pp.OneOrMore(group)
grammar.ignore('#' + pp.restOfLine)

out = grammar.parse_string(sys.stdin.read(), parse_all=True)

out.pprint()
