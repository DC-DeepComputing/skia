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
	"net/http/httptest"

	"github.com/chromedp/cdproto/network"
	"github.com/chromedp/cdproto/runtime"
	"github.com/chromedp/chromedp"

	"go.skia.org/infra/go/cleanup"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/td"

	_ "embed"
)

//go:embed harness.html
var harnessHtml string

func main() {
	var (
		// Required properties for this task.
		dmFlagsStr = flag.String("dm_flags", "", "JSON-encoded array of flags to pass to dm invocation")
		buildPath  = flag.String("build", "", "The directory where the built wasm/js code is.")
		projectID  = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskID     = flag.String("task_id", "", "ID of the task")
		taskName   = flag.String("task_name", "", "Name of the task.")
		workCache  = flag.String("work_cache", "", "The cached directory to store task artifacts.")

		// Debugging flags.
		verbose     = flag.Bool("verbose", false, "If true, prints extra logging")
		local       = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
	)

	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	var chromeBin string
	if err := td.Do(ctx, td.Props("download and install chrome"), func(context.Context) error {
		workCacheAbs := td.MustGetAbsolutePathOfFlag(ctx, *workCache, "work_cache")
		var err error
		chromeBin, err = installChrome(ctx, workCacheAbs)
		return err
	}); err != nil {
		td.Fatal(ctx, err)
	}

	harnessPath := "/run_wasm_dm.html"
	server, err := serve(ctx, *buildPath, harnessPath)
	if err != nil {
		td.Fatal(ctx, err)
	}
	cleanup.AtExit(func() {
		sklog.Infof("shutting down server on %s", server.URL)
		server.Close()
	})

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

	err = td.Do(ctx, td.Props("run dm in chrome"), func(ctx context.Context) error {
		var status int
		argv := *dmFlagsStr
		invocation := fmt.Sprintf(`window.run(%s)`, argv)
		if err := chromedp.Run(ctx,
			network.Enable(),
			chromedp.Navigate(fmt.Sprintf("%s/%s", server.URL, harnessPath)),
			chromedp.Poll(`typeof window.run === 'function'`, nil),
			chromedp.Evaluate(invocation, &status, func(p *runtime.EvaluateParams) *runtime.EvaluateParams {
				return p.WithAwaitPromise(true)
			}),
		); err != nil {
			return err
		}

		if status != 0 {
			return fmt.Errorf("dm failed; exit code %d", status)
		}
		return nil
	})

	if err != nil {
		td.Fatal(ctx, err)
	}
}

func serve(ctx context.Context, builtPath, harnessPath string) (*httptest.Server, error) {
	ctx = td.StartStep(ctx, td.Props("start localhost server").Infra())
	defer td.EndStep(ctx)

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

	srv := httptest.NewServer(addHeaders(mux))
	sklog.Infof("Listening on %s...", srv.URL)
	return srv, nil
}
