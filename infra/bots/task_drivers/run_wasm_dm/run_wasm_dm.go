// Copyright 2020 The Chromium Authors. All rights reserved.
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
	"os"
	"path"
	"path/filepath"
	"sync"
	"time"

	"github.com/chromedp/cdproto/network"
	"github.com/chromedp/cdproto/runtime"
	"github.com/chromedp/chromedp"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		// Required properties for this task.
		builtPath     = flag.String("built_path", "", "The directory where the built wasm/js code will be.")
		_             = flag.String("git_commit", "", "The commit at which we are testing.")
		_             = flag.String("gold_ctl_path", "", "Path to the goldctl binary")
		goldHashesURL = flag.String("gold_hashes_url", "", "URL from which to download pre-existing hashes")
		goldKeys      = common.NewMultiStringFlag("gold_key", nil, "The keys that will tag this data")
		_             = flag.String("node_bin_path", "", "Path to the node bin directory (should have npm also). This directory *must* be on the PATH when this executable is called, otherwise, the wrong node or npm version may be found (e.g. the one on the system), even if we are explicitly calling npm with the absolute path.")
		projectID     = flag.String("project_id", "", "ID of the Google Cloud project.")
		_             = flag.String("resource_path", "", "The directory housing the images, fonts, and other assets used by tests.")
		taskID        = flag.String("task_id", "", "task id this data was generated on")
		taskName      = flag.String("task_name", "", "Name of the task.")
		_             = flag.String("test_harness_path", "", "Path to test harness folder (tools/run-wasm-gm-tests)")
		webGLVersion  = flag.Int("webgl_version", 2, "The version of web gl to use. 0 means CPU")
		workPath      = flag.String("work_path", "", "The directory to use to store temporary files (e.g. pngs and JSON)")

		// Provided for tryjobs
		_ = flag.String("changelist_id", "", "The id the Gerrit CL. Omit for primary branch.")
		_ = flag.String("tryjob_id", "", "The id of the Buildbucket job for tryjobs. Omit for primary branch.")
		// Because we pass in patchset_order via a placeholder, it can be empty string. As such, we
		// cannot use flag.Int, because that errors on "" being passed in.
		_ = flag.String("patchset_order", "0", "Represents if this is the nth patchset")

		// Debugging flags.
		local       = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		_           = flag.String("service_account_path", "", "Used in local mode for authentication. Non-local mode uses Luci config.")
	)

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	// builtAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *builtPath, "built_path")
	// goldctlAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *goldCtlPath, "gold_ctl_path")
	// nodeBinAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *nodeBinPath, "node_bin_path")
	// resourceAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *resourcePath, "resource_path")
	// testHarnessAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *testHarnessPath, "test_harness_path")
	workAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *workPath, "work_path")

	goldctlWorkPath := filepath.Join(workAbsPath, "goldctl")
	if err := os_steps.MkdirAll(ctx, goldctlWorkPath); err != nil {
		td.Fatal(ctx, err)
	}
	testsWorkPath := filepath.Join(workAbsPath, "tests")
	if err := os_steps.MkdirAll(ctx, testsWorkPath); err != nil {
		td.Fatal(ctx, err)
	}
	if *goldHashesURL == "" {
		td.Fatalf(ctx, "Must supply --gold_hashes_url")
	}

	// patchset := 0
	// if *patchsetOrder != "" {
	// 	p, err := strconv.Atoi(*patchsetOrder)
	// 	if err != nil {
	// 		td.Fatalf(ctx, "Invalid patchset_order %q", *patchsetOrder)
	// 	}
	// 	patchset = p
	// }

	keys := *goldKeys
	switch *webGLVersion {
	case 0:
		keys = append(keys, "cpu_or_gpu:CPU")
	case 1:
		keys = append(keys, "cpu_or_gpu:GPU", "extra_config:WebGL1")
	case 2:
		keys = append(keys, "cpu_or_gpu:GPU", "extra_config:WebGL2")
	default:
		td.Fatalf(ctx, "Invalid value for webgl_version, must be 0, 1, 2 got %d", *webGLVersion)
	}

	chromeInstallDir := filepath.Join(workAbsPath, "chrome")
	if err := os_steps.MkdirAll(ctx, chromeInstallDir); err != nil {
		td.Fatal(ctx, err)
	}

	chromeBin, err := installChrome(ctx, *workPath, chromeInstallDir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	wg := &sync.WaitGroup{}
	server, err := serve(ctx, wg, fmt.Sprintf(":%s", port), *builtPath)
	if err != nil {
		td.Fatal(ctx, err)
	}

	opts := append(chromedp.DefaultExecAllocatorOptions[:],
		chromedp.ExecPath(chromeBin),
		chromedp.NoSandbox,
	)
	ctx, cancel := chromedp.NewExecAllocator(ctx, opts...)
	defer cancel()
	ctx, cancel = chromedp.NewContext(
		ctx,
		chromedp.WithDebugf(sklog.Debugf),
		chromedp.WithErrorf(sklog.Errorf),
		chromedp.WithLogf(sklog.Infof),
	)
	defer cancel()

	chromedp.ListenTarget(ctx, func(ev interface{}) {
		switch ev := ev.(type) {
		case *network.EventResponseReceived:
			if ev.Response.Status < 200 || ev.Response.Status >= 300 {
				sklog.Warningf("%d %s: %s", ev.Response.Status, ev.Response.StatusText, ev.Response.URL)
			}

		case *runtime.EventConsoleAPICalled:
			for _, arg := range ev.Args {
				sklog.Infof("console: %s", arg.Value)
			}

		case *runtime.EventExceptionThrown:
			td.Fatal(ctx, fmt.Errorf("exception: %s\n", ev.ExceptionDetails.Exception.Description))

		case *network.EventLoadingFailed:
			td.Fatal(ctx, fmt.Errorf("load failed: %s %s\n", ev.Type, ev.ErrorText))
		}
	})

	html := ""
	if err := chromedp.Run(ctx,
		network.Enable(),
		chromedp.Navigate(fmt.Sprintf("http://localhost:%s", port)),
		chromedp.Evaluate(`console.log("this is a test log")`, nil),
		chromedp.OuterHTML("html", &html),
		chromedp.Click(`#go`, chromedp.NodeVisible),
		chromedp.Sleep(10*time.Second),
		chromedp.Click(`#go`, chromedp.NodeVisible),
		chromedp.Sleep(10*time.Second),
	); err != nil {
		sklog.Fatal(err)
	}

	sklog.Infof("html: %s", html)

	sklog.Infof("done execution; shutting down server")
	if err := server.Shutdown(ctx); err != nil {
		sklog.Fatal(err)
	}
	wg.Wait()
}

func serve(ctx context.Context, wg *sync.WaitGroup, addr, builtPath string) (*http.Server, error) {
	mux := http.NewServeMux()
	srv := &http.Server{Addr: addr, Handler: mux}
	mux.HandleFunc("/", func(res http.ResponseWriter, _ *http.Request) {
		fmt.Fprint(res, indexHtml)
	})
	mux.Handle("/js/", http.StripPrefix("/js/", http.FileServer(http.Dir(builtPath))))

	sklog.Infof("builtPath: %s\n", builtPath)
	entries, _ := os_steps.ReadDir(ctx, builtPath)
	for _, e := range entries {
		sklog.Info(e.Name())
	}

	sklog.Infof("Listening on %s...", addr)

	wg.Add(1)
	go func() {
		defer wg.Done()
		if err := srv.ListenAndServe(); err != http.ErrServerClosed {
			td.Fatal(ctx, err)
		}
	}()
	return srv, nil
}

const (
	port      = "8989"
	indexHtml = `
<!doctype html>
<meta charset="utf-8" />

<head>
    <script type="module">
        import createModule from "./js/dm.js";

        export async function run() {
            const m = await createModule({
                noInitialRun: true,
            });

            const argv = [ "--help" ]
            m.callMain(argv);
        }

        const go = document.getElementById("go");
        go.onclick = () => {
            run();
        };
        console.log("i'm alive");
    </script>
</head>

<body>
    <button id="go">Go</button>
</body>

</html>`
)

type chromeVersions struct {
	Timestamp time.Time       `json:"timestamp"`
	Versions  []chromeVersion `json:"versions"`
}

type chromeVersion struct {
	Version   string           `json:"version"`
	Revision  string           `json:"revision"`
	Downloads versionDownloads `json:"downloads"`
}

type versionDownloads struct {
	Chrome []versionDownload `json:"chrome"`
}

type versionDownload struct {
	Platform string `json:"platform"`
	URL      string `json:"url"`
}

func installChrome(ctx context.Context, workPath, installDir string) (string, error) {
	ctx = td.StartStep(ctx, td.Props("install chrome").Infra())
	defer td.EndStep(ctx)

	latestVersion, err := fetchChromeLatestVersionNum()
	if err != nil {
		return "", err
	}
	sklog.Infof("latest chrome version is %s", latestVersion)

	versions, err := fetchChromeVersions()
	if err != nil {
		return "", err
	}

	platform := "linux64"
	zipUrl := ""
	for _, v := range versions.Versions {
		if v.Version == latestVersion {
			for _, p := range v.Downloads.Chrome {
				if p.Platform == platform {
					zipUrl = p.URL
					break
				}
			}
		}
		if len(zipUrl) > 0 {
			break
		}
	}
	if len(zipUrl) == 0 {
		return "", fmt.Errorf("failed to find zip url for chrome %s platform %s", latestVersion, platform)
	}

	sklog.Infof("fetching chrome %s...", zipUrl)
	zipfile := path.Join(workPath, "chrome.zip")
	if err := downloadFile(zipUrl, zipfile); err != nil {
		return "", err
	}

	if err := extractZip(ctx, zipfile, installDir); err != nil {
		return "", err
	}

	chromeBin := path.Join(installDir, fmt.Sprintf("chrome-%s", platform), "chrome")
	return chromeBin, nil
}

func fetchChromeLatestVersionNum() (string, error) {
	url := "https://googlechromelabs.github.io/chrome-for-testing/LATEST_RELEASE_STABLE"
	resp, err := http.Get(url)
	if err != nil {
		return "", fmt.Errorf("failed to fetch %s: %w", url, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("failed to fetch %s: %s", url, resp.Status)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", fmt.Errorf("failed to read body: %w", err)
	}
	return string(body), nil
}

func fetchChromeVersions() (*chromeVersions, error) {
	url := "https://googlechromelabs.github.io/chrome-for-testing/known-good-versions-with-downloads.json"
	resp, err := http.Get(url)
	if err != nil {
		return nil, fmt.Errorf("failed to fetch %s: %w", url, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to fetch %s: %s", url, resp.Status)
	}

	var versions chromeVersions
	if err := json.NewDecoder(resp.Body).Decode(&versions); err != nil {
		return nil, fmt.Errorf("failed to decode %s: %w", url, err)
	}

	return &versions, nil
}

func downloadFile(url, destPath string) error {
	resp, err := http.Get(url)
	if err != nil {
		return fmt.Errorf("failed to fetch: %w", err)
	}
	defer resp.Body.Close()

	f, err := os.Create(destPath)
	if err != nil {
		return fmt.Errorf("failed to create file: %w", err)
	}
	defer f.Close()

	if _, err := io.Copy(f, resp.Body); err != nil {
		return fmt.Errorf("failed to write: %w", err)
	}

	return nil
}

func extractZip(ctx context.Context, zipPath, targetDir string) error {
	runCmd := &exec.Command{
		Name:      "unzip",
		Args:      []string{zipPath, "-d", targetDir},
		LogStderr: true,
	}
	_, err := exec.RunCommand(ctx, runCmd)
	if err != nil {
		return err
	}
	return nil
}
