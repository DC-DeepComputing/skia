// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
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

type GNIExporter struct {
	workspaceDir string                  // The Bazel workspace path.
	openFileFn   interfaces.FileOpenFunc // Function to open a file for writing.
	gniFileMap   map[string]io.Writer    // Map *.gni filenames to file writers.
}

type gniPattern struct {
	reg *regexp.Regexp // reg to match workspace relative directory path.
	gni string         //workspace relative *.gni file path.
}

// TODO: Verify accuracy of comment. It still seems to be used.
// https://source.chromium.org/search?q=skia_core_defines&ss=chromium
const coreGNIFooter = `
skia_core_sources += skia_pathops_sources

skia_core_public += skia_pathops_public

skia_core_defines = []  #  Used to be used by Chromium, but no longer.`

// This list is processed in order to map directory paths to *.gni files.
var dirToGniMapper = []gniPattern{
	{reg: regexp.MustCompile(`^include/core$`), gni: "gn/core.gni"},
	{reg: regexp.MustCompile(`^include/docs$`), gni: "gn/pdf.gni"},
	{reg: regexp.MustCompile(`^include/effects$`), gni: "gn/effects.gni"},
	{reg: regexp.MustCompile(`^include/pathops$`), gni: "gn/core.gni"},
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
var mergeMap = map[string]string{
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

// Map of GNI file names to footer text to be appended to the end of the file.
var footerMap = map[string]string{
	"gn/core.gni": coreGNIFooter,
}

// NewGNIExporter creates an exporter that will export to GN's (*.gni) files.
func NewGNIExporter(workspaceDir string, openFileFn interfaces.FileOpenFunc) *GNIExporter {
	return &GNIExporter{
		workspaceDir: workspaceDir,
		openFileFn:   openFileFn,
		gniFileMap:   make(map[string]io.Writer),
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

func filePath(target string) (string, error) {
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

func getRuleGNIName(r *build.Rule) (string, error) {
	n, err := getRuleSimpleName(r.GetName())
	if err != nil {
		return "", skerr.Wrap(err)
	}
	if v, ok := ruleNameMap[n]; ok {
		return v, nil
	}
	return n, nil
}

func isTargetCppHeaderFile(target string) bool {
	_, _, t, err := parseRule(target)
	if err != nil {
		return false
	}
	ext := strings.ToLower(filepath.Ext(t))
	return ext == ".h" || ext == ".hpp"
}

func skipRule(r *build.Rule) bool {
	if strings.Contains(r.GetName(), "public") {
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

// Map a rule location directory to the desired output *.gni file path.
func (e *GNIExporter) getLocationGNFilePath(ruleLocationDir string) string {
	wsRelativePath := strings.TrimPrefix(ruleLocationDir[len(e.workspaceDir):], "/")
	for _, entry := range dirToGniMapper {
		if entry.reg.Match([]byte(wsRelativePath)) {
			return filepath.Join(e.workspaceDir, entry.gni)
		}
	}
	const defaultGNI = "gn/XXX.gni"
	return filepath.Join(e.workspaceDir, defaultGNI)
}

// Get the desired output *.gni file path for the given rule.
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
	return e.getLocationGNFilePath(dir), nil
}

// Obtain a writer to use for the given Bazel rule. The .gni configuration
// will be written using this writer.
func (e *GNIExporter) getRuleWriter(r *build.Rule) (io.Writer, error) {
	gnPath, err := e.getRuleGNFilePath(r)
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	w, ok := e.gniFileMap[gnPath]
	if ok {
		return w, nil
	}

	w, err = e.openFileFn(gnPath)
	if err != nil {
		return nil, skerr.Wrap(err)
	}
	writeGNFileHeader(w)

	e.gniFileMap[gnPath] = w
	return w, nil
}

// Export the *.gni configuration to the given rule to the appropriate
// output file.
func (e *GNIExporter) exportRule(r *build.Rule, qr *analysis_v2.CqueryResult, state *writeState) error {
	if util.In(r.GetName(), state.writtenRules) {
		return nil
	}
	state.writtenRules = append(state.writtenRules, r.GetName())
	if skipRule(r) {
		return nil
	}
	if _, ok := mergeMap[r.GetName()]; ok {
		// Will find and write this rule later on when the target rule is exported.
		return nil
	}
	files, err := getRuleFiles(r, "srcs")
	if err != nil {
		return skerr.Wrap(err)
	}
	items, err := getRuleFiles(r, "hdrs")
	if err != nil {
		return skerr.Wrap(err)
	}
	files = append(files, items...)

	// If there are any rules to be merged into this rule, collect the
	// data from those rules and merge into this rules data.
	for otherName, targetName := range mergeMap {
		if targetName != r.GetName() {
			continue
		}
		or, err := findRule(qr, otherName)
		if err != nil {
			return skerr.Wrap(err)
		}
		state.writtenRules = append(state.writtenRules, or.GetName())
		items, err = getRuleFiles(or, "srcs")
		if err != nil {
			return skerr.Wrap(err)
		}
		files = append(files, items...)
		items, err = getRuleFiles(or, "hdrs")
		if err != nil {
			return skerr.Wrap(err)
		}
		files = append(files, items...)
	}

	if len(files) == 0 {
		return nil
	}

	for idx, target := range files {
		relativePath, err := filePath(target)
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
			return files[i] < files[j]
		}
		return jsfx < isfx
	})
	writer, err := e.getRuleWriter(r)
	if err != nil {
		return skerr.Wrap(err)
	}

	targetName, err := getRuleGNIName(r)
	if err != nil {
		return skerr.Wrap(err)
	}

	fmt.Fprintf(writer, "# %s\n", r.GetName())
	fmt.Fprintf(writer, "%s = [\n", targetName)

	printedIncludeComment := false
	hasSrcs := false
	if strings.HasPrefix(files[0], "$_src") {
		hasSrcs = true
	}
	for _, target := range files {
		if hasSrcs && !printedIncludeComment && strings.HasPrefix(target, "$_include") {
			fmt.Fprintf(writer, "\n  # Includes\n")
			printedIncludeComment = true
		}
		fmt.Fprintf(writer, "  %q,\n", target)
	}

	fmt.Fprintln(writer, "]")
	fmt.Fprintln(writer)

	return nil
}

// Export will convert all rules defined in the Bazel query
// command result to *.gni files within the workspace.
func (e *GNIExporter) Export(qcmd interfaces.QueryCommand) error {

	var state writeState

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
		err := e.exportRule(r, qr, &state)
		if err != nil {
			return skerr.Wrapf(err, `error exporting rule %q`, r.GetName())
		}
	}

	// Append the hardcoded file footers.
	for fn, w := range e.gniFileMap {
		for sfx, footer := range footerMap {
			if strings.HasSuffix(fn, sfx) {
				fmt.Fprintln(w, footer)
				break
			}
		}
	}
	return nil
}

// Make sure GNIExporter fulfills the Exporter interface.
var _ interfaces.Exporter = (*GNIExporter)(nil)
