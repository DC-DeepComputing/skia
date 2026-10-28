---
name: skia/use-lowercase-html-tags
description: When writing HTML markup or emitting HTML in generator tools, use lowercase tag names like <html> and <body> rather than uppercase tags
rubric: readability
severity: nit
---

**Rule.** When authoring HTML files or generating HTML markup in tools, write
all HTML tag names in lowercase (such as <html>, <head>, <body>, <style>, and
<div>) rather than uppercase or mixed-case. Keep tag casing uniform across
markup files and string templates.

**Why.** Although HTML parsers treat tag names as case-insensitive, modern web
standards and Skia codebase conventions standardize on lowercase markup. Using
uppercase tags or mixing casing (such as uppercase <HEAD>/<BODY> alongside
lowercase <script>) creates visual inconsistency across tools and documents.

## Examples

Bad:

```cpp
"<HTML>\n"
"<HEAD>\n"
"   <TITLE>SkVM Disassembler Output</TITLE>\n"
"   <STYLE>\n"
"   </STYLE>\n"
"    <script>\n"
"    </script>\n"
"</HEAD>\n"
"<BODY onload='initialize();'>\n"
```

Good:

```cpp
"<html>\n"
"<head>\n"
"   <title>SkVM Disassembler Output</title>\n"
"   <style>\n"
"   </style>\n"
"    <script>\n"
"    </script>\n"
"</head>\n"
"<body onload='initialize();'>\n"
```
