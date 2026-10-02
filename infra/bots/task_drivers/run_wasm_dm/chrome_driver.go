// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"fmt"

	"github.com/chromedp/cdproto/network"
	"github.com/chromedp/cdproto/runtime"
	"github.com/chromedp/chromedp"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/td"
)

type chromeDriver struct {
	ctx      context.Context
	shutdown context.CancelFunc
}

func newChromeDriver(ctx context.Context, cacheDir string, verbose bool) *chromeDriver {
	ctx = td.StartStep(ctx, td.Props("set up chrome").Infra())
	defer td.EndStep(ctx)

	chromeBin, err := installChrome(ctx, cacheDir)
	if err != nil {
		td.Fatal(ctx, skerr.Wrapf(err, "failed to install chrome"))
	}

	ctx, shutdown := newChromeContext(ctx, chromeBin, verbose)
	return &chromeDriver{
		ctx:      ctx,
		shutdown: shutdown,
	}
}

func (cd *chromeDriver) Shutdown() {
	cd.shutdown()
}

func (cd *chromeDriver) EvalJsAsync(jsExpr string, res any) error {
	return chromedp.Run(cd.ctx,
		chromedp.Evaluate(jsExpr, res, func(p *runtime.EvaluateParams) *runtime.EvaluateParams {
			return p.WithAwaitPromise(true)
		}),
	)
}

func newChromeContext(ctx context.Context, chromeBin string, verbose bool) (context.Context, context.CancelFunc) {
	stepCtx := td.StartStep(ctx, td.Props("create chrome devtools context").Infra())
	defer td.EndStep(stepCtx)

	chromeCtx, cancelChrome := chromedp.NewExecAllocator(ctx,
		append(chromedp.DefaultExecAllocatorOptions[:],
			chromedp.ExecPath(chromeBin),
			chromedp.NoSandbox,
		)...)

	opts := []chromedp.ContextOption{chromedp.WithErrorf(sklog.Errorf)}
	if verbose {
		opts = append(opts, chromedp.WithDebugf(sklog.Debugf), chromedp.WithLogf(sklog.Infof))
	}
	tabCtx, cancelTab := chromedp.NewContext(chromeCtx, opts...)

	startChromeDpListener(tabCtx)

	return tabCtx, func() {
		cancelTab()
		cancelChrome()
	}
}

func startChromeDpListener(ctx context.Context) {
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
}
