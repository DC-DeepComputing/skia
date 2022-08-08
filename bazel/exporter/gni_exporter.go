// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"bytes"
	"fmt"
	"io"
	"path/filepath"
	"regexp"
	"sort"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"go.skia.org/skia/bazel/exporter/build_proto/build"
	"go.skia.org/skia/bazel/exporter/interfaces"
	"google.golang.org/protobuf/proto"
)

// writeState tracks the state of an in-progress write of the workspace.
type convertedState struct {
	convertedRules []string // All converted Bazel rules (bazel rule name).
}

// gniFileChunk represents a valid chunk of a GNI file.
type gniFileChunk []byte

// gniFileList represents a valid chunk of a GNI file which represents a
// file list. This is of the form:
//
//    file_sources = [
//      "$_include/core/SkPicture.cpp",
//      "$_include/core/SkPicture.h",
//    ]
type gniFileList struct {
	gniVarName string       // The file list variable name in the GNI file.
	data       gniFileChunk // GNI file list data written as text data.
}

type gniFileContents struct {
	wsGNIPath string        // Workspace relative path to *.gni file.
	data      []gniFileList // Unordered list of GNI chunks to be written to |wsGNIPath|.
}

// GNIExporter is an object responsible for exporting rules defined in a
// Bazel workspace as file lists in GNI format (GN's *.gni files). This exporter
// is very tightly coupled to the Skia Bazel rules and GNI configuration.
type GNIExporter struct {
	workspaceDir    string                      // The Bazel workspace path.
	openFileFn      interfaces.FileOpenFunc     // Function to open a file for writing.
	gniFileContents map[string]*gniFileContents // Map *.gni filenames to mutable file data object pointers.
}

// Represents an association between a workspace relative path (identified b
// a regular expression) and the output GNI file.
type gniPattern struct {
	reg *regexp.Regexp // reg to match workspace relative directory path.
	gni string         // Workspace relative *.gni file path.
}

// This list is processed in order to map directory paths to *.gni files.
var dirToGniMapper = []gniPattern{
	{reg: regexp.MustCompile(`^include/core$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/docs$`), gni: "gn/pdf.gni"},
	{reg: regexp.MustCompile(`^include/effects$`), gni: "gn/effects.gni"},
	{reg: regexp.MustCompile(`^include/pathops$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/private$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/private/chromium$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/utils$`), gni: "gn/utils.gni"},
	{reg: regexp.MustCompile(`^src/core$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^src/effects/imagefilters$`), gni: "gn/effects_imagefilters.gni"},
	{reg: regexp.MustCompile(`^src/opts$`), gni: "gn/opts.gni"},
	{reg: regexp.MustCompile(`^src/pathops$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^src/pdf$`), gni: "gn/pdf.gni"},
	{reg: regexp.MustCompile(`^src/shaders/gradients$`), gni: "gn/effects.gni"},
	{reg: regexp.MustCompile(`^src/shaders$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^src/text/gpu$`), gni: "gn/gpu.gni"},
	{reg: regexp.MustCompile(`^src/text$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^src/utils$`), gni: "gn/utils.gni"},
	{reg: regexp.MustCompile(`^include/private/gpu.*`), gni: "gn/gpu.gni"},
	{reg: regexp.MustCompile(`^src/effects.*`), gni: "gn/effects.gni"},
	{reg: regexp.MustCompile(`^src/gpu.*`), gni: "gn/gpu.gni"},
	{reg: regexp.MustCompile(`^src/ports.*`), gni: "gn/sksl.gni"},
	{reg: regexp.MustCompile(`^src/image.*`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/sksl.*`), gni: "gn/sksl.gni"},
	{reg: regexp.MustCompile(`^include/gpu.*`), gni: "gn/gpu.gni"},
	{reg: regexp.MustCompile(`^src/sksl.*`), gni: "gn/sksl.gni"},
	{reg: regexp.MustCompile(`.*skcms.*`), gni: "modules/skcms/skcms.gni"},
}

// A map of rules which should be merged into a target rule.
// Note: the function processing this list will not look for multi-level
//       merges. In other words, this will not work:
//
//            "A" → "B"
//            "B" → "C"
//
var mergeMap = map[string]string{
	"//include/private:private_hdrs":          "//src/core:core_srcs",
	"//include/private/chromium:private_hdrs": "//src/core:core_srcs",
	"//include/sksl:public_hdrs":              "//src/sksl:core_srcs",
	"//src/core:core_hdrs":                    "//src/core:core_srcs",
	"//src/core:sksl_hdrs":                    "//src/core:core_srcs",
	"//src/core:sksl_srcs":                    "//src/core:core_srcs",
	"//src/effects:effects_hdrs":              "//src/effects:effects_srcs",
	"//src/image:core_hdrs":                   "//src/core:core_srcs",
	"//src/image:core_srcs":                   "//src/core:core_srcs",
	"//src/pathops:pathops_hdrs":              "//src/pathops:pathops_srcs",
	"//src/pdf:pdf_hdrs":                      "//src/pdf:pdf_srcs",
	"//src/shaders:shader_hdrs":               "//src/core:core_srcs",
	"//src/shaders:shader_srcs":               "//src/core:core_srcs",
	"//src/shaders/gradients:gradient_hdrs":   "//src/effects:effects_srcs",
	"//src/shaders/gradients:gradient_srcs":   "//src/effects:effects_srcs",
	"//src/sksl:core_hdrs":                    "//src/sksl:core_srcs",
	"//src/sksl/analysis:analysis_hdrs":       "//src/sksl:core_srcs",
	"//src/sksl/analysis:analysis_srcs":       "//src/sksl:core_srcs",
	"//src/sksl/codegen:core_srcs":            "//src/sksl:core_srcs",
	"//src/sksl/codegen:gpu_hdrs":             "//src/sksl/codegen:gpu_srcs",
	"//src/sksl/codegen:srcs":                 "//src/sksl:core_srcs",
	"//src/sksl/dsl:srcs":                     "//src/sksl:core_srcs",
	"//src/sksl/dsl/priv:srcs":                "//src/sksl:core_srcs",
	"//src/sksl/ir:ir_hdrs":                   "//src/sksl:core_srcs",
	"//src/sksl/ir:ir_srcs":                   "//src/sksl:core_srcs",
	"//src/sksl/tracing:srcs":                 "//src/sksl:core_srcs",
	"//src/sksl/transform:transform_srcs":     "//src/sksl:core_srcs",
	"//src/text:text_hdrs":                    "//src/core:core_srcs",
	"//src/text:text_srcs":                    "//src/core:core_srcs",
	"//src/utils:core_hdrs":                   "//src/utils:core_srcs",
	"//src/utils:json_hdrs":                   "//src/utils:core_srcs",
	"//src/utils:json_srcs":                   "//src/utils:core_srcs",
}

// Override the generated rule (technically GNI file set) name. This is
// to match the current hardcoded file set names.
var ruleNameMap = map[string]string{
	"include_core_public_hdrs":                   "skia_core_public",
	"include_docs_public_hdrs":                   "skia_pdf_public",
	"include_effects_public_hdrs":                "skia_effects_public",
	"include_pathops_public_hdrs":                "skia_pathops_public",
	"include_utils_public_hdrs":                  "skia_utils_public",
	"src_core_core_srcs":                         "skia_core_sources",
	"src_effects_effects_srcs":                   "skia_effects_sources",
	"src_effects_imagefilters_imagefilters_hdrs": "skia_effects_imagefilter_public",
	"src_effects_imagefilters_imagefilters_srcs": "skia_effects_imagefilter_sources",
	"src_pathops_pathops_srcs":                   "skia_pathops_sources",
	"src_pdf_pdf_srcs":                           "skia_pdf_sources",
	"src_sksl_codegen_gpu_srcs":                  "skia_sksl_gpu_sources",
	"src_sksl_core_srcs":                         "skia_sksl_sources",
	"src_utils_core_srcs":                        "skia_utils_sources",
}

// fileListWriteOrder specifies the order of file lists to be written to a GNI file.
// Any file list not specified in this list is written after those specified, and
// in an undetermined order. This exists to ensure that the file contents are written
// in a stable ordering.
var fileListWriteOrder = map[string][]string{
	"gn/core.gni": {
		"skia_core_public",
		"skia_core_sources",
		"skia_pathops_public",
		"skia_pathops_sources"},
	"gn/effects.gni": {
		"skia_effects_public",
		"skia_effects_sources"},
	"gn/effects_imagefilters.gni": {
		"skia_effects_imagefilter_public",
		"skia_effects_imagefilter_sources"},
	"gn/gpu.gni": {
		"skia_gpu_sources",
		"skia_gl_gpu_sources",
		"skia_null_gpu_sources",
		"skia_gpu_sources",
		"skia_vk_sources",
		"skia_direct3d_sources",
		"skia_dawn_sources",
		"skia_metal_sources",
		"skia_native_gpu_sources",
		"skia_shared_gpu_sources"},
	"gn/pdf.gni": {
		"skia_pdf_public",
		"skia_pdf_sources"},
	"gn/sksl.gni": {
		"skia_sksl_sources",
		"skia_sksl_tracing_sources",
		"skia_sksl_gpu_sources",
		"skslc_deps"},
	"gn/utils.gni": {
		"skia_utils_public",
		"skia_utils_sources"},
}

// TODO: Verify accuracy of comment. It still seems to be used.
// https://source.chromium.org/search?q=skia_core_defines&ss=chromium
const coreGNIFooter = `
skia_core_sources += skia_pathops_sources

skia_core_public += skia_pathops_public

skia_core_defines = []  #  Used to be used by Chromium, but no longer.`

// Map of GNI file names to footer text to be appended to the end of the file.
var footerMap = map[string]string{
	"gn/core.gni": coreGNIFooter,
}

// NewGNIExporter creates an exporter that will export to GN's (*.gni) files.
func NewGNIExporter(workspaceDir string, openFileFn interfaces.FileOpenFunc) *GNIExporter {
	return &GNIExporter{
		workspaceDir:    workspaceDir,
		openFileFn:      openFileFn,
		gniFileContents: make(map[string]*gniFileContents),
	}
}

// Retrieve all rule attributes which are file targets.
func getRuleFiles(r *build.Rule, attrName string) ([]string, error) {
	items, err := getRuleStringArrayAttribute(r, attrName)
	if err != nil {
		return nil, skerr.Wrap(err)
	}

	var files []string
	for _, item := range items {
		if !isExternalRule(item) && isFileTarget(item) {
			files = append(files, item)
		}
	}
	return files, nil
}

// Convert the file path portion of a Bazel target name into a workspace
// relative path using variables to specify the base folder. The variables
// are one of $_src, $_include, or $_modules.
func makeRelativeFilePathForGNI(target string) (string, error) {
	_, path, t, err := parseRule(target)
	if err != nil {
		return "", skerr.Wrap(err)
	}
	wsRelativePath := filepath.Join(strings.TrimPrefix(path, "/"), t)
	if strings.HasPrefix(wsRelativePath, "src/") {
		return "$_src/" + strings.TrimPrefix(wsRelativePath, "src/"), nil
	}
	if strings.HasPrefix(wsRelativePath, "include/") {
		return "$_include/" + strings.TrimPrefix(wsRelativePath, "include/"), nil
	}
	if strings.HasPrefix(wsRelativePath, "modules/") {
		return "$_modules/" + strings.TrimPrefix(wsRelativePath, "modules/"), nil
	}

	return "", skerr.Fmt("can't find path for %q\n", target)
}

// Given a Bazel rule, return the GNI file list variable name in which
// to contain all files from the given rule.
func getRuleGNIVariableName(r *build.Rule) (string, error) {
	n, err := getRuleSimpleName(r.GetName())
	if err != nil {
		return "", skerr.Wrap(err)
	}
	// Check if there are specific overrides.
	if v, ok := ruleNameMap[n]; ok {
		return v, nil
	}
	return n, nil
}

// Is the file path portion of the given |target| a C/C++ header file?
func isTargetCppHeaderFile(target string) bool {
	_, _, t, err := parseRule(target)
	if err != nil {
		return false
	}
	ext := strings.ToLower(filepath.Ext(t))
	return ext == ".h" || ext == ".hpp"
}

// Should the given rule be skipped during the export process?
func shouldSkipRule(r *build.Rule) bool {
	if strings.Contains(r.GetName(), "public") {
		return false
	}
	if r.GetName() == "//include/private:private_hdrs" {
		return false
	}
	vis, err := getRuleStringArrayAttribute(r, "visibility")
	if err != nil {
		return false
	}
	if len(vis) != 1 || !strings.HasSuffix(vis[0], ":__pkg__") {
		return false
	}
	// Iterate over srcs because rules may put headers in srcs. Only if srcs
	// is made up solely of headers will it be skipped.
	srcs, err := getRuleStringArrayAttribute(r, "srcs")
	if err != nil || len(srcs) == 0 {
		return true
	}
	for _, item := range srcs {
		if isFileTarget(item) && !isTargetCppHeaderFile(item) {
			return false
		}
	}
	return true
}

// Write the *.gni file header.
func writeGNFileHeader(writer io.Writer) {
	const gniHeader = `# DO NOT EDIT: This is a generated file.

_src = get_path_info("../src", "abspath")
_include = get_path_info("../include", "abspath")
_modules = get_path_info("../modules", "abspath")`

	fmt.Fprintln(writer, gniHeader)
	fmt.Fprintln(writer)
}

// Print a warning, to stdout, of all files in the sorted list of
// files which are duplicated entries.
func warnDuplicates(sortedSlice []string) {
	for i, e := range sortedSlice {
		if i == len(sortedSlice)-1 {
			continue
		}
		if e == sortedSlice[i+1] {
			fmt.Printf("Warning, duplicate file: %s\n", e)
		}
	}
}

// Get the file list (|gniFileList|) object for the given file list variable name.
func (fd *gniFileContents) getFileList(fileListVariableName string) (gniFileList, bool) {
	for i := 0; i < len(fd.data); i++ {
		if fd.data[i].gniVarName == fileListVariableName {
			return fd.data[i], true
		}
	}
	return gniFileList{}, false
}

// Given an absolute path return a workspace relative path.
func (e *GNIExporter) absToWorkspacePath(absPath string) string {
	if !filepath.IsAbs(absPath) {
		panic("filepath not absolute")
	}
	return strings.TrimPrefix(absPath[len(e.workspaceDir):], "/")
}

// Given a workspace relative path return an absolute path.
func (e *GNIExporter) workspaceToAbsPath(wsPath string) string {
	if filepath.IsAbs(wsPath) {
		panic("filepath already absolute")
	}
	return filepath.Join(e.workspaceDir, wsPath)
}

// Retrieve (or create) a gniFileContents object for the given *.gni relative file path.
func (e *GNIExporter) getFileData(gniPath string) *gniFileContents {
	if d, ok := e.gniFileContents[gniPath]; ok {
		return d
	}
	d := &gniFileContents{wsGNIPath: gniPath}
	e.gniFileContents[gniPath] = d
	return d
}

// Map a rule location directory to the desired output *.gni file path.
// Returns a workspace relative path - not absolute.
func (e *GNIExporter) getLocationGNFilePath(absRuleLocationDir string) string {
	wsRelativePath := e.absToWorkspacePath(absRuleLocationDir)
	for _, entry := range dirToGniMapper {
		if entry.reg.Match([]byte(wsRelativePath)) {
			return filepath.Join(e.workspaceDir, entry.gni)
		}
	}
	// TODO: Remove when development is complete.
	const defaultGNI = "gn/XXX.gni"
	return filepath.Join(e.workspaceDir, defaultGNI)
}

// Get the desired output *.gni file path for the given rule.
// Returns a workspace relative path - not absolute.
func (e *GNIExporter) getRuleGNFilePath(r *build.Rule) (string, error) {
	// TODO: Un-hack this. This is one case where not all files in a directory map to
	// the same GNI file.
	if strings.Contains(r.GetName(), ":gpu_") {
		return "gn/gpu.gni", nil
	}
	dir, err := getLocationDir(r.GetLocation())
	if err != nil {
		return "", skerr.Wrap(err)
	}
	absPath := e.getLocationGNFilePath(dir)
	return e.absToWorkspacePath(absPath), nil
}

// Retrieve all sources ("srcs" attribute) and headers ("hdrs" attribute)
// values from the given rule and append to the provided list of files.
func getSrcsAndHdrs(r *build.Rule, files []string) ([]string, error) {
	items, err := getRuleFiles(r, "srcs")
	if err != nil {
		return []string{}, skerr.Wrap(err)
	}
	files = append(files, items...)

	items, err = getRuleFiles(r, "hdrs")
	if err != nil {
		return []string{}, skerr.Wrap(err)
	}
	files = append(files, items...)

	return files, nil
}

// Convert the given rule into a chunk of valid GNI configuration text. The
// converted project data is saved in GNIExporter instance for later writing.
func (e *GNIExporter) convertRule(r *build.Rule, qr *analysis_v2.CqueryResult, state *convertedState) error {
	if util.In(r.GetName(), state.convertedRules) {
		return nil
	}
	state.convertedRules = append(state.convertedRules, r.GetName())
	if shouldSkipRule(r) {
		return nil
	}
	if _, ok := mergeMap[r.GetName()]; ok {
		// Will find and write this rule later when the target rule is exported.
		return nil
	}
	files, err := getSrcsAndHdrs(r, []string{})
	if err != nil {
		return skerr.Wrap(err)
	}

	// If there are any rules to be merged into this rule, collect the
	// data from those rules and merge into this rules data.
	for otherName, ruleVariableName := range mergeMap {
		if ruleVariableName != r.GetName() {
			continue
		}
		or, err := findRule(qr, otherName)
		if err != nil {
			return skerr.Wrap(err)
		}
		state.convertedRules = append(state.convertedRules, or.GetName())

		files, err = getSrcsAndHdrs(or, files)
		if err != nil {
			return skerr.Wrap(err)
		}
	}

	if len(files) == 0 {
		return nil
	}

	// Convert all workspace relative paths into the format used in the GNI
	// file which uses variables as the root dir.
	for idx, target := range files {
		relativePath, err := makeRelativeFilePathForGNI(target)
		if err != nil {
			return skerr.Wrap(err)
		}
		files[idx] = relativePath
	}

	sort.Slice(files, func(i, j int) bool {
		// Generally sort alphabetically, but make $_include/ after &_src.
		isfx := files[i][0:strings.Index(files[i], "/")]
		jsfx := files[j][0:strings.Index(files[j], "/")]
		if isfx == jsfx {
			// GNI files are traditionally sorted ignoring case.
			return strings.ToLower(files[i]) < strings.ToLower(files[j])
		}
		return jsfx < isfx
	})
	warnDuplicates(files)

	ruleVariableName, err := getRuleGNIVariableName(r)
	if err != nil {
		return skerr.Wrap(err)
	}

	wsGNIPath, err := e.getRuleGNFilePath(r)
	if err != nil {
		return skerr.Wrap(err)
	}
	gniFileContents := e.getFileData(wsGNIPath)

	var contents bytes.Buffer
	fmt.Fprintf(&contents, "# %s\n", r.GetName())
	fmt.Fprintf(&contents, "%s = [\n", ruleVariableName)

	printedIncludeComment := false
	hasSrcs := false
	if strings.HasPrefix(files[0], "$_src") {
		hasSrcs = true
	}
	for _, target := range files {
		if hasSrcs && !printedIncludeComment && strings.HasPrefix(target, "$_include") {
			fmt.Fprintf(&contents, "\n  # Includes\n")
			printedIncludeComment = true
		}
		fmt.Fprintf(&contents, "  %q,\n", target)
	}
	fmt.Fprintln(&contents, "]")

	// Save and write later.
	gniFileContents.data = append(gniFileContents.data,
		gniFileList{gniVarName: ruleVariableName, data: contents.Bytes()})

	return nil
}

// Write all GNI file data to the appropriate files.
func (e *GNIExporter) writeFileData(gniFileContents *gniFileContents) error {
	writer, err := e.openFileFn(e.workspaceToAbsPath(gniFileContents.wsGNIPath))
	if err != nil {
		return skerr.Wrap(err)
	}

	writeGNFileHeader(writer)

	writtenVars := []string{}

	// Write out the GNI data.
	if variableWriteOrder, ok := fileListWriteOrder[gniFileContents.wsGNIPath]; ok {
		for i := 0; i < len(variableWriteOrder); i++ {
			variableName := variableWriteOrder[i]
			gniFileList, ok := gniFileContents.getFileList(variableName)
			if !ok {
				// TODO: Make this an error when all file sets are matched to rules.
				fmt.Printf("cannot find %s variable data\n", variableName)
				continue
			}
			_, err := writer.Write(gniFileList.data)
			if err != nil {
				return skerr.Wrap(err)
			}
			fmt.Fprintln(writer)
			writtenVars = append(writtenVars, variableName)
		}
	}

	for i := 0; i < len(gniFileContents.data); i++ {
		r := gniFileContents.data[i]
		if util.In(r.gniVarName, writtenVars) {
			continue
		}
		_, err := writer.Write(r.data)
		if err != nil {
			return skerr.Wrap(err)
		}
		fmt.Fprintln(writer)
		writtenVars = append(writtenVars, r.gniVarName)
		fmt.Fprintln(writer)
	}

	// Append the hardcoded file footers.
	for sfx, footer := range footerMap {
		if strings.HasSuffix(gniFileContents.wsGNIPath, sfx) {
			fmt.Fprintln(writer, footer)
			break
		}
	}
	return nil
}

// Export will convert all rules defined in the Bazel query
// command result to *.gni files within the workspace.
func (e *GNIExporter) Export(qcmd interfaces.QueryCommand) error {

	var state convertedState

	in, err := qcmd.Read()
	if err != nil {
		return skerr.Wrapf(err, "error reading bazel cquery data")
	}
	qr := &analysis_v2.CqueryResult{}
	if err := proto.Unmarshal(in, qr); err != nil {
		return skerr.Wrapf(err, "failed to unmarshal cquery result")
	}

	for _, result := range qr.GetResults() {
		t := result.GetTarget()
		r := t.GetRule()
		err := e.convertRule(r, qr, &state)
		if err != nil {
			return skerr.Wrapf(err, `error exporting rule %q`, r.GetName())
		}
	}

	for _, f := range e.gniFileContents {
		err = e.writeFileData(f)
		if err != nil {
			return skerr.Wrap(err)
		}
	}

	return nil
}

// Make sure GNIExporter fulfills the Exporter interface.
var _ interfaces.Exporter = (*GNIExporter)(nil)
