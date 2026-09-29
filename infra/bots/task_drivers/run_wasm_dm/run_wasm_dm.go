// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"fmt"
	"net/http"
	"path/filepath"
	"sync"

	"github.com/chromedp/cdproto/network"
	"github.com/chromedp/cdproto/runtime"
	"github.com/chromedp/chromedp"

	"go.skia.org/infra/go/common"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		// Required properties for this task.
		dmFlagsStr    = flag.String("dm_flags", "", "JSON-encoded array of flags to pass to dm invocation")
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
		verbose     = flag.Bool("verbose", false, "If true, prints extra logging")
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
	harnessPath := "/run_wasm_dm.html"
	server, err := serve(ctx, wg, fmt.Sprintf(":%s", port), *builtPath, harnessPath)
	if err != nil {
		td.Fatal(ctx, err)
	}

	ctx, cancel := chromedp.NewExecAllocator(ctx,
		append(chromedp.DefaultExecAllocatorOptions[:],
			chromedp.ExecPath(chromeBin),
			chromedp.NoSandbox,
		)...)
	defer cancel()

	opts := []chromedp.ContextOption{chromedp.WithErrorf(sklog.Errorf)}
	if *verbose {
		opts = append(opts, chromedp.WithDebugf(sklog.Debugf), chromedp.WithLogf(sklog.Infof))
	}
	ctx, cancel = chromedp.NewContext(ctx, opts...)
	defer cancel()

	chromedp.ListenTarget(ctx, func(ev interface{}) {
		switch ev := ev.(type) {
		case *network.EventResponseReceived:
			if ev.Response.Status < 200 || ev.Response.Status >= 300 {
				sklog.Warningf("%d %s: %s", ev.Response.Status, ev.Response.StatusText, ev.Response.URL)
			}

		case *runtime.EventConsoleAPICalled:
			for _, arg := range ev.Args {
				switch ev.Type {
				case "warn":
					sklog.Warningf("[js console]: %s", arg.Value)
				case "error":
					sklog.Errorf("[js console]: %s", arg.Value)
				default:
					sklog.Infof("[js console]: %s", arg.Value)
				}
			}

		case *runtime.EventExceptionThrown:
			td.Fatal(ctx, fmt.Errorf("exception: %s\n", ev.ExceptionDetails.Exception.Description))

		case *network.EventLoadingFailed:
			td.Fatal(ctx, fmt.Errorf("load failed: %s %s\n", ev.Type, ev.ErrorText))
		}
	})

	argv := *dmFlagsStr
	invocation := fmt.Sprintf(`window.run(%s)`, argv)
	var status int
	if err := chromedp.Run(ctx,
		network.Enable(),
		chromedp.Navigate(fmt.Sprintf("http://localhost:%s%s", port, harnessPath)),
		chromedp.Poll(`typeof window.run === 'function'`, nil),
		chromedp.Evaluate(invocation, &status, func(p *runtime.EvaluateParams) *runtime.EvaluateParams {
			return p.WithAwaitPromise(true)
		}),
	); err != nil {
		sklog.Fatal(err)
	}

	sklog.Infof("done execution (exit code %d); shutting down server", status)
	if err := server.Shutdown(ctx); err != nil {
		sklog.Fatal(err)
	}
	wg.Wait()

	if status != 0 {
		td.Fatalf(ctx, "dm failed; exit code %d", status)
	}
}

func serve(ctx context.Context, wg *sync.WaitGroup, addr, builtPath, harnessPath string) (*http.Server, error) {
	mux := http.NewServeMux()

	mux.HandleFunc(harnessPath, func(res http.ResponseWriter, r *http.Request) {
		fmt.Fprint(res, harnessHtml)
	})

	// Requests for anything besides the harness page are served from the build dir
	const prefix = "/"
	mux.Handle(prefix, http.StripPrefix(prefix, http.FileServer(http.Dir(builtPath))))

	// WASM pthreads require these headers.
	addHeaders := func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			w.Header().Set("Cross-Origin-Opener-Policy", "same-origin")
			w.Header().Set("Cross-Origin-Embedder-Policy", "require-corp")
			next.ServeHTTP(w, r)
		})
	}
	srv := &http.Server{Addr: addr, Handler: addHeaders(mux)}

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
	port        = "8989"
	harnessHtml = `
<!doctype html>
<meta charset="utf-8" />
<head>
	<script type="module">
		import createModule from "/dm.js";

		window.run = async function(argv = []) {
			const status = await new Promise((resolve, reject) => {
				if (argv[0] === "dm") {
					argv = argv.slice(1);
				}

				console.log("calling wasm dm with args: " + JSON.stringify(argv));
				try {
					createModule({
						arguments: argv,
						onExit: resolve,
						onAbort: (msg) => reject(new Error("abort: " + msg)),
					});
				} catch (e) {
					reject(e);
				}
			});

			console.log("dm finished; status: " + status);
			return status;
		}

		console.log("Page is alive");
	</script>
</head>
<body></body>
</html>`
)
