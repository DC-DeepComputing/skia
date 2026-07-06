// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Destruction-Order Use-After-Free (UAF) Static Analyzer
//
// What it detects:
//   This tool identifies potential Use-After-Free (UAF) or read-after-free vulnerabilities
//   arising from incorrect C++ member variable declaration orders during destruction.
//
//   In C++, class members are constructed in declaration order and destroyed in reverse
//   declaration order. If a member variable 'MemberB' depends on a sibling member 'MemberA'
//   (either because 'MemberB' is initialized using 'MemberA' in the constructor, or through
//   type-to-type semantic relations), 'MemberA' must be declared BEFORE 'MemberB' so that
//   'MemberA' outlives 'MemberB'.
//
//   If they are declared in the wrong order, 'MemberA' is destroyed first. If 'MemberB'
//   attempts to access 'MemberA' during its destruction, it encounters a dangling reference/pointer.
//
// How to Run:
//   go run ./tools/destruction_analyzer [directory/file]
//
//   Example (scan include/):
//     go run ./tools/destruction_analyzer include/
//
// How to Test:
//   go test ./tools/destruction_analyzer/...
//
// NOTE regarding Bazel:
//   This tool relies on tree-sitter CGO bindings to parse C++ code correctly. Since Skia's
//   rules_go/Bazel toolchain explicitly disables CGO transitively (pure-Go build configuration),
//   compiling or testing this tool via Bazel targets is unsupported. Always use standard Go CLI
//   tooling ('go run', 'go test') which automatically enables CGO locally.

package main

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	sitter "github.com/smacker/go-tree-sitter"
	cpp "github.com/tree-sitter/tree-sitter-cpp/bindings/go"
)

// Warning represents a flagged potential UAF vulnerability.
type Warning struct {
	Class string `json:"class"`
	Field string `json:"field"` // Message format: "MemberB depends on MemberA"
	File  string `json:"file"`
	Line  int    `json:"line"`
}

// Semantic type-to-type dependencies.
// If TypeB is in this map under TypeA, then a member of TypeA depends on a member of TypeB.
// Therefore, the member of TypeA must be destroyed first, which means TypeB's member
// must be declared before TypeA's member (so TypeB's member is destroyed last).
var typeDependencies = map[string][]string{
	"TaskList":          {"Resource", "TextureProxy"},
	"GrDrawingManager":  {"GrProxyProvider"},
	"DrawBufferManager": {"UploadBufferManager"},
}

func main() {
	if len(os.Args) < 2 {
		// Default to scanning the entire repository
		cwd, err := os.Getwd()
		if err != nil {
			fmt.Printf("Error getting current working directory: %v\n", err)
			os.Exit(1)
		}
		scanDir(cwd)
		return
	}

	target := os.Args[1]
	scanDir(target)
}

// scanDir walks through target recursively, analyzing header files.
func scanDir(target string) {
	var allWarnings []Warning

	err := filepath.Walk(target, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		// Skip hidden folders (like .git, .cache)
		if info.IsDir() && strings.HasPrefix(info.Name(), ".") {
			return filepath.SkipDir
		}
		// Analyze C++ header files (.h)
		if !info.IsDir() && strings.HasSuffix(info.Name(), ".h") {
			content, err := os.ReadFile(path)
			if err != nil {
				return fmt.Errorf("error reading %s: %v", path, err)
			}
			warnings, err := AnalyzeSource(content)
			if err != nil {
				return fmt.Errorf("error analyzing %s: %v", path, err)
			}
			for i := range warnings {
				warnings[i].File = path
				allWarnings = append(allWarnings, warnings[i])
			}
		}
		return nil
	})

	if err != nil {
		fmt.Printf("Error during scan: %v\n", err)
		os.Exit(1)
	}

	// Output structured JSON as requested in the plan
	output, err := json.MarshalIndent(allWarnings, "", "  ")
	if err != nil {
		fmt.Printf("Error formatting JSON: %v\n", err)
		os.Exit(1)
	}
	fmt.Println(string(output))

	if len(allWarnings) > 0 {
		os.Exit(1)
	}
}

// AnalyzeSource parses a C++ source snippet and flags potential destruction-order UAFs.
func AnalyzeSource(source []byte) ([]Warning, error) {
	// Preprocess: Replace SK_API and SK_SPI with spaces to preserve byte offsets
	sourceStr := string(source)
	sourceStr = strings.ReplaceAll(sourceStr, "SK_API", "      ")
	sourceStr = strings.ReplaceAll(sourceStr, "SK_SPI", "      ")
	source = []byte(sourceStr)

	parser := sitter.NewParser()
	lang := sitter.NewLanguage(cpp.Language())
	parser.SetLanguage(lang)

	tree := parser.Parse(nil, source)
	root := tree.RootNode()

	var warnings []Warning
	var walk func(*sitter.Node)
	walk = func(node *sitter.Node) {
		if node.Type() == "class_specifier" || node.Type() == "struct_specifier" {
			warnings = append(warnings, analyzeClass(node, source)...)
		}
		for i := 0; i < int(node.ChildCount()); i++ {
			walk(node.Child(i))
		}
	}
	walk(root)

	return warnings, nil
}

type fieldInfo struct {
	Index     int
	TypeName  string
	IsTrivial bool
}

// analyzeClass inspects a single class/struct for member declaration order and constructor dependencies.
func analyzeClass(classNode *sitter.Node, source []byte) []Warning {
	var warnings []Warning

	className := getClassName(classNode, source)

	// 1. Locate the field declaration list
	var fieldDeclList *sitter.Node
	for i := 0; i < int(classNode.ChildCount()); i++ {
		if classNode.Child(i).Type() == "field_declaration_list" {
			fieldDeclList = classNode.Child(i)
			break
		}
	}
	if fieldDeclList == nil {
		return nil
	}

	// 2. Map all member fields to their declaration index and type name
	fields := make(map[string]fieldInfo)
	var fieldList []string
	var constructors []*sitter.Node

	for i := 0; i < int(fieldDeclList.ChildCount()); i++ {
		child := fieldDeclList.Child(i)
		if child.Type() == "field_declaration" {
			typeName := getCleanTypeName(child, source)
			rawOuterName := getRawOuterTypeName(child, source)
			rawTypeName := ""
			if child.ChildCount() > 0 {
				rawTypeName = child.Child(0).Content(source)
			}
			isPointer := strings.Contains(rawTypeName, "*")
			isReference := strings.Contains(rawTypeName, "&")
			isTrivial := isPointer || isReference || isTrivialType(rawOuterName)

			names := extractFieldIdentifiers(child, source)
			for _, name := range names {
				if _, exists := fields[name]; !exists {
					fields[name] = fieldInfo{
						Index:     len(fieldList),
						TypeName:  typeName,
						IsTrivial: isTrivial,
					}
					fieldList = append(fieldList, name)
				}
			}
		} else if isConstructor(child, className, source) {
			constructors = append(constructors, child)
		}
	}

	// 3. (a) Analyze semantic type-to-type dependencies first
	for nameB, infoB := range fields {
		if infoB.IsTrivial {
			continue // Skip trivial fields since they do not execute destructors
		}
		if deps, ok := typeDependencies[infoB.TypeName]; ok {
			for _, depType := range deps {
				for nameA, infoA := range fields {
					if infoA.TypeName == depType {
						if infoA.Index > infoB.Index {
							// Determine line number of field B for precise reporting
							line := 1
							for i := 0; i < int(fieldDeclList.ChildCount()); i++ {
								child := fieldDeclList.Child(i)
								if child.Type() == "field_declaration" {
									names := extractFieldIdentifiers(child, source)
									for _, n := range names {
										if n == nameB {
											line = int(child.StartPoint().Row + 1)
											break
										}
									}
								}
							}
							warnings = append(warnings, Warning{
								Class: className,
								Field: fmt.Sprintf("%s depends on %s", nameB, nameA),
								Line:  line,
							})
						}
					}
				}
			}
		}
	}

	// 3. (b) Analyze each constructor's dependencies
	for _, ctor := range constructors {
		// Analyze member initializer list
		var initList *sitter.Node
		for i := 0; i < int(ctor.ChildCount()); i++ {
			if ctor.Child(i).Type() == "field_initializer_list" {
				initList = ctor.Child(i)
				break
			}
		}

		if initList != nil {
			for i := 0; i < int(initList.ChildCount()); i++ {
				init := initList.Child(i)
				if init.Type() == "field_initializer" {
					memberB := getFieldInitializerName(init, source)
					idxB, isMemberB := fields[memberB]
					if isMemberB && !idxB.IsTrivial { // Skip trivial dependent fields
						// Extract all referenced identifiers in the initializer expression
						refs := extractReferencedIdentifiers(init, source)
						for _, ref := range refs {
							if ref == memberB {
								continue
							}
							idxA, isMemberA := fields[ref]
							if isMemberA && idxA.Index > idxB.Index {
								warnings = append(warnings, Warning{
									Class: className,
									Field: fmt.Sprintf("%s depends on %s", memberB, ref),
									Line:  int(init.StartPoint().Row + 1),
								})
							}
						}
					}
				}
			}
		}

		// Analyze constructor body assignments
		var body *sitter.Node
		for i := 0; i < int(ctor.ChildCount()); i++ {
			if ctor.Child(i).Type() == "compound_statement" {
				body = ctor.Child(i)
				break
			}
		}

		if body != nil {
			// Find all assignment expressions inside the body recursively
			var findAssignments func(*sitter.Node)
			findAssignments = func(n *sitter.Node) {
				if n.Type() == "assignment_expression" {
					if n.ChildCount() >= 3 {
						lhs := n.Child(0)
						rhs := n.Child(2)

						// Extract left-hand side member name
						lhsNames := extractFieldIdentifiers(lhs, source)
						if len(lhsNames) == 0 {
							lhsNames = extractReferencedIdentifiers(lhs, source)
						}

						for _, memberB := range lhsNames {
							idxB, isMemberB := fields[memberB]
							if isMemberB && !idxB.IsTrivial { // Skip trivial dependent fields
								refs := extractReferencedIdentifiers(rhs, source)
								for _, ref := range refs {
									if ref == memberB {
										continue
									}
									idxA, isMemberA := fields[ref]
									if isMemberA && idxA.Index > idxB.Index {
										warnings = append(warnings, Warning{
											Class: className,
											Field: fmt.Sprintf("%s depends on %s", memberB, ref),
											Line:  int(n.StartPoint().Row + 1),
										})
									}
								}
							}
						}
					}
				}
				for i := 0; i < int(n.ChildCount()); i++ {
					findAssignments(n.Child(i))
				}
			}
			findAssignments(body)
		}
	}

	return warnings
}

// isTrivialType returns true if the C++ type has a trivial destructor (primitive or math struct).
func isTrivialType(rawOuterName string) bool {
	// Raw pointers or references are handled before calling this, but as safety:
	if strings.Contains(rawOuterName, "*") || strings.Contains(rawOuterName, "&") {
		return true
	}

	// STL containers/pointers are non-trivial
	if strings.HasPrefix(rawOuterName, "std::") || strings.HasPrefix(rawOuterName, "sk_sp") || strings.HasPrefix(rawOuterName, "skia_private::") {
		return false
	}

	// 1. Primitive C++ types (lowercase names represents standard types: int, float, size_t, std::byte)
	if len(rawOuterName) > 0 {
		firstChar := rawOuterName[0]
		if firstChar >= 'a' && firstChar <= 'z' {
			return true
		}
	}

	// 2. Specific known trivial mathematical, structural, or helper types in Skia
	switch rawOuterName {
	case "SkPoint", "SkIPoint", "SkRect", "SkIRect", "SkScalar", "SkVector", "SkMatrix", "SkSize", "SkISize", "skvx::float2", "skvx::float4", "float2", "float4", "SkPathVerb", "SkColor":
		return true
	}

	return false
}

// getClassName returns the type identifier of a class/struct specifier.
func getClassName(node *sitter.Node, source []byte) string {
	for i := 0; i < int(node.ChildCount()); i++ {
		child := node.Child(i)
		if child.Type() == "type_identifier" {
			return child.Content(source)
		}
	}
	return "Anonymous"
}

// getRawOuterTypeName returns the raw type name before template unwinding (e.g., std::unique_ptr for std::unique_ptr<int>).
func getRawOuterTypeName(node *sitter.Node, source []byte) string {
	if node.ChildCount() == 0 {
		return ""
	}
	typeNode := node.Child(0)
	content := typeNode.Content(source)

	// Strip template arguments (everything after '<')
	if idx := strings.Index(content, "<"); idx != -1 {
		content = content[:idx]
	}
	return strings.Trim(content, "*& \t\n\r")
}

// getCleanTypeName extracts the clean C++ type name, stripping pointers, refs, and templates.
func getCleanTypeName(node *sitter.Node, source []byte) string {
	if node.ChildCount() == 0 {
		return ""
	}
	typeNode := node.Child(0)

	content := typeNode.Content(source)

	// Handle nested templates (e.g. unique_ptr<GrDrawingManager> -> GrDrawingManager)
	for {
		start := strings.Index(content, "<")
		end := strings.LastIndex(content, ">")
		if start == -1 || end == -1 || start >= end {
			break
		}
		content = content[start+1 : end]
	}

	// Handle sk_sp specifically (e.g. sk_sp<Resource> -> Resource)
	if strings.HasPrefix(content, "sk_sp<") {
		content = content[6 : len(content)-1]
	}

	return strings.Trim(content, "*& \t\n\r")
}

// extractFieldIdentifiers finds any field_identifier nodes in the subtree.
func extractFieldIdentifiers(node *sitter.Node, source []byte) []string {
	var names []string
	var find func(*sitter.Node)
	find = func(n *sitter.Node) {
		// Skip nested classes, structs, functions, or function declarators to avoid polluting field names
		if n.Type() == "class_specifier" || n.Type() == "struct_specifier" || n.Type() == "function_definition" || n.Type() == "function_declarator" {
			return
		}
		if n.Type() == "field_identifier" {
			names = append(names, n.Content(source))
		}
		for i := 0; i < int(n.ChildCount()); i++ {
			find(n.Child(i))
		}
	}
	find(node)
	return names
}

// extractReferencedIdentifiers finds all identifier and field_identifier nodes in the subtree.
func extractReferencedIdentifiers(node *sitter.Node, source []byte) []string {
	var names []string
	var find func(*sitter.Node)
	find = func(n *sitter.Node) {
		// Skip nested classes, structs, functions, or function declarators to avoid polluting field names
		if n.Type() == "class_specifier" || n.Type() == "struct_specifier" || n.Type() == "function_definition" || n.Type() == "function_declarator" {
			return
		}
		if n.Type() == "identifier" || n.Type() == "field_identifier" {
			names = append(names, n.Content(source))
		}
		for i := 0; i < int(n.ChildCount()); i++ {
			find(n.Child(i))
		}
	}
	find(node)
	return names
}

// isConstructor returns true if a function definition is a constructor of className.
func isConstructor(node *sitter.Node, className string, source []byte) bool {
	if node.Type() != "function_definition" {
		return false
	}
	var decl *sitter.Node
	for i := 0; i < int(node.ChildCount()); i++ {
		if node.Child(i).Type() == "function_declarator" {
			decl = node.Child(i)
			break
		}
	}
	if decl == nil {
		return false
	}
	for i := 0; i < int(decl.ChildCount()); i++ {
		child := decl.Child(i)
		if child.Type() == "identifier" || child.Type() == "field_identifier" || child.Type() == "qualified_identifier" {
			name := child.Content(source)
			if name == className || (len(name) > len(className) && name[len(name)-len(className):] == className && name[len(name)-len(className)-1] == ':') {
				return true
			}
		}
	}
	return false
}

// getFieldInitializerName extracts the initialized field name from a field_initializer node.
func getFieldInitializerName(node *sitter.Node, source []byte) string {
	for i := 0; i < int(node.ChildCount()); i++ {
		child := node.Child(i)
		if child.Type() == "field_identifier" {
			return child.Content(source)
		}
	}
	return ""
}
