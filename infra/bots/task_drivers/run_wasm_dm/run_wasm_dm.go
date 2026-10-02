// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"encoding/base64"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"net/http"
	"net/http/httptest"
	"net/url"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"strings"

	_ "embed"

	"github.com/chromedp/cdproto/network"
	"github.com/chromedp/chromedp"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/golden/go/jsonio"
	"go.skia.org/infra/golden/go/types"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

//go:embed harness.html
var harnessHtml string

func main() {

	var (
		// Required flags for this task regardless of where it's running.
		dmFlagsStr      = flag.String("dm_flags", "", "JSON-encoded array of flags to pass to dm invocation")
		dmPropertiesStr = flag.String("dm_properties", "", "JSON-encoded array of properties to pass to dm invocation")
		buildPath       = flag.String("build", "", "The directory where the built wasm/js code is.")
		workCache       = flag.String("work_cache", "", "The cached directory to store task dependencies.")
		workPath        = flag.String("work_path", "work", "The directory to use to store temporary files.")

		// Required flags for running on CQ
		projectID = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskID    = flag.String("task_id", "", "ID of the task")
		taskName  = flag.String("task_name", "", "Name of the task.")
		// gitCommit     = flag.String("git_commit", "", "The commit at which we are testing.")
		// goldCtlPath   = flag.String("gold_ctl_path", "", "Path to the goldctl binary")
		goldHashesURL = flag.String("gold_hashes_url", "", "URL from which to download pre-existing hashes")

		// Provided for tryjobs
		// tryjobID      = flag.String("tryjob_id", "", "The id of the Buildbucket job for tryjobs. Omit for primary branch.")
		// changelistID  = flag.String("changelist_id", "", "The id of the Gerrit CL. Omit for primary branch.")
		// patchsetOrder = flag.String("patchset_order", "0", "Represents if this is the nth patchset")

		// Debugging flags.
		verbose     = flag.Bool("verbose", false, "If true, prints extra logging")
		local       = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
	)

	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)
	if err := os_steps.MkdirAll(ctx, *workPath); err != nil {
		td.Fatal(ctx, err)
	}

	const harnessUrl = "/run_wasm_dm.html"
	server := startLocalServer(ctx, *buildPath, harnessUrl)
	defer server.Close()

	cd := newChromeDriver(ctx, td.MustGetAbsolutePathOfFlag(ctx, *workCache, "work_cache"), *verbose)
	defer cd.shutdown()

	dmInputs := dmInputs{
		flags:           *dmFlagsStr,
		properties:      *dmPropertiesStr,
		knownHashes:     mustFetchFile(ctx, *goldHashesURL),
		knownHashesPath: newOpfsPath("/opfs/hashes.txt"),
		writeDir:        newOpfsPath("/opfs/dm"),
	}
	runDm(ctx, cd, &dmInputs, path.Join(server.URL, harnessUrl))

	dmResultsDir := transferImagesToHost(ctx, cd, dmInputs.writeDir, *workPath)
	uploadInfo := gatherImagesForUpload(ctx, dmResultsDir, dmInputs.knownHashes)
	uploadImagesToGold(ctx, uploadInfo, *local, *workPath)
}

type dmInputs struct {
	flags           string
	properties      string
	knownHashes     []byte
	knownHashesPath opfsFilePath
	writeDir        opfsFilePath
}

type goldUploadProps struct {
	goldResults    *jsonio.GoldResults
	digestOnly     []jsonio.Result
	imageAndDigest []jsonio.Result
	imageDir       string
}

type opfsFilePath struct {
	jsPath   string // path as it should be referenced from js-world
	wasmPath string // path as it should be referenced from wasm-world
}

func newOpfsPath(p string) opfsFilePath {
	// The OPFS root mount defined in the Skia wasm fs shim.
	const opfsRoot = "/opfs"
	jsPath := strings.TrimPrefix(p, opfsRoot)
	wasmPath := path.Join(opfsRoot, jsPath)
	return opfsFilePath{
		jsPath:   jsPath,
		wasmPath: wasmPath,
	}
}

func runDm(ctx context.Context, cd *chromeDriver, inputs *dmInputs, harnessUrl string) {
	ctx = td.StartStep(ctx, td.Props("run dm"))
	defer td.EndStep(ctx)

	// Navigate to the harness page and wait for it to load.
	if err := chromedp.Run(cd.ctx,
		network.Enable(),
		chromedp.Navigate(harnessUrl),
		chromedp.Poll(`typeof window.run === 'function'`, nil),
	); err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "navigate to %s", harnessUrl))
	}

	// Upload hashes file and assemble dm args
	if err := cd.EvalJsAsync(fmt.Sprintf("window.writeOpfsFile(%q, %q)", inputs.knownHashesPath.jsPath, base64.StdEncoding.EncodeToString(inputs.knownHashes)), nil); err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "writeOpfsFile"))
	}
	argv, err := jsonListAppend(ctx, inputs.flags,
		"--uninterestingHashesFile", inputs.knownHashesPath.wasmPath,
		"--writePath", inputs.writeDir.wasmPath,
		"-m", "pictureshader",
		"--properties",
	)
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "construct dm flags"))
	}
	argv, err = jsonListConcatMap(ctx, argv, inputs.properties)
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "construct dm flags"))
	}

	// Run dm.
	var status int
	if err := cd.EvalJsAsync(fmt.Sprintf("window.run(%s)", argv), &status); err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "evaluating js"))
	} else if status != 0 {
		td.Fatal(ctx, fmt.Errorf("dm returned non-zero status %d", status))
	}
}

func transferImagesToHost(ctx context.Context, cd *chromeDriver, dmWritePath opfsFilePath, workPath string) string {
	ctx = td.StartStep(ctx, td.Props("transfer images to host"))
	defer td.EndStep(ctx)

	var paths []string
	if err := cd.EvalJsAsync(fmt.Sprintf("listOpfsDir(%q)", dmWritePath.jsPath), &paths); err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "evaluating js: listOpfsDir"))
	}

	// Recreate the directory hierarchy for each file and copy it to local disk.
	// Not using os_steps here as it would create quite a lot of log output.
	for _, p := range paths {
		dir, filename := filepath.Split(filepath.Join(workPath, p))
		if err := os.MkdirAll(dir, 0o755); err != nil {
			td.Fatal(ctx, skerr.Wrapf(err, "mkdirs"))
		}

		var b64 string
		expr := fmt.Sprintf(`readOpfsFile(%q)`, p)
		if err := cd.EvalJsAsync(expr, &b64); err != nil {
			td.Fatal(ctx, skerr.Wrapf(err, "evaluating js: readOpfsFile"))
		}
		data, err := base64.StdEncoding.DecodeString(b64)
		if err != nil {
			td.Fatal(ctx, skerr.Wrapf(err, "base64 decode"))
		}

		dstFile := filepath.Join(dir, filename)
		if err := os.WriteFile(dstFile, data, 0o644); err != nil {
			td.Fatal(ctx, skerr.Wrapf(err, "evaluating js"))
		}
	}

	// The js path is already relative (to the opfs mount), so the local path is
	// just our work dir + the js output path.
	return filepath.Join(workPath, dmWritePath.jsPath)
}

func gatherImagesForUpload(ctx context.Context, dmWriteDir string, knownHashesBytes []byte) *goldUploadProps {
	ctx = td.StartStep(ctx, td.Props("gather images for upload"))
	defer td.EndStep(ctx)

	// Read dm results file
	gr, err := loadDmJson(filepath.Join(dmWriteDir, "dm.json"))
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "load dm.json file"))
	}

	// Index the known hashes
	known := map[types.Digest]struct{}{}
	for _, digest := range strings.Fields(string(knownHashesBytes)) {
		known[types.Digest(digest)] = struct{}{}
	}

	// Collect the new hashes
	seen := map[types.Digest]struct{}{}
	digestOnly := []jsonio.Result{}
	imageAndDigest := []jsonio.Result{}
	for _, r := range gr.Results {
		_, isKnown := known[r.Digest]
		_, isSeen := seen[r.Digest]
		if isKnown || isSeen {
			// Gold already has this image, or an earlier result in this loop
			// will upload, so we just reference it by digest.
			digestOnly = append(digestOnly, r)
		} else {
			imageAndDigest = append(imageAndDigest, r)
			seen[r.Digest] = struct{}{}
		}
	}

	td.StepText(ctx, "number of new hashes", strconv.Itoa(len(imageAndDigest)))
	return &goldUploadProps{
		goldResults:    gr,
		digestOnly:     digestOnly,
		imageAndDigest: imageAndDigest,
		imageDir:       dmWriteDir,
	}
}

func uploadImagesToGold(ctx context.Context, props *goldUploadProps, dryRun bool, workDir string) {
	ctx = td.StartStep(ctx, td.Props("upload images to gold"))
	defer td.EndStep(ctx)

	gold, err := newGoldClient(ctx, goldClientProps{
		DryRun:  dryRun,
		WorkDir: workDir,
	})
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "create gold client"))
	}

	if err := gold.Init(props.goldResults, props.digestOnly); err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "init gold with known results"))
	}

	for _, r := range props.imageAndDigest {
		name := types.TestName(r.Key[types.PrimaryKeyField])
		png := filepath.Join(props.imageDir, string(r.Digest)+".png")
		if err := gold.UploadImage(name, png, r.Digest, r.Key, r.Options); err != nil {
			td.Fatal(ctx, skerr.Wrapf(err, "upload to gold: %s (%s)", name, string(r.Digest)))
		}
	}

	if err := gold.Finalize(); err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "finalize gold client"))
	}

	td.StepText(ctx, "# new images uploaded to gold", strconv.Itoa(len(props.imageAndDigest)))
}

func loadDmJson(path string) (*jsonio.GoldResults, error) {
	b, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var gr jsonio.GoldResults
	if err := json.Unmarshal(b, &gr); err != nil {
		return nil, fmt.Errorf("unmarshal %s: %w", path, err)
	}
	if err := gr.UpdateLegacyFields(); err != nil {
		return nil, err
	}
	if err := gr.Validate(); err != nil {
		return nil, fmt.Errorf("dm.json failed validation: %w", err)
	}
	return &gr, nil
}

func startLocalServer(ctx context.Context, buildPath string, harnessPath string) *httptest.Server {
	ctx = td.StartStep(ctx, td.Props("start localhost server").Infra())
	defer td.EndStep(ctx)

	mux := http.NewServeMux()

	// The harness page is served at a known location.
	mux.HandleFunc(harnessPath, func(res http.ResponseWriter, r *http.Request) {
		fmt.Fprint(res, harnessHtml)
	})

	// Requests for anything besides the harness page are served from the build dir
	const prefix = "/"
	mux.Handle(prefix, http.StripPrefix(prefix, http.FileServer(http.Dir(buildPath))))

	// WASM pthreads require these headers.
	addHeaders := func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			w.Header().Set("Cross-Origin-Opener-Policy", "same-origin")
			w.Header().Set("Cross-Origin-Embedder-Policy", "require-corp")
			next.ServeHTTP(w, r)
		})
	}

	return httptest.NewServer(addHeaders(mux))
}

func jsonListAppend(ctx context.Context, jsonList string, newArgs ...string) (string, error) {
	var args []string
	if err := json.Unmarshal([]byte(jsonList), &args); err != nil {
		return "", err
	}
	result, err := json.Marshal(append(args, newArgs...))
	if err != nil {
		return "", err
	}
	return string(result), nil
}

func jsonListConcatMap(ctx context.Context, jsonList, jsonMap string) (string, error) {
	var m map[string]string
	if err := json.Unmarshal([]byte(jsonMap), &m); err != nil {
		return "", err
	}

	parts := make([]string, 0, 2*len(m))
	for k, v := range m {
		parts = append(parts, k, v)
	}

	return jsonListAppend(ctx, jsonList, parts...)
}

func mustFetchFile(ctx context.Context, urlStr string) []byte {
	ctx = td.StartStep(ctx, td.Props(fmt.Sprintf("fetch remote file %s", urlStr)).Infra())
	defer td.EndStep(ctx)

	u, err := url.Parse(urlStr)
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "bad url: '%s'", urlStr))
	}

	client := td.HttpClient(ctx, httputils.DefaultClientConfig().With2xxOnly().Client())
	resp, err := client.Get(u.String())
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "GET failed: '%s'", u.String()))
	}

	defer resp.Body.Close()
	data, err := io.ReadAll(resp.Body)
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "read failed: '%s'", u.String()))
	}

	return data
}
