// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"net/http"
	"net/http/httptest"
	"net/url"
	"os"
	"path/filepath"

	_ "embed"

	"github.com/chromedp/chromedp"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

//go:embed harness.html
var harnessHtml []byte

func main() {
	var (
		// Required flags for this task regardless of where it's running.
		dmFlagsStr      = flag.String("dm_flags", "", "JSON-encoded array of flags to pass to dm invocation")
		dmPropertiesStr = flag.String("dm_properties", "", "JSON-encoded array of properties to pass to dm invocation")
		workPath        = flag.String("work_path", "", "The directory to store temporary files.")
		chromeCache     = flag.String("chrome_cache", "", "The directory to store the Chrome binary.")
		buildPath       = flag.String("build", "", "The directory where the built wasm/js code is.")

		// Required flags for running on CQ
		projectID     = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskID        = flag.String("task_id", "", "ID of the task")
		taskName      = flag.String("task_name", "", "Name of the task.")
		goldHashesURL = flag.String("gold_hashes_url", "", "URL from which to download pre-existing hashes")

		// Debugging flags.
		local = flag.Bool(
			"local",
			false,
			"True if running locally (as opposed to on the bots)",
		)
		outputSteps = flag.String(
			"o",
			"",
			"If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.",
		)
	)

	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)
	if err := os_steps.MkdirAll(ctx, *workPath); err != nil {
		td.Fatal(ctx, err)
	}

	cd, err := newChromeDriver(ctx, td.MustGetAbsolutePathOfFlag(ctx, *chromeCache, "chrome_cache"))
	if err != nil {
		td.Fatal(ctx, err)
	}
	defer cd.shutdown()

	knownHashes := mustFetchFile(ctx, *goldHashesURL)
	imagesOutDir := filepath.Join(td.MustGetAbsolutePathOfFlag(ctx, *workPath, "work_path"), "images")
	server, err := startLocalServer(ctx, knownHashes, *buildPath, imagesOutDir)
	if err != nil {
		td.Fatal(ctx, err)
	}
	defer server.Close()

	argv := dmArgv{
		writePath:  "/tmp/images", // inside js filesystem
		match:      "pictureshader",
		properties: mustUnmarshalFlag[map[string]string](ctx, *dmPropertiesStr, "dm_properties"),
		extraFlags: mustUnmarshalFlag[[]string](ctx, *dmFlagsStr, "dm_flags"),
	}
	if err := runDm(ctx, cd, server.URL, &argv); err != nil {
		td.Fatal(ctx, err)
	}
}

type dmArgv struct {
	writePath  string
	match      string
	properties map[string]string
	extraFlags []string
}

func (a dmArgv) ToJson() ([]byte, error) {
	argv := []string{}
	if len(a.writePath) > 0 {
		argv = append(argv, "--writePath", a.writePath)
	}
	if len(a.match) > 0 {
		argv = append(argv, "-m", a.match)
	}
	if len(a.properties) > 0 {
		argv = append(argv, "--properties")
		for k, v := range a.properties {
			argv = append(argv, k, v)
		}
	}
	argv = append(argv, a.extraFlags...)
	return json.Marshal(argv)
}

func runDm(ctx context.Context, cd *chromeDriver, url string, argv *dmArgv) error {
	ctx = td.StartStep(ctx, td.Props("run dm").Infra())
	defer td.EndStep(ctx)

	argvJson, err := argv.ToJson()
	if err != nil {
		return td.FailStep(ctx, err)
	}

	// Load harness page and wait for it to load
	if err := chromedp.Do(cd.ctx, chromedp.Navigate(url)); err != nil {
		return err
	} else if _, err := chromedp.Run(cd.ctx, chromedp.Poll[bool](`typeof window.run === 'function'`)); err != nil {
		return err
	}

	// Run
	rc, err := chromedp.Run(cd.ctx,
		chromedp.Evaluate[int](fmt.Sprintf(`window.run(%s)`, argvJson), chromedp.EvalAwaitPromise),
	)
	if err != nil {
		return td.FailStep(ctx, err)
	}

	sklog.Infof("dm exit code: %d", rc)
	return nil
}

func startLocalServer(ctx context.Context, knownHashes []byte, buildPath, outDir string) (*httptest.Server, error) {
	ctx = td.StartStep(ctx, td.Props("start localhost server").Infra())
	defer td.EndStep(ctx)

	if _, err := os_steps.Stat(ctx, filepath.Join(buildPath, "dm.js")); err != nil {
		return nil, td.FailStep(ctx, skerr.Wrapf(err, "can't find dm.json, did you provide the right build path?"))
	}

	harnessPath := filepath.Join(buildPath, "index.html")
	if err := os_steps.WriteFile(ctx, harnessPath, harnessHtml, 0o644); err != nil {
		return nil, td.FailStep(ctx, err)
	}

	knownHashesPath := filepath.Join(buildPath, "known-hashes.txt")
	if err := os_steps.WriteFile(ctx, knownHashesPath, knownHashes, 0o644); err != nil {
		return nil, td.FailStep(ctx, err)
	}

	mux := http.NewServeMux()
	mux.Handle("/", http.FileServer(http.Dir(buildPath)))

	// Suppress the misleading 404 for favicon, which we don't care about.
	mux.HandleFunc("/favicon.ico", func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusNoContent)
	})

	// Accept a file from JS and write it to disk.
	mux.HandleFunc("POST /upload", func(w http.ResponseWriter, r *http.Request) {
		dst := filepath.Join(outDir, filepath.FromSlash(r.URL.Query().Get("path")))
		os.MkdirAll(filepath.Dir(dst), 0o755)
		f, err := os.Create(dst)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}
		defer f.Close()
		io.Copy(f, r.Body)
	})

	// WASM pthreads require these headers.
	addHeaders := func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			w.Header().Set("Cross-Origin-Opener-Policy", "same-origin")
			w.Header().Set("Cross-Origin-Embedder-Policy", "require-corp")
			next.ServeHTTP(w, r)
		})
	}

	return httptest.NewServer(addHeaders(mux)), nil
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

func mustUnmarshalFlag[T any](ctx context.Context, data string, message string) T {
	var value T
	if err := json.Unmarshal([]byte(data), &value); err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "unmarshalling required flag: %s", message))
	}
	return value
}
